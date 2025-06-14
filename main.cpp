/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"


#include <queue>
#include <mutex>
#include <cmath>
#include <vector>
#include <algorithm>

AnalogIn lm35(A3);
AnalogIn termistor(A5);
constexpr float V_REF = 3.3f;
constexpr int NUM_MUESTRAS = 30;
constexpr float R_SERIE = 218.0f;

std::queue<float> colaLM35;
std::queue<float> colaTermistor;
Mutex console_mutex;

Thread threadLM35;
Thread threadTermistor;
Thread threadimprimir;

float voltaje = 0.0;

// Función para obtener el promedio de una cola
float calcular_promedio(std::queue<float> &cola) {
    float suma = 0.0f;
    int tam = cola.size();
    std::queue<float> temp = cola;

    while (!temp.empty()) {
        suma += temp.front();
        temp.pop();
    }
    return tam > 0 ? suma / tam : 0.0f;
}

// Función para calcular la mediana de una cola
float calcular_mediana(std::queue<float> &cola) {
    std::vector<float> datos;
    std::queue<float> temp = cola;

    while (!temp.empty()) {
        datos.push_back(temp.front());
        temp.pop();
    }

    if (datos.empty()) return 0.0f;

    std::sort(datos.begin(), datos.end());
    int tam = datos.size();

    return (tam % 2 == 0) ? (datos[tam / 2 - 1] + datos[tam / 2]) / 2.0f : datos[tam / 2];
}

// Función para imprimir temperatura separando parte entera y decimal
void imprimir_temperatura(const char* nombre, float temperatura_promedio, float temperatura_mediana) {
    int ent_prom = static_cast<int>(temperatura_promedio);
    int dec_prom = static_cast<int>((temperatura_promedio - ent_prom) * 100);

    int ent_mediana = static_cast<int>(temperatura_mediana);
    int dec_mediana = static_cast<int>((temperatura_mediana - ent_mediana) * 100);

    console_mutex.lock();
    printf("%s - Promedio: %d.%02d°C | Mediana: %d.%02d°C\n\r", 
           nombre, ent_prom, abs(dec_prom), ent_mediana, abs(dec_mediana));
    console_mutex.unlock();
}

void imprimir_crudo(const char* nombre, float lectura)
{
    int ent = static_cast<int>(lectura);
    int dec = static_cast<int>((lectura - ent) * 100);

    console_mutex.lock();
    printf("%s - Leido: %d.%02d°C \n\r", 
           nombre, ent, abs(dec));
    console_mutex.unlock();
}


// Función de lectura del LM35
void leerLM35() {
    while (true) {
        float voltaje = lm35.read();
        float tempLM35 = voltaje * 100.0f;

        if (colaLM35.size() == NUM_MUESTRAS) 
        {
            imprimir_temperatura("LM35", calcular_promedio(colaLM35), calcular_mediana(colaLM35));   
             
             // Vaciar la cola después de imprimir
            while (!colaLM35.empty()) colaLM35.pop();
        }
        colaLM35.push(tempLM35);
        ThisThread::sleep_for(100ms);
    }
}

// Función de lectura del termistor
void leerTermistor() {
    int lectura;
    while (true) {
        float voltaje = termistor.read()*V_REF;
        float resistencia_termistor = (voltaje * R_SERIE) / (V_REF - voltaje);
        float tempTermistor = 1.0f / (0.001129148 + (0.000234125 * log(resistencia_termistor)) + (0.0000000876741 * pow(log(resistencia_termistor), 3))) - 273.15;

        if (colaTermistor.size() == NUM_MUESTRAS) 
        {
            
           //imprimir_crudo("Leido", resistencia_termistor);
            imprimir_temperatura("Termistor", calcular_promedio(colaTermistor), calcular_mediana(colaTermistor));    

            // Vaciar la cola después de imprimir
            while (!colaTermistor.empty()) colaTermistor.pop();
        }    
        colaTermistor.push(tempTermistor);
        
        ThisThread::sleep_for(100ms);
    }
}

int main() {
    printf("Iniciando lectura de sensores...\n");

    threadLM35.start(leerLM35);
    threadTermistor.start(leerTermistor);


    
    console_mutex.lock();
    printf("Arranque del programa\n\r");
    console_mutex.unlock();   
    while(true)
    {

    }

}
