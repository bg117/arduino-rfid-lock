#include "module.h"

#include <EEPROM.h>
#include <SPI.h>

bool uidExistsInRecord(byte *const &uid, const EEPROMAccessRecord &record)
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

    SPI.begin(); // initialize SPI bus

    // initialize the RC522 module
    m_mfrc522.PCD_Init();

    // read the access record from EEPROM
    EEPROM.get(0, m_accessRecord);
}

bool Module::checkAccess(byte *const &detected)
{
    // for each record in the accessRecord collection
    for (int i = 0; i < m_accessRecord.count; i++)
    {
        // compare the detected UID with the stored UID
        if (memcmp(detected, m_accessRecord.uids[i], 4) == 0)
            return true;
    }

    return false;
}

bool Module::writeAccessRecord(byte *const &cardUID)
{
    // check if the record is full
    if (m_accessRecord.count >= 255)
        return false; // record is full; fail

    // if exists, return true
    if (uidExistsInRecord(cardUID, m_accessRecord))
        return true;

    // write the card UID to EEPROM
    memcpy(m_accessRecord.uids[m_accessRecord.count], cardUID, 4); // copy the UID to the collection
    m_accessRecord.count++;                                        // increment the record count
    EEPROM.put(0, m_accessRecord);

    return true;
}

bool Module::readCardUID(byte *&cardUID)
{
    cardUID = nullptr;

    // ignore first 5 return value of isNewCardPresent (sometimes it returns faulty)
    for (int i = 0; i < 5; i++)
        isNewCardPresent();

    if (!isNewCardPresent())
        return false;

    // return the UID
    cardUID = m_mfrc522.uid.uidByte;
    return true;
}

bool Module::isNewCardPresent()
{
    // look for new cards
    if (!m_mfrc522.PICC_IsNewCardPresent() || !m_mfrc522.PICC_ReadCardSerial())
        return false;
    return true;
}
