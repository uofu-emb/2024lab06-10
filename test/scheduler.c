#include <unity_config.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <unity.h>
#include "helper.c"

SemaphoreHandle_t semaphore;
TaskHandle_t main_Task;

#define TEST_DURATION 5000

void run_anaylzer

void priority_inversion(void *args) {
    char *name = (char *)args;
    printf("Start priority inversion %s\n", name);
    while(1) {
        xSemaphoreTake()
    }
}

void main_thread(void *args) {

    xTaskCreate(main_thread, "main_thread", NULL, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY+10, &main_Task);

}

int main(void) {
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    xTaskCreate(main_thread, "main_thread", NULL, configMINIMAL_STACK_SIZE, tskIDLE_PRIORITY+10, &main_Task);
    vTaskStartScheduler();
    return 0;
}
