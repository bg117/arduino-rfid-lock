#include "module.h"

namespace
{
    Module module;

    // traffic light LEDs
    constexpr int R_LED_PIN = 4;
    constexpr int G_LED_PIN = 2;
    constexpr int Y_LED_PIN = 3;

    // key lock switch (for override/EEPROM write)
    constexpr int KEY_PIN = 5;

    // relay pin for solenoid lock control
    constexpr int RELAY_PIN = 6;

    void entryRoutine(byte *cardUID);
    void engageLock();
    void disengageLock();
}

void setup()
{
    module.init();

    // initialize the pins
    pinMode(R_LED_PIN, OUTPUT);
    pinMode(G_LED_PIN, OUTPUT);
    pinMode(Y_LED_PIN, OUTPUT);
    pinMode(KEY_PIN, INPUT);
    pinMode(RELAY_PIN, OUTPUT);
}

void loop()
{
    static bool lit = false; // keep track of the key lock switch state every loop
    bool keyInserted = digitalRead(KEY_PIN) == HIGH;

    // if the key lock switch state has changed
    if (lit != keyInserted)
    {
        lit = keyInserted;
        digitalWrite(Y_LED_PIN, keyInserted ? HIGH : LOW); // turn on yellow LED if key is inserted
    }

    byte *cardUID;
    bool readStatus = module.readCardUID(cardUID);
    if (!readStatus)
        return;

    if (keyInserted)
    {
        // try to write the card UID to EEPROM
        if (module.writeAccessRecord(cardUID))
            digitalWrite(G_LED_PIN, HIGH); // turn on green LED if successful
        else
            digitalWrite(R_LED_PIN, HIGH); // turn on red LED if failed

        delay(1000);
        digitalWrite(G_LED_PIN, LOW);
        digitalWrite(R_LED_PIN, LOW);
    }
    else
    {
        entryRoutine(cardUID);
    }
}

namespace
{
    void entryRoutine(byte *cardUID)
    {
        // check if the detected card is in the access record
        if (module.checkAccess(cardUID))
        {
            engageLock();
            delay(2000);
            disengageLock();
        }
        else
        {
            digitalWrite(R_LED_PIN, HIGH);
            delay(1000);
            digitalWrite(R_LED_PIN, LOW);
        }
    }

    void engageLock()
    {
        // turn on green LED and relay
        digitalWrite(G_LED_PIN, HIGH);
        digitalWrite(RELAY_PIN, HIGH);
    }

    void disengageLock()
    {
        // turn off green LED and relay
        digitalWrite(G_LED_PIN, LOW);
        digitalWrite(RELAY_PIN, LOW);
    }
}
