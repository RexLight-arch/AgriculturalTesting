#ifndef __SGP30_H_
#define __SGP30_H_

#include "stdbool.h"
#include "string.h"

#include "main.h"
#include "usart.h"
#include "gpio.h"

HAL_StatusTypeDef sgp30_init(void);
HAL_StatusTypeDef sgp30_sample(float *co2, float *tvoc);

#endif 
