#include <module.h>

#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>

MFRC522 mfrc522_1 = MFRC522(Module::MFRC522_1_SS_PIN, Module::MFRC522_1_RST_PIN);

const char *const LOG_FORMAT PROGMEM = "%04d-%02d-%02dT%02d:%02d:%02dZ %02X%02X%02X%02X %d%c";

bool uidExistsInRecord(byte *const &uid);

void Module::init()
{
    // Serial init
    Serial.begin(9600);
    while (!Serial)
        ;

    // initialize the pins
    pinMode(SD_MISO_ACTIVATE_PIN, OUTPUT);

    // initialize the RC522 module
    SPI.begin();
    mfrc522_1.PCD_Init();
    mfrc522_1.PCD_SetAntennaGain(mfrc522_1.RxGain_max);
    delay(4);                            // delay to stabilize the module
    mfrc522_1.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

    // initialize the SD module
    activateSDModule();
    if (!SD.begin(SD_CS_PIN))
    {
        Serial.println(F("SD card initialization failed!"));
        // fail
        while (true)
            ;
    }

    deactivateSDModule();
}

bool Module::checkAccess(byte *const &detected)
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

bool Module::writeAccessRecord(byte *const &cardUID)
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

bool Module::readCardUID(byte *&cardUID)
{
    cardUID = nullptr;

    // look for new cards
    if (!mfrc522_1.PICC_IsNewCardPresent() || !mfrc522_1.PICC_ReadCardSerial())
        return false;

    // return the UID
    cardUID = mfrc522_1.uid.uidByte;
    mfrc522_1.PICC_HaltA();

    return true;
}

bool Module::isNewCardPresent()
{
    // check if the same card is still present
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);
    MFRC522::StatusCode result = mfrc522_1.PICC_WakeupA(bufferATQA, &bufferSize);
    mfrc522_1.PICC_HaltA();

    return result == MFRC522::STATUS_OK;
}

void Module::logAccess(byte *const &cardUID, bool accessOrWrite, bool granted)
{
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

void Module::deactivateSDModule()
{
    // deactivate MISO
    digitalWrite(SD_MISO_ACTIVATE_PIN, LOW);
}

void Module::activateSDModule()
{
    // activate MISO
    digitalWrite(SD_MISO_ACTIVATE_PIN, HIGH);
}

bool uidExistsInRecord(byte *const &uid)
{
    byte recordCount;
    EEPROM.get(0, recordCount);

    for (int i = 0; i < recordCount; i++)
    {
        byte record[4];
        EEPROM.get(1 + i * 4, record);

        if (memcmp(uid, record, 4) == 0)
            return true;
    }

    return false;
}
