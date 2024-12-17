/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "helper.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"
//
int count = 0;
bool on = false;
//Blink Task was left in for debugging not necesary for running program
//#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
//#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE


// Task Handles
TaskHandle_t emperorHandle = NULL;
TaskHandle_t kingHandle = NULL;
TaskHandle_t baronHandle = NULL;
TaskHandle_t blinkHandle = NULL;

// Shared Semaphore
SemaphoreHandle_t sharedSemaphore = NULL;

// Blink Task: Simulates LED Blinking
void blink_task(void *params) {
    const TickType_t delay = pdMS_TO_TICKS(500);  // 500ms delay for blink
    while (1) {
        printf("Blink Task: LED ON\n");
        vTaskDelay(delay);
        printf("Blink Task: LED OFF\n");
        vTaskDelay(delay);
    }
}

// Supervisor Task: Emperor
void emperorTask(void *params) {
    printf("Emperor: Supervising tasks...\n");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Periodic supervision
    }
}

// High-priority Task: King
void kingTask(void *params) {
    SemaphoreHandle_t semaphore = (SemaphoreHandle_t)params;

    vTaskDelay(pdMS_TO_TICKS(500));  // Delayed start for King
    printf("King: Attempting to acquire semaphore...\n");

    if (xSemaphoreTake(semaphore, portMAX_DELAY)) {
        printf("King: Acquired semaphore! Performing high-priority task...\n");
        vTaskDelay(pdMS_TO_TICKS(2000));  // Simulate task execution
        xSemaphoreGive(semaphore);
        printf("King: Released semaphore.\n");
    }

    vTaskDelete(NULL);
}

// Low-priority Task: Baron
void baronTask(void *params) {
    SemaphoreHandle_t semaphore = (SemaphoreHandle_t)params;

    printf("Baron: Acquiring semaphore...\n");
    if (xSemaphoreTake(semaphore, portMAX_DELAY)) {
        printf("Baron: Holding semaphore... Simulating long execution...\n");
        vTaskDelay(pdMS_TO_TICKS(4000));  // Simulate long execution
        xSemaphoreGive(semaphore);
        printf("Baron: Released semaphore.\n");
    }

    vTaskDelete(NULL);
}

int main(void) {
    // Initialize the stdio for USB communication
    stdio_init_all();
    sleep_ms(2000);  // Allow USB enumeration

    printf("Starting Priority Inversion Demonstration with Blink Task...\n");

    // Create a binary semaphore
    sharedSemaphore = xSemaphoreCreateBinary();
    if (sharedSemaphore == NULL) {
        printf("Failed to create semaphore!\n");
        while (1);  // Halt
    }

    // Initially give the semaphore so Baron can acquire it first
    xSemaphoreGive(sharedSemaphore);

    // Create tasks
    xTaskCreate(blink_task, "BlinkTask", 1024, NULL, 1, &blinkHandle);  // Priority 1
    xTaskCreate(emperorTask, "Emperor", 1024, NULL, 3, &emperorHandle);  // Priority 3
    xTaskCreate(kingTask, "King", 1024, sharedSemaphore, 2, &kingHandle);  // Priority 2
    xTaskCreate(baronTask, "Baron", 1024, sharedSemaphore, 1, &baronHandle);  // Priority 1

    // Start the scheduler
    vTaskStartScheduler();

    // Code should never reach here
    while (1);
    return 0;
}
/*
void main_task(__unused void *params) {
    xTaskCreate(blink_task, "BlinkThread",
                BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
    char c;
    while(c = getchar()) {
        if (c <= 'z' && c >= 'a') putchar(c - 32);
        else if (c >= 'A' && c <= 'Z') putchar(c + 32);
        else putchar(c);
    }
}

int main( void )
{
    stdio_init_all();
    const char *rtos_name;
    rtos_name = "FreeRTOS";
    TaskHandle_t task;
    xTaskCreate(main_task, "MainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);
    vTaskStartScheduler();
    return 0;
}
*/