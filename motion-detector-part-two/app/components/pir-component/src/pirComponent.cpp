#include "pirComponent.hpp"

PirComponent::PirComponent()
{
    ESP_LOGD(TAG, "Loading the Motion Dectector");
}

esp_err_t PirComponent::initializeGpio()
{
    ESP_LOGD(TAG, "Initialzing the GPIO pins");
    esp_err_t returnCode = ESP_OK;

    gpio_config_t pirConfig = gpio_config_t{
        .pin_bit_mask = GPIO_SEL_2,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    returnCode = gpio_config(&pirConfig);
    if (returnCode != ESP_OK)
    {
        ESP_LOGE(TAG, "The pir GPIO PIN was not initialized!");
        return returnCode;
    } // end if

    gpio_config_t realyConfig = gpio_config_t{
        .pin_bit_mask = GPIO_SEL_14,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    returnCode = gpio_config(&realyConfig);
    if (returnCode != ESP_OK)
    {
        ESP_LOGE(TAG, "The relay GPIO PIN was not initialized!");
    } // end if
    return returnCode;
}

esp_err_t PirComponent::startMotionDetector()
{
    ESP_LOGD(TAG, "Starting the Motion detector");
    esp_err_t operationState = initializeGpio();

    if (operationState == ESP_OK)
    {
        auto pirMotionDetectorTask = [](void *parameters)
        {
            ESP_LOGD("motionTask", "Starting the Motion Dectector Rtos task");
            for (;;)
            {
                int motionDetectedState = gpio_get_level(GPIO_NUM_2);
                if (motionDetectedState == 1)
                {
                    ESP_LOGD("motionTask", "Motion Detected");
                    gpio_set_level(GPIO_NUM_14, 1);
                    vTaskDelay(pdMS_TO_TICKS(CONFIG_PIR_KEEP_ALIVE_S * 1000));
                }
                else
                {
                    gpio_set_level(GPIO_NUM_14, 0);
                    ESP_LOGD("motionTask", "No Motion Detected");
                }

                vTaskDelay(pdMS_TO_TICKS(CONFIG_PIR_SENSITIVITY_MS));
            }
        };

        xTaskCreate(pirMotionDetectorTask, "motionDetectorTask", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
    } // end if

    return operationState;
}

PirComponent::~PirComponent()
{
    ESP_LOGD(TAG, "Killing the Motion Dectector");
}