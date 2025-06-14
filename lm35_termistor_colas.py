from machine import ADC, Pin
import time
import math
from collections import deque

# Configuración de pines analógicos
lm35 = ADC(Pin(3))  # Ajusta el pin según tu ESP32-C3
termistor = ADC(Pin(5))  # Ajusta el pin según tu ESP32-C3

lm35.atten(ADC.ATTN_11DB)  # Configurar la atenuación para mejor rango de medición
termistor.atten(ADC.ATTN_11DB)

V_REF = 3.3  # Voltaje de referencia
NUM_MUESTRAS = 30
R_SERIE = 218.0  # Resistencia en serie con el termistor

colaLM35 = deque(maxlen=NUM_MUESTRAS)
colaTermistor = deque(maxlen=NUM_MUESTRAS)

# Función para obtener el promedio
def calcular_promedio(cola):
    return sum(cola) / len(cola) if cola else 0.0

# Función para obtener la mediana
def calcular_mediana(cola):
    datos = sorted(cola)
    tam = len(datos)
    if tam == 0:
        return 0.0
    return datos[tam // 2] if tam % 2 else (datos[tam // 2 - 1] + datos[tam // 2]) / 2.0

# Lectura del LM35
def leer_lm35():
    voltaje = lm35.read() * (V_REF / 4095)  # Conversión de ADC a voltaje
    return voltaje * 100.0  # LM35 genera 10mV por °C

# Lectura del termistor con ecuación Steinhart-Hart
def leer_termistor():
    voltaje = termistor.read() * (V_REF / 4095)
    resistencia = (voltaje * R_SERIE) / (V_REF - voltaje)
    temp = 1.0 / (0.001129148 + (0.000234125 * math.log(resistencia)) + (0.0000000876741 * math.pow(math.log(resistencia), 3))) - 273.15
    return temp

# Función para imprimir valores con parte entera y decimal
def imprimir_temperatura(nombre, temperatura_prom, temperatura_mediana):
    ent_prom = int(temperatura_prom)
    dec_prom = abs(int((temperatura_prom - ent_prom) * 100))
    ent_mediana = int(temperatura_mediana)
    dec_mediana = abs(int((temperatura_mediana - ent_mediana) * 100))
    
    print(f"{nombre} - Promedio: {ent_prom}.{dec_prom:02d}°C | Mediana: {ent_mediana}.{dec_mediana:02d}°C")

# Bucle principal
while True:
    tempLM35 = leer_lm35()
    tempTermistor = leer_termistor()

    # Agregar valores a las colas
    colaLM35.append(tempLM35)
    colaTermistor.append(tempTermistor)

    # Calcular estadísticas y mostrar resultados
    imprimir_temperatura("LM35", calcular_promedio(colaLM35), calcular_mediana(colaLM35))
    imprimir_temperatura("Termistor", calcular_promedio(colaTermistor), calcular_mediana(colaTermistor))

    # Vaciar las colas después de imprimir
    colaLM35.clear()
    colaTermistor.clear()

    time.sleep(0.1)  # Ajuste de ciclo a 100ms