#include "module.h"

Module module;

void entryRoutine(byte *cardUID);

void setup()
{
    module.init();
}

void loop()
{
    static bool lit = false; // keep track of the key lock switch state every loop
    bool keyInserted = digitalRead(Module::KEY_PIN) == HIGH;

    // if the key lock switch state has changed
    if (lit != keyInserted)
    {
        lit = keyInserted;
        digitalWrite(Module::Y_LED_PIN, keyInserted ? HIGH : LOW); // turn on yellow LED if key is inserted
    }

    byte *cardUID;
    bool readStatus = module.readCardUID(cardUID);
    if (!readStatus)
        return;

    if (keyInserted)
    {
        // try to write the card UID to EEPROM
        if (module.writeAccessRecord(cardUID))
            digitalWrite(Module::G_LED_PIN, HIGH); // turn on green LED if successful
        else
            digitalWrite(Module::R_LED_PIN, HIGH); // turn on red LED if failed

        delay(1000);
        digitalWrite(Module::G_LED_PIN, LOW);
        digitalWrite(Module::R_LED_PIN, LOW);
    }
    else
    {
        entryRoutine(cardUID);
    }
}

void entryRoutine(byte *cardUID)
{
    // check if the detected card is in the access record
    if (module.checkAccess(cardUID))
    {
        module.engageLock();
        delay(2000);
        module.disengageLock();
    }
    else
    {
        digitalWrite(Module::R_LED_PIN, HIGH);
        delay(1000);
        digitalWrite(Module::R_LED_PIN, LOW);
    }
}
