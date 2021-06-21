#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>

#include "Trill.h"

#define SDA_PIN CONFIG_I2C_SDA_PIN
#define SCL_PIN CONFIG_I2C_SCL_PIN
#define I2C_PORT_NUM I2C_NUM_0

static const char* TAG = "main";

void app_main(void)
{
    esp_err_t ret;

    // Init NVS flash
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Init I2C
    i2c_config_t i2c_config;
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = SDA_PIN;
    i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.scl_io_num = SCL_PIN;
    i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.master.clk_speed = 100000;

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT_NUM, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT_NUM, I2C_MODE_MASTER, 0, 0, 0));

    ret = ESP_FAIL;
    for (int retries = 0; retries < 5 && ret != ESP_OK; retries++) {
        ret = touch_sensors_init(I2C_PORT, &touch_bar_event_callback);
    }
    }
