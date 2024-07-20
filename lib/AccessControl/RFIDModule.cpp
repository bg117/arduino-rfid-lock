#include <RFIDModule.h>

#include <SPI.h>

void RFIDModule::init(int ssPin, int rstPin)
{
    SPI.begin();
    m_mfrc522.PCD_Init(ssPin, rstPin);
    m_mfrc522.PCD_SetAntennaGain(m_mfrc522.RxGain_max);
    delay(4);                          // delay to stabilize the module
    m_mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
}

bool RFIDModule::readCardUID(byte *&cardUID)
{
    cardUID = nullptr;

    // look for new cards
    if (!m_mfrc522.PICC_IsNewCardPresent() || !m_mfrc522.PICC_ReadCardSerial())
        return false;

    // return the UID
    cardUID = m_mfrc522.uid.uidByte;
    m_mfrc522.PICC_HaltA();

    return true;
}

bool RFIDModule::isNewCardPresent()
{
    // check if the same card is still present
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);
    MFRC522::StatusCode result = m_mfrc522.PICC_WakeupA(bufferATQA, &bufferSize);
    m_mfrc522.PICC_HaltA();

    return result == MFRC522::STATUS_OK;
}
