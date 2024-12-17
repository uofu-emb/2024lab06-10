#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"
#include "pico/stdlib.h"

// Task Handles
TaskHandle_t kingHandle = NULL;
TaskHandle_t baronHandle = NULL;
TaskHandle_t emperorHandle = NULL;

// Global counters to track execution
volatile uint32_t kingCounter = 0;
volatile uint32_t baronCounter = 0;

// Unity setup and teardown functions
void setUp(void) {
    // This runs before each test
    kingCounter = 0;
    baronCounter = 0;
}

void tearDown(void) {
    // This runs after each test
    printf("Test complete. Cleaning up.\n");
}

// Function: busy_busy (runs continuously)
void busy_busy(void *params) {
    while (1) {
        (*(volatile uint32_t *)params)++;
    }
}

// Function: busy_yield (yields CPU)
void busy_yield(void *params) {
    while (1) {
        (*(volatile uint32_t *)params)++;
        taskYIELD();
    }
}

// Supervisor Task: Emperor (prints status periodically)
void emperorTask(void *params) {
    while (1) {
        printf("Emperor: King=%lu, Baron=%lu\n", kingCounter, baronCounter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Test 1: Same Priority - Both busy_busy
void test_same_priority_busy_busy(void) {

    xTaskCreate(busy_busy, "King", 1024, (void *)&kingCounter, 1, &kingHandle);
    xTaskCreate(busy_busy, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);

    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);

    printf("Test: Same Priority - Both busy_busy\n");
    TEST_ASSERT_EQUAL(kingCounter, baronCounter);
}

// Test 2: Same Priority - Both busy_yield
void test_same_priority_busy_yield(void) {


    xTaskCreate(busy_yield, "King", 1024, (void *)&kingCounter, 1, &kingHandle);
    xTaskCreate(busy_yield, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);

    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);

    printf("Test: Same Priority - Both busy_yield\n");
    TEST_ASSERT_EQUAL(kingCounter, baronCounter);
}

// Test 3: Same Priority - King busy_busy, Baron busy_yield
void test_same_priority_mixed(void) {


    xTaskCreate(busy_busy, "King", 1024, (void *)&kingCounter, 1, &kingHandle);
    xTaskCreate(busy_yield, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);

    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);

    printf("Test: Same Priority - King busy_busy, Baron busy_yield\n");
    TEST_ASSERT(kingCounter > baronCounter);
}

// Test 4: Different Priority - King busy_busy (Higher), Baron busy_busy (Lower)
void test_different_priority_busy_busy_high_first(void) {

    xTaskCreate(busy_busy, "King", 1024, (void *)&kingCounter, 2, &kingHandle);
    xTaskCreate(busy_busy, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);

    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);

    printf("Test: Different Priority - King (Higher) busy_busy\n");
    TEST_ASSERT(kingCounter > baronCounter);
}

// Test 5: Different Priority - Baron busy_busy starts first
void test_different_priority_busy_busy_low_first(void) {

    xTaskCreate(busy_busy, "Baron", 1024, (void *)&baronCounter, 1, &baronHandle);
    vTaskDelay(pdMS_TO_TICKS(100));  // Let Baron start first
    xTaskCreate(busy_busy, "King", 1024, (void *)&kingCounter, 2, &kingHandle);

    vTaskDelay(pdMS_TO_TICKS(1000));
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);

    printf("Test: Different Priority - Baron starts first\n");
    TEST_ASSERT(kingCounter > baronCounter);
}

// Main Test Runner
void main_thread(void *params) {
      while (1) {  // Continuous test loop
    UNITY_BEGIN();

    // Run Tests
    RUN_TEST(test_same_priority_busy_busy);
    RUN_TEST(test_same_priority_busy_yield);
    RUN_TEST(test_same_priority_mixed);
    RUN_TEST(test_different_priority_busy_busy_high_first);
    RUN_TEST(test_different_priority_busy_busy_low_first);

    UNITY_END();
     printf("All tests completed. Restarting in 5 seconds...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));  // Wait 5 seconds before restarting
      }
    vTaskDelete(NULL);
}

int main(void) {
    stdio_init_all();
    sleep_ms(2000);  // Allow USB enumeration

    printf("Starting Priority Inversion Tests...\n");

    xTaskCreate(emperorTask, "Emperor", 1024, NULL, 3, &emperorHandle);  // Supervisor task
    xTaskCreate(main_thread, "MainThread", 2048, NULL, 2, NULL);  // Run tests

    vTaskStartScheduler();  // Start the scheduler

    while (1);  // Should never reach here
    return 0;
}
