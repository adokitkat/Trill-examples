#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/i2c.h>

#include "trill_sensor.h"

#define SDA_PIN CONFIG_I2C_SDA_PIN
#define SCL_PIN CONFIG_I2C_SCL_PIN
#define I2C_PORT_NUM I2C_NUM_0

static const char* TAG = "main";

//float* square_matrix_input = NULL;

//static void trill_square_callback(trill_state state, int16_t raw_value);

static void print_square_matrix(float* in) {
  for (int row = 0; row < TRILL_SQUARE_COORDINATE_LOCAL * TRILL_SQUARE_COORDINATE_LOCAL; row++) {
    if (row % TRILL_SQUARE_COORDINATE_LOCAL == 0) {
      printf("\n");
    }
    printf("%c", (int)in[row] == 1 ? 'X' : ' ');
  }
  printf("\n");
}

static void trill_ring_func()
{
    ;
}

static void trill_bar_func()
{
    ;

}

static void trill_square_task()
{
    float* in_matrix;
    for( ;; )
    {
        in_matrix = trill_square_fetch();
        if (in_matrix) {
            //tf_gesture_predictor_run(in_matrix, 28 * 28 * sizeof(float), &prediction, PRINT_GESTURE_DATA);
            //if (prediction.probability > 0.95f) { sendKeysFromGesture(prediction.label); }
            //menu_draw_gestures(&prediction);
            //printf("Prediction: %s, prob: %f\n", getNameOfPrediction(prediction.label),  prediction.probability);
            ESP_LOGI(TAG, "Input registered");
            print_square_matrix(in_matrix);
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

static void trill_bar_event_callback(trill_state state, int16_t raw_value);

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
        ret = trill_init(I2C_PORT_NUM, &trill_bar_event_callback);
    }

    BaseType_t status;
    status = xTaskCreate(trill_square_task, "trill_square_task", 4096, NULL, tskIDLE_PRIORITY, NULL);
    assert(status == pdPASS);
    status = xTaskCreate(trill_bar_task, "trill_bar_task", 4096, NULL, tskIDLE_PRIORITY, NULL);
    assert(status == pdPASS);
}

static void trill_bar_event_callback(trill_state state, int16_t raw_value)
{
  uint8_t key = 0;//consumer_cmd_t key = 0;

  switch (state) {
    case TRILL_TOUCH_START:
    case TRILL_TOUCH_IDLE:
    case TRILL_TOUCH_END:
      break;
    case TRILL_MOVING_UP:
      key = 233;//HID_CONSUMER_VOLUME_UP;
      break;
    case TRILL_MOVING_DOWN:
      key = 234;//HID_CONSUMER_VOLUME_DOWN;
      break;
    default:
      ESP_LOGE(TAG, "Unknown touch bar state %d", state);
      break;
  }
  if (key != 0) {
      //printf("%d\n", key);
      printf("Trill Bar: %s\n", key == 233 ? "UP" : "DOWN");
    /*
    if (ble_hid_request_access(50) == ESP_OK) {
      ble_hid_send_consumer_key(key, true);
      ble_hid_send_consumer_key(key, false);
      ble_hid_give_access();
    }
    */
  }
}