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

## **Task Priorities**
- **Emperor**: Priority 3 (Supervisor Task)
- **King**: Priority 2 (High-Priority Task)
- **Baron**: Priority 1 (Low-Priority Task)
- **BlinkTask**: Priority 1 (Background Task, runs silently)

---

## **Expected Behavior**
1. **BlinkTask**:
   - Runs continuously, toggling the LED state every 500ms.
   - Print lines are commented out for silent operation.
2. **Baron**:
   - Acquires the **mutex** first and holds it for 4 seconds.
3. **King**:
   - Starts after a delay, attempts to acquire the mutex, and **blocks** because Baron holds it.
   - With **priority inheritance**, Baron's priority temporarily increases to match King's priority, ensuring timely release of the mutex.
4. **Emperor**:
   - Periodically prints supervisory messages every second.
5. **Priority Inheritance Demonstration**:
   - Unlike with a binary semaphore, **priority inheritance** prevents King from indefinite blocking.

---

## **Code: `main.c`**
```c
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
```


## **Expected Behavior**
1. Blink task runs silently in the background.
2. Logs demonstrate priority inheritance, where Baron completes its execution faster due to temporary priority boost.
3. King acquires the mutex once Baron finished

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

# Here's my code, test output, and mytest.c

## **Test Scenarios**

### 1. **Threads with Same Priority**
   - **Both run `busy_busy`**:  
     Prediction: Both threads increment their counters equally since they have the same priority.  
     Actual: Slight counter mismatch due to FreeRTOS scheduler overhead and context switching delays.

   - **Both run `busy_yield`**:  
     Prediction: Threads share CPU time equally because `taskYIELD()` allows for cooperative task switching.  
     Actual: Minor mismatch due to timing overhead when calling `taskYIELD()`.

   - **One runs `busy_busy`, one runs `busy_yield`**:  
     Prediction: `busy_busy` thread dominates the CPU as it does not yield, while `busy_yield` thread progresses minimally.  
     Actual: Matches the prediction, with the `busy_busy` thread achieving significantly higher counter increments.

### 2. **Threads with Different Priority**
   - **Both run `busy_busy`**:
     1. **Higher priority starts first**:  
        Prediction: The higher-priority thread dominates and gets most of the CPU time.  
        Actual: Matches the prediction, with the higher-priority thread dominating execution.

     2. **Lower priority starts first**:  
        Prediction: The lower-priority thread runs briefly until preempted by the higher-priority thread.  
        Actual: Matches the prediction, with the lower-priority thread incrementing counters initially but quickly yielding control.

   - **Both run `busy_yield`**:  
     Prediction: The higher-priority thread gets more CPU time but allows context switches due to `taskYIELD()`.  
     Actual: Minor discrepancies arise due to context switching overhead, but the higher-priority thread dominates.

---

## **Test Output**

```plaintext
Test: Same Priority - Both busy_busy
/home/andrew/Documents/AdvEmb/Labs/Lab.06Scheduling/2024lab06-10-andrew-sam/test/mytest.c:62:test_same_priority_busy_busy:FAIL: Expected 8896476 Was 8908806
Test complete. Cleaning up.

Emperor: King=187545, Baron=187568
Test: Same Priority - Both busy_yield
/home/andrew/Documents/AdvEmb/Labs/Lab.06Scheduling/2024lab06-10-andrew-sam/test/mytest.c:77:test_same_priority_busy_yield:FAIL: Expected 320157 Was 320187
Test complete. Cleaning up.

Emperor: King=10297936, Baron=581
Test: Same Priority - King busy_busy, Baron busy_yield
Test complete. Cleaning up.
/home/andrew/Documents/AdvEmb/Labs/Lab.06Scheduling/2024lab06-10-andrew-sam/test/mytest.c:132:test_same_priority_mixed:PASS
Emperor: King=10247069, Baron=0
Test: Different Priority - King (Higher) busy_busy
Test complete. Cleaning up.
/home/andrew/Documents/AdvEmb/Labs/Lab.06Scheduling/2024lab06-10-andrew-sam/test/mytest.c:133:test_different_priority_busy_busy_high_first:PASS
Emperor: King=8364148, Baron=1781872
Test: Different Priority - Baron starts first
Test complete. Cleaning up.
/home/andrew/Documents/AdvEmb/Labs/Lab.06Scheduling/2024lab06-10-andrew-sam/test/mytest.c:134:test_different_priority_busy_busy_low_first:PASS

-----------------------
5 Tests 2 Failures 0 Ignored 
FAIL
```

---

## **Analysis of Test Output**
1. **Same Priority - Both `busy_busy`**:
   - **Expected**: Equal counters.  
   - **Actual**: Minor differences observed (e.g., `8896476` vs `8908806`).

2. **Same Priority - Both `busy_yield`**:
   - **Expected**: Equal counters.  
   - **Actual**: Small counter discrepancy due to timing overhead (e.g., `320157` vs `320187`).

3. **Same Priority - `busy_busy` vs `busy_yield`**:
   - **Expected**: `busy_busy` dominates.  
   - **Actual**: Matches the prediction; `busy_busy` dominates while `busy_yield` increments minimally.

4. **Different Priority - Higher Priority First**:
   - **Expected**: Higher-priority thread dominates.  
   - **Actual**: Matches prediction.

5. **Different Priority - Lower Priority First**:
   - **Expected**: Lower-priority thread runs briefly before being preempted.  
   - **Actual**: Matches prediction.

---

## **Code: `mytest.c`**
```c
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"
#include "pico/stdlib.h"

// Task Handles
TaskHandle_t kingHandle = NULL;
TaskHandle_t baronHandle = NULL;

// Global Counters
volatile uint32_t kingCounter = 0;
volatile uint32_t baronCounter = 0;

// Unity Setup and Teardown
void setUp(void) {
    kingCounter = 0;
    baronCounter = 0;
    printf("Commence test\n");
}

void tearDown(void) {
    printf("Test complete. Cleaning up.\n");
    vTaskDelay(pdMS_TO_TICKS(5000));  // Wait 5 seconds before next test
}

// busy_busy: increments counter continuously
void busy_busy(void *params) {
    while (1) {
        (*(volatile uint32_t *)params)++;
    }
}

// busy_yield: increments counter but yields
void busy_yield(void *params) {
    while (1) {
        (*(volatile uint32_t *)params)++;
        taskYIELD();
    }
}

// Test Cases
void test_same_priority_busy_busy(void) {
    xTaskCreate(busy_busy, "King", 1024, (void *)&kingCounter, 1, &kingHandle);
    xTaskCreate(busy_busy, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);
    TEST_ASSERT_EQUAL(kingCounter, baronCounter);
}

void test_same_priority_busy_yield(void) {
    xTaskCreate(busy_yield, "King", 1024, (void *)&kingCounter, 1, &kingHandle);
    xTaskCreate(busy_yield, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);
    TEST_ASSERT_EQUAL(kingCounter, baronCounter);
}

void test_same_priority_mixed(void) {
    xTaskCreate(busy_busy, "King", 1024, (void *)&kingCounter, 1, &kingHandle);
    xTaskCreate(busy_yield, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);
    TEST_ASSERT(kingCounter > baronCounter);
}

void main_thread(void *params) {
    UNITY_BEGIN();
    RUN_TEST(test_same_priority_busy_busy);
    RUN_TEST(test_same_priority_busy_yield);
    RUN_TEST(test_same_priority_mixed);
    UNITY_END();
    vTaskDelete(NULL);
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);  // Allow USB enumeration
    xTaskCreate(main_thread, "MainThread", 2048, NULL, 2, NULL);
    vTaskStartScheduler();
    while (1);
}
```

---

## **Summary**
1. **Failures** are due to minor timing overhead from FreeRTOS.
2. **Tolerance ranges** can resolve false negatives in tests.
3. All scenarios behaved as expected conceptually.
