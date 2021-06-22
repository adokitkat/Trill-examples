#include "trill_sensor.h"
#include "Trill.h"
#include "driver/i2c.h"
#include <string.h>
#include "esp_log.h"

static const char* TAG = "trill_sensor";

float input[TRILL_SQUARE_COORDINATE_LOCAL][TRILL_SQUARE_COORDINATE_LOCAL];
static Trill trillSquare;
static Trill trillBar;
static trill_callback_t* bar_event_callback;

esp_err_t trill_init(i2c_port_t i2c_port, trill_callback_t* trill_callback)
{
    esp_err_t ret = ESP_OK;
    int trill_err;

    trill_err = trillSquare.setup(Trill::TRILL_SQUARE, i2c_port);
    if (trill_err) {
        ret = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to init TRILL_SQUARE: %d", trill_err);
    }
    

    trill_err = trillBar.setup(Trill::TRILL_BAR, i2c_port);
    if (trill_err) {
        ret = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to init TRILL_BAR: %d", trill_err);   
    }
    /* else {
      TaskHandle_t touch_task_handle;
      BaseType_t status = xTaskCreate(touch_bar_task, "touchbar_task", 2048, NULL, tskIDLE_PRIORITY, &touch_task_handle);
      assert(status == pdPASS);
    }
    */
    bar_event_callback = trill_callback;

    return ret;
}

float* trill_square_fetch(void)
{
    bool touchActive = false;
    uint16_t input_x, input_y;
    uint8_t x, y, a, b;

    for ( ;; ) {
        trillSquare.read();
        
        if (trillSquare.getNumTouches() > 0 && trillSquare.getNumHorizontalTouches() > 0) {
            if (!touchActive) {
                memset(input, 0, sizeof(input));
            }
            input_x = TRILL_SQUARE_COORDINATE_MAX - trillSquare.touchLocation(0);
            input_y = trillSquare.touchHorizontalLocation(0);

            input_x = input_x >= TRILL_SQUARE_COORDINATE_MAX ? (TRILL_SQUARE_COORDINATE_MAX - 1) : input_x;
            input_y = input_y >= TRILL_SQUARE_COORDINATE_MAX ? (TRILL_SQUARE_COORDINATE_MAX - 1) : input_y;
            
            //x = MAX((input_x / TRILL_SQUARE_DIVIDER_FACTOR) - 1, 0);
            //y = MAX((input_y / TRILL_SQUARE_DIVIDER_FACTOR) - 1, 0);

            x = (input_x / TRILL_SQUARE_DIVIDER_FACTOR);
            y = (input_y / TRILL_SQUARE_DIVIDER_FACTOR);

            input[x][y] = 1;
            touchActive = true;

        } else if(touchActive) {
            touchActive = false;
            return (float*)input;

        } else {
            return NULL;
        }
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_DELAY));
    }

    return NULL;
}


void trill_bar_task(void* params)
{
    bool touchActive = false;
    int16_t input_val;
    int16_t prev_val;

    for ( ;; ) {
        trillBar.read();
        if (trillBar.getNumTouches() == 1) {
            input_val = TRILL_SQUARE_COORDINATE_MAX - trillBar.touchLocation(0);
            input_val = input_val + TRILL_BAR_MIN_INPUT_VAL; // Make input between 0 and max_input + min_input;
            if (!touchActive) {
                // Since sometimes if the Trill bar is dirty or similar it outputs random touches
                // This is to verify that there still is a touch after TRILL_BAR_READ_INTERVAL_MS
                bool check = true;
                int16_t prev_check_val = input_val;
                for (int i = 0; i < 10; i++) {
                    vTaskDelay(pdMS_TO_TICKS(TRILL_BAR_READ_INTERVAL_MS / 10));
                    trillBar.read();
                    if (trillBar.getNumTouches() == 0 || trillBar.touchLocation(0) == input_val) {
                        check = false;
                        break;
                    }
                }
                if (check) {
                    touchActive = true;
                    prev_val = input_val;
                }
                bar_event_callback(TRILL_TOUCH_START, input_val);
            } else {
                if (abs(input_val - prev_val) > TRILL_BAR_MIN_CHANGE) {
                    if (input_val > prev_val) {
                        bar_event_callback(TRILL_MOVING_UP, input_val);
                    } else {
                        bar_event_callback(TRILL_MOVING_DOWN, input_val);
                    }
                } else {
                    bar_event_callback(TRILL_TOUCH_IDLE, input_val);
                }
            }

            prev_val = input_val;
        } else if (touchActive) {
            touchActive = false;
            bar_event_callback(TRILL_TOUCH_END, input_val);
        }
        vTaskDelay(pdMS_TO_TICKS(TRILL_BAR_READ_INTERVAL_MS));
    }
}