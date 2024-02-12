/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "i2c-lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_6
uint8_t RHI, RHD, TI, TD, SUM;
uint8_t isAlarmActive = 0;
uint32_t pMillis, cMillis;
float Temp = 0;
float RH = 0;
float talarm=24;
float valarm=60;
float fanSpeed = 20.0;

void microDelay (uint16_t delay)
{
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	  while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}

uint8_t DHT11_Start (void)
{
  uint8_t Response = 0;
  GPIO_InitTypeDef GPIO_InitStructPrivate = {0};

  GPIO_InitStructPrivate.Pin = DHT11_PIN;
  GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructPrivate.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // postavljanje nožice kao izlaz
  HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 0);   // postavljanje nožice u nisko stanje
  HAL_Delay(18);   // čekati 18ms
  HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 1);   // postavljanje nožice u visoko stanje
  microDelay (30);   // čekati 30us

  GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructPrivate.Pull = GPIO_PULLUP;

  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // postavljanje nožice kao ulaz
  microDelay (40);
  if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))
  {
    microDelay (80);
    if ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN))) Response = 1;
  }
  pMillis = HAL_GetTick();
  cMillis = HAL_GetTick();
  while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
  {
    cMillis = HAL_GetTick();
  }
  return Response;
}

uint8_t DHT11_Read (void)
{
  uint8_t a,b;
  for (a=0;a<8;a++)
  {
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
    {  // čekati dok nožica ne ode u visoko stanje
      cMillis = HAL_GetTick();
    }
    microDelay (40);   // čekati 40 us
    if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))   // ako je nožica u niskom stanju
      b&= ~(1<<(7-a));
    else
      b|= (1<<(7-a));
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)) && pMillis + 2 > cMillis)
    {  // čekati dok nožica ne dođe u visoko stanje
      cMillis = HAL_GetTick();
    }
  }
  return b;
}


/***********************||LCD||**********************************/

void display_Temp (float Temp)
{
	char str[20]={0};
	lcd_put_cur(0,0);

	sprintf (str, "Temp:%.2f", Temp);
	lcd_send_string(str);
	lcd_send_data('C');
}

void display_RH (float RH)
{
	char str[20]={0};
	lcd_put_cur(1,0);

	sprintf (str, "Vlaga:%.2f", RH);
	lcd_send_string(str);
	lcd_send_data('%');
}
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
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  HAL_TIM_Base_Start_IT(&htim6);


  lcd_init();
  lcd_send_string("pokretanje>>>>");
  HAL_Delay(2000);
  lcd_clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  display_Temp(Temp);
	  display_RH(RH);

	  if(DHT11_Start())
	  {
	 	RHI = DHT11_Read(); // integral vlage
	 	RHD = DHT11_Read(); // decimala vlage
	 	TI = DHT11_Read(); // integral temperature
	 	TD = DHT11_Read(); // decimala temperature
	 	SUM = DHT11_Read(); // provjera zbroja
	 	if (RHI + RHD + TI + TD == SUM)
	 	{

	 	  Temp = (float)TI + (float)(TD/10.0);

	 	  RH = (float)RHI + (float)(RHD/10.0);

	 	}

	 	if (Temp > talarm || RH > valarm)
	 	{
	 	   isAlarmActive = 1;
	 	}
	    else
	 	{
	 	   isAlarmActive = 0;
	 	}

	 	if (isAlarmActive)
	    {
	 	fanSpeed = 80.0; 	// postavljanje brzinee ventilatora na 80%
	 	}
	    else
	 	{
	    fanSpeed = 20.0;    // postavljanje brzinee ventilatora na 20%
	 	}
	 	               // Set PWM duty cycle based on fanSpeed
	 	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, fanSpeed);
	    HAL_Delay(1000);

	  }

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    	if(isAlarmActive==0)
    	{
    		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1); // promjena stanja LED-ice
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

#ifdef  USE_FULL_ASSERT
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
