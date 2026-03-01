/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h" // for sprintf
#include "ADS1115.h"
#include "ssd1306.h"
#include "fonts.h"
#include "i2c-lcd.h"
#include "dht11.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// ADS ACS Config
#define NUM_SAMPLES                  10
#define GAIN_ONE_FACTOR              0.000125f
#define GAIN_SIXTEEN_FACTOR          0.0000078125f
#define EMP_ADC1_ZERO_CURRENT_OFFSET 2900
#define EMP_ADC2_ZERO_CURRENT_OFFSET 1300
#define ACS712_SENSITIVITY           0.185f
#define EMP_V_A2_IN_CONSTANT         0.091160986f
#define EMP_V_A2_OUT_CONSTANT        0.091669765f
#define PWM_INCREMENT                50 // when using button (open loop)

// MPPT Config for closed loop feedback
// using PWM resolution of 1000
#define MPPT_DELTA_DUTY     10
#define MPPT_DUTY_MIN       100
#define MPPT_DUTY_MAX       900

// MPPT state variables
float prev_p_in      = 0.0f;
float prev_v_in      = 0.0f;
float prev_duty_q1   = 500;          // depends on the battery used for charging.

// Fan control
#define FAN_DELTA_DUTY     50
#define FAN_MIN 0
#define FAN_MAX 500

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim9;
DMA_HandleTypeDef hdma_tim1_ch1;
DMA_HandleTypeDef hdma_tim1_ch2;

/* USER CODE BEGIN PV */
ADS1xx5_I2C i2c1;
ADS1xx5_I2C i2c2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM9_Init(void);
/* USER CODE BEGIN PFP */
void delay_us(uint16_t us);
void check_button(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t duty_q1 = 50;
float d_q1_percent = 0;

// ADC raw accumulator variables (need to hold sum of NUM_SAMPLES readings
int32_t raw_adc1_a0 = 0; // measures input current
int32_t raw_adc1_a2 = 0; // measures input voltage

int32_t raw_adc2_a0 = 0; // measures output current
int32_t raw_adc2_a2 = 0; // measures output voltage

// Mean ADC values (averaged readings)
int16_t mean_adc1[3]; // ADC readings are signed, max value 32,767
int16_t raw_adc1[3]; // raw readings

int16_t mean_adc2[3]; // ADC readings are signed, max value 32,767
int16_t raw_adc2[3]; // raw readings

// voltage calculations (floating point for precision)
float mean_adc1_A0_v, mean_adc1_A2_v, mean_v_in, mean_i_in, mean_p_in;
float mean_adc2_A0_v, mean_adc2_A2_v, mean_v_out, mean_i_out, mean_p_out;

// Display variables as strings
char buf_p_in[6], buf_i_in[5], buf_v_in[6];
char buf_p_out[6], buf_i_out[5], buf_v_out[6], buf_duty[5];

int buttonState = 1, buttonPressed = 0;

DHT11_DataTypeDef DHT11_Data;
float Temperature, Humidity, error;

uint32_t fanAmplitude = 500;
uint8_t Temp_set = 25;


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *) &duty_q1, 1); // for switch
  //HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1); // for synchronous switch

  /* Fan Control */
  HAL_TIM_Base_Start(&htim9);
  HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t *) &fanAmplitude, 1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);

  // SD pin IR2104.
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);


//  // LCD Configuration
  lcd_init();
  lcd_clear();
//  lcd_init();
//  lcd_clear();
//  lcd_put_cur(0, 0);
//  lcd_send_string("YELLO!");
//  HAL_Delay(10000);


  // ADS Configuration
  // ADC 1
  ADS1115(&i2c1, &hi2c1, ADS_ADDR_VDD);


  // ADC 2
  ADS1115(&i2c2, &hi2c1, ADS_ADDR_GND);



  // OLED Configuration
  SSD1306_Init();



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  check_button(); // if button pressed increment duty_q1 by 50.

	  /* Fan control */


	  raw_adc1_a0 = 0;
	  raw_adc1_a2 = 0;
	  raw_adc2_a0 = 0;
	  raw_adc2_a2 = 0;

	  for (int i = 0; i < NUM_SAMPLES; i++){
		  // raw current measurements
		  raw_adc1[0] = ADSreadADC_Differential_0_1(&i2c1, GAIN_SIXTEEN);
		  raw_adc2[0] = ADSreadADC_Differential_0_1(&i2c2, GAIN_SIXTEEN);

//		  if (raw_adc1[0] < 0){
//			  raw_adc1[0] = 0;
//		  }
//
//		  if (raw_adc2[0] < 0){
//			  raw_adc2[0] = 0;
//		  }

		  raw_adc1_a0 += raw_adc1[0];
		  raw_adc2_a0 += raw_adc2[0];



		  // raw voltage measurements
		  raw_adc1[2] = ADSreadADC_SingleEnded(&i2c1, 2, GAIN_ONE);
		  raw_adc2[2] = ADSreadADC_SingleEnded(&i2c2, 2, GAIN_ONE);
//		  if (raw_adc1[2] < 0){
//			  raw_adc1[2] = 0;
//		  }
//		  if (raw_adc2[2] < 0){
//			  raw_adc2[2] = 0;
//		  }
		  raw_adc1_a2 += raw_adc1[2];
		  raw_adc2_a2 += raw_adc2[2];
	  }

	  // mean current measurements
	  mean_adc1[0] = raw_adc1_a0 / NUM_SAMPLES;
	  mean_adc2[0] = raw_adc2_a0 / NUM_SAMPLES;

	  mean_adc1_A0_v = (mean_adc1[0] - EMP_ADC1_ZERO_CURRENT_OFFSET) * GAIN_SIXTEEN_FACTOR;
	  mean_adc2_A0_v = (mean_adc2[0] - EMP_ADC2_ZERO_CURRENT_OFFSET) * GAIN_SIXTEEN_FACTOR;
	  mean_i_in = (mean_adc1_A0_v) / ACS712_SENSITIVITY;
	  mean_i_out = (mean_adc2_A0_v) / ACS712_SENSITIVITY;

	  // mean voltage measurements
	  mean_adc1[2] = raw_adc1_a2 / NUM_SAMPLES;
	  mean_adc2[2] = raw_adc2_a2 / NUM_SAMPLES;

	  mean_adc1_A2_v = mean_adc1[2] * GAIN_ONE_FACTOR;
	  mean_adc2_A2_v = mean_adc2[2] * GAIN_ONE_FACTOR;

	  mean_v_in = mean_adc1_A2_v / EMP_V_A2_IN_CONSTANT;
	  mean_v_out = mean_adc2_A2_v / EMP_V_A2_OUT_CONSTANT;

	  //// BIDIRECTIONAL?! //////
//	  if (mean_i_in < 0){
//	  	  mean_i_in = 0;
//	  }
//
//	  if (mean_i_out < 0){
//	  	  mean_i_out = 0;
//	  }
//
//	  if (mean_v_in < 0){
//		  mean_v_in = 0;
//	  }
//
//	  if (mean_v_out < 0){
//		  mean_v_out = 0;
//	  }

	  // Calculate (instantaneous real) power
	  mean_p_in = mean_v_in * mean_i_in;
	  mean_p_out = mean_v_out * mean_i_out;

	  /* Display in OLEDs and LCDs */
	  sprintf(buf_p_in, "%.2f", mean_p_in);
	  sprintf(buf_i_in, "%.2f", mean_i_in);
	  sprintf(buf_v_in, "%.2f", mean_v_in);

	  sprintf(buf_p_out, "%.2f", mean_p_out);
	  sprintf(buf_i_out, "%.2f", mean_i_out);
	  sprintf(buf_v_out, "%.2f", mean_v_out);

	  d_q1_percent = (float) duty_q1 / 1000 * 100;
	  sprintf(buf_duty, "%.1f", d_q1_percent);

	  // display IN variables in SSD
	  SSD1306_GotoXY(5, 0);
	  SSD1306_Puts("P:", &Font_11x18, 1);
	  SSD1306_Puts (buf_p_in, &Font_11x18, 1);
	  SSD1306_Puts (" W", &Font_11x18, 1);


	  SSD1306_GotoXY(5, 20);
	  SSD1306_Puts("I:", &Font_11x18, 1);
	  SSD1306_Puts (buf_i_in, &Font_11x18, 1);
	  SSD1306_Puts (" A", &Font_11x18, 1);

	  SSD1306_GotoXY(5, 40);
	  SSD1306_Puts("V:", &Font_11x18, 1);
	  SSD1306_Puts (buf_v_in, &Font_11x18, 1);
	  SSD1306_Puts (" V", &Font_11x18, 1);
	  SSD1306_UpdateScreen();

	  // display OUT variables in LCD

	  lcd_put_cur(0,0);
	  lcd_send_string("P:");
	  lcd_send_string(buf_p_out);
	  lcd_send_string("W ");

	  lcd_send_string("I:");
	  lcd_send_string(buf_i_out);
	  lcd_send_string("A");

	  lcd_put_cur(1,0);
	  lcd_send_string("V:");
	  lcd_send_string(buf_v_out);
	  lcd_send_string("V ");

	  lcd_send_string("d:");
	  lcd_send_string(buf_duty);
	  lcd_send_string("%");

	  DHT11_GetData(&DHT11_Data);
	  Temperature = DHT11_Data.Temperature;
	  Humidity = DHT11_Data.Humidity;
	  error = Temperature - Temp_set;

	  if (error > 0)
	      fanAmplitude += DELTA_DUTY;
	  else if (error < 0)
	      fanAmplitude -= DELTA_DUTY;

	  // Clamp
	  if (fanAmplitude > FAN_MAX) fanAmplitude = FAN_MAX;
	  if (fanAmplitude < FAN_MIN) fanAmplitude = FAN_MIN;

	  /* MPPT Algorithm a simple perturb and observe */
	  // compute power
	  mean_p_in  = mean_v_in  * mean_i_in;
	  mean_p_out = mean_v_out * mean_i_out;

	  // P&O MPPT
	  float delta_p = mean_p_in - prev_p_in;

	  if (delta_p > 0.0f)
	      duty_q1 += MPPT_DELTA_DUTY;
	  else if (delta_p < 0.0f)
	      duty_q1 -= MPPT_DELTA_DUTY;

	  // Clamp
	  if (duty_q1 > MPPT_DUTY_MAX) duty_q1 = MPPT_DUTY_MAX;
	  if (duty_q1 < MPPT_DUTY_MIN) duty_q1 = MPPT_DUTY_MIN;

	  // Store previous
	  prev_p_in = mean_p_in;

	  HAL_Delay(50);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 10-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 180-1;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 65535;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void check_button(void){

	uint32_t last_button_time = 0;
	uint8_t button_was_pressed = 0;

	// Read button state (active low on PC13)
	uint8_t button_is_pressed = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);

	if (button_is_pressed && !button_was_pressed){
		if (HAL_GetTick() - last_button_time > 500){
			// debounce time passed, valid press
			if (duty_q1 <= 900){
				duty_q1 += PWM_INCREMENT;
			}
			else {
				duty_q1 = 100;
			}

			last_button_time = HAL_GetTick();
		}
	}
	button_was_pressed = button_is_pressed;
}


// delay function
void delay_us(uint16_t us){
    /* Since maximum ARR = 65,535, each tick at 90MHz will take 1 / 90/(PSC + 1) us
     * the MAX_DELAY = 65us per cycle.
     * For delays > 65us, we break it into multiple cycles.
     */

    const uint16_t MAX_DELAY = 65; // Maximum delay per cycle in microseconds

    while (us > MAX_DELAY) {
        __HAL_TIM_SET_COUNTER(&htim1, 0);
        while (__HAL_TIM_GET_COUNTER(&htim1) < MAX_DELAY);
        us -= MAX_DELAY;
    }

    // Handle remaining delay
    if (us > 0) {
        __HAL_TIM_SET_COUNTER(&htim1, 0);
        while (__HAL_TIM_GET_COUNTER(&htim1) < us);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
