#ifndef PINOS_H
#define PINOS_H

#include <Arduino.h>
#include <Servo.h>
#include "HX711.h"
#include "AS7341.h"

// Definição dos Pinos dos Botões
extern const int PINO_BOTAO_AVANCAR;
extern const int PINO_BIFURCACAO;

// Objeto global da balança para ser usado nos outros arquivos
extern HX711 balanca;
extern AS7341 as; // Objeto global do sensor de cor

// Servo motor
extern Servo servo_motor;

// Motor de Passo Linear (ELEVADOR)
#define STEP_ELEV 12
#define DIR_ELEV 7
#define ENABLE_ELEV 13

// Vórtex (relé)
#define RELE_PIN 8

// Eletroímã
#define ELETROIMA 11

// Bombas Peristálticas
#define BOMBA_AM 5 // Amarelo
#define BOMBA_VM 6 // Vermelho
#define BOMBA_AZ 9 // Azul

// Chaves fim de curso
#define CHAVE_FIM_CIMA 14
#define CHAVE_FIM_BAIXO 15
#define CHAVE_FIM_ESTICADO 16
#define CHAVE_FIM_RECOLHIDO 17

// Função de inicialização de todo o hardware
void inicializarPinosEHardware();

#endif