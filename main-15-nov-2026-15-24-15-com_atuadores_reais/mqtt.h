#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h" // Onde estão definidos SSID, senha do Wi-Fi e IP/Broker MQTT

// Declarações globais para o arquivo principal acessar
extern PubSubClient client;
extern bool receitaDisponivel;
extern float pesoVermelho, pesoAmarelo, pesoAzul, pesoBranco, pesoPreto;
extern float volumeTotalLote;
extern String corHexAlvo;
// Importa as variáveis globais que estão lá no mqtt.cpp
extern bool respostaServidorRecebida;
extern bool corAprovadaPeloServidor;
extern bool flag_iTES;

extern bool volumeEnviado;
extern bool respostaVolumeRecebida;
extern bool volumeAprovadoPeloServidor;
extern float pesoAlvoTotalGrams;

// Funções do módulo MQTT
void inicializarMQTT();
void manterMQTT();
void reconectarMQTT();
void publicarStatus(const char* estado);
void publicarProgresso(const char* bomba, float valor);
void publicarVortex(bool ligado, unsigned long tempoRestanteSeg);

#endif