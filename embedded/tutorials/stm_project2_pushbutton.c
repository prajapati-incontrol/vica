#include "main.h"
#include <stdio.h>

void SystemClock_Config(void);



int main(void)
{
  HAL_Init();
  SystemClock_Config();

  // reset and  control clock - AHB enable
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

  // Setup control registers for the LED output: Port A Pin 5
  // mode
  GPIOA->MODER |= GPIO_MODER_MODER5_0;
  GPIOA->MODER &= ~GPIO_MODER_MODER5_1;

  // type
  GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;

  //speed: low speed
  GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR5_0;
  GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR5_1;
  // if high speed: 11
  //GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5_0 | GPIO_OSPEEDER_OSPEEDR5_1;


  // pull up pull down
  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5_0;
  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5_1;



  // Setup control registers for the push button input: Port C Pin 13
  // mode
  GPIOC->MODER &= ~GPIO_MODER_MODER13; // both zeros

  // pull up pull down: internally pull down to the user button
  // clear both bits
  GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD13;
  GPIOC->PUPDR |= GPIO_PUPDR_PUPD13_0; // 01


  while (1)
  {
	  // turn on the LED when the button is pressed.
	  // GPIOC->IDR has 0000 0000 0000 0000 16 bits each for an Input pin. Pin HIGH, bit 1 and vice versa.
	  // GPIOC_IDR_IDR1 0000 0000 0000 0010 is a mask.
	  if (GPIOC->IDR & GPIO_IDR_ID13){ // Initially, BUTTON NOT PRESSED IS CONNECTED TO PC13.
		  // BUTTON NOT PRESSED
		  //GPIOA->BSRR |= GPIO_BSRR_BS_5;

	  }
	  else {
		  // BUTTON PRESSED.
          //GPIOA->BSRR |= GPIO_BSRR_BR_5;
	  }

	  // when button is pressed (IDR)
	  // Turn on LED (BSRR)

	  // when button is not pressed
	  // Turn off the LED (BRR)

  }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
