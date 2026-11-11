import os
import colorsys
import json
import asyncio
import numpy as np
import paho.mqtt.client as mqtt
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

# --- CONFIGURAÇÃO DO MQTT (HiveMQ Público) ---
MQTT_BROKER = "broker.hivemq.com"
MQTT_PORT = 1883
MQTT_TOPICO = "pwa/tintas/receita"
PREFIXO_TOPICO = "pwa/tintas/maquina"
TOPICOS_ESCUTA = [
    (f"{PREFIXO_TOPICO}/status/+", 0),   # Ex: pwa/tintas/maquina/status/livre
    (f"{PREFIXO_TOPICO}/bomba/+/+", 0),     # Ex: pwa/tintas/maquina/bomba/vermelho/5.40
    (f"{PREFIXO_TOPICO}/vortex/+", 0)
]

# Inicializa o cliente MQTT
mqtt_client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
  if rc == 0:
    print("Conectado ao Broker MQTT com sucesso!")
    client.subscribe(TOPICOS_ESCUTA)
  else:
    print(f"Falha na conexão MQTT, código de retorno: {rc}")

def on_message(client, userdata, msg):
    try:
        topico = msg.topic
        print(f"📥 MQTT Recebido no tópico: {topico}")

        payload = msg.payload.decode("utf-8")

        partes = topico.split("/")
        # partes[3] é "status", "bomba", "vortex" ou "sensor_cor"
        
        dados_websocket = {}

        if "status" in topico:
            # O estado (livre, ocupado, etc) está no final do tópico!
            estado = partes[-1] 
            dados_websocket = {"tipo": "status_maquina", "estado": estado}
            
        elif "bomba" in topico:
            # Exemplo: .../bomba/vermelho/5.40
            cor = partes[-2]   # vermelho
            valor = partes[-1]   # 5.40
            dados_websocket = {"tipo": "progresso_bomba", "bomba": cor, "valor": valor}

        elif "vortex" in topico:
            # Exemplo: .../vortex/3
            valor = partes[-1]   # 3
            dados_websocket = {"tipo": "progresso_vortex", "tempo": valor}

        elif "sensor_cor" in topico:
            dados_espectro = json.loads(payload)
            hex_calculado = espectro_8_para_rgb_calibrado(dados_espectro)
            
            # Envia o Hex resultante para o PWA atualizar o visor do sensor
            dados_websocket = {"tipo": "sensor_cor", "cor": hex_calculado}

        if dados_websocket and loop_do_fastapi:
            asyncio.run_coroutine_threadsafe(
                manager.broadcast(dados_websocket), 
                loop_do_fastapi
            )
    except Exception as e:
        print(f"Erro ao processar mensagem MQTT: {e}")

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Tenta conectar ao broker público de forma não-bloqueante
try:
  mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
  mqtt_client.loop_start()  # Inicia a thread em background para manter a conexão
except Exception as e:
  print(f"Aviso: Não foi possível conectar ao broker MQTT: {e}")

# Captura o loop principal do FastAPI para threads do MQTT poderem usá-lo
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
            await websocket.receive_text() # Mantém a conexão aberta
    except WebSocketDisconnect:
        manager.disconnect(websocket)

class ComandoCor(BaseModel):
  cor: str
  volumeTotal: float = 20.0

class ReceitaMQTT(BaseModel):
  cor: str
  volumeTotal: float
  pesos: dict

def hex_to_rybwk(hex_color):
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

  branco = max(0, (1 - 0.5) * 2)
  preto = max(0, (0.5 - 1) * 2)

  neutro = 1 - s
  preto += neutro * 0.5

  total = vermelho + amarelo + azul + branco + preto
  if total == 0:
    total = 1

  return {
      "vermelho": vermelho / total,
      "amarelo": amarelo / total,
      "azul": azul / total,
      "branco": branco / total,
      "preto": preto / total,
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

  # Mantém o limiar leve da v4 original
  soma_bruta = sum(bruto)
  if soma_bruta < 5000:
    return "#000000"

  normalizado = []
  for i in range(8):
    limpo = max(0, bruto[i] - PRETO_REF[i])
    amplitude = BRANCO_REF[i] - PRETO_REF[i]
    if amplitude <= 0:
      amplitude = 1
    val_norm = limpo / amplitude
    normalizado.append(val_norm)

  # Mapeamento original da v4 para as cores quentes (que funcionavam bem)
  r = normalizado[5] * 0.7 + normalizado[6] * 1.0 + normalizado[7] * 0.9
  g = normalizado[3] * 0.5 + normalizado[4] * 1.0 + normalizado[5] * 0.6

  # CORREÇÃO DO AZUL: Multiplicamos o canal b por 1.4 para matar o roxo e fixar o azul
  b = (normalizado[0] * 0.6 + normalizado[1] * 1.0) * 1.4

  max_val = max(r, g, b, 0.001)
  r_int = int(min(255, (r / max_val) * 255))
  g_int = int(min(255, (g / max_val) * 255))
  b_int = int(min(255, (b / max_val) * 255))

  # Converte para Hexadecimal
  hex_cor = f"#{r_int:02x}{g_int:02x}{b_int:02x}"
  return hex_cor

# Função de conversão para peso
def volume_para_peso(volume):
  densidade = 1.17
  peso = volume * densidade
  return peso


@app.post("/cor")
def receber_cor(comando: ComandoCor):
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

  # 1. Proporções primárias
  proporcoes = hex_to_rybwk(cor)

  # 2. Calcula o volume e o peso correspondente em gramas para cada tinta
  pesos_tintas = {}
  densidade = 1.17

  for tinta, proporcao in proporcoes.items():
    ml = proporcao * volume_total
    gramas = ml * densidade
    pesos_tintas[tinta] = round(gramas, 2)

  print(
      f"Cor: {cor} | Vol. Total: {volume_total}mL | Pesos calculados (g):"
      f" {pesos_tintas}"
  )
  return {
      "ok": True,
      "cor": cor,
      "volumeTotal": volume_total,
      "pesos": pesos_tintas,
  }

@app.post("/enviar-mqtt")
def enviar_mqtt(receita: ReceitaMQTT):
  # ADICIONE ESTES PRINTS PARA DEPURAR NO TERMINAL DO UVICORN
  print("--- Rota /enviar-mqtt acionada ---")
  print(f"Cor recebida: {receita.cor}")
  print(f"Volume total: {receita.volumeTotal}")
  print(f"Pesos recebidos: {receita.pesos}")

  payload_mqtt = {
      "cor_hex": receita.cor,
      "volume_total_ml": receita.volumeTotal,
      "pesos_g": receita.pesos,
  }

  try:
    # Garante que o client está conectado, se não tenta reconectar
    if not mqtt_client.is_connected():
      print("Cliente MQTT desconectado. Tentando reconectar...")
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