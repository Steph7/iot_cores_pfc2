#include "mqtt.h"
#include <WiFiS3.h>          // <--- OBRIGATÓRIO PARA O UNO R4 WIFI
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Tópicos MQTT
const char* MQTT_TOPICO_PEDIDOS = "pwa/tintas/receita";
const char* MQTT_TOPICO_STATUS = "pwa/tintas/maquina/status";
const char* MQTT_TOPICO_PROGRESSO = "pwa/tintas/maquina/bomba";
const char* MQTT_TOPICO_RESPOSTA_COR = "pwa/tintas/maquina/resposta_cor";
const char* MQTT_TOPICO_RESPOSTA_VOLUME = "pwa/tintas/maquina/resposta_volume";
const char* MQTT_TOPICO_VORTEX = "pwa/tintas/maquina/vortex";

// Variáveis globais para armazenar a receita recebida
bool receitaDisponivel = false;
float pesoVermelho = 0, pesoAmarelo = 0, pesoAzul = 0, pesoBranco = 0, pesoPreto = 0;
float volumeTotalLote = 0;
String corHexAlvo = "";

// Variáveis de controle vindas da resposta do servidor de cor
bool respostaServidorRecebida = false;
bool corAprovadaPeloServidor = false;
bool volumeEnviado = false;
bool respostaVolumeRecebida = false;
bool volumeAprovadoPeloServidor = false;
float pesoAlvoTotalGrams = 0.0; // Você preenche isso quando a receita chega via MQTT


// ===== WIFI (Uno R4 WiFi) =====
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a rede Wi-Fi: ");
  Serial.println(ssid);

  // Verifica se o módulo WiFi do R4 está presente
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("❌ Erro: Módulo WiFi do Uno R4 não foi detectado!");
    while (true);
  }

  // Conexão usando WiFiS3
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\n✅ Wi-Fi conectado com sucesso!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// ===== MQTT RECONNECT =====
void reconectarMQTT() {
  String clientId = "R4Tintas_" + String(random(0xffff), HEX);
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect(clientId.c_str())) {
      Serial.println(" Conectado!");

      publicarStatus("livre");

      client.subscribe(MQTT_TOPICO_PEDIDOS);
      client.subscribe(MQTT_TOPICO_RESPOSTA_COR);
      client.subscribe(MQTT_TOPICO_RESPOSTA_VOLUME);
      
    } else {
      Serial.print(" Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 2 segundos...");
      delay(2000);
    }
  }
}

// ===== CALLBACK MQTT =====
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n----------------------------------------");
  Serial.print("Mensagem MQTT recebida no tópico: ");
  Serial.println(topic);

  String payloadJson = "";
  for (unsigned int i = 0; i < length; i++) {
    payloadJson += (char)payload[i];
  }

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, payloadJson);

  if (error) {
    Serial.print("Falha ao ler o JSON: ");
    Serial.println(error.f_str());
    return;
  }

  if (doc.containsKey("pesos_g")) {
    corHexAlvo = String(doc["cor_hex"] | "#000000");
    volumeTotalLote = doc["volume_total_ml"];

    pesoVermelho = doc["pesos_g"]["vermelho"] | 0.0;
    pesoAmarelo  = doc["pesos_g"]["amarelo"]  | 0.0;
    pesoAzul     = doc["pesos_g"]["azul"]     | 0.0;
    pesoBranco   = doc["pesos_g"]["branco"]   | 0.0;
    pesoPreto    = doc["pesos_g"]["preto"]    | 0.0;

    pesoAlvoTotalGrams = pesoAmarelo + pesoVermelho + pesoAzul;

    Serial.print("Cor Alvo (Hex): "); Serial.println(corHexAlvo);
    Serial.print("Volume Total: "); Serial.print(volumeTotalLote); Serial.println(" mL");

    receitaDisponivel = true;
    publicarStatus("ocupado");
  }
  else if (String(topic) == MQTT_TOPICO_RESPOSTA_COR) {
    const char* resultado = doc["resultado_cor"];

    if (resultado && strcmp(resultado, "ok") == 0) {
      Serial.println("[MQTT] Servidor APROVOU a cor via AS7341!");
      corAprovadaPeloServidor = true;
      respostaServidorRecebida = true;
    } 
    else if (resultado && strcmp(resultado, "corrigir") == 0) {
      Serial.println("[MQTT] Servidor REPROVOU a cor. Aplicando correção incremental...");
      
      if (doc.containsKey("novos_pesos")) {
        JsonObject novosPesos = doc["novos_pesos"];
        pesoAmarelo  = novosPesos["amarelo"]  | 0.0;
        pesoVermelho = novosPesos["vermelho"] | 0.0;
        pesoAzul     = novosPesos["azul"]     | 0.0;
      }

      corAprovadaPeloServidor = false;
      respostaServidorRecebida = true; 
    }
  }
  else if (String(topic) == MQTT_TOPICO_RESPOSTA_VOLUME) {
    const char* resultado = doc["resultado_volume"];

    if (resultado && strcmp(resultado, "ok") == 0) {
      Serial.println("[MQTT] Servidor APROVOU o volume!");
      volumeAprovadoPeloServidor = true;
      respostaVolumeRecebida = true;
    } 
    else if (resultado && strcmp(resultado, "corrigir") == 0) {
      Serial.println("[MQTT] Servidor REPROVOU o volume. Aplicando correção...");
      volumeAprovadoPeloServidor = false;
      respostaVolumeRecebida = true; 
    }
  }

  Serial.println("----------------------------------------");
}

void inicializarMQTT() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(512);
  client.setCallback(callback);
}

void manterMQTT() {
  if (!client.connected()) {
    reconectarMQTT();
  }
  client.loop();
}

void publicarStatus(const char* estado) {
  String topicoCompleto = String(MQTT_TOPICO_STATUS) + "/" + String(estado);
  client.publish(topicoCompleto.c_str(), "", true);
}

void publicarProgresso(const char* bomba, float valor) {
  String topicoCompleto = String(MQTT_TOPICO_PROGRESSO) + "/" + String(bomba) + "/" + String(valor, 2);
  client.publish(topicoCompleto.c_str(), "", true);
}

void publicarVortex(bool ligado, unsigned long tempoRestanteSeg) {
  StaticJsonDocument<192> doc;
  doc["tipo"] = "vortex";
  doc["ligado"] = ligado;
  doc["tempo_restante_seg"] = tempoRestanteSeg;

  char buffer[192];
  serializeJson(doc, buffer);

  client.publish(MQTT_TOPICO_VORTEX, buffer, true);
}