#pragma once

#include <MFRC522.h>

namespace Module
{
    void init();

    bool readCardUID(byte *&cardUID);
    bool checkAccess(byte *const &cardUID);
    bool writeAccessRecord(byte *const &cardUID);
    bool isNewCardPresent();

    void logAccess(byte *const &cardUID, bool accessOrWrite, bool granted);

    void deactivateSDModule();
    void activateSDModule();
    
    // for RC522
    constexpr int MFRC522_1_RST_PIN = 9;
    constexpr int MFRC522_1_SS_PIN = 10;

    // for SD card module
    constexpr int SD_CS_PIN = 7;
    constexpr int SD_MISO_ACTIVATE_PIN = 8; // really quirky fix for poorly-designed SD card modules (MISO line isn't brought down after use)
};