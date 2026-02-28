/*
 * dht11.h
 *
 *  Created on: Jan 26, 2026
 *      Author: sohamprajapati
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "stm32f4xx_hal.h"


typedef struct {
	float Temperature;
	float Humidity;
} DHT11_DataTypeDef;



//void delay_us(uint16_t us);
void DHT11_GetData (DHT11_DataTypeDef *DHT_Data);


#endif /* INC_DHT11_H_ */
