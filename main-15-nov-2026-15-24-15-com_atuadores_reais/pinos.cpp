#include "pinos.h"    

// Instância física da balança
HX711 balanca;
const int DOUT_PIN = 2;
const int CLK_PIN = 3;
const float FATOR_CALIBRACAO = 7050.0;

// Instância do sensor de cor AS7341
AS7341 as;

// Servo motor
Servo servo_motor;

void tararComSeguranca() {
    Serial.println("[BALANÇA] Tarando balança com estabilização...");
    balanca.tare(); // Zera a balança
    delay(200);     // Dá um respiro para o hardware absorver o zero
    
    // Lê e descarta algumas leituras iniciais para limpar o buffer do HX711
    for (int i = 0; i < 5; i++) {
        balanca.get_units(1);
        delay(20);
    }
    Serial.println("[BALANÇA] Balança zerada e pronta!");
}

void inicializarPinosEHardware() {

    // Inicialização da balança
    balanca.begin(DOUT_PIN, CLK_PIN);
    balanca.set_scale(FATOR_CALIBRACAO);
    //balanca.tare();
    tararComSeguranca();
    //Serial.println("[HARDWARE] Pinos e Balança inicializados com sucesso!");

    // Inicialização do Sensor de Cor AS7341
    if (!as.begin()) {
        Serial.println("[ERRO] AS7341 não encontrado! Verifique a fiação I2C.");
    } else {
        as.SetIntegrationTime(29, 599);
        as.SetGain(AS7341_GAIN_256);
        as.ControlLed(true, 50); // Liga o LED integrado do sensor (intensidade 50)
        Serial.println("[HARDWARE] AS7341 inicializado com sucesso!");
    }

    //Servo
    servo_motor.attach(10); 

    // Elevador
	pinMode(STEP_ELEV, OUTPUT);
    pinMode(DIR_ELEV, OUTPUT);
    pinMode(ENABLE_ELEV, OUTPUT);

	digitalWrite(ENABLE_ELEV, LOW);
	//digitalWrite(DIR_ELEV, HIGH); // sentido de rotação

    // Vórtex
	pinMode(RELE_PIN, OUTPUT);
    digitalWrite(RELE_PIN, LOW); 

	// Eletroímã
	pinMode(ELETROIMA, OUTPUT);
    digitalWrite(ELETROIMA, LOW); 

    // Bombas
    pinMode(BOMBA_AM, OUTPUT);
	pinMode(BOMBA_VM, OUTPUT);
	pinMode(BOMBA_AZ, OUTPUT);
    pinMode(BOMBA_AM, HIGH);
	pinMode(BOMBA_VM, HIGH);
	pinMode(BOMBA_AZ, HIGH);

	// Chaves Fim de Curso
	pinMode(CHAVE_FIM_CIMA, INPUT_PULLUP);
	pinMode(CHAVE_FIM_BAIXO, INPUT_PULLUP);
	pinMode(CHAVE_FIM_ESTICADO, INPUT_PULLUP);
	pinMode(CHAVE_FIM_RECOLHIDO, INPUT_PULLUP);

}