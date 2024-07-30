#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define tlacitko GPIO_NUM_22
TaskHandle_t handle_vypis;
SemaphoreHandle_t semafor = NULL;
static const char *TAG = "semaphoreMutex";

void vypisHello(void *pvParametr)
{
    while (1)
    {
        // Wait for semaphore to be available
        if (xSemaphoreTake(semafor, portMAX_DELAY) == pdTRUE)
        {
            // Print numbers
            for (int i = 0; i < 50; i++)
            {
                ESP_LOGI(TAG, "%d\n", i);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            // Release the semaphore
            xSemaphoreGive(semafor);
        }
        else
        {
            ESP_LOGI(TAG, "Semaphore not taken!");
        }
    }
}

void IRAM_ATTR gpio_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (semafor != NULL)
    {
        xSemaphoreGiveFromISR(semafor, &xHigherPriorityTaskWoken);
    }
    if (xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}


void configure_interrupt()
{
    // Configure GPIO pin as input
    gpio_set_direction(tlacitko, GPIO_MODE_INPUT);

    // Set interrupt type for GPIO pin (e.g., rising edge, falling edge, etc.)
    gpio_set_intr_type(tlacitko, GPIO_INTR_ANYEDGE);

    // Install GPIO ISR service
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    // Hook ISR handler to GPIO pin
    gpio_isr_handler_add(tlacitko, gpio_isr_handler, NULL);
}

void app_main()
{
    configure_interrupt();
    semafor = xSemaphoreCreateMutex();

    if (semafor == NULL)
    {
        ESP_LOGI(TAG, "Semaphore creation failed");
    }
    else
    {
        ESP_LOGI(TAG, "Semaphore creation succesfull");
    }

    xTaskCreate(vypisHello, "vypisHello", 2048, NULL, 6, NULL);
}