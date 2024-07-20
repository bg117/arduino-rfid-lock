#pragma once

#include <MFRC522.h>

class RFIDModule
{
public:
    void init(int ssPin, int rstPin);
    bool readCardUID(byte *&cardUID);
    bool isNewCardPresent();

private:
    MFRC522 m_mfrc522;
};