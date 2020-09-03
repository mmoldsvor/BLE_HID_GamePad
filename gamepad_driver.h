#include <math.h>
#include "nrf.h"

typedef enum 
{
    GAMEPAD_BUTTONS_1,
    GAMEPAD_BUTTONS_2,
    GAMEPAD_LEFT_STICK,
    GAMEPAD_RIGHT_STICK,
    GAMEPAD_LEFT_TRIGGER,
    GAMEPAD_RIGHT_TRIGGER,
} gamepad_evt_type_t;

typedef struct
{
    gamepad_evt_type_t  evt_type;
    uint8_t             evt_value[2];
} gamepad_evt_t;

typedef struct
{
  uint8_t buttons_1;
  uint8_t buttons_2;
  uint8_t left_stick_x;
  uint8_t left_stick_y;
  uint8_t right_stick_x;
  uint8_t right_stick_y;
  uint8_t left_trigger;
  uint8_t right_trigger;
} gamepad_report_t;

typedef void (*gamepad_evt_handler_t) (gamepad_evt_t);

void initialize_saadc(uint8_t resolution);

void configure_stick(uint8_t stick_index, uint8_t vertical_pin, uint8_t horizontal_pin);