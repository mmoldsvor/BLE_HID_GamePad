#include <stdbool.h>
#include "nrf.h"

typedef enum 
{
    GAMEPAD_BUTTONS,
    GAMEPAD_LEFT_STICK,
    GAMEPAD_RIGHT_STICK,
    GAMEPAD_TRIGGERS,
} gamepad_evt_type_t;

typedef struct
{
    gamepad_evt_type_t  evt_type;
    uint8_t             value[2];
} gamepad_evt_t;

typedef void (*gamepad_evt_handler_t) (gamepad_evt_t);

typedef struct
{
    gamepad_evt_handler_t gamepad_handle;
    uint8_t *p_buttons;
    uint8_t button_count;
} gamepad_init_t;


void initialize_gamepad(gamepad_evt_handler_t gamepad_on_evt);

void initialize_saadc(uint8_t resolution);
void configure_stick(uint8_t stick_index, uint8_t vertical_pin, uint8_t horizontal_pin);

void initialize_gpiote();
void configure_buttons(uint8_t *p_buttons, uint8_t button_count);