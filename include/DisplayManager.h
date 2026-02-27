#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <SSD1306Wire.h>
#include "Config.h"

class DisplayManager {
public:
    DisplayManager();
    void init();
    void showMenu(const char* options[], int count, int selectedIndex);
    void showDashboard(String ip, float down, float up, int rssi);
    void showStatus(String msg, int prog);
    void showSystemInfo(uint32_t heap, uint32_t uptime, String resetReason);

    void showLoRaStats(String lastPacket, int rssi, float snr, uint32_t count);

    // display control
    void turnOff();
    void turnOn();

private:
    SSD1306Wire display;
};

#endif