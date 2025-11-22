#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "freertos_tasks.h"
#include "bmsParams.h"
#include "utils.h"

/*User Definitions*/
#define STACK_SIZE 500
#define MAX_QUEUE_LEN sizeof(bmsParams) << 1
#define QUEUE_DATA_SIZE sizeof(rxByte)

/* Structures that will hold the TCB of the task being created. */
StaticTask_t xTransmitBuffer;
StaticTask_t xReceiveBuffer;

/* Buffers that the task being created will use as its stack. Note this is
   an array of StackType_t variables. The size of StackType_t is dependent o
   the RTOS port. */
StackType_t xTransmitStack[ STACK_SIZE ];
StackType_t xReceiveStack[ STACK_SIZE ];

//task handles
TaskHandle_t vTxTaskHandle;
TaskHandle_t vRxTaskHandle;

//queue
QueueHandle_t uartRxQueue;
uint8_t xQueueStack[QUEUE_DATA_SIZE * MAX_QUEUE_LEN];
StaticQueue_t xQueueBuffer;
uint8_t			rxByte;

typedef enum {
    WAIT_FOR_HEADER,
    WAIT_FOR_PAYLOAD
} UartState_t;

void initFreeRTOS()
{
	//Initialize tasks
	const char *TxName = "TxTask";
	xTaskCreateStatic(vUartTransmitTask, TxName, STACK_SIZE, (void *) 1, 3, xTransmitStack, &xTransmitBuffer);
	vTxTaskHandle = xTaskGetHandle(TxName);

	const char *RxName = "RxTask";
	xTaskCreateStatic(vUartReceiveTask, RxName, STACK_SIZE, (void *) 1, 3, xReceiveStack, &xReceiveBuffer);
	vRxTaskHandle = xTaskGetHandle(RxName);

	//initialize queue
	uartRxQueue = xQueueCreateStatic( MAX_QUEUE_LEN, QUEUE_DATA_SIZE, xQueueStack, &xQueueBuffer);

	//Initialize transfers
	HAL_UART_Receive_IT(&huart1, &rxByte, 1);
	//start scheduler
	vTaskStartScheduler();
}


/*---------- TASKS ---------------*/
void vUartTransmitTask(void *pvParameters)
{
	for(;;) {
		//Attempt to toggle DMA transfer
		if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)&bmsParams, sizeof(bmsParams)) == HAL_OK) {
			// Wait for the DMA to complete
			// Blocks the task until the TX callback calls vTaskNotifyGiveFromISR
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}

		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}

void vUartReceiveTask(void *pvParameters)
{
	uint8_t last_read_byte;
	BMS_Params_t possible;
	possible.header = configBMS_HEADER;
	UartState_t state = WAIT_FOR_HEADER;
	uint8_t *index;

	for (;;) {
		// Wait for a byte from the queue
		if (xQueueReceive(uartRxQueue, &last_read_byte, pdMS_TO_TICKS(10)) == pdTRUE) {

			switch(state) {

				case WAIT_FOR_HEADER:
					if (last_read_byte == 0xA5) {
						// Example action for 0xA5
						HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
					} else if (last_read_byte == configBMS_HEADER) {
						index = (uint8_t*)&possible + 1;
						state = WAIT_FOR_PAYLOAD;
					}
					break;

				case WAIT_FOR_PAYLOAD:
					*(index++) = last_read_byte;

					if (index == (uint8_t*)(&possible + 1)) {
						processPacket(&possible, &bmsParams, sizeof(bmsParams)-configBMS_CHECKSUM_PASSES);
						state = WAIT_FOR_HEADER;  // Back to waiting for header
					}
					break;

				default:
					state = WAIT_FOR_HEADER;
					break;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(1));
	}
}

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
