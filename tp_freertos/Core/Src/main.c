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
#include "cmsis_os.h"
#include "spi.h"
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
#include <string.h>





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

TaskHandle_t handle_Task_bidon;
TaskHandle_t h_task_led = NULL;
TaskHandle_t h_task_give = NULL;
TaskHandle_t h_task_take = NULL;
QueueHandle_t xQueue1,xQueue2;
QueueHandle_t q_wait = NULL;
SemaphoreHandle_t sem1;
SemaphoreHandle_t sem2;
SemaphoreHandle_t sem3;
char pcWriteBuffer[400];
int * buffer;
int wait=100;

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
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

// 1.2 changement de l’état de la LED toutes les 100ms
void task_blink_led(void * unused)
{
	while (1)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		printf ("LED State is changed\r\n");
		vTaskDelay(100);
	}
}
/*1.2 Semaphores , taskGive et taskTake, avec deux priorités differentes.TaskGive donne un sémaphore toutes les 100ms */
void taskGive(void * unused)
{
	while (1)
	{
		printf("taskGive: donne sem: wait=%d\r\n",wait);
		xSemaphoreGive(sem1);
		printf("taskGive: sem donné: wait=%d\r\n",wait);
		vTaskDelay(wait);
		wait+=100;
	}
}
void taskTake(void * unused)
{
	while (1)
	{
		printf("taskTake: prend sem: wait=%d\r\n",wait);
		if (xSemaphoreTake(sem1, 1000)==pdFALSE)
		{
			printf("timeout, reset\r\n");
			NVIC_SystemReset();
		}
		printf("taskTake: sem pris: wait=%d\r\n",wait);
		//vTaskDelay(100);
	}
}

/* 1.3 Notificatons: le même fonctionnement en utilisant des task notifications à la place des sémaphores */
//void taskGive(void * unused)
//{
//	while (1)
//	{
//		printf("taskGive: donne sem: wait=%d\r\n",wait);
//		xTaskNotifyGive(h_task_take);
//		printf("taskGive: sem donné: wait=%d\r\n",wait);
//		vTaskDelay(wait);
//		wait+=100;
//
//	}
//}
//void taskTake(void * unused)
//{
//	int wait_take;
//	while (1)
//	{
//		printf("taskTake: prend sem: wait=%d\r\n",wait);
//		if (ulTaskNotifyTake(pdTRUE, 1000)==pdFALSE)
//		{
//			printf("timeout, reset\r\n");
//			NVIC_SystemReset();
//		}
//		printf("taskTake: sem pris: wait=%d\r\n",wait);
//		//vTaskDelay(100);
//	}
//}

/* 1.4 Queue : Envoie de la valeur du Timer */
//void taskGive(void * unused)
//{
//	int wait_give=100;
//	while (1)
//	{
//		printf("taskGive: send queue: wait=%d\r\n",wait_give);
//		if(xQueueSend(q_wait, &wait_give,portMAX_DELAY)==errQUEUE_FULL)
//		{
//			printf("Queue is full\r\n");
//		}
//		printf("taskGive: queue sent: wait=%d\r\n",wait_give);
//		vTaskDelay(wait_give);
//		wait_give+=100;
//
//
//	}
//}
//void taskTake(void * unused)
//{
//	int wait_take=0;
//	while (1)
//	{
//		printf("taskTake: receive queue: wait=%d\r\n",wait_take);
//		if(xQueueReceive(q_wait, (void *)&wait_take,1000)==pdFALSE)
//		{
//			printf("timeout, reset\r\n");
//			NVIC_SystemReset();
//		}
//		printf("taskTake: queue received: wait=%d\r\n",wait_take);
//		//vTaskDelay(100);
//	}
//}

int fonction(int argc, char ** argv) {
	printf("Fonction bidon\r\n");
	printf("Nous avons recu %d arguments \r\n",argc);

	for(int i=0;i<argc;i++)
	{
		printf("\tArgument numero %d : %s\r\n",i,argv[i]);
	}

	return 0;
}


/* une fonction led(), appelable depuis le shell*/
int led(int argc , char ** argv){
	printf("Allumer une Led \r\n");
	int valeur ;
	valeur = atoi(argv[1]);
	xQueueSend(xQueue1,&valeur,portMAX_DELAY);
	xSemaphoreGive(sem2);
	return 0;
}


/*Le clignotement de la LED s’effectue dans la tâche vTaskLed*/
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
/*fonction spam qui affiche du texte dans la liaison série*/
 int spam(int argc , char ** argv){
	int j=0 ;
	char mess[100]; // Déclaration d'une variable mess pour stocker le message
	for (j=0;j<strlen(argv[1])+1;j++){  // Utilisation de <= pour inclure le caractère nul
		mess[j]=argv[1][j];
	}
	xQueueSend(xQueue2,&mess,portMAX_DELAY);//Envoi du message à la file d'attente
	xSemaphoreGive(sem3);// Libération du sémaphore
	return 0;
}
 /*L'affichage du message s’effectue dans la tâche vTaskLed*/
void vTaskSpam(void *p){
	char message[10];
	int j=0;
	while(1){
		if(uxQueueMessagesWaiting(xQueue2)!=0){
			for(j=0;j<10;j++){
				xQueueReceive(xQueue2,&message,portMAX_DELAY);  //reçoit boit aux lettres xQueue2 avec les messages.
				printf("%s \r\n",&message[j]);
			}
		}
		else{
				xSemaphoreTake(sem3,portMAX_DELAY);  // changer la priorité.
					}
	 }
}
/*tache bidon infinie*/
void Task_bidon( void *pvParameters)
{
	for(;;) {
	}

}
/*dépassement de la taille de la pile: fonction appelée automatiquement par FreeRTOS*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	for (;;) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
				vTaskDelay(100);
	    }
}
void vATaskX(void *pvParameters)
{
	// Création d'un tableau de taille importante pour provoquer un dépassement de pile
	char bigArray[1000];
	for(;;)
	{
		printf("Une tache est ici\r\n");

	}

}

/*afficher le taux d’utilisation du CPU : démarrage du timer*/
//void configureTimerForRunTimeStats(void)
//{
//	// Start timer 2
//	HAL_TIM_Base_Start(&htim2);
//}
///*on récupére la valeur du timer*/
//unsigned long getRunTimeCounterValue(void)
//{
//	return (unsigned long)__HAL_TIM_GET_COUNTER(&htim2);
//}
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//
//	if (htim->Instance == TIM2) {
//		HAL_IncTick();
//		FreeRTOSRunTimeTicks++;
//	}
//
//}
int statut (int argc, char ** argv){
	printf("Etat d'utilisation du CPU :\r\n");
	vTaskGetRunTimeStats(pcWriteBuffer); // Récupérer les statistiques de temps d'exécution des tâches et les stocker dans le buffer

	printf("%s\r\n", pcWriteBuffer);   // Afficher les statistiques de temps d'exécution des tâches

	vTaskList(pcWriteBuffer); // Récupérer la liste des tâches et les stocker dans le buffer

	printf("%s\r\n", pcWriteBuffer);// Afficher la liste des tâches
	return 0;
}
int testDevId (int argc, char ** argv){
	uint8_t address = 0x00;
	uint8_t p_data;
	uint16_t size = 1;
	HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, GPIO_PIN_RESET); //ChipSelect a 0
	HAL_SPI_Transmit(&hspi2, &address, 1, HAL_MAX_DELAY); //On transmet un msg a l'adresse 0x00
	HAL_SPI_Receive(&hspi2, &p_data, size, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin, GPIO_PIN_SET); // chipSelect a 1
	printf("%X \r\n", p_data); // On printf la valeur renvoyé, rst value
	return 0;
}


void vTaskShell(void * p) {
	shell_init();
	shell_add('f', fonction, "Une fonction inutile");
	shell_add('l', led, "Allumer Led");
	shell_add('s',spam, "Afficher message");
	shell_add('c', statut, "Statut du CPU");
	shell_add('t', testDevId, "valeur DevId");
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
		//TaskHandle_t xHandle1 = NULL;
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
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
	sem1 = xSemaphoreCreateBinary();
	sem2 = xSemaphoreCreateBinary();
	xQueue1= xQueueCreate( 10, sizeof( unsigned long ) );
	xQueue2=xQueueCreate(10 ,sizeof(unsigned long));
		//xSemaphoreGive(sem1);
	/* Création d'une tache Led pour le clignotement de la led avec une gestion d'erreur si le sémaphore n’est pas acquis */
	if (xTaskCreate(task_blink_led, "Blink LED", TASK_LED_STACK_DEPTH, NULL, TASK_LED_PRIORITY, &h_task_led) != pdPASS)
	{
		printf("Error creating task shell\r\n");
		Error_Handler();
	}
	/* Création de la tache SHELL  */
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

		/* Création d'une tache bidon pour vérifier les limites de la pile */
		for (int z = 0; z < 400; z++) {
			xReturned = xTaskCreate(Task_bidon, "Task_bidon", 256, NULL, 9, &handle_Task_bidon);
				 if (xReturned == pdPASS){
						printf("la tache %d se cree\r\n", z);
					}
					else
					{
						printf("Error, la tache n'est pas cree\r\n");
						Error_Handler();
					}
				}
		/* Création des taches give et take */

		q_wait = xQueueCreate(Q_WAIT_LENGTH, Q_WAIT_SIZE);
			if (xTaskCreate(taskGive, "taskGive", TASK_LED_STACK_DEPTH, NULL, 2, &h_task_give) != pdPASS)
				{
					printf("Error creating task shell\r\n");
					Error_Handler();
				}
			if (xTaskCreate(taskTake, "taskTake", TASK_LED_STACK_DEPTH, NULL, 3, &h_task_take) != pdPASS)
				{
					printf("Error creating task shell\r\n");
					Error_Handler();
				}
			/* Création de la tache X pour Overflow */
			xTaskCreate(vATaskX, "TaskX", 100, NULL, 1, NULL);
			/* Création de la tache led */
		  //xTaskCreate(vTaskLed,"led",STACK_SIZE,(void * )NULL,3	,&xHandle1);

			vTaskStartScheduler();// BOUCLE infinie


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
