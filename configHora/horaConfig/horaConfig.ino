/**
 * @file rtc_sync_dual.ino
 * @brief Sincronización simultánea de dos módulos RTC DS3231 usando ESP32.
 * 
 * Este programa inicializa dos buses I2C independientes para comunicarse
 * con dos módulos DS3231 y escribir la misma hora en ambos dispositivos,
 * logrando que queden sincronizados.
 * 
 * La hora utilizada se basa en la fecha y hora de compilación del sistema,
 * con una compensación ajustable para corregir el retardo entre la carga
 * del programa y su ejecución.
 * 
 * Además, se realiza una verificación periódica para comprobar la diferencia
 * de tiempo entre ambos RTC.
 * 
 * @note Es necesario ajustar el valor de UPLOAD_DELAY_SEC para mejorar la precisión.
 * @warning Ambos RTC deben estar correctamente conectados a sus respectivos buses I2C.
 */

#include <Wire.h>
#include "RTClib.h"

// ============================
// CONFIGURACIÓN DE PINES
// ============================

#define SDA1 21 /**< Pin SDA para RTC1 */
#define SCL1 22 /**< Pin SCL para RTC1 */

#define SDA2 25 /**< Pin SDA para RTC2 */
#define SCL2 26 /**< Pin SCL para RTC2 */

// ============================
// AJUSTE IMPORTANTE
// ============================

/**
 * @brief Compensación del retardo de carga del programa.
 * 
 * Este valor corrige el desfase entre la hora de compilación
 * y el momento real en que el código se ejecuta en el ESP32.
 */
#define UPLOAD_DELAY_SEC 27

// ============================
// OBJETOS I2C Y RTC
// ============================

TwoWire I2C_RTC1 = TwoWire(0); /**< Bus I2C para RTC1 */
TwoWire I2C_RTC2 = TwoWire(1); /**< Bus I2C para RTC2 */

RTC_DS3231 rtc1; /**< Objeto RTC para el primer módulo */
RTC_DS3231 rtc2; /**< Objeto RTC para el segundo módulo */

// ============================
// SETUP
// ============================

/**
 * @brief Inicializa los RTC y realiza la sincronización.
 * 
 * - Configura la comunicación serial.
 * - Inicializa dos buses I2C independientes.
 * - Verifica la conexión de ambos RTC.
 * - Sincroniza la escritura al inicio exacto de un segundo.
 * - Ajusta ambos RTC con la misma hora compensada.
 * - Muestra una verificación inicial.
 */
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== CONFIGURADOR DE RTCs DS3231 ===");

    // Inicializar buses I2C
    I2C_RTC1.begin(SDA1, SCL1, 100000);
    I2C_RTC2.begin(SDA2, SCL2, 100000);

    // Inicializar RTCs
    if (!rtc1.begin(&I2C_RTC1)) {
        Serial.println("Error: RTC1 no detectado");
        while (1); /**< Detiene ejecución si falla */
    }

    if (!rtc2.begin(&I2C_RTC2)) {
        Serial.println("Error: RTC2 no detectado");
        while (1); /**< Detiene ejecución si falla */
    }

    Serial.println("Ambos RTC detectados");

    // ============================
    // SINCRONIZACIÓN AL SEGUNDO
    // ============================

    Serial.println("Sincronizando al inicio de segundo...");

    uint32_t t0 = millis();

    /**
     * @brief Espera hasta el inicio exacto del siguiente segundo.
     */
    while ((millis() - t0) < (1000 - (t0 % 1000)));

    // ============================
    // OBTENER Y AJUSTAR HORA
    // ============================

    /**
     * @brief Obtiene la fecha y hora de compilación.
     */
    DateTime now = DateTime(F(__DATE__), F(__TIME__));

    /**
     * @brief Aplica compensación por retardo de carga.
     */
    DateTime adjusted = now + TimeSpan(0, 0, 0, UPLOAD_DELAY_SEC);

    // Escribir misma hora en ambos RTC
    rtc1.adjust(adjusted);
    rtc2.adjust(adjusted);

    Serial.println("RTCs configurados con compensación");

    // ============================
    // VERIFICACIÓN INICIAL
    // ============================

    Serial.println("\n--- Verificación inicial ---");
    printTime("RTC1", rtc1.now());
    printTime("RTC2", rtc2.now());

    Serial.println("\nMonitoreando cada 3 segundos...\n");
}

// ============================
// LOOP
// ============================

/**
 * @brief Monitorea continuamente la sincronización de los RTC.
 * 
 * - Lee la hora de ambos módulos.
 * - Imprime los valores.
 * - Calcula la diferencia en segundos.
 */
void loop() {
    DateTime t1 = rtc1.now();
    DateTime t2 = rtc2.now();

    printTime("RTC1", t1);
    printTime("RTC2", t2);

    int diff = t1.unixtime() - t2.unixtime(); /**< Diferencia en segundos */

    Serial.print("Diferencia (s): ");
    Serial.println(diff);

    Serial.println("----------------------");

    delay(3000); /**< Intervalo de monitoreo */
}

// ============================
// FUNCIÓN DE IMPRESIÓN
// ============================

/**
 * @brief Imprime una marca de tiempo en formato legible.
 * 
 * @param label Nombre del RTC (ej: "RTC1")
 * @param t Objeto DateTime con la hora a imprimir
 */
void printTime(const char* label, DateTime t) {
    Serial.print(label);
    Serial.print(": ");

    Serial.print(t.year());
    Serial.print("/");
    Serial.print(t.month());
    Serial.print("/");
    Serial.print(t.day());
    Serial.print(" ");

    Serial.print(t.hour());
    Serial.print(":");
    Serial.print(t.minute());
    Serial.print(":");
    Serial.println(t.second());
}