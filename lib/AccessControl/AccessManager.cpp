#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>

#include <AccessManager.h>

const char *const LOG_FORMAT PROGMEM = "%04d-%02d-%02dT%02d:%02d:%02dZ %02X%02X%02X%02X %d%c";

void AccessManager::init(const int sdCSPin, const int sdMISOActivatePin)
{
    m_sdCSPin = sdCSPin;
    m_sdMISOActivatePin = sdMISOActivatePin;

    // initialize the pins
    Serial.print(F("Initializing SD card..."));

    // activate MISO line from transistor
    activateSDModule();

    // we'll use the initialization code from the utility libraries
    // since we're just testing if the card is working!
    if (!SD.begin(SPI_FULL_SPEED, sdCSPin))
    {
        Serial.println(F("initialization failed. Things to check:"));
        Serial.println(F("* is a card inserted?"));
        Serial.println(F("* is your wiring correct?"));
        Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
        while (true)
            ;
    }
    else
    {
        Serial.println(F("Wiring is correct and a card is present."));
    }

    deactivateSDModule();
}

bool AccessManager::checkAccess(byte *const &detected)
{
    byte recordCount;
    EEPROM.get(0, recordCount);

    // for each record in the accessRecord collection
    for (int i = 0; i < recordCount; i++)
    {
        // get the record
        byte record[4];
        EEPROM.get(1 + i * 4, record); // 1 byte for the record count,
                                       // 4 bytes for each record

        // compare the detected UID with the stored UID
        if (memcmp(detected, record, 4) == 0)
            return true;
    }

    return false;
}

bool AccessManager::writeAccessRecord(byte *const &cardUID)
{
    byte recordCount;
    EEPROM.get(0, recordCount);

    // check if the record is full
    if (recordCount >= 255)
        return false; // record is full; fail

    // if exists, return true
    if (uidExistsInRecord(cardUID))
        return true;

    EEPROM.put(1 + recordCount * 4, cardUID); // write record

    recordCount++;              // increment the record count
    EEPROM.put(0, recordCount); // write to "count" field

    return true;
}

void AccessManager::logAccess(byte *const &cardUID, const bool accessOrWrite, const bool granted) const {
    activateSDModule();

    // open the log file
    File logFile = SD.open(F("log.txt"), FILE_WRITE);

    if (!logFile)
    {
        Serial.println(F("Failed to open log file!"));
        deactivateSDModule();
        return;
    }

    // write the log in the format
    char buffer[37] = {0};
    sprintf(buffer, LOG_FORMAT, 1970, 0, 0, 0, 0, 0,        // timestamp
            cardUID[0], cardUID[1], cardUID[2], cardUID[3], // UID
            granted ? 1 : 0, accessOrWrite ? 'w' : 'a');    // access status

    logFile.println(buffer);
    Serial.println(buffer);

    logFile.close();

    // deactivate SD card CS pin
    deactivateSDModule();
}

void AccessManager::deactivateSDModule() const
{
    // deactivate MISO
    digitalWrite(m_sdMISOActivatePin, LOW);
}

void AccessManager::activateSDModule() const {
    // activate MISO
    digitalWrite(m_sdMISOActivatePin, HIGH);
}

bool AccessManager::uidExistsInRecord(byte *const &uid)
{
    byte recordCount;
    EEPROM.get(0, recordCount);

    // for each record in the EEPROM
    for (int i = 0; i < recordCount; i++)
    {
        byte record[4];
        EEPROM.get(1 + i * 4, record);

        // compare the UID with the record
        if (memcmp(uid, record, 4) == 0)
            return true; // exists
    }

    return false; // not exists
}
