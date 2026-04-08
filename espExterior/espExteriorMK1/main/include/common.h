#pragma once

typedef enum {
    EVENTO_INICIO_PROCESO
} evento_tipo_t;

typedef struct {
    evento_tipo_t tipo;
} evento_t;

typedef struct {
    float temperatura;
    float humedad;

    uint8_t hora;
    uint8_t minuto;
    uint8_t segundo;

    uint8_t dia;
    uint8_t mes;
    uint16_t anio;

} datos_ambiente_t;