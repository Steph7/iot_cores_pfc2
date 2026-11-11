#include "Automaton.h"

void setupPin()
{
}

// This function will be called every Arduino loop, put here something that you want to repeat always
void doEveryLoop(){
/*
    Example of reading events through the Serial port

    if (!Serial.available())
    {
        return;
    }

    String input = Serial.readStringUntil('\n');
    input.trim();

    int actualEvent = input.toInt();

*/
}

// This set of functions should be implemented in a way to handle the controllable events of the system
bool EventControllable_LbAM(){
	return false;
}

bool EventControllable_LbVM(){
	return false;
}

bool EventControllable_LbAZ(){
	return false;
}

bool EventControllable_LbBR(){
	return false;
}

bool EventControllable_LbPR(){
	return false;
}

bool EventControllable_SUB(){
	return false;
}

bool EventControllable_DESC(){
	return false;
}

bool EventControllable_LVOR(){
	return false;
}

bool EventControllable_iTES(){
	return false;
}

bool EventControllable_IpC(){
	return false;
}




// This set of functions should be implemented in a way to handle the uncontrollable events of the system
bool EventUncontrollable_DbAM(){
	return false;
}

bool EventUncontrollable_DbVM(){
	return false;
}

bool EventUncontrollable_DbAZ(){
	return false;
}

bool EventUncontrollable_DbBR(){
	return false;
}

bool EventUncontrollable_DbPR(){
	return false;
}

bool EventUncontrollable_F_SUB(){
	// Se a chave fim de curso fim_subida ativada -> True
	return false;
}

bool EventUncontrollable_F_DESC(){
	// Se a chave fim de curso fim_descida ativada -> True
	return false;
}

bool EventUncontrollable_DVOR(){
	// Se o vórtex foi desligado
	return false;
}

bool EventUncontrollable_cor_OK(){
	// Se a cor obtida pelo sensor de cor corresponder a cor esperada
	return false;
}

bool EventUncontrollable_cor_N(){
	// Se a cor obtida pelo sensor de cor não corresponder a cor esperada
	return false;
}

bool EventUncontrollable_volume_OK(){
	// Se o peso da balança corresponder ao peso do volume esperado
	return false;
}

bool EventUncontrollable_volume_N(){
	// Se o peso da balança não corresponder ao peso do volume esperado
	return false;
}



// Here the expected actions should be implemented in each state of the system 
void StateActionAutomaton0_G1State0()
{
	// Desliga a bomba amarela
	Serial.println("A0S0");
 	delay(500);
}

void StateActionAutomaton0_G1State1()
{
	// Liga a bomba amarela
	// enquanto o peso não correponder ao do volume desejado, mantém ligada
	// depois dispara evento -> desliga_amarelo 
	Serial.println("A0S1");
 	delay(500);
}

void StateActionAutomaton1_G2State0()
{
	// Desliga a bomba vermelha
	Serial.println("A1S0");
 	delay(500);
}

void StateActionAutomaton1_G2State1()
{
	// Liga a bomba vermelha
	// enquanto o peso não correponder ao do volume desejado, mantém ligada
	// depois dispara evento -> desliga_vermelho 
	Serial.println("A1S1");
 	delay(500);
}

void StateActionAutomaton2_G3State0()
{
	Serial.println("A2S0");
 	delay(500);
}

void StateActionAutomaton2_G3State1()
{
	Serial.println("A2S1");
 	delay(500);
}

void StateActionAutomaton3_G4State0()
{
	Serial.println("A3S0");
 	delay(500);
}

void StateActionAutomaton3_G4State1()
{
	Serial.println("A3S1");
 	delay(500);
}

void StateActionAutomaton4_G5State0()
{
	Serial.println("A4S0");
 	delay(500);
}

void StateActionAutomaton4_G5State1()
{
	Serial.println("A4S1");
 	delay(500);
}

void StateActionAutomaton5_G6State0()
{
	Serial.println("A5S0");
 	delay(500);
}

void StateActionAutomaton5_G6State1()
{
	Serial.println("A5S1");
 	delay(500);
}

void StateActionAutomaton5_G6State2()
{
	Serial.println("A5S2");
 	delay(500);
}

void StateActionAutomaton6_G7State0()
{
	Serial.println("A6S0");
 	delay(500);
}

void StateActionAutomaton6_G7State1()
{
	Serial.println("A6S1");
 	delay(500);
}

void StateActionAutomaton7_G8State0()
{
	Serial.println("A7S0");
 	delay(500);
}

void StateActionAutomaton7_G8State1()
{
	Serial.println("A7S1");
 	delay(500);
}

void StateActionAutomaton8_G9State0()
{
	Serial.println("A8S0");
 	delay(500);
}

void StateActionAutomaton8_G9State1()
{
	Serial.println("A8S1");
 	delay(500);
}

