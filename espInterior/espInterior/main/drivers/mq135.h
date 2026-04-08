#ifndef MQ135_H
#define MQ135_H

void mq135_init(void);

// 🔥 nuevas funciones
int mq135_read_raw(void);
float mq135_read_voltage(void);

// usada por el sistema
float mq135_read(void);

#endif