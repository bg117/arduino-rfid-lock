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
    bool checkAccess(byte *const &cardUID);
    bool writeAccessRecord(byte *const &cardUID);
    bool isNewCardPresent();

private:
    // for RC522
    static constexpr int MFRC522_1_RST_PIN = 9;
    static constexpr int MFRC522_1_SS_PIN = 10;

    MFRC522 m_mfrc522_1 = MFRC522(MFRC522_1_SS_PIN, MFRC522_1_RST_PIN);
    EEPROMAccessRecord m_accessRecord;
};