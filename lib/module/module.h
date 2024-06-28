#pragma once

#include <MFRC522.h>

// access record on EEPROM
struct EEPROMAccessRecord
{
    byte count;
    byte uids[255][4];
}; // sizeof == 1021; if not, i'll die

class Module
{
public:
    void init();

    bool readCardUID(byte *&cardUID);
    bool checkAccess(byte *detected);
    bool writeAccessRecord(byte *cardUID);

private:
    // for RC522
    static constexpr int RST_PIN = 9;
    static constexpr int SS_PIN = 10;

    MFRC522 m_mfrc522 = MFRC522(SS_PIN, RST_PIN);
    EEPROMAccessRecord m_accessRecord;
};