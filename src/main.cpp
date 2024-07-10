#include <module.h>
#include <SPI.h>

// traffic light LEDs
constexpr int R_LED_PIN = 4;
constexpr int G_LED_PIN = 2;
constexpr int Y_LED_PIN = 3;

// key lock switch (for override/EEPROM write)
constexpr int KEY_PIN = 5;

// MOSFET pin for solenoid lock control
constexpr int MOSFET_PIN = 6;

void entryRoutine(byte *const &cardUID);
void engageLock();
void disengageLock();
void delayWhileNoNewCard();

void setup()
{
    Module::init();

    // initialize the pins
    pinMode(R_LED_PIN, OUTPUT);
    pinMode(G_LED_PIN, OUTPUT);
    pinMode(Y_LED_PIN, OUTPUT);
    pinMode(KEY_PIN, INPUT);
    pinMode(MOSFET_PIN, OUTPUT);

    // write HIGH to all the LEDs and MOSFET for 1/4 a second then back again
    digitalWrite(R_LED_PIN, HIGH);
    digitalWrite(G_LED_PIN, HIGH);
    digitalWrite(Y_LED_PIN, HIGH);

    delay(250);

    digitalWrite(R_LED_PIN, LOW);
    digitalWrite(G_LED_PIN, LOW);
    digitalWrite(Y_LED_PIN, LOW);
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
    bool readStatus = Module::readCardUID(cardUID);
    if (!readStatus)
        return;

    if (keyInserted)
    {
        // try to write the card UID to EEPROM
        bool writeStatus = Module::writeAccessRecord(cardUID);

        // log the access attempt
        Module::logAccess(cardUID, true, writeStatus);

        if (writeStatus)
            digitalWrite(G_LED_PIN, HIGH); // turn on green LED if successful
        else
            digitalWrite(R_LED_PIN, HIGH); // turn on red LED if failed

        delayWhileNoNewCard(); // wait for the card to be removed
        digitalWrite(G_LED_PIN, LOW);
        digitalWrite(R_LED_PIN, LOW);
    }
    else
    {
        entryRoutine(cardUID);
    }
}

void entryRoutine(byte *const &cardUID)
{
    // check if the detected card is in the access record
    bool access = Module::checkAccess(cardUID);
    // log the access attempt
    Module::logAccess(cardUID, false, access);
    if (access)
    {
        engageLock();
        delayWhileNoNewCard();
        disengageLock();
    }
    else
    {
        digitalWrite(R_LED_PIN, HIGH);
        delayWhileNoNewCard();
        digitalWrite(R_LED_PIN, LOW);
    }
}

void engageLock()
{
    // turn on green LED and MOSFET
    digitalWrite(G_LED_PIN, HIGH);
    digitalWrite(MOSFET_PIN, HIGH);
}

void disengageLock()
{
    // turn off green LED and MOSFET
    digitalWrite(G_LED_PIN, LOW);
    digitalWrite(MOSFET_PIN, LOW);
}

void delayWhileNoNewCard()
{
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
    } while (Module::isNewCardPresent()); // keep reading until new card
}
