/*
 * application.c
 *
 * (c) 2015 Tuwuh Sarwoprasojo
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationIdleHook(void);

void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	 free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	 internally by FreeRTOS API functions that create tasks, queues, software
	 timers, and semaphores.  The size of the FreeRTOS heap is set by the
	 configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for (;;)
		;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void) pcTaskName;
	(void) pxTask;

	/* Run time stack overflow checking is performed if
	 configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	 function is called if a stack overflow is detected. */
	for (;;)
		;
}

void vApplicationIdleHook(void)
{
	volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	 does nothing useful, other than report the amout of FreeRTOS heap that
	 remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if (xFreeStackSpace > 100) {
		/* By now, the kernel has allocated everything it is going to, so
		 if there is a lot of heap remaining unallocated then
		 the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		 reduced accordingly. */
	}
}

