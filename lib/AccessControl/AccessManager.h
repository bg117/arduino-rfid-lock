#pragma once

class AccessManager
{
public:
    void init(int sdCSPin, int sdMISOActivatePin);

    static bool checkAccess(byte *const &detected);
    static bool writeAccessRecord(byte *const &cardUID);
    void logAccess(byte *const &cardUID, bool accessOrWrite, bool granted) const;

private:
    static bool uidExistsInRecord(byte *const &uid);
    void deactivateSDModule() const;
    void activateSDModule() const;
    
    int m_sdCSPin;
    int m_sdMISOActivatePin;
};
