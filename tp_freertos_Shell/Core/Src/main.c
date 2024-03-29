/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"
#include "FreeRTOS.h"
#include <queue.h>
#include <semphr.h>

QueueHandle_t xQueue1,xQueue2;
SemaphoreHandle_t sem1;
SemaphoreHandle_t sem2;
SemaphoreHandle_t sem3;

uint8_t Data[]={0,0,0};

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
#define STACK_SIZE 1000
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fonction(int argc, char ** argv) {
	printf("Fonction bidon\r\n");
	printf("Nous avons recu %d arguments \r\n",argc);

	for(int i=0;i<argc;i++)
	{
		printf("\tArgument numero %d : %s\r\n",i,argv[i]);
	}

	return 0;
}

int led(int argc , char ** argv){
	printf("Allumer une Led \r\n");
	int valeur ;
	valeur = atoi(argv[1]);
	xQueueSend(xQueue1,&valeur,portMAX_DELAY);
	xSemaphoreGive(sem2);
	return 0;
}



void vTaskLed(void *p){
	int Data =0;
	while(1){
		if(uxQueueMessagesWaiting(xQueue1)==0){ //il n'y a pas de message en attente
			if(Data!=0){ // si Data est différent de 0 ça veut dire que la durée de clignotement != 0
					HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_1); // on change l'état de Pin 1 ( LED ) avec le delay de Data
					vTaskDelay(Data);
			}
			else{
				xSemaphoreTake(sem2,portMAX_DELAY);  // changer la priorité.
			}
		}
		else{
			xQueueReceive(xQueue1,&Data,portMAX_DELAY);  //reçoit boit aux lettres xQueue1 avec les données de Data.
			if(Data==0){
				HAL_GPIO_WritePin(GPIOI, GPIO_PIN_1,RESET); // eteindre la Led.
			}
		}
	}
}


void vTaskShell(void * p) {
	shell_init();
	shell_add('f', fonction, "Une fonction inutile");
	shell_add('l', led, "Allumer Led");
	shell_run();
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		/* Prevent unused argument(s) compilation warning */
	  //UNUSED(huart);
		xSemaphoreGiveFromISR(sem1, &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

int main(void)
{
  /* USER CODE BEGIN 1 */
	    BaseType_t xReturned;
		TaskHandle_t xHandle = NULL;
		TaskHandle_t xHandle1 = NULL;
		//TaskHandle_t xHandle2 = NULL;
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	sem1 = xSemaphoreCreateBinary();
		sem2 = xSemaphoreCreateBinary();
		xQueue1= xQueueCreate( 10, sizeof( unsigned long ) );
		xQueue2=xQueueCreate(10 ,sizeof(unsigned long));
		//xSemaphoreGive(sem1);

		xReturned = xTaskCreate(
				vTaskShell,      	/* Function that implements the task. */
				"Shell",         	/* Text name for the task. */
				STACK_SIZE,      	/* Stack size in words, not bytes. */
				( void * ) NULL,    /* Parameter passed into the task. */
				2,					/* Priority at which the task is created. */
				&xHandle );      	/* Used to pass out the created task's handle. */

		if( xReturned == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY ) {
			printf("Task Shell creation error: Could not allocate required memory\r\n");
		}

		xTaskCreate(vTaskLed,"led",STACK_SIZE,(void * )NULL,3	,&xHandle1);
		//xTaskCreate(vTaskSensor,"sensor",STACK_SIZE,(void * )NULL,1	,&xHandle1);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
		while (1)
		{
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
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
