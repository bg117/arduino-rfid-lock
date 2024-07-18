#pragma once

#include <SD.h>

class AccessManager
{
public:
    AccessManager(int sdCSPin, int sdMISOActivatePin);

    void init();
    bool checkAccess(byte *const &detected);
    bool writeAccessRecord(byte *const &cardUID);
    void logAccess(byte *const &cardUID, bool accessOrWrite, bool granted);

private:
    bool uidExistsInRecord(byte *const &uid);
    void deactivateSDModule();
    void activateSDModule();
    
    int m_sdCSPin;
    int m_sdMISOActivatePin;
};