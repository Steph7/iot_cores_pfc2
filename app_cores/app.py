import os
import colorsys
import json
import asyncio
import math
import numpy as np
import paho.mqtt.client as mqtt
import statistics
import threading
from pathlib import Path
from uuid import uuid4
from fastapi import FastAPI, File, HTTPException, UploadFile, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from google.cloud import vision
from pydantic import BaseModel

os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "cred.json"

app = FastAPI()

UPLOAD_DIR = Path("uploads")
UPLOAD_DIR.mkdir(exist_ok=True)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# --- GERENCIADOR DE CONEXÕES WEBSOCKET ---
class ConnectionManager:
    def __init__(self):
        self.active_connections: list[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        self.active_connections.remove(websocket)

    async def broadcast(self, message: dict):
        for connection in self.active_connections:
            try:
                await connection.send_json(message)
            except Exception:
                pass

manager = ConnectionManager()

# --- VARIÁVEIS DE ESTADO DO LOTE ATUAL ---
lote_atual = {
    "ativo": False,
    "cor_alvo": "#FFFFFF",
    "volume_total_ml": 20.0,
    "pesos_acumulados": {"vermelho": 0.0, "amarelo": 0.0, "azul": 0.0},
    "fase_correcao": False
}

# CONTROLE DAS AMOSTRAS DO SENSOR AS7341
TOTAL_AMOSTRAS = 20

amostras_sensor = []
lock_amostras = threading.Lock()

# --- CONFIGURAÇÃO DO MQTT (HiveMQ Público) ---
MQTT_BROKER = "broker.hivemq.com"
#MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
MQTT_TOPICO = "pwa/tintas/receita"
PREFIXO_TOPICO = "pwa/tintas/maquina"

TOPICOS_ESCUTA = [
    (f"{PREFIXO_TOPICO}/status/+", 0),      # Ex: pwa/tintas/maquina/status/livre
    (f"{PREFIXO_TOPICO}/bomba/+/+", 0),     # Ex: pwa/tintas/maquina/bomba/vermelho/5.40
    (f"{PREFIXO_TOPICO}/vortex/+", 0),      # Ex: pwa/tintas/maquina/vortex/3
    (f"{PREFIXO_TOPICO}/sensor_cor", 0)     # Tópico onde o Arduino envia os dados do AS7341
]

# Variável para memorizar a cor alvo solicitada na receita atual
ultima_cor_alvo = "#FFFFFF"

# Inicializa o cliente MQTT
mqtt_client = mqtt.Client()

def filtrar_outliers(amostras):
    """
    Remove valores discrepantes usando o intervalo interquartil.
    O processamento é feito individualmente para cada canal.
    """
    if len(amostras) < 5:
        return amostras

    filtradas = []
    for amostra in amostras:
        filtradas.append(amostra.copy())

    for canal in range(8):
        valores = [a[f"f{canal+1}"] for a in amostras]
        valores_ordenados = sorted(valores)

        q1 = np.percentile(valores_ordenados, 25)
        q3 = np.percentile(valores_ordenados, 75)
        iqr = q3 - q1

        limite_inferior = q1 - 1.5 * iqr
        limite_superior = q3 + 1.5 * iqr

        for i, valor in enumerate(valores):
            if valor < limite_inferior or valor > limite_superior:
                filtradas[i][f"f{canal+1}"] = None

    resultado = []
    for amostra in filtradas:
        if any(amostra[f"f{i}"] is None for i in range(1, 9)):
            continue
        resultado.append(amostra)

    return resultado


def calcular_media_amostras(amostras):
    """
    Calcula a média dos oito canais das amostras válidas.
    """
    resultado = {}
    for canal in range(1, 9):
        valores = [
            amostra[f"f{canal}"]
            for amostra in amostras
        ]
        resultado[f"f{canal}"] = round(
            statistics.mean(valores), 2
        )
    return resultado

def on_connect(client, userdata, flags, rc):
  if rc == 0:
    print("Conectado ao Broker MQTT com sucesso!")
    client.subscribe(TOPICOS_ESCUTA)
  else:
    print(f"Falha na conexão MQTT, código de retorno: {rc}")

def on_message(client, userdata, msg):
    print(f"🔍 DEBUG GLOBAL MQTT -> Tópico: '{msg.topic}' | Payload: '{msg.payload.decode('utf-8')}'")
    try:
        global ultima_cor_alvo, lote_atual
        topico = msg.topic
        payload = msg.payload.decode("utf-8")
        print(f"📥 MQTT Recebido no tópico: {topico} | Payload: {payload}")

        partes = topico.split("/")
        dados_websocket = {}

        # 1. Tratamento de Status da Máquina
        if "status" in topico:
            estado = partes[-1] 
            dados_websocket = {"tipo": "status_maquina", "estado": estado}
            
        # 2. Tratamento de Progresso das Bombas
        elif "bomba" in topico:
            try:
                if payload and payload.strip():
                    valor = float(payload)
                    cor = partes[-1]  
                else:
                    valor = float(partes[-1])
                    cor = partes[-2]
                
                dados_websocket = {"tipo": "progresso_bomba", "bomba": cor, "valor": valor}
            except Exception as e:
                print(f"⚠️ Erro ao converter valor da bomba: {e}")

        # 3. Tratamento do Vortex
        elif "vortex" in topico:
            try:
                dados_vortex = json.loads(payload) if payload and payload.strip() else {}
            except Exception:
                dados_vortex = {}

            ligado = bool(dados_vortex.get("ligado", False))
            tempo_restante = int(dados_vortex.get("tempo_restante_seg", 0))

            dados_websocket = {
                "tipo": "vortex",
                "ligado": ligado,
                "tempo_restante_seg": tempo_restante,
            }

        # 4. Tratamento do Sensor de Cor AS7341 (Qualidade)
        elif "sensor_cor" in topico:
            if not payload.strip():
                return

            dados_espectro = json.loads(payload)

            # Validação leve para garantir que o JSON tem os canais espectrais
            if "f1" not in dados_espectro:
                return

            # Guarda uma amostra recebida
            with lock_amostras:
                amostras_sensor.append(dados_espectro)
                quantidade = len(amostras_sensor)

                print(f"[SENSOR] Amostra {quantidade}/{TOTAL_AMOSTRAS} recebida")

                # Aguarda completar as 20 amostras
                if quantidade < TOTAL_AMOSTRAS:
                    return

                # Copia as amostras e limpa o buffer
                conjunto_amostras = amostras_sensor.copy()
                amostras_sensor.clear()

            print("[SENSOR] 20 amostras recebidas. Iniciando processamento...")

            # Remove outliers
            amostras_validas = filtrar_outliers(conjunto_amostras)

            if len(amostras_validas) < 5:
                print("[SENSOR] Poucas amostras válidas. Solicitando nova leitura.")
                dados_websocket = {
                    "tipo": "sensor_cor",
                    "status_qc": "erro_leitura",
                    "mensagem": "Amostras insuficientes"
                }
                return

            # Calcula a média
            espectro_medio = calcular_media_amostras(amostras_validas)
            print(f"[SENSOR] Amostras válidas: {len(amostras_validas)}")
            print(f"[SENSOR] Espectro médio: {espectro_medio}")

            # Converte o espectro médio para RGB
            hex_calculado = espectro_8_para_rgb_calibrado(espectro_medio)
            print(f"[COR] Alvo: {ultima_cor_alvo} | Medida: {hex_calculado}")

            def hex_to_rgb(hex_str):
                hex_str = hex_str.lstrip("#")
                return tuple(int(hex_str[i:i+2], 16) for i in (0, 2, 4))

            r_alvo, g_alvo, b_alvo = hex_to_rgb(ultima_cor_alvo)
            r_lido, g_lido, b_lido = hex_to_rgb(hex_calculado)

            distancia_cor = math.sqrt(
                (r_alvo - r_lido) ** 2 +
                (g_alvo - g_lido) ** 2 +
                (b_alvo - b_lido) ** 2
            )

            TOLERANCIA_MAXIMA = 90.0
            resposta_arduino = {}

            if distancia_cor <= TOLERANCIA_MAXIMA:
                print(f"✅ [COR] APROVADA! Erro: {distancia_cor:.2f}")
                resposta_arduino = {"resultado_cor": "ok"}
            else:
                print(f"⚠️ [COR] REPROVADA! Erro: {distancia_cor:.2f}. Calculando correção...")

                densidade = 1.17
                volume_max_g = lote_atual["volume_total_ml"] * densidade
                peso_atual_total = sum(lote_atual["pesos_acumulados"].values())
                espaco_restante_g = max(0.0, volume_max_g - peso_atual_total)

                # Limita o incremento a no máximo 2g por rodada de correção
                incremento_g = min(espaco_restante_g, 2.0)

                # CORREÇÃO APLICADA AQUI: g_lido em vez de r_lido
                diff_r = r_alvo - r_lido
                diff_g = g_alvo - g_lido  
                diff_b = b_alvo - b_lido

                correcao_pesos = {"amarelo": 0.0, "vermelho": 0.0, "azul": 0.0}

                if diff_g > 15:
                    correcao_pesos["amarelo"] = round(incremento_g * 0.5, 2)
                if diff_r > 15:
                    correcao_pesos["vermelho"] = round(incremento_g * 0.5, 2)
                if diff_b > 15:
                    correcao_pesos["azul"] = round(incremento_g * 0.5, 2)

                # Atualiza acumulados do lote
                for k in correcao_pesos:
                    lote_atual["pesos_acumulados"][k] += correcao_pesos[k]

                resposta_arduino = {
                    "resultado_cor": "corrigir",
                    "novos_pesos": correcao_pesos
                }

            # Responde ao Arduino via MQTT
            TOPICO_RESPOSTA_ARDUINO = f"{PREFIXO_TOPICO}/resposta_cor"
            client.publish(TOPICO_RESPOSTA_ARDUINO, json.dumps(resposta_arduino))

            dados_websocket = {
                "tipo": "sensor_cor",
                "cor": hex_calculado,
                "status_qc": resposta_arduino["resultado_cor"],
                "erro": round(distancia_cor, 2),
                "amostras_recebidas": len(conjunto_amostras),
                "amostras_validas": len(amostras_validas)
            }

        # 5. Tratamento da Verificação de Volume (IPC)
        elif "volume" in topico:
            if not payload.strip():
                return

            dados_volume = json.loads(payload)
            peso_medido = dados_volume.get("peso_liquido", 0.0)
            
            # Converte volume alvo (mL) para gramas usando a densidade (1.17)
            densidade = 1.17
            volume_alvo_ml = lote_atual.get("volume_total_ml", 20.0)
            peso_alvo_g = volume_alvo_ml * densidade
            
            # Tolerância de volume (ex: ± 10% ou margem fixa de 2g)
            diferenca_peso = abs(peso_alvo_g - peso_medido)
            TOLERANCIA_VOLUME = 5 # gramas

            resposta_volume = {}
            if diferenca_peso <= TOLERANCIA_VOLUME:
                print(f"✅ [VOLUME] APROVADO! Medido: {peso_medido}g | Alvo: {peso_alvo_g}g")
                resposta_volume = {"resultado_volume": "ok"}
            else:
                print(f"⚠️ [VOLUME] FORA DO PADRÃO! Medido: {peso_medido}g | Alvo: {peso_alvo_g}g")
                resposta_volume = {"resultado_volume": "corrigir"}

            # Publica a resposta de volta para o Arduino
            client.publish(f"{PREFIXO_TOPICO}/resposta_volume", json.dumps(resposta_volume))
        
        # Envia via WebSocket para o navegador
        if dados_websocket and loop_do_fastapi:
            asyncio.run_coroutine_threadsafe(
                manager.broadcast(dados_websocket), 
                loop_do_fastapi
            )
            
    except Exception as e:
        print(f"❌ Erro ao processar mensagem MQTT no on_message: {e}")

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

try:
  mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
  mqtt_client.loop_start() 
except Exception as e:
  print(f"Aviso: Não foi possível conectar ao broker MQTT: {e}")

loop_do_fastapi = None
@app.on_event("startup")
async def startup_event():
    global loop_do_fastapi
    loop_do_fastapi = asyncio.get_running_loop()

# --- ROTA WEBSOCKET PARA O FRONTEND ---
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        manager.disconnect(websocket)

class ComandoCor(BaseModel):
  cor: str
  volumeTotal: float = 20.0

class ReceitaMQTT(BaseModel):
  cor: str
  volumeTotal: float
  pesos: dict

def hex_to_ryb(hex_color):
    hex_color = hex_color.lstrip("#")
    r = int(hex_color[0:2], 16) / 255
    g = int(hex_color[2:4], 16) / 255
    b = int(hex_color[4:6], 16) / 255

    m, l, s = colorsys.rgb_to_hls(r, g, b)
    matiz = m * 360

    vermelho = azul = amarelo = 0.0

    if matiz < 60:
        vermelho = 1
        amarelo = matiz / 60
    elif matiz < 120:
        amarelo = 1
        vermelho = 1 - (matiz - 60) / 60
    elif matiz < 180:
        amarelo = 1
        azul = (matiz - 120) / 60
    elif matiz < 240:
        azul = 1
        amarelo = 1 - (matiz - 180) / 60
    elif matiz < 300:
        azul = 1
        vermelho = (matiz - 240) / 60
    else:
        vermelho = 1
        azul = 1 - (matiz - 300) / 60

    total = vermelho + amarelo + azul
    if total == 0:
        total = 1

    return {
        "vermelho": vermelho / total,
        "amarelo": amarelo / total,
        "azul": azul / total,
    }

PRETO_REF = [510, 2400, 1400, 1300, 1500, 1300, 1050, 630]
BRANCO_REF = [1350, 7000, 4500, 4000, 4800, 4200, 3500, 2150]

def espectro_8_para_rgb_calibrado(leituras_espectro):
  if isinstance(leituras_espectro, dict):
    bruto = [
        leituras_espectro.get("f1", 0),
        leituras_espectro.get("f2", 0),
        leituras_espectro.get("f3", 0),
        leituras_espectro.get("f4", 0),
        leituras_espectro.get("f5", 0),
        leituras_espectro.get("f6", 0),
        leituras_espectro.get("f7", 0),
        leituras_espectro.get("f8", 0),
    ]
  else:
    bruto = leituras_espectro

  soma_bruta = sum(bruto)
  if soma_bruta < 500:
    return "#000000"

  normalizado = []
  for i in range(8):
    limpo = max(0, bruto[i] - PRETO_REF[i])
    amplitude = BRANCO_REF[i] - PRETO_REF[i]
    if amplitude <= 0:
      amplitude = 1
    val_norm = limpo / amplitude
    normalizado.append(val_norm)

  r = normalizado[5] * 0.7 + normalizado[6] * 1.0 + normalizado[7] * 0.9
  g = normalizado[3] * 0.5 + normalizado[4] * 1.0 + normalizado[5] * 0.6
  b = (normalizado[0] * 0.6 + normalizado[1] * 1.0) * 1.4

  max_val = max(r, g, b, 0.001)
  r_int = int(min(255, (r / max_val) * 255))
  g_int = int(min(255, (g / max_val) * 255))
  b_int = int(min(255, (b / max_val) * 255))

  return f"#{r_int:02x}{g_int:02x}{b_int:02x}"

@app.post("/cor")
def receber_cor(comando: ComandoCor):
  global lote_atual
  # LIMPEZA DE SEGURANÇA DO BUFFER DO SENSOR
  '''with lock_amostras:
    amostras_sensor.clear()
  print("[SERVIDOR] Buffer de amostras do sensor limpo para novo lote.")
  '''
  cor = comando.cor
  volume_total = comando.volumeTotal

  if volume_total > 20:
    raise HTTPException(400, "O volume máximo permitido é de 20 mL.")

  if (
      len(cor) != 7
      or not cor.startswith("#")
      or any(c not in "0123456789abcdefABCDEF" for c in cor[1:])
  ):
    raise HTTPException(400, "Cor inválida")

  volume_inicial_dosagem = volume_total * 0.80
  proporcoes = hex_to_ryb(cor)
  pesos_tintas = {}
  densidade = 1.17

  for tinta, proporcao in proporcoes.items():
    ml = proporcao * volume_inicial_dosagem
    gramas = ml * densidade
    pesos_tintas[tinta] = round(gramas, 2)

  lote_atual = {
      "ativo": True,
      "cor_alvo": cor,
      "volume_total_ml": volume_total,
      "pesos_acumulados": pesos_tintas.copy(),
      "fase_correcao": False
  }

  return {
      "ok": True,
      "cor": cor,
      "volumeTotal": volume_total,
      "pesos": pesos_tintas,
  }

@app.post("/enviar-mqtt")
def enviar_mqtt(receita: ReceitaMQTT):
  global ultima_cor_alvo
  ultima_cor_alvo = receita.cor  

  print("--- Rota /enviar-mqtt acionada ---")
  print(f"Cor alvo salva: {ultima_cor_alvo}")
  print(f"Pesos recebidos: {receita.pesos}")

  payload_mqtt = {
      "cor_hex": receita.cor,
      "volume_total_ml": receita.volumeTotal,
      "pesos_g": receita.pesos,
  }

  try:
    if not mqtt_client.is_connected():
      mqtt_client.reconnect()

    mqtt_client.publish(MQTT_TOPICO, json.dumps(payload_mqtt))
    print(f"SUCESSO! Mensagem publicada no tópico '{MQTT_TOPICO}'")
    return {"ok": True, "mensagem": "Receita enviada via MQTT com sucesso!"}
  except Exception as e:
    print(f"ERRO CRÍTICO AO PUBLICAR NO MQTT: {str(e)}")
    raise HTTPException(500, f"Erro ao publicar no MQTT: {str(e)}")

@app.post("/imagem")
async def receber_imagem(arquivo: UploadFile = File(...)):
  if arquivo.content_type not in {"image/jpeg", "image/png", "image/webp"}:
    raise HTTPException(400, "Formato de imagem inválido")

  conteudo = await arquivo.read()
  if len(conteudo) > 5 * 1024 * 1024:
    raise HTTPException(413, "Imagem maior que 5 MB")

  extensoes = {
      "image/jpeg": ".jpg",
      "image/png": ".png",
      "image/webp": ".webp",
  }
  nome = f"{uuid4().hex}{extensoes[arquivo.content_type]}"
  destino = UPLOAD_DIR / nome
  destino.write_bytes(conteudo)

  try:
    client = vision.ImageAnnotatorClient()
    with open(destino, "rb") as image_file:
      content = image_file.read()

    image = vision.Image(content=content)
    response = client.image_properties(image=image)
    props = response.image_properties_annotation

    cores_detectadas = []
    if props and props.dominant_colors:
      for color in props.dominant_colors.colors:
        rgb = color.color
        cores_detectadas.append({
            "rgb": {"red": rgb.red, "green": rgb.green, "blue": rgb.blue},
            "score": round(color.score, 2),
            "pixel_fraction": round(color.pixel_fraction, 2),
        })

    if response.error.message:
      raise HTTPException(
          500, f"Erro na Vision API: {response.error.message}"
      )
  except Exception as e:
    return {
        "ok": True,
        "arquivo": nome,
        "aviso": f"Erro na Vision API: {str(e)}",
    }
  return {"ok": True, "arquivo": nome, "cores_dominantes": cores_detectadas}

app.mount("/", StaticFiles(directory="static", html=True), name="static")