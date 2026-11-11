
#ifndef AUTOMATON_H
#define AUTOMATON_H

#include "Event.h"

typedef void (*GenericAction)();

class Automaton
{
public:
    Automaton(int numStates, Event *enabledEventStates, int (*MakeTransition)(int state, Event eventOccurred), void (*Loop)(int state));
    Automaton();
    ~Automaton();

    int getActualState();                                  // Returns the current state of the automaton
    int getNumStates();                                    // Returns the total number of states in the automaton
    Event getEnabledEvent();                               // Returns the value of enabled events for the automaton
    void setActualState(int state);                        // Sets the current state of the automaton
    void setEvent(Event event);                            // Sets the value of the current event of the automaton
    Event *enabledEventStates;                             // Pointer to the list of enabled event values
    int (*MakeTransition)(int state, Event eventOccurred); // Pointer to the state transition function
    void (*Loop)(int state);                               // Pointer to the current state execution function

private:
    int actualState = 0; // Current state of the automaton
    Event actualEvent;   // Value of the current event of the automaton
    int numStates;       // Total number of states in the automaton
};

void setupPin();
void doEveryLoop();
void getEventControllable(Event &eventControllable);     // Obtains Controllablble events
void getEventUncontrollable(Event &eventUncontrollable); // Obtains Uncontrollablble events

void Automaton0Loop_G1(int State); 

void Automaton1Loop_G2(int State); 

void Automaton2Loop_G3(int State); 

void Automaton3Loop_G4(int State); 

void Automaton4Loop_G5(int State); 

void Automaton5Loop_G6(int State); 

void Automaton6Loop_G7(int State); 

void Automaton7Loop_G8(int State); 

void Automaton8Loop_G9(int State); 

void Automaton9Loop_REDSupG1G2B2(int State); 

void Automaton10Loop_REDSupG1G8G9B1(int State); 

void Automaton11Loop_REDSupG2G3B3(int State); 

void Automaton12Loop_REDSupG3G4B4(int State); 

void Automaton13Loop_REDSupG4G5B5(int State); 

void Automaton14Loop_REDSupG5G6G8B6(int State); 

void Automaton15Loop_REDSupG6G7B7(int State); 

void Automaton16Loop_REDSupG8G9B8(int State); 



bool EventControllable_LbAM();
bool EventControllable_LbVM();
bool EventControllable_LbAZ();
bool EventControllable_LbBR();
bool EventControllable_LbPR();
bool EventControllable_SUB();
bool EventControllable_DESC();
bool EventControllable_LVOR();
bool EventControllable_iTES();
bool EventControllable_IpC();


bool EventUncontrollable_DbAM();
bool EventUncontrollable_DbVM();
bool EventUncontrollable_DbAZ();
bool EventUncontrollable_DbBR();
bool EventUncontrollable_DbPR();
bool EventUncontrollable_F_SUB();
bool EventUncontrollable_F_DESC();
bool EventUncontrollable_DVOR();
bool EventUncontrollable_cor_OK();
bool EventUncontrollable_cor_N();
bool EventUncontrollable_volume_OK();
bool EventUncontrollable_volume_N();


void StateActionAutomaton0_G1State0();
void StateActionAutomaton0_G1State1();

void StateActionAutomaton1_G2State0();
void StateActionAutomaton1_G2State1();

void StateActionAutomaton2_G3State0();
void StateActionAutomaton2_G3State1();

void StateActionAutomaton3_G4State0();
void StateActionAutomaton3_G4State1();

void StateActionAutomaton4_G5State0();
void StateActionAutomaton4_G5State1();

void StateActionAutomaton5_G6State0();
void StateActionAutomaton5_G6State1();
void StateActionAutomaton5_G6State2();

void StateActionAutomaton6_G7State0();
void StateActionAutomaton6_G7State1();

void StateActionAutomaton7_G8State0();
void StateActionAutomaton7_G8State1();

void StateActionAutomaton8_G9State0();
void StateActionAutomaton8_G9State1();



int MakeTransitionAutomaton0_G1(int State, Event eventOccurred);
int MakeTransitionAutomaton1_G2(int State, Event eventOccurred);
int MakeTransitionAutomaton2_G3(int State, Event eventOccurred);
int MakeTransitionAutomaton3_G4(int State, Event eventOccurred);
int MakeTransitionAutomaton4_G5(int State, Event eventOccurred);
int MakeTransitionAutomaton5_G6(int State, Event eventOccurred);
int MakeTransitionAutomaton6_G7(int State, Event eventOccurred);
int MakeTransitionAutomaton7_G8(int State, Event eventOccurred);
int MakeTransitionAutomaton8_G9(int State, Event eventOccurred);
int MakeTransitionAutomaton9_REDSupG1G2B2(int State, Event eventOccurred);
int MakeTransitionAutomaton10_REDSupG1G8G9B1(int State, Event eventOccurred);
int MakeTransitionAutomaton11_REDSupG2G3B3(int State, Event eventOccurred);
int MakeTransitionAutomaton12_REDSupG3G4B4(int State, Event eventOccurred);
int MakeTransitionAutomaton13_REDSupG4G5B5(int State, Event eventOccurred);
int MakeTransitionAutomaton14_REDSupG5G6G8B6(int State, Event eventOccurred);
int MakeTransitionAutomaton15_REDSupG6G7B7(int State, Event eventOccurred);
int MakeTransitionAutomaton16_REDSupG8G9B8(int State, Event eventOccurred);


#endif