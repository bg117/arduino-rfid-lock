#pragma once

#include <MFRC522.h>

class RFIDModule
{
public:
    RFIDModule(int ssPin, int rstPin);
    
    void init();
    bool readCardUID(byte *&cardUID);
    bool isNewCardPresent();

private:
    MFRC522 m_mfrc522;
};