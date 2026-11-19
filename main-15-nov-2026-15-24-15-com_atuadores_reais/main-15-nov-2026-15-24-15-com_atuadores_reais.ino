#include "Automaton.h"
#include "Event.h"

// Define the number of automatons
#define NUMBER_AUTOMATON 7
#define NUMBER_SUPERVISOR 6

int actualState;

// Functions
void executeTransition(Event eventOccurred);

// Initialize Events
int firstEventUncontrollable = 0;
int firstEventControllable = 0;

Event eventControllable = createEvent();
Event eventUncontrollable = createEvent();

Event eventEnabledControllable = createEvent();
Event eventEnabledUncontrollable = createEvent();

Event emptyEvent = createEvent();
Event eventEnabled = createEvent();

Event eventOccurred = createEvent();

// Create a vector to store the automatons
Automaton automata[NUMBER_AUTOMATON];
Automaton supervisor[NUMBER_SUPERVISOR];

uint8_t eventDataAUT0EV0[3] = {0b11111101, 0b11111111, 0b11111111};
uint8_t eventDataAUT0EV1[3] = {0b11111110, 0b11111111, 0b11111111};

Event enabledEventStatesAutomaton0[2] = { createEventFromData(eventDataAUT0EV0), createEventFromData(eventDataAUT0EV1)};
uint8_t eventDataAUT1EV0[3] = {0b11111111, 0b01111111, 0b11111111};
uint8_t eventDataAUT1EV1[3] = {0b11111111, 0b10111111, 0b11111111};

Event enabledEventStatesAutomaton1[2] = { createEventFromData(eventDataAUT1EV0), createEventFromData(eventDataAUT1EV1)};
uint8_t eventDataAUT2EV0[3] = {0b11111111, 0b11011111, 0b11111111};
uint8_t eventDataAUT2EV1[3] = {0b11111111, 0b11101111, 0b11111111};

Event enabledEventStatesAutomaton2[2] = { createEventFromData(eventDataAUT2EV0), createEventFromData(eventDataAUT2EV1)};
uint8_t eventDataAUT3EV0[3] = {0b11111111, 0b11110011, 0b11111111};
uint8_t eventDataAUT3EV1[3] = {0b11111111, 0b11111000, 0b11111111};
uint8_t eventDataAUT3EV2[3] = {0b11111111, 0b11110100, 0b11111111};

Event enabledEventStatesAutomaton3[3] = { createEventFromData(eventDataAUT3EV0), createEventFromData(eventDataAUT3EV1), createEventFromData(eventDataAUT3EV2)};
uint8_t eventDataAUT4EV0[3] = {0b11111111, 0b11111111, 0b01111111};
uint8_t eventDataAUT4EV1[3] = {0b11111111, 0b11111111, 0b10111111};

Event enabledEventStatesAutomaton4[2] = { createEventFromData(eventDataAUT4EV0), createEventFromData(eventDataAUT4EV1)};
uint8_t eventDataAUT5EV0[3] = {0b11111111, 0b11111111, 0b11001111};
uint8_t eventDataAUT5EV1[3] = {0b11111111, 0b11111111, 0b11110111};

Event enabledEventStatesAutomaton5[2] = { createEventFromData(eventDataAUT5EV0), createEventFromData(eventDataAUT5EV1)};
uint8_t eventDataAUT6EV0[3] = {0b11111111, 0b11111111, 0b11111001};
uint8_t eventDataAUT6EV1[3] = {0b11111111, 0b11111111, 0b11111110};

Event enabledEventStatesAutomaton6[2] = { createEventFromData(eventDataAUT6EV0), createEventFromData(eventDataAUT6EV1)};
uint8_t eventDataAUT7EV0[3] = {0b11111111, 0b10111111, 0b11111111};
uint8_t eventDataAUT7EV1[3] = {0b11111100, 0b11111111, 0b11111111};

Event enabledEventStatesAutomaton7[2] = { createEventFromData(eventDataAUT7EV0), createEventFromData(eventDataAUT7EV1)};
uint8_t eventDataAUT8EV0[3] = {0b11111111, 0b11111111, 0b11000000};
uint8_t eventDataAUT8EV1[3] = {0b11111110, 0b11111111, 0b11001001};
uint8_t eventDataAUT8EV2[3] = {0b11111110, 0b11111111, 0b11110000};
uint8_t eventDataAUT8EV3[3] = {0b11111110, 0b11111111, 0b11000110};

Event enabledEventStatesAutomaton8[4] = { createEventFromData(eventDataAUT8EV0), createEventFromData(eventDataAUT8EV1), createEventFromData(eventDataAUT8EV2), createEventFromData(eventDataAUT8EV3)};
uint8_t eventDataAUT9EV0[3] = {0b11111111, 0b11101111, 0b11111111};
uint8_t eventDataAUT9EV1[3] = {0b11111111, 0b00111111, 0b11111111};

Event enabledEventStatesAutomaton9[2] = { createEventFromData(eventDataAUT9EV0), createEventFromData(eventDataAUT9EV1)};
uint8_t eventDataAUT10EV0[3] = {0b11111111, 0b11010110, 0b11110111};
uint8_t eventDataAUT10EV1[3] = {0b11111111, 0b11100100, 0b11110111};
uint8_t eventDataAUT10EV2[3] = {0b11111111, 0b11001101, 0b11110111};
uint8_t eventDataAUT10EV3[3] = {0b11111111, 0b11000000, 0b11111111};

Event enabledEventStatesAutomaton10[4] = { createEventFromData(eventDataAUT10EV0), createEventFromData(eventDataAUT10EV1), createEventFromData(eventDataAUT10EV2), createEventFromData(eventDataAUT10EV3)};
uint8_t eventDataAUT11EV0[3] = {0b11111111, 0b11111101, 0b00111111};
uint8_t eventDataAUT11EV1[3] = {0b11111111, 0b11110000, 0b11111111};
uint8_t eventDataAUT11EV2[3] = {0b11111111, 0b11110010, 0b00111111};

Event enabledEventStatesAutomaton11[3] = { createEventFromData(eventDataAUT11EV0), createEventFromData(eventDataAUT11EV1), createEventFromData(eventDataAUT11EV2)};
uint8_t eventDataAUT12EV0[3] = {0b11111111, 0b11111111, 0b11111110};
uint8_t eventDataAUT12EV1[3] = {0b11111111, 0b11111111, 0b11000111};

Event enabledEventStatesAutomaton12[2] = { createEventFromData(eventDataAUT12EV0), createEventFromData(eventDataAUT12EV1)};


void setup()
{

    // Initialize the automatons
    
		automata[0] = Automaton(2,enabledEventStatesAutomaton0,&MakeTransitionAutomaton0_G1,&Automaton0Loop_G1);
		automata[1] = Automaton(2,enabledEventStatesAutomaton1,&MakeTransitionAutomaton1_G2,&Automaton1Loop_G2);
		automata[2] = Automaton(2,enabledEventStatesAutomaton2,&MakeTransitionAutomaton2_G3,&Automaton2Loop_G3);
		automata[3] = Automaton(3,enabledEventStatesAutomaton3,&MakeTransitionAutomaton3_G6,&Automaton3Loop_G6);
		automata[4] = Automaton(2,enabledEventStatesAutomaton4,&MakeTransitionAutomaton4_G7,&Automaton4Loop_G7);
		automata[5] = Automaton(2,enabledEventStatesAutomaton5,&MakeTransitionAutomaton5_G8,&Automaton5Loop_G8);
		automata[6] = Automaton(2,enabledEventStatesAutomaton6,&MakeTransitionAutomaton6_G9,&Automaton6Loop_G9);
		supervisor[0] = Automaton(2,enabledEventStatesAutomaton7,&MakeTransitionAutomaton7_REDSupG1G2B2,&Automaton7Loop_REDSupG1G2B2);
		supervisor[1] = Automaton(4,enabledEventStatesAutomaton8,&MakeTransitionAutomaton8_REDSupG1G8G9B1,&Automaton8Loop_REDSupG1G8G9B1);
		supervisor[2] = Automaton(2,enabledEventStatesAutomaton9,&MakeTransitionAutomaton9_REDSupG2G3B3,&Automaton9Loop_REDSupG2G3B3);
		supervisor[3] = Automaton(4,enabledEventStatesAutomaton10,&MakeTransitionAutomaton10_REDSupG3G6G8B6,&Automaton10Loop_REDSupG3G6G8B6);
		supervisor[4] = Automaton(3,enabledEventStatesAutomaton11,&MakeTransitionAutomaton11_REDSupG6G7B7,&Automaton11Loop_REDSupG6G7B7);
		supervisor[5] = Automaton(2,enabledEventStatesAutomaton12,&MakeTransitionAutomaton12_REDSupG8G9B8,&Automaton12Loop_REDSupG8G9B8);

    Serial.begin(9600);
    setupPin();
}

// Execute the main cycle
void loop()
{

    // Get the system events
    doEveryLoop();
    getEventControllable(eventControllable);
    getEventUncontrollable(eventUncontrollable);

    // Free the events that are physically possible
    zeroEvent(eventEnabled);
    // Check the events that are physically possible
    for (int i = 0; i < NUMBER_AUTOMATON; i++)
    {
        actualState = automata[i].getActualState();
        bitwiseOr(eventEnabled, eventEnabled, automata[i].enabledEventStates[actualState]);
    }

    bitwiseAnd(eventEnabledUncontrollable, eventUncontrollable, eventEnabled);
    bitwiseAnd(eventEnabledControllable, eventControllable, eventEnabled);

    if (!areEqual(eventEnabledUncontrollable, emptyEvent))
    {
        for (int i = 0; i < NUMBER_EVENT; i++)
        {
            int index = (firstEventUncontrollable + i) % NUMBER_EVENT;
            // Get one Uncontrollable Event each time
            bool existEvent = getBit(eventEnabledUncontrollable, index);
            if (existEvent)
            {
                setBit(eventOccurred, index, true);
                executeTransition(eventOccurred);
                setBit(eventUncontrollable, index, false);

                zeroEvent(eventOccurred);
                break;
            }
        }

        zeroEvent(eventEnabledUncontrollable);
        firstEventUncontrollable = (firstEventUncontrollable + 1) % NUMBER_EVENT;
    }
    else
    {

        // Check the enabled events
        oneEvent(eventEnabled);
        for (int i = 0; i < NUMBER_SUPERVISOR; i++)
        {
            bitwiseAnd(eventEnabled, eventEnabled, supervisor[i].getEnabledEvent());
        }

        // Check if any enabled Controllable event was detected
        bitwiseAnd(eventEnabledControllable, eventEnabledControllable, eventEnabled);

        if (!areEqual(eventEnabledControllable, emptyEvent))
        {

            for (int i = 0; i < NUMBER_EVENT; i++)
            {
                int index = (firstEventControllable + i) % NUMBER_EVENT;

                // Get one Controllable Event each time
                bool existEvent = getBit(eventEnabledControllable, index);

                if (existEvent)
                {
                    setBit(eventOccurred, index, true);
                    executeTransition(eventOccurred);
                    setBit(eventControllable, index, false);

                    zeroEvent(eventOccurred);
                    break;
                }
            }

            zeroEvent(eventEnabledControllable);
            firstEventControllable = (firstEventControllable + 1) % NUMBER_EVENT;
        }

        zeroEvent(eventEnabled);
    }

    // Execute the Loop function for each automaton
    for (int i = 0; i < NUMBER_AUTOMATON; i++)
    {
        actualState = automata[i].getActualState();
        automata[i].Loop(actualState);
    }
}

void executeTransition(Event eventOccurred)
{
    // Execute the state transition for each supervisor
    for (int k = 0; k < NUMBER_SUPERVISOR; k++)
    {

        actualState = supervisor[k].getActualState();
        int nextState = supervisor[k].MakeTransition(actualState, eventOccurred);
        supervisor[k].setActualState(nextState);
    }

    // Execute the state transition for each automaton
    for (int j = 0; j < NUMBER_AUTOMATON; j++)
    {

        actualState = automata[j].getActualState();
        int nextState = automata[j].MakeTransition(actualState, eventOccurred);
        automata[j].setActualState(nextState);
    }
}
