/**
 * main.c
 *
 * (c) 2015 Tuwuh Sarwoprasojo
 */

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

TimerHandle_t xLEDTimer1 = NULL;
TimerHandle_t xLEDTimer2 = NULL;

static void prvLEDTimer1Callback(TimerHandle_t xTimer)
{
	static unsigned int ledState = 0;

	if (ledState) {
		GPIO_ResetBits(GPIOC, (1 << 8));
	} else {
		GPIO_SetBits(GPIOC, (1 << 8));
	}
	ledState ^= 1;
}

static void prvLEDTimer2Callback(TimerHandle_t xTimer)
{
	static unsigned int ledState = 0;

	if (ledState) {
		GPIO_ResetBits(GPIOC, (1 << 9));
	} else {
		GPIO_SetBits(GPIOC, (1 << 9));
	}
	ledState ^= 1;
}

static void prvPushbuttonTask(void* pvParameters)
{
	uint8_t timerState = 0;

	while (1) {
		// Wait for a button cycle: pressed then released. Debounce for some ms.
		while (!GPIO_ReadInputDataBit(GPIOA, (1 << 0)));
		vTaskDelay(50 / portTICK_PERIOD_MS);
		while (GPIO_ReadInputDataBit(GPIOA, (1 << 0)));
		vTaskDelay(50 / portTICK_PERIOD_MS);

		timerState ^= 1;

		if (timerState) {
			xTimerStop(xLEDTimer1, 0);
		} else {
			xTimerStart(xLEDTimer1, 0);
		}
	}
}

int main(int argc, char* argv[])
{
	// Ensure that all 4 interrupt priority bits are used as the pre-emption priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	// Enable GPIO Peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	// Configure Pushbutton pins in input mode
	GPIO_InitStructure.GPIO_Pin = (1 << 0);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Configure LED pins in output push/pull mode
	GPIO_InitStructure.GPIO_Pin = (1 << 8) | (1 << 9);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Setup FreeRTOS
	xLEDTimer1 = xTimerCreate("LEDTimer1", (300 / portTICK_PERIOD_MS), pdTRUE,
			(void*) 0, prvLEDTimer1Callback);
	xTimerStart(xLEDTimer1, 0);

	xLEDTimer2 = xTimerCreate("LEDTimer2", (370 / portTICK_PERIOD_MS), pdTRUE,
			(void*) 0, prvLEDTimer2Callback);
	xTimerStart(xLEDTimer2, 0);

	xTaskCreate(prvPushbuttonTask, "Pushbutton", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

	// Start FreeRTOS
	vTaskStartScheduler();

	// Infinite loop, should not reach here
	while (1) {};
}
