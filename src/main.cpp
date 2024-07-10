#include "RFIDModule.h"
#include "AccessManager.h"

// traffic light LEDs
constexpr int R_LED_PIN = 4;
constexpr int G_LED_PIN = 2;
constexpr int Y_LED_PIN = 3;

// key lock switch (for override/EEPROM write)
constexpr int KEY_PIN = 5;

// MOSFET pin for solenoid lock control
constexpr int MOSFET_PIN = 6;

// for RC522
constexpr int MFRC522_1_RST_PIN = 9;
constexpr int MFRC522_1_SS_PIN = 10;

// for SD card module
constexpr int SD_CS_PIN = 7;
constexpr int SD_MISO_ACTIVATE_PIN = 8;

RFIDModule rfid(MFRC522_1_SS_PIN, MFRC522_1_RST_PIN);
AccessManager accessManager(SD_CS_PIN, SD_MISO_ACTIVATE_PIN);

void handleKeyInserted(byte *const &cardUID);
void entryRoutine(byte *const &cardUID);
void engageLock();
void disengageLock();
void waitForCardRemoval();

void setup()
{
    // Serial init
    Serial.begin(9600);
    while (!Serial)
        ;

    rfid.init();
    accessManager.init();

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
    static bool keyInsertedPrev = false; // keep track of the key lock switch state every loop
    bool keyInserted = digitalRead(KEY_PIN) == HIGH;

    // if the key lock switch state has changed
    if (keyInsertedPrev != keyInserted)
    {
        keyInsertedPrev = keyInserted;
        digitalWrite(Y_LED_PIN, keyInserted ? HIGH : LOW); // turn on yellow LED if key is inserted
    }

    byte *cardUID;
    if (rfid.readCardUID(cardUID))
    {
        if (keyInserted)
            handleKeyInserted(cardUID);
        else
            entryRoutine(cardUID);
    }
}

void handleKeyInserted(byte *const &cardUID)
{
    // try to write the card UID to EEPROM
    bool writeStatus = accessManager.writeAccessRecord(cardUID);

    // log the access attempt
    accessManager.logAccess(cardUID, true, writeStatus);

    if (writeStatus)
        digitalWrite(G_LED_PIN, HIGH); // turn on green LED if successful
    else
        digitalWrite(R_LED_PIN, HIGH); // turn on red LED if failed

    waitForCardRemoval(); // wait for the card to be removed
    digitalWrite(G_LED_PIN, LOW);
    digitalWrite(R_LED_PIN, LOW);
}

void entryRoutine(byte *const &cardUID)
{
    // check if the detected card is in the access record
    bool access = accessManager.checkAccess(cardUID);

    // log the access attempt
    accessManager.logAccess(cardUID, false, access);

    if (access)
    {
        engageLock();
        waitForCardRemoval();
        disengageLock();
    }
    else
    {
        digitalWrite(R_LED_PIN, HIGH);
        waitForCardRemoval();
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

void waitForCardRemoval()
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
    } while (rfid.isNewCardPresent()); // keep reading until new card
}
