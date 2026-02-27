#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_task_wdt.h>
#include <RadioLib.h>
#include "Config.h"
#include "DisplayManager.h"

// --- Instâncias Globais ---
DisplayManager ui;
unsigned long lastInteraction = 0;
bool isDisplayOn = true;
const unsigned long DISPLAY_TIMEOUT = 15000;

// Declaração do Rádio
SX1276 lora = new Module(LORA_NSS, LORA_DIO0, LORA_RST, LORA_DIO1);

// --- Variáveis de Estado e Tráfego ---
SystemState currentState = STATE_MENU;
float totalDown = 0; 
float totalUp = 0;
unsigned long lastNetCheck = 0;

// --- Variáveis LoRa ---
String lastLoraData = "Aguardando...";
int loraRSSI = 0; 
float loraSNR = 0; 
uint32_t packetCount = 0;

// --- Configuração do Menu ---
int menuIndex = 0;
const char* menuOptions[] = {"WiFi Monitor", "LoRa Stats", "System Info"};
int totalOptions = 3;

// --- Controle de Botão ---
unsigned long buttonPressTime = 0;
bool lastButtonState = HIGH;

// Função de Diagnóstico de Reset
String getResetReason() {
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
        case ESP_RST_POWERON:  return "Power On";
        case ESP_RST_SW:       return "Software";
        case ESP_RST_WDT:      return "Watchdog";
        case ESP_RST_BROWNOUT: return "Brownout";
        default:               return "Outro";
    }
}

void handleButton() {
    bool currentStateBtn = digitalRead(BUTTON_PIN);
    
    // se button pressionado
    if (lastButtonState == HIGH && currentStateBtn == LOW) {
        buttonPressTime = millis();
        lastInteraction = millis(); // resetar timer de inatividade

        // se tela desligada liga ela mas nao como clique de comando
        if (!isDisplayOn) {
            ui.turnOn();
            isDisplayOn = true;
            // o primeiro clique apenas "acorda" a tela
            lastButtonState = LOW;
            return; 
        }
    }

    // processamento de cliques
    if (lastButtonState == LOW && currentStateBtn == HIGH) {
        unsigned long duration = millis() - buttonPressTime;
        lastInteraction = millis(); // Reseta ao soltar o botão também
        
        if (duration > 800) { 
            currentState = (currentState == STATE_MENU) ? (SystemState)(menuIndex + 1) : STATE_MENU;
            if (currentState == STATE_LORA_RECEIVER) lora.startReceive();
        } else if (duration > 50 && currentState == STATE_MENU) {
            menuIndex = (menuIndex + 1) % totalOptions;
        }
    }
    lastButtonState = currentStateBtn;
}

void updateNetworkStats() {
    if (WiFi.status() == WL_CONNECTED && (millis() - lastNetCheck > 10000)) {
        HTTPClient http;
        http.begin("http://www.google.com");
        http.setTimeout(1500); 
        if (http.GET() > 0) {
            totalDown += (http.getSize() > 0) ? (http.getSize() / 1024.0) : 0.5;
            totalUp += 0.2;
        }
        http.end();
        lastNetCheck = millis();
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    ui.init();
    
    // Watchdog para evitar travamentos (10 segundos, true para reboot)
    // 10 segundos de timeout, true para disparar pânico (reboot)
    #if ESP_IDF_VERSION_MAJOR >= 5
        esp_task_wdt_config_t twdt_config = {
            .timeout_ms = 10000,
            .idle_core_mask = 0, // Monitora todos os cores
            .trigger_panic = true
        };
        // Se a struct ainda falhar
        esp_task_wdt_reconfigure(&twdt_config);
    #else
        esp_task_wdt_init(10, true);
    #endif
    
    esp_task_wdt_add(NULL); // Adiciona o loop atual ao Watchdog

    Serial.print(F("[LoRa] Inicializando... "));
    int state = lora.begin(915.0);
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("Sucesso!"));
    } else {
        Serial.printf("Erro (%d)\n", state);
    }

    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    esp_task_wdt_reset();
    handleButton();

    // auto desligar tela
    if (isDisplayOn && (millis() - lastInteraction > DISPLAY_TIMEOUT)) {
        ui.turnOff();
        isDisplayOn = false;
        Serial.println("[Sistema] Tela em standby para preservar hardware.");
    }

    // atualiza a tela se ela estiver ligada (Economiza I2C e CPU)
    if (isDisplayOn) {
        switch (currentState) {
            case STATE_MENU:
                ui.showMenu(menuOptions, totalOptions, menuIndex);
                break;
            case STATE_WIFI_STATS:
                updateNetworkStats();
                ui.showDashboard(WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Off", totalDown, totalUp, WiFi.RSSI());
                break;
            case STATE_LORA_RECEIVER: {
                String str;
                int state = lora.readData(str);
                if (state == RADIOLIB_ERR_NONE) {
                    packetCount++; lastLoraData = str;
                    loraRSSI = lora.getRSSI(); loraSNR = lora.getSNR();
                    lora.startReceive();
                }
                ui.showLoRaStats(lastLoraData, loraRSSI, loraSNR, packetCount);
            } break;
            case STATE_SYSTEM_INFO:
                ui.showSystemInfo(ESP.getFreeHeap(), millis(), getResetReason());
                break;
        }
    } else {
        // sistema continua processando rede e rádio em background
        updateNetworkStats(); 
        
        String str;
        if (lora.readData(str) == RADIOLIB_ERR_NONE) {
            packetCount++;
            lora.startReceive();
        }
    }

    delay(10);
}