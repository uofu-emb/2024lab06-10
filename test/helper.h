// helper.h
#ifndef HELPER_H
#define HELPER_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Semaphore shared across threads
extern SemaphoreHandle_t sharedSemaphore;

// Thread prototypes
void emperor_task(void *params);
void king_task(void *params);
void baron_task(void *params);

// Helper functions
void create_tasks(void);
void delete_tasks(void);

#endif // HELPER_H