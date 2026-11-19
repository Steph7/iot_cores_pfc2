#include "Automaton.h"
#include "mqtt.h"
#include "pinos.h"

// Variáveis de controle dos botões
bool ultimoEstadoBotao1 = HIGH;
bool ultimoEstadoBotao2 = HIGH;

// Contadores de tentativas
int tentativasCor = 0;
int tentativasVolume = 0;
const int MAX_TENTATIVAS = 2;

// Controle das amostras
bool amostraEnviada = false;

// Quantidade de amostras por teste
const int TOTAL_AMOSTRAS = 20;

// Variáveis globais
const float TOLERANCIA_GRAMAS = 5; 
bool volumeAvaliado = false;
bool volumeAprovadoLocal = false;
float pesoTotalInjetadoReal = 0.0;

// Variáveis de controle do vórtex 
bool vortexAtivo = false;
bool vortexEmPausa = false;
unsigned long inicioCicloVortex = 0;
unsigned long inicioPausaVortex = 0;
unsigned long tempoRestanteVortex = 0;
unsigned long tempoLigadoNoBlocoAtual = 0;
int status_vortex = 0;
unsigned long ultimoEnvioVortexMQTT = 0;

// Função Auxiliar - Vórtex
void iniciarVortex() {
  // Calcula o tempo total em milissegundos
  float minutos = (float)pesoAlvoTotalGrams / 3.0;
  tempoRestanteVortex = (unsigned long)(minutos * 60000.0);

  vortexAtivo = true;
  vortexEmPausa = false;
  tempoLigadoNoBlocoAtual = 0;
  inicioCicloVortex = millis();

  digitalWrite(RELE_PIN, HIGH);
  status_vortex = 1;
  Serial.println("[VORTEX] Ligado.");
}

// Enumeração das etapas
enum EtapaProcesso {
    ETAPA_AGUARDANDO_INICIO = 0,
    ETAPA_BOMBA_AMARELA,
    ETAPA_BOMBA_VERMELHA,
    ETAPA_BOMBA_AZUL,
    ETAPA_DESCER,
    ETAPA_VORTEX,
    ETAPA_SUBIR,
    ETAPA_TESTE,
    ETAPA_IPC,
    ETAPA_CICLO_CONCLUIDO
};

EtapaProcesso etapaAtual = ETAPA_AGUARDANDO_INICIO;

// Flags
bool flag_LBAM = false;
bool flag_LBVM = false;
bool flag_LBAZ = false;
bool flag_DESC = false;
bool flag_LVOR = false;
bool flag_SUB  = false;
bool flag_iTES = false;
bool flag_IPC  = false;


// ==========================================
// REINICIALIZAÇÃO
// ==========================================

void reiniciarCicloTotal() {
    flag_LBAM = false;
    flag_LBVM = false;
    flag_LBAZ = false;
    flag_DESC = false;
    flag_LVOR = false;
    flag_SUB  = false;
    flag_iTES = false;
    flag_IPC  = false;

    tentativasCor = 0;
    tentativasVolume = 0;
    amostraEnviada = false;

    // Reseta acumuladores de volume e avaliação
    pesoTotalInjetadoReal = 0.0;
    volumeAvaliado = false;
    volumeAprovadoLocal = false;

    // Garante que o estado interno do vórtex também seja resetado
    vortexAtivo = false;
    vortexEmPausa = false;
    tempoRestanteVortex = 0;

    respostaServidorRecebida = false;
    corAprovadaPeloServidor = false;

    etapaAtual = ETAPA_AGUARDANDO_INICIO;

    publicarStatus("livre");

    Serial.println("\n==================================================");
    Serial.println(" LOTE FINALIZADO. AGUARDANDO NOVA RECEITA MQTT.");
    Serial.println("==================================================\n");
}


// ==========================================
// ENVIO DAS AMOSTRAS DO SENSOR AS7341
// ==========================================


void publicarLeituraSensorCor() {
    Serial.println("[SENSOR] Iniciando coleta e envio de 20 amostras...");


    for (int amostra = 1; amostra <= TOTAL_AMOSTRAS; amostra++) {
        manterMQTT();

        uint16_t value[10];
        as.ReadAllChannels(value);

        char payloadJson[300];
        snprintf(payloadJson, sizeof(payloadJson),
            "{\"tipo\":\"analise_cor\","
            "\"amostra\":%d,"
            "\"total_amostras\":%d,"
            "\"f1\":%u,"
            "\"f2\":%u,"
            "\"f3\":%u,"
            "\"f4\":%u,"
            "\"f5\":%u,"
            "\"f6\":%u,"
            "\"f7\":%u,"
            "\"f8\":%u}",
            amostra,
            TOTAL_AMOSTRAS,
            value[0], value[1], value[2], value[3],
            value[4], value[5], value[6], value[7]
        );

        if (client.connected()) {
            client.publish("pwa/tintas/maquina/sensor_cor", payloadJson);
        } else {
            Serial.println("[SENSOR] MQTT desconectado! Tentando reconectar...");
            reconectarMQTT();
        }

        client.loop();
        delay(100);
    }

    Serial.println("[SENSOR] Todas as 20 amostras enviadas. Aguardando verificação do servidor...");
}



// ==========================================
// SETUP & LOOP
// ==========================================

void setupPin() {
    Serial.begin(9600);
    inicializarMQTT();
    inicializarPinosEHardware();

    Serial.println("\n==================================================");
    Serial.println(" SISTEMA DE DOSAGEM COM CORRECAO AUTOMATICA");
    Serial.println("==================================================");
    Serial.println("Aguardando receita MQTT.");
}

void doEveryLoop() {
    manterMQTT();

    if (etapaAtual == ETAPA_AGUARDANDO_INICIO && receitaDisponivel) {
        receitaDisponivel = false;
        tentativasCor = 0;
        tentativasVolume = 0;
        pesoTotalInjetadoReal = 0.0;
        volumeAvaliado = false;
        amostraEnviada = false;
        respostaServidorRecebida = false;
        corAprovadaPeloServidor = false;

        Serial.println("[MQTT] Receita recebida! Iniciando lote.");
        servo_motor.write(100);
        balanca.tare();
        etapaAtual = ETAPA_BOMBA_AMARELA;
    }

    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        input.toUpperCase();

        if (etapaAtual == ETAPA_AGUARDANDO_INICIO && (input == "S" || input == "START")) {
            Serial.println("[SERIAL] START recebido!");
            tentativasCor = 0;
            tentativasVolume = 0;
            pesoTotalInjetadoReal = 0.0;
            volumeAvaliado = false;
            etapaAtual = ETAPA_BOMBA_AMARELA;
        } else if (input == "RESET") {
            reiniciarCicloTotal();
            Serial.println("[SERIAL] Sistema resetado!");
        }
    }
}


// ==========================================
// EVENTOS CONTROLÁVEIS
// ==========================================

bool EventControllable_LbAM() {
    if (etapaAtual == ETAPA_BOMBA_AMARELA && !flag_LBAM) {
        flag_LBAM = true;
        Serial.println("[AUTO] Bomba Amarela");
        publicarProgresso("amarelo", pesoAmarelo);
        return true;
    }
    return false;
}

bool EventControllable_LbVM() {
    if (etapaAtual == ETAPA_BOMBA_VERMELHA && !flag_LBVM) {
        flag_LBVM = true;
        Serial.println("[AUTO] Bomba Vermelha");
        publicarProgresso("vermelho", pesoVermelho);
        return true;
    }
    return false;
}

bool EventControllable_LbAZ() {
    if (etapaAtual == ETAPA_BOMBA_AZUL && !flag_LBAZ) {
        flag_LBAZ = true;
        Serial.println("[AUTO] Bomba Azul");
        publicarProgresso("azul", pesoAzul);
        return true;
    }
    return false;
}

bool EventControllable_DESC() {
    if (etapaAtual == ETAPA_DESCER && !flag_DESC) {
        flag_DESC = true;
        Serial.println("[AUTO] Descer");
        return true;
    }
    return false;
}

bool EventControllable_LVOR() {
    if (etapaAtual == ETAPA_VORTEX && !flag_LVOR) {
        flag_LVOR = true;
        status_vortex = 1;
        Serial.println("[AUTO] Valvula");
        return true;
    }
    return false;
}

bool EventControllable_SUB() {
    if (etapaAtual == ETAPA_SUBIR && !flag_SUB) {
        flag_SUB = true;
        Serial.println("[AUTO] Subir");
        return true;
    }
    return false;
}

bool EventControllable_iTES() {
    if (etapaAtual == ETAPA_TESTE && !flag_iTES) {
        flag_iTES = true;
        Serial.println("[AUTO] Inicio do teste (Aguardando servidor...)");
        return true;
    }
    return false;
}

bool EventControllable_IpC() {
    if (etapaAtual == ETAPA_IPC && !flag_IPC) {
        flag_IPC = true;
        Serial.println("[AUTO] Verificacao de volume");
        return true;
    }
    return false;
}


// ==========================================
// EVENTOS INCONTROLÁVEIS
// ==========================================


bool EventUncontrollable_DbAM() {
    if (etapaAtual == ETAPA_BOMBA_AMARELA && flag_LBAM && balanca.is_ready()) {
    //if (etapaAtual == ETAPA_BOMBA_AMARELA && flag_LBAM) {
        float pesoAtual = balanca.get_units(2);
        if (pesoAtual < 0) pesoAtual = 0;

        Serial.print("Peso Amarelo: "); Serial.println(pesoAtual);
        publicarProgresso("amarelo", pesoAtual);

        float margemInercia = 0.8; // Gramas antes do alvo para desligar (ajuste se necessário)
        
        if (pesoAtual >= (pesoAmarelo - margemInercia)) {
        //if (pesoAtual >= pesoAmarelo) {
            Serial.println("[BOMBA] Amarelo concluído.");
            publicarProgresso("amarelo", pesoAtual);

            pesoTotalInjetadoReal += pesoAtual;

            balanca.tare();
            delay(100);
            etapaAtual = ETAPA_BOMBA_VERMELHA;
            return true;
        }
    }
    return false;
}

bool EventUncontrollable_DbVM() {
    if (etapaAtual == ETAPA_BOMBA_VERMELHA && flag_LBVM && balanca.is_ready()) {
    //if (etapaAtual == ETAPA_BOMBA_VERMELHA && flag_LBVM) {
        float pesoAtual = balanca.get_units(2);
        if (pesoAtual < 0) pesoAtual = 0;

        Serial.print("Peso Vermelho: "); Serial.println(pesoAtual);
        publicarProgresso("vermelho", pesoAtual);

        float margemInercia = 0.8; // Gramas antes do alvo para desligar (ajuste se necessário)
        
        if (pesoAtual >= (pesoVermelho - margemInercia)) {
        //if (pesoAtual >= pesoVermelho) {
            Serial.println("[BOMBA] Vermelho concluído.");
            publicarProgresso("vermelho", pesoAtual);

            pesoTotalInjetadoReal += pesoAtual;

            balanca.tare();
            delay(100);
            etapaAtual = ETAPA_BOMBA_AZUL;
            return true;
        }
    }
    return false;
}

bool EventUncontrollable_DbAZ() {
    if (etapaAtual == ETAPA_BOMBA_AZUL && flag_LBAZ && balanca.is_ready()) {
    //if (etapaAtual == ETAPA_BOMBA_AZUL && flag_LBAZ) {
        float pesoAtual = balanca.get_units(2);
        if (pesoAtual < 0) pesoAtual = 0;

        Serial.print("Peso Azul: "); Serial.println(pesoAtual);
        publicarProgresso("azul", pesoAtual);

        float margemInercia = 0.8; // Gramas antes do alvo para desligar (ajuste se necessário)
        
        if (pesoAtual >= (pesoAzul - margemInercia)) {
        //if (pesoAtual >= pesoAzul) {
            Serial.println("[BOMBA] Azul concluído.");
            publicarProgresso("azul", pesoAtual);

            pesoTotalInjetadoReal += pesoAtual;

            balanca.tare();
            delay(100);
            etapaAtual = ETAPA_DESCER;
            return true;
        }
    }
    return false;
}



bool EventUncontrollable_F_DESC() {
    if (etapaAtual == ETAPA_DESCER && flag_DESC && digitalRead(CHAVE_FIM_BAIXO) == LOW) {
        delay(30); 
        
        // Dupla checagem para garantir que o contato permaneceu fechado
        if (digitalRead(CHAVE_FIM_BAIXO) == LOW) {
            Serial.println("[FIM DE CURSO] F_DESC acionado (Cabeçote embaixo).");
            etapaAtual = ETAPA_VORTEX;
            return true;
        }
    }
    return false;
}

bool EventUncontrollable_DVOR() {
    if (etapaAtual == ETAPA_VORTEX && flag_LVOR && status_vortex == 0) {
        Serial.println("[BOTAO 1] DVOR acionado.");
        etapaAtual = ETAPA_SUBIR;
        return true;
    }
    return false;
}

bool EventUncontrollable_F_SUB() {
    if (etapaAtual == ETAPA_SUBIR && flag_SUB && digitalRead(CHAVE_FIM_CIMA) == LOW) {
        delay(30);

        if (digitalRead(CHAVE_FIM_CIMA) == LOW) {
            // Esticar servo motor
            servo_motor.write(100); 
            Serial.println("[FIM DE CURSO] F_SUB acionado (Cabeçote em cima).");
            etapaAtual = ETAPA_TESTE;
            return true;
        }
    }
    return false;
}


// ==========================================
// BIFURCAÇÃO DE COR
// ==========================================

bool EventUncontrollable_cor_OK() {
    if (etapaAtual == ETAPA_TESTE && respostaServidorRecebida && corAprovadaPeloServidor) {
        Serial.println("[SERVIDOR] Cor aprovada!");
        respostaServidorRecebida = false;
        amostraEnviada = false;
        etapaAtual = ETAPA_IPC;
        return true;
    }
    return false;
}

bool EventUncontrollable_cor_N() {
    if (etapaAtual == ETAPA_TESTE && respostaServidorRecebida && !corAprovadaPeloServidor) {
        Serial.println("[SERVIDOR] Cor reprovada! Aplicando nova correção...");
        tentativasCor++;

        respostaServidorRecebida = false;
        corAprovadaPeloServidor = false;
        amostraEnviada = false;

        if (tentativasCor <= MAX_TENTATIVAS) {
            Serial.println("[CORRECAO] Reiniciando dosagem com novos pesos incrementais.");
            etapaAtual = ETAPA_BOMBA_AMARELA;

            flag_LBAM = false;
            flag_LBVM = false;
            flag_LBAZ = false;
            flag_DESC = false;
            flag_LVOR = false;
            flag_SUB = false;
            flag_iTES = false;
            flag_IPC = false;
        } else {
            Serial.println("[ALERTA] Limite de correcoes de cor atingido! Abortando lote.");
            reiniciarCicloTotal();
        }
        return true;
    }
    return false;
}


// ==========================================
// BIFURCAÇÃO DE VOLUME
// ==========================================

bool EventUncontrollable_volume_OK() {
    if (etapaAtual == ETAPA_IPC && volumeAvaliado && volumeAprovadoLocal) {
        Serial.println("[VOLUME] Transição volume_OK disparada.");
        volumeAvaliado = false;
        reiniciarCicloTotal();
        return true;
    }
    return false;
}

bool EventUncontrollable_volume_N() {
    if (etapaAtual == ETAPA_IPC && volumeAvaliado && !volumeAprovadoLocal) {
        tentativasVolume++;
        volumeAvaliado = false;

        if (tentativasVolume <= MAX_TENTATIVAS) {
            Serial.println("[VOLUME] Transição volume_N disparada. Aplicando correção.");
            etapaAtual = ETAPA_BOMBA_AMARELA;
            
            // Reseta flags das bombas
            flag_LBAM = false;
            flag_LBVM = false;
            flag_LBAZ = false;
            flag_DESC = false;
            flag_LVOR = false;
            flag_SUB = false;
            flag_iTES = false;
            flag_IPC = false;
        } else {
            Serial.println("[ALERTA] Limite de correções de volume atingido!");
            reiniciarCicloTotal();
        }
        return true;
    }
    return false;
}



// ==========================================
// AÇÕES DOS ESTADOS
// ==========================================

void StateActionAutomaton0_G1State0() {
    // Bomba amarela desligada
    digitalWrite(BOMBA_AM, LOW);
}

void StateActionAutomaton0_G1State1() {
    // Liga a bomba amarela
    //digitalWrite(BOMBA_AM, HIGH);
}

void StateActionAutomaton1_G2State0() {
    // Bomba vermelha desligada
    digitalWrite(BOMBA_VM, LOW);
}

void StateActionAutomaton1_G2State1() {
    // Liga a bomba vermelha
    //digitalWrite(BOMBA_VM, HIGH);
}

void StateActionAutomaton2_G3State0() {
    // Bomba azul desligada
    digitalWrite(BOMBA_AZ, LOW);
}

void StateActionAutomaton2_G3State1() {
    // Liga a bomba azul
    //digitalWrite(BOMBA_AZ, HIGH);
}

void StateActionAutomaton3_G6State0() {
    // Elevador desligado
	digitalWrite(ENABLE_ELEV, HIGH);

	// Eletroímã desligado
	digitalWrite(ELETROIMA, LOW);
}


void StateActionAutomaton3_G6State1() {
    // SOBE
	// Ligar eletroímã 
    digitalWrite(ELETROIMA, HIGH);

    // Recolher servo motor
    servo_motor.write(0);

    // Verificar se o servo realmente está recolhido
    if (digitalRead(CHAVE_FIM_RECOLHIDO) == LOW) {
        // Ligar elevador
        // SUBIR
        digitalWrite(ENABLE_ELEV, LOW);
        digitalWrite(DIR_ELEV, LOW);

        digitalWrite(STEP_ELEV, HIGH);
        delayMicroseconds(1500);
        digitalWrite(STEP_ELEV, LOW);
        delayMicroseconds(1500);
    }
}

void StateActionAutomaton3_G6State2() {
    // DESCE
	// Ligar eletroímã 
    digitalWrite(ELETROIMA, HIGH);

    // Recolher servo motor
    servo_motor.write(0);

    // Verificar se o servo realmente está recolhido
    if (digitalRead(CHAVE_FIM_RECOLHIDO) == LOW) {
        // Ligar elevador
        // DESCER
        digitalWrite(ENABLE_ELEV, LOW);
        digitalWrite(DIR_ELEV, HIGH);

        digitalWrite(STEP_ELEV, HIGH);
        delayMicroseconds(1500);
        digitalWrite(STEP_ELEV, LOW);
        delayMicroseconds(1500);
    }
}

void StateActionAutomaton4_G7State0() {
    // Vórtex (relé) desligado
	digitalWrite(RELE_PIN, LOW);
}

/*
void StateActionAutomaton4_G7State1() {
  if (!vortexAtivo) {
    iniciarVortex();
    return;
  }

  unsigned long agora = millis();

  // Se estiver na pausa de 30s entre blocos de 2 min
  if (vortexEmPausa) {
    if (agora - inicioPausaVortex >= 30000UL) {
      vortexEmPausa = false;
      tempoLigadoNoBlocoAtual = 0;
      inicioCicloVortex = agora;
      digitalWrite(RELE_PIN, HIGH); // Religar após pausa de 30s
      Serial.println("[VORTEX] Retomando agitacao.");
    }
    return;
  }

  // Tempo decorrido desde que ligou/retomou o bloco atual
  unsigned long decorridoBloco = agora - inicioCicloVortex;

  // Se já completou 2 minutos (120.000 ms) neste bloco e ainda há tempo restante para o total
  if (decorridoBloco >= 120000UL && tempoRestanteVortex > 120000UL) {
    tempoRestanteVortex -= 120000UL;
    digitalWrite(RELE_PIN, LOW); // Desliga para pausa de 30s
    vortexEmPausa = true;
    inicioPausaVortex = agora;
    Serial.println("[VORTEX] Pausa de 30s.");
    return;
  }

  // Verifica se o tempo total restante já acabou
  if (decorridoBloco >= tempoRestanteVortex) {
    digitalWrite(RELE_PIN, LOW); // Desliga vórtex
    vortexAtivo = false;
    vortexEmPausa = false;
    tempoRestanteVortex = 0;
    status_vortex = 0; // Dispara evento -> desliga_vortex
    Serial.println("A6S1 - Vortex finalizado.");
  }
}
*/

/*
void StateActionAutomaton4_G7State1() {
    // Vórtex (relé) ligado
	digitalWrite(RELE_PIN, HIGH);
}
*/
bool timer2MinAtivo = false;
unsigned long inicio2MinVortex = 0;
const unsigned long DURACAO_1MIN_MS = 60000UL; // 1 minutos = 60.000 ms

void StateActionAutomaton4_G7State1() {
    unsigned long agora = millis();

    // Inicia a contagem de 2 minutos na primeira vez que entra aqui
    if (!timer2MinAtivo) {
        timer2MinAtivo = true;
        inicio2MinVortex = agora;
        ultimoEnvioVortexMQTT = agora;

        digitalWrite(RELE_PIN, HIGH); // Liga o relé (ajuste para LOW se o módulo for ativo em nível baixo)
        publicarVortex(true, DURACAO_1MIN_MS / 1000UL); // 120s
        Serial.println("[VORTEX] Ligado por 2 minutos...");
        return;
    }

    unsigned long decorrido = agora - inicio2MinVortex;

    // Após 2 minutos, desliga o relé e notifica fim
    if (decorrido >= DURACAO_1MIN_MS) {
        digitalWrite(RELE_PIN, LOW); // Desliga o relé (ajuste para HIGH se o módulo for ativo em nível baixo)
        timer2MinAtivo = false;      // Libera para um próximo ciclo quando voltar a rodar
        status_vortex = 0;           // Se o seu autômato usar status_vortex para avançar
        publicarVortex(false, 0);
        Serial.println("[VORTEX] 2 minutos concluídos, relé desligado.");
        return;
    }

    // Atualiza a contagem regressiva no MQTT a cada 1 segundo enquanto ligado
    if (agora - ultimoEnvioVortexMQTT >= 1000UL) {
        ultimoEnvioVortexMQTT = agora;
        unsigned long restanteMs = DURACAO_1MIN_MS - decorrido;
        unsigned long restanteSeg = (restanteMs + 999UL) / 1000UL;
        publicarVortex(true, restanteSeg);
    }
}

void StateActionAutomaton5_G8State0() {}

void StateActionAutomaton5_G8State1() {
    respostaServidorRecebida = false;
    corAprovadaPeloServidor = false;

    if (!amostraEnviada) {
        Serial.println("[ESTADO TESTE] Enviando 20 amostras do AS7341...");
        publicarLeituraSensorCor();
        amostraEnviada = true;
    }
}

void StateActionAutomaton6_G9State0() {}

void StateActionAutomaton6_G9State1() {
    if (!volumeAvaliado) {
        Serial.print("Peso líquido total acumulado: ");
        Serial.print(pesoTotalInjetadoReal);
        Serial.print(" g | Alvo esperado: ");
        Serial.print(pesoAlvoTotalGrams);
        Serial.println(" g");

        // Compara direto a soma das tintas com a tolerância
        float diferenca = abs(pesoTotalInjetadoReal - pesoAlvoTotalGrams);
        const float TOLERANCIA_GRAMAS = 5;

        if (diferenca <= TOLERANCIA_GRAMAS) {
            Serial.println("[VOLUME] Aprovado localmente!");
            volumeAprovadoLocal = true;
        } else {
            Serial.println("[VOLUME] Fora da tolerância! Reprovado.");
            volumeAprovadoLocal = false;
        }

        volumeAvaliado = true;
    }
}