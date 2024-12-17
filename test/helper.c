#include "helper.h"
#include <stdio.h>

// Forward declarations for thread functions
void emperorTask(void *params);
void kingTask(void *params);
void baronTask(void *params);

void setupThreads(TaskHandle_t *emperorHandle, TaskHandle_t *kingHandle, TaskHandle_t *baronHandle, SemaphoreHandle_t *semaphore) {
    // Create binary semaphore
    *semaphore = xSemaphoreCreateBinary();
    if (*semaphore == NULL) {
        printf("Failed to create semaphore!\n");
        return;
    }

    // Create tasks
    xTaskCreate(emperorTask, "Emperor", 2048, (void *)*semaphore, 3, emperorHandle);
    xTaskCreate(kingTask, "King", 2048, (void *)*semaphore, 2, kingHandle);
    xTaskCreate(baronTask, "Baron", 2048, (void *)*semaphore, 1, baronHandle);

    printf("Threads setup complete.\n");
}

void teardownThreads(TaskHandle_t emperorHandle, TaskHandle_t kingHandle, TaskHandle_t baronHandle, SemaphoreHandle_t semaphore) {
    // Delete tasks
    vTaskDelete(emperorHandle);
    vTaskDelete(kingHandle);
    vTaskDelete(baronHandle);

    // Delete semaphore
    vSemaphoreDelete(semaphore);

    printf("Threads teardown complete.\n");
}
