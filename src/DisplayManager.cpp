#include "DisplayManager.h"

DisplayManager::DisplayManager() : display(0x3c, OLED_SDA, OLED_SCL) {}

void DisplayManager::init() {
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW); delay(50); digitalWrite(OLED_RST, HIGH);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void DisplayManager::showMenu(const char* options[], int count, int selectedIndex) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 0, "-- MENU PRINCIPAL --");
    display.drawHorizontalLine(0, 12, 128);
    
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    for(int i = 0; i < count; i++) {
        String prefix = (i == selectedIndex) ? "> " : "  ";
        display.drawString(10, 16 + (i * 12), prefix + options[i]);
    }
    display.display();
}

void DisplayManager::showSystemInfo(uint32_t heap, uint32_t uptime, String resetReason) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "SYSTEM HEALTH");
    display.drawHorizontalLine(0, 12, 128);
    
    display.drawString(0, 16, "Heap: " + String(heap) + " B");
    display.drawString(0, 28, "Uptime: " + String(uptime / 1000) + "s");
    display.drawString(0, 40, "Reset: " + resetReason);
    
    display.drawString(0, 52, "[Segure p/ Voltar]");
    display.display();
}

void DisplayManager::showDashboard(String ip, float down, float up, int rssi) {
    display.clear();
    display.drawString(0, 0, "GATEWAY MONITOR v2");
    display.drawHorizontalLine(0, 12, 128);
    display.drawString(0, 16, "IP: " + ip);
    display.drawString(0, 42, "D: " + String(down, 1) + "KB");
    display.drawString(64, 42, "U: " + String(up, 1) + "KB");
    
    int bar = map(rssi, -100, -30, 0, 120);
    display.drawProgressBar(0, 54, 120, 8, constrain(bar, 0, 100));
    display.display();
}

void DisplayManager::showStatus(String msg, int prog) {
    display.clear();
    display.drawString(0, 10, msg);
    display.drawProgressBar(0, 40, 120, 8, prog);
    display.display();
}

void DisplayManager::showLoRaStats(String lastPacket, int rssi, float snr, uint32_t count) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "LORA RECEIVER");
    display.drawHorizontalLine(0, 12, 128);
    
    display.drawString(0, 16, "Pkts: " + String(count));
    display.drawString(0, 28, "RSSI: " + String(rssi) + " dBm");
    display.drawString(64, 28, "SNR: " + String(snr) + " dB");
    
    display.drawString(0, 42, "Last Data:");
    display.drawString(0, 52, lastPacket.substring(0, 20)); // Mostra os primeiros 20 caracteres
    
    display.display();
}

void DisplayManager::turnOff() {
    display.displayOff();
}

void DisplayManager::turnOn() {
    display.displayOn();
}