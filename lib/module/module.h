#pragma once

#include <MFRC522.h>
#include <SPI.h>
#include <eeprom.h>
#include <string.h>

// access record on EEPROM
struct EEPROMAccessRecord
{
    byte count;
    byte uids[255][4];
}; // sizeof == 1021; if not, i'll die

class Module
{
public:
    EEPROMAccessRecord accessRecord;

    void init();

    bool readCardUID(byte *&cardUID);

    bool checkAccess(byte *detected);

    bool writeAccessRecord(byte *cardUID);

    void engageLock();
    void disengageLock();

    // pins

    // for RC522
    static constexpr int RST_PIN = 9;
    static constexpr int SS_PIN = 10;

    // traffic light LEDs
    static constexpr int R_LED_PIN = 2;
    static constexpr int G_LED_PIN = 3;
    static constexpr int Y_LED_PIN = 4;

    // key lock switch (for override/EEPROM write)
    static constexpr int KEY_PIN = 5;

    // relay pin for solenoid lock control
    static constexpr int RELAY_PIN = 6;
private:
    MFRC522 mfrc522 = MFRC522(SS_PIN, RST_PIN);
};