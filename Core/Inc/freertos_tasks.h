/*
 * freertos_tasks.h
 *
 *  Created on: Oct 31, 2025
 *      Author: aiden
 */

#ifndef INC_FREERTOS_TASKS_H_
#define INC_FREERTOS_TASKS_H_

#include "FreeRTOS.h"
#include "queue.h"

/*Function Prototypes*/
void initFreeRTOS(void);
void vUartTransmitTask(void*);
void vUartReceiveTask(void*);

//task handles
extern TaskHandle_t vTxTaskHandle;
extern TaskHandle_t vRxTaskHandle;

extern uint8_t rxByte;
extern QueueHandle_t uartRxQueue;

#endif /* INC_FREERTOS_TASKS_H_ */
