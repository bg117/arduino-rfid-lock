#include "module.h"

#include <EEPROM.h>
#include <SPI.h>

bool uidExistsInRecord(byte *uid, const EEPROMAccessRecord &record)
{
    for (int i = 0; i < record.count; i++)
    {
        if (memcmp(uid, record.uids[i], 4) == 0)
            return true;
    }

    return false;
}

void Module::init()
{
    // initialize the pins
    pinMode(RST_PIN, OUTPUT);
    pinMode(SS_PIN, OUTPUT);
    pinMode(R_LED_PIN, OUTPUT);
    pinMode(G_LED_PIN, OUTPUT);
    pinMode(Y_LED_PIN, OUTPUT);
    pinMode(KEY_PIN, INPUT);
    pinMode(RELAY_PIN, OUTPUT);

    SPI.begin(); // initialize SPI bus

    // initialize the RC522 module
    mfrc522.PCD_Init();

    // read the access record from EEPROM
    EEPROM.get(0, accessRecord);
}

bool Module::checkAccess(byte *detected)
{
    // for each record in the accessRecord collection
    for (int i = 0; i < accessRecord.count; i++)
    {
        // compare the detected UID with the stored UID
        if (memcmp(detected, accessRecord.uids[i], 4) == 0)
            return true;
    }

    return false;
}

void Module::engageLock()
{
    // turn on green LED and relay
    digitalWrite(G_LED_PIN, HIGH);
    digitalWrite(RELAY_PIN, HIGH);
}

void Module::disengageLock()
{
    // turn off green LED and relay
    digitalWrite(G_LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
}

bool Module::writeAccessRecord(byte *cardUID)
{
    // check if the record is full
    if (accessRecord.count >= 255)
        return false; // record is full; fail

    // if exists, return true
    if (uidExistsInRecord(cardUID, accessRecord))
        return true;

    // write the card UID to EEPROM
    memcpy(accessRecord.uids[accessRecord.count], cardUID, 4); // copy the UID to the collection
    accessRecord.count++;									   // increment the record count
    EEPROM.put(0, accessRecord);

    return true;
}

bool Module::readCardUID(byte *&cardUID)
{
    // look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
        return false;

    // select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
        return false;

    // return the UID
    cardUID = mfrc522.uid.uidByte;
    return true;
}