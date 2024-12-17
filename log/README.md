# Lab 6. Scheduling

## Learning objectives.
* Evaluate production/consumption ratio to prevent overflow.
* Classify timing requirements as hard, firm, or soft real-time requirements
* Prioritize tasks based on timing requirements.
* Identify cases of priority inversion.
* Identify the difference between cooperative and preemptive prioritization.
* Identify reschedule points in the operating system.
* Analyze and select scheduling algorithms based on their ability to meet real-time requirements
* Build systems that meet hard timing requirements.

## Pre-lab
Watch this video about producer/consumer throughput.

https://www.youtube.com/watch?v=AnHiAWlrYQc&ab_channel=ParamountPlus

Review these sections of the documentation

https://www.freertos.org/Documentation/01-FreeRTOS-quick-start/01-Beginners-guide/01-RTOS-fundamentals

https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities

https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/04-Task-scheduling

Read chapter 12 in Lee & Seshia.

Read sections 6.1 and 6.3 of this chapter.

https://www.sciencedirect.com/science/article/pii/B9780123749574000153#s0145

# Lab

When you create supervisor threads, you will need to set the priority higher than subordinate threads, which forces the supervisor thread to always preempt others.

https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities

To measure the runtime of threads, the runtime statistics can be gathered with task utility functions. Of particular interest are `vTaskGetInfo` and `uxTaskGetSystemState`. Other timing functions are available that may be useful. Check to see if there is any configuration changes that need to be made to enable metric gathering.

https://www.freertos.org/Documentation/02-Kernel/04-API-references/03-Task-utilities/00-Task-utilities

When dealing with indeterminate data in tests due to variations in runtime, you may need to test bounded ranges.

https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md#test_assert_int_within-delta-expected-actual

### Activity 0
Induce priority inversion.

1. create two preemptable threads (and a supervisor to manage them).
1. create a semaphore shared between the two. Create it with xSemaphoreCreateBinary.
1. set one thread to higher priority. set it to delay start.
1. have the lower priority thread acquire the semphore first.
1. Predict the behavior of the system.


# Code
### **Task Priorities**
- **Emperor**: Priority 3 (Supervisor Task)
- **King**: Priority 2 (High Priority Task)
- **BlinkTask** and **Baron**: Priority 1 (Lower Priority Tasks)

---

### **Expected Behavior**
1. **BlinkTask**:
   - Runs continuously, toggling the LED state with a delay of 500ms.
2. **Baron**:
   - Acquires the semaphore first and holds it for 4 seconds.
3. **King**:
   - Starts after a delay, attempts to acquire the semaphore, and **blocks** because Baron holds it.
4. **Emperor**:
   - Periodically prints supervisory messages every second.
5. **Demonstration of Priority Inversion**:
   - **King** (higher priority) is forced to wait for **Baron** (lower priority) to release the semaphore.
   - This simulates priority inversion, a situation where a higher-priority task is blocked by a lower-priority task.

---

### **Code: `main.c`**
```c
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
```


# Predicted bevavior of the system with induced priority inversion
The task of higherpriority denoted by vTaskHighPriority will block the thread from progressing and wait for the Semaphore which never arrives. Without using Mutex or Priority inheritance any task of higher priorty can prempt a task of a lower priorty causing the thread to freeze. 

### Activity 1
Repeat the previous experiment, but this time create the semaphore with xSemaphoreCreateMutex

### Activity 2
In this activity, you'll create two competing threads that use the following functions
```
void busy_busy(void)
{
    for (int i = 0; ; i++);
}

void busy_yield(void)
{
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}
```
Write tests for two threads running the following scenarios. Try to predict the runtime for each thread.
1. Threads with same priority:
    1. Both run `busy_busy`.
    1. Both run `busy_yield`
    1. One run `busy_busy` one run `busy_yield`
1. Threads with different priority.
    1. Both run `busy_busy`.
        1. Higher priority starts first.
        1. Lower priority starts first.
    1. Both run `busy_yield`.

Make sure you are setting priorities according to the priority order presented in the documentation.

# Reference implementation
Reference implementation is located at https://github.com/uofu-emb/rtos.06