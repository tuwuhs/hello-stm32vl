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
		GPIO_ResetBits(GPIOC, GPIO_Pin_8);
	} else {
		GPIO_SetBits(GPIOC, GPIO_Pin_8);
	}
	ledState ^= 1;
}

static void prvLEDTimer2Callback(TimerHandle_t xTimer)
{
	static unsigned int ledState = 0;

	if (ledState) {
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
	} else {
		GPIO_SetBits(GPIOC, GPIO_Pin_9);
	}
	ledState ^= 1;
}

static void prvPushbuttonTask(void* pvParameters)
{
	uint8_t timerState = 0;

	while (1) {
		// Wait for a button cycle: pressed then released. Debounce for some ms.
		while (!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0));
		vTaskDelay(50 / portTICK_PERIOD_MS);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0));
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
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Configure LED pins in output push/pull mode
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Enable clocks for port A and DAC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	// Set up PA.4 as DAC channel 1 output
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// DAC channel 1 configuration
	DAC_InitTypeDef DAC_InitStructure;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = 0;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);

	// Enable DAC Channel 1
	DAC_Cmd(DAC_Channel_1, ENABLE);

	// Test write to DAC
	DAC_SetChannel1Data(DAC_Align_12b_R, 1024);
	DAC_SetChannel1Data(DAC_Align_12b_R, 2048);
	DAC_SetChannel1Data(DAC_Align_12b_R, 3072);

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