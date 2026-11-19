#include "Automaton.h"
#include "Event.h"

Automaton::Automaton(int numStates, Event *enabledEventStates, int (*MakeTransition)(int state, Event eventOccurred), void (*Loop)(int state))
    : numStates(numStates), enabledEventStates(enabledEventStates), MakeTransition(MakeTransition), Loop(Loop) {}

Automaton::Automaton() {}

Automaton::~Automaton() {}

void Automaton::setEvent(Event event)
{
    actualEvent = event;
}

void Automaton::setActualState(int state)
{
    actualState = state;
}

int Automaton::getActualState()
{
    return actualState;
}

int Automaton::getNumStates()
{
    return numStates;
}

Event Automaton::getEnabledEvent()
{
    return enabledEventStates[actualState];
}

void getEventControllable(Event &eventControllable){
	if(EventControllable_LbAM()){
		setBit(eventControllable,EVENT_LBAM, true);
	}
	if(EventControllable_LbVM()){
		setBit(eventControllable,EVENT_LBVM, true);
	}
	if(EventControllable_LbAZ()){
		setBit(eventControllable,EVENT_LBAZ, true);
	}
	if(EventControllable_SUB()){
		setBit(eventControllable,EVENT_SUB, true);
	}
	if(EventControllable_DESC()){
		setBit(eventControllable,EVENT_DESC, true);
	}
	if(EventControllable_LVOR()){
		setBit(eventControllable,EVENT_LVOR, true);
	}
	if(EventControllable_iTES()){
		setBit(eventControllable,EVENT_ITES, true);
	}
	if(EventControllable_IpC()){
		setBit(eventControllable,EVENT_IPC, true);
	}
}


void getEventUncontrollable(Event &eventUncontrollable){
	if(EventUncontrollable_DbAM()){
		setBit(eventUncontrollable,EVENT_DBAM, true);
	}
	if(EventUncontrollable_DbVM()){
		setBit(eventUncontrollable,EVENT_DBVM, true);
	}
	if(EventUncontrollable_DbAZ()){
		setBit(eventUncontrollable,EVENT_DBAZ, true);
	}
	if(EventUncontrollable_F_SUB()){
		setBit(eventUncontrollable,EVENT_F_SUB, true);
	}
	if(EventUncontrollable_F_DESC()){
		setBit(eventUncontrollable,EVENT_F_DESC, true);
	}
	if(EventUncontrollable_DVOR()){
		setBit(eventUncontrollable,EVENT_DVOR, true);
	}
	if(EventUncontrollable_cor_OK()){
		setBit(eventUncontrollable,EVENT_COR_OK, true);
	}
	if(EventUncontrollable_cor_N()){
		setBit(eventUncontrollable,EVENT_COR_N, true);
	}
	if(EventUncontrollable_volume_OK()){
		setBit(eventUncontrollable,EVENT_VOLUME_OK, true);
	}
	if(EventUncontrollable_volume_N()){
		setBit(eventUncontrollable,EVENT_VOLUME_N, true);
	}
}


GenericAction ActionAutomatons0[2]={ &StateActionAutomaton0_G1State0,&StateActionAutomaton0_G1State1 };
GenericAction ActionAutomatons1[2]={ &StateActionAutomaton1_G2State0,&StateActionAutomaton1_G2State1 };
GenericAction ActionAutomatons2[2]={ &StateActionAutomaton2_G3State0,&StateActionAutomaton2_G3State1 };
GenericAction ActionAutomatons3[3]={ &StateActionAutomaton3_G6State0,&StateActionAutomaton3_G6State1,&StateActionAutomaton3_G6State2 };
GenericAction ActionAutomatons4[2]={ &StateActionAutomaton4_G7State0,&StateActionAutomaton4_G7State1 };
GenericAction ActionAutomatons5[2]={ &StateActionAutomaton5_G8State0,&StateActionAutomaton5_G8State1 };
GenericAction ActionAutomatons6[2]={ &StateActionAutomaton6_G9State0,&StateActionAutomaton6_G9State1 };
GenericAction ActionAutomatons7[1]={[](){}};
GenericAction ActionAutomatons8[1]={[](){}};
GenericAction ActionAutomatons9[1]={[](){}};
GenericAction ActionAutomatons10[1]={[](){}};
GenericAction ActionAutomatons11[1]={[](){}};
GenericAction ActionAutomatons12[1]={[](){}};


void Automaton0Loop_G1(int State){
	ActionAutomatons0[State]();
}

void Automaton1Loop_G2(int State){
	ActionAutomatons1[State]();
}

void Automaton2Loop_G3(int State){
	ActionAutomatons2[State]();
}

void Automaton3Loop_G6(int State){
	ActionAutomatons3[State]();
}

void Automaton4Loop_G7(int State){
	ActionAutomatons4[State]();
}

void Automaton5Loop_G8(int State){
	ActionAutomatons5[State]();
}

void Automaton6Loop_G9(int State){
	ActionAutomatons6[State]();
}

void Automaton7Loop_REDSupG1G2B2(int State){
	ActionAutomatons7[State]();
}

void Automaton8Loop_REDSupG1G8G9B1(int State){
	ActionAutomatons8[State]();
}

void Automaton9Loop_REDSupG2G3B3(int State){
	ActionAutomatons9[State]();
}

void Automaton10Loop_REDSupG3G6G8B6(int State){
	ActionAutomatons10[State]();
}

void Automaton11Loop_REDSupG6G7B7(int State){
	ActionAutomatons11[State]();
}

void Automaton12Loop_REDSupG8G9B8(int State){
	ActionAutomatons12[State]();
}



int MakeTransitionAutomaton0_G1(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_LBAM))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DBAM))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton1_G2(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_LBVM))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DBVM))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton2_G3(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_LBAZ))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DBAZ))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton3_G6(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_SUB))){ 
		return 1;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_DESC))){ 
		return 2;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_F_SUB))){ 
		return 0;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_F_DESC))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton4_G7(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_LVOR))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DVOR))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton5_G8(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_ITES))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_COR_OK))){ 
		return 0;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_COR_N))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton6_G9(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_IPC))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_VOLUME_OK))){ 
		return 0;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_VOLUME_N))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton7_REDSupG1G2B2(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_DBAM))){ 
		return 1;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_DBVM))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_LBAM))){ 
		return 0;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DBVM))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_LBVM))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton8_REDSupG1G8G9B1(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_DBAM))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_LBAM))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DBAM))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_ITES))){ 
		return 2;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_IPC))){ 
		return 3;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_DBAM))){ 
		return 2;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_COR_OK))){ 
		return 1;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_COR_N))){ 
		return 0;
	}
	if (State == 3 && (getBit(eventOccurred,EVENT_DBAM))){ 
		return 3;
	}
	if (State == 3 && (getBit(eventOccurred,EVENT_VOLUME_OK))){ 
		return 1;
	}
	if (State == 3 && (getBit(eventOccurred,EVENT_VOLUME_N))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton9_REDSupG2G3B3(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_DBVM))){ 
		return 1;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_DBAZ))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_LBVM))){ 
		return 0;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DBAZ))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_LBAZ))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton10_REDSupG3G6G8B6(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_COR_OK))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_COR_N))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_F_DESC))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_LBAZ))){ 
		return 1;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_SUB))){ 
		return 2;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_COR_OK))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_COR_N))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_F_DESC))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DBAZ))){ 
		return 2;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_COR_OK))){ 
		return 2;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_COR_N))){ 
		return 2;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_F_DESC))){ 
		return 2;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_F_SUB))){ 
		return 3;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_DESC))){ 
		return 0;
	}
	if (State == 3 && (getBit(eventOccurred,EVENT_COR_OK))){ 
		return 3;
	}
	if (State == 3 && (getBit(eventOccurred,EVENT_COR_N))){ 
		return 3;
	}
	if (State == 3 && (getBit(eventOccurred,EVENT_ITES))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton11_REDSupG6G7B7(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_F_DESC))){ 
		return 1;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_F_SUB))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_DESC))){ 
		return 0;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_DVOR))){ 
		return 2;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_LVOR))){ 
		return 1;
	}
	if (State == 2 && (getBit(eventOccurred,EVENT_SUB))){ 
		return 0;
	}

	return(State);
}

int MakeTransitionAutomaton12_REDSupG8G9B8(int State, Event eventOccurred) 
{ 
	if (State == 0 && (getBit(eventOccurred,EVENT_COR_OK))){ 
		return 1;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_COR_N))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_VOLUME_OK))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_VOLUME_N))){ 
		return 0;
	}
	if (State == 0 && (getBit(eventOccurred,EVENT_ITES))){ 
		return 0;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_VOLUME_OK))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_VOLUME_N))){ 
		return 1;
	}
	if (State == 1 && (getBit(eventOccurred,EVENT_IPC))){ 
		return 0;
	}

	return(State);
}


