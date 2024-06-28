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

    void entryRoutine(byte *const &cardUID);
    void engageLock();
    void disengageLock();
    void delayWhileNoNewCard(byte *const &cardUID);
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

        delayWhileNoNewCard(cardUID); // wait for the card to be removed
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
    void entryRoutine(byte *const &cardUID)
    {
        // check if the detected card is in the access record
        if (module.checkAccess(cardUID))
        {
            engageLock();
            delayWhileNoNewCard(cardUID); // wait for the card to be removed
            disengageLock();
        }
        else
        {
            digitalWrite(R_LED_PIN, HIGH);
            delayWhileNoNewCard(cardUID); // wait for the card to be removed
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

    void delayWhileNoNewCard(byte *const &cardUID)
    {
        byte *newUID;         // dummy variable
        bool firstRep = true; // first repetition flag
        do
        {
            // if it's the first repetition, delay for 2 seconds
            if (firstRep)
            {
                firstRep = false;
                delay(2000);
            }
            else
            {
                delay(1000);
            }
        } while (module.readCardUID(newUID) && newUID == cardUID); // keep reading until a new card is detected
    }
}
