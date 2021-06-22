#ifndef __TRILL_SENSOR__
#define __TRILL_SENSOR__

#include <esp_err.h>
#include <driver/i2c.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Input from Trill sensor is 1792 x 1792
#define TRILL_SQUARE_COORDINATE_MAX 1792
// That's a large matrix, to rounding it into a 28x28 matrix.
#define TRILL_SQUARE_COORDINATE_LOCAL CONFIG_TRILL_SQUARE_COORDINATE_LOCAL // Default: 28
// Could be: 1, 2, 4, 7, 8, 14, 16, 28, 32, 56, 64, 112, 128, 224, 256, 448, 896, 1792
#define TRILL_SQUARE_DIVIDER_FACTOR (TRILL_SQUARE_COORDINATE_MAX / TRILL_SQUARE_COORDINATE_LOCAL) // Default: 64

#define SAMPLE_DELAY 2
#define TRILL_BAR_READ_INTERVAL_MS 50
#define TRILL_BAR_MIN_INPUT_VAL 1408
#define TRILL_BAR_MIN_CHANGE 25

typedef enum trill_state {
    TRILL_TOUCH_START,
    TRILL_TOUCH_IDLE,
    TRILL_TOUCH_END,
    TRILL_MOVING_UP,
    TRILL_MOVING_DOWN,
    TRILL_MOVING_RIGHT,
    TRILL_MOVING_LEFT
} trill_state;

typedef void trill_callback_t(trill_state state, int16_t raw_value);

esp_err_t trill_init(i2c_port_t i2c_port, trill_callback_t* trill_callback);
float* trill_square_fetch(void);
void trill_bar_task(void* params);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TRILL_SENSOR__ */