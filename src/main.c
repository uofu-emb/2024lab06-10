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


// Task Handles
TaskHandle_t emperorHandle = NULL;
TaskHandle_t kingHandle = NULL;
TaskHandle_t baronHandle = NULL;
TaskHandle_t blinkHandle = NULL;

// Shared Mutex
SemaphoreHandle_t sharedMutex = NULL;

// Blink Task: Simulates LED Blinking (Background Task)
void blink_task(void *params) {
    const TickType_t delay = pdMS_TO_TICKS(500);  // 500ms delay for blink

    const uint LED_PIN = 25;  // Onboard LED pin for Raspberry Pi Pico
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1);  // Turn LED ON
        vTaskDelay(delay);
        gpio_put(LED_PIN, 0);  // Turn LED OFF
        vTaskDelay(delay);
    }
}


// Supervisor Task: Emperor
void emperorTask(void *params) {
    printf("Emperor: Supervising tasks...\n");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Periodic supervision
        printf("Emperor: Checking system status...\n");
    }
}

// High-priority Task: King
void kingTask(void *params) {
    SemaphoreHandle_t mutex = (SemaphoreHandle_t)params;

    vTaskDelay(pdMS_TO_TICKS(500));  // Delayed start for King
    printf("King: Attempting to acquire mutex...\n");

    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
        printf("King: Acquired mutex! Performing high-priority task...\n");
        vTaskDelay(pdMS_TO_TICKS(2000));  // Simulate task execution
        xSemaphoreGive(mutex);
        printf("King: Released mutex.\n");
    }

    vTaskDelete(NULL);
}

// Low-priority Task: Baron
void baronTask(void *params) {
    SemaphoreHandle_t mutex = (SemaphoreHandle_t)params;

    printf("Baron: Acquiring mutex...\n");
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
        printf("Baron: Holding mutex... Simulating long execution...\n");
        vTaskDelay(pdMS_TO_TICKS(4000));  // Simulate long execution
        xSemaphoreGive(mutex);
        printf("Baron: Released mutex.\n");
    }

    vTaskDelete(NULL);
}

int main(void) {
    // Initialize stdio for USB communication
    stdio_init_all();
    sleep_ms(2000);  // Allow USB enumeration

    printf("Starting Priority Inversion Demonstration with Mutex...\n");

    // Create a mutex
    sharedMutex = xSemaphoreCreateMutex();
    if (sharedMutex == NULL) {
        printf("Failed to create mutex!\n");
        while (1);  // Halt
    }

    // Create tasks
    xTaskCreate(blink_task, "BlinkTask", 1024, NULL, 1, &blinkHandle);  // Priority 1 (Background)
    xTaskCreate(emperorTask, "Emperor", 1024, NULL, 3, &emperorHandle);  // Priority 3
    xTaskCreate(kingTask, "King", 1024, sharedMutex, 2, &kingHandle);  // Priority 2
    xTaskCreate(baronTask, "Baron", 1024, sharedMutex, 1, &baronHandle);  // Priority 1

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