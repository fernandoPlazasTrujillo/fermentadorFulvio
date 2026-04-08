#include <Wire.h>
#include "RTClib.h"

// ============================
// CONFIGURACIÓN DE PINES
// ============================

// RTC 1
#define SDA1 21
#define SCL1 22

// RTC 2
#define SDA2 25
#define SCL2 26

// ============================
// AJUSTE IMPORTANTE
// ============================
// Cambia este valor según tu medición
#define UPLOAD_DELAY_SEC 27   // <-- ESTE ES EL VALOR QUE DEBES CALIBRAR

// ============================
// OBJETOS I2C Y RTC
// ============================

TwoWire I2C_RTC1 = TwoWire(0);
TwoWire I2C_RTC2 = TwoWire(1);

RTC_DS3231 rtc1;
RTC_DS3231 rtc2;

// ============================
// SETUP
// ============================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== CONFIGURADOR DE RTCs DS3231 ===");

    // Inicializar buses I2C
    I2C_RTC1.begin(SDA1, SCL1, 100000);
    I2C_RTC2.begin(SDA2, SCL2, 100000);

    // Inicializar RTCs
    if (!rtc1.begin(&I2C_RTC1)) {
        Serial.println("❌ Error: RTC1 no detectado");
        while (1);
    }

    if (!rtc2.begin(&I2C_RTC2)) {
        Serial.println("❌ Error: RTC2 no detectado");
        while (1);
    }

    Serial.println("✔ Ambos RTC detectados");

    // ============================
    // SINCRONIZACIÓN AL SEGUNDO
    // ============================

    Serial.println("⏳ Sincronizando al inicio de segundo...");

    uint32_t t0 = millis();
    while ((millis() - t0) < (1000 - (t0 % 1000)));

    // ============================
    // OBTENER Y AJUSTAR HORA
    // ============================

    DateTime now = DateTime(F(__DATE__), F(__TIME__));

    // Aplicar compensación por tiempo de carga
    DateTime adjusted = now + TimeSpan(0, 0, 0, UPLOAD_DELAY_SEC);

    rtc1.adjust(adjusted);
    rtc2.adjust(adjusted);

    Serial.println("✔ RTCs configurados con compensación");

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

void loop() {
    DateTime t1 = rtc1.now();
    DateTime t2 = rtc2.now();

    printTime("RTC1", t1);
    printTime("RTC2", t2);

    int diff = t1.unixtime() - t2.unixtime();
    Serial.print("Diferencia (s): ");
    Serial.println(diff);

    Serial.println("----------------------");

    delay(3000);
}

// ============================
// FUNCIÓN DE IMPRESIÓN
// ============================

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