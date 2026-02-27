#include "Secrets.h"

#ifndef CONFIG_H
#define CONFIG_H

// Pinos do OLED (Heltec LoRa 32 V2)
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

// Pinos LoRa - Heltec V2
#define LORA_MOSI 27
#define LORA_MISO 19
#define LORA_SCK  5
#define LORA_NSS  18
#define LORA_RST  14
#define LORA_DIO0 26
#define LORA_DIO1 35

// Pinos Botoes
#define BUTTON_PIN 0  // Botão PRG na placa

// Estados do Sistema (Menu)
enum SystemState {
    STATE_MENU,
    STATE_WIFI_STATS,
    STATE_LORA_RECEIVER,
    STATE_SYSTEM_INFO
};

#endif