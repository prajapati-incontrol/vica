/*
 * dht11.c
 *
 *  Created on: Jan 26, 2026
 *      Author: sohamprajapati
 */
#include "dht11.h"
#include "stm32f4xx_hal.h"

#define timer htim9
extern TIM_HandleTypeDef htim9;

#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_4

uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM; uint8_t Presence = 0;

static void delay_us(uint16_t us){
	// STATIC: the function shall be used in this file only.
	// INLINE: useful for optimization, and fast running. It basically says to paste the function
	// content when callign this function.
	__HAL_TIM_SET_COUNTER(&timer,0);
	while(__HAL_TIM_GET_COUNTER(&timer) < us);
}


void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}


void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void DHT11_Start(void){
	Set_Pin_Output(DHT11_PORT, DHT11_PIN);
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 0);
	delay_us(18000);
	HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 1);
	delay_us(20);
	Set_Pin_Input(DHT11_PORT, DHT11_PIN);
}

uint8_t DHT11_Check_Response(void){
	uint8_t Response = 0;
	delay_us(40);
	if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))){
		delay_us(80);
		if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) Response = 1;
		else Response = -1;
	}

	while ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)));
	return Response;
}

uint8_t DHT11_Read(void){ // just reads 1 byte only

	uint8_t i, j;
	for (j=0; j<8; j++){
		// wait for pin to go high
		while(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)));

		// now data starts getting transmitted from DHT11
		delay_us(40);

		// if pin status after 40us is HIGH => 1 (pulse = 70us) else 0 (pulse 26-28us)
		if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))){ // write 0
			i &= ~(1<<(7 - j)); // write 0 starting from left (MSB)
		}
		else i|= (1<<(7-j)); // write 1
		while ((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))); // every bit of data starts with 50 us LOW
	}
	return i;
}

void DHT11_GetData (DHT11_DataTypeDef *DHT11_Data){

	DHT11_Start();
	Presence = DHT11_Check_Response();
	Rh_byte1 = DHT11_Read();
	Rh_byte2 = DHT11_Read();
	Temp_byte1 = DHT11_Read();
	Temp_byte2 = DHT11_Read();
	SUM = DHT11_Read(); // not commonly known definition of checksum

	if (SUM == (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2)){
		DHT11_Data->Temperature = Temp_byte1;
		DHT11_Data->Humidity = Rh_byte1;
	}
}



