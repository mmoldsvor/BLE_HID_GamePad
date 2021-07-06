#include "gamepad_driver.h"

static gamepad_init_t gamepad_init;
static int16_t saadc_buffer[4] = {0};

void initialize_gamepad(gamepad_evt_handler_t gamepad_on_evt)
{
  gamepad_init.gamepad_handle = gamepad_on_evt;

  gamepad_init.analog_values.left_stick[0] = 127;
  gamepad_init.analog_values.left_stick[1] = 127;
  gamepad_init.analog_values.right_stick[0] = 127;
  gamepad_init.analog_values.right_stick[1] = 127;
}

void initialize_saadc(uint8_t resolution)
{
  NRF_SAADC->RESOLUTION = resolution << SAADC_RESOLUTION_VAL_Pos;
  NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Over8x;
  
  NRF_SAADC->RESULT.PTR = (uint32_t) &saadc_buffer[0];
  NRF_SAADC->RESULT.MAXCNT = 4;
  
  NRF_PPI->CHEN |= (1 << 0);
  NRF_PPI->CH[0].EEP = (uint32_t) &NRF_SAADC->EVENTS_END;
  NRF_PPI->CH[0].TEP = (uint32_t) &NRF_SAADC->TASKS_START;

  NRF_SAADC->INTENSET = 1 << 1;
  NVIC_EnableIRQ(SAADC_IRQn);
  NVIC_SetPriority(SAADC_IRQn, 7);

  NRF_SAADC->ENABLE = 1;
  NRF_SAADC->TASKS_START = 1;
}

void SAADC_IRQHandler()
{
  if (NRF_SAADC->EVENTS_END)
  {
    NRF_SAADC->EVENTS_END = 0;

    gamepad_evt_t gamepad_evt;

    uint8_t horizontal  = saadc_buffer[0] > 0 ? saadc_buffer[0] : 0;
    uint8_t vertical    = saadc_buffer[1] > 0 ? saadc_buffer[1] : 0;
    if (gamepad_init.analog_values.left_stick[0] != horizontal || gamepad_init.analog_values.left_stick[1] != vertical)
    {
        gamepad_init.analog_values.left_stick[0] = horizontal;
        gamepad_init.analog_values.left_stick[1] = vertical;

        gamepad_evt.evt_type = GAMEPAD_LEFT_STICK;
        gamepad_evt.value[0] = horizontal;
        gamepad_evt.value[1] = vertical;

        gamepad_init.gamepad_handle(gamepad_evt);
    }

    horizontal  = saadc_buffer[2] > 0 ? saadc_buffer[2] : 0;
    vertical    = saadc_buffer[3] > 0 ? saadc_buffer[3] : 0;
    if (gamepad_init.analog_values.right_stick[0] != horizontal || gamepad_init.analog_values.right_stick[1] != vertical)
    {
        gamepad_init.analog_values.right_stick[0] = horizontal;
        gamepad_init.analog_values.right_stick[1] = vertical;

        gamepad_evt.evt_type = GAMEPAD_RIGHT_STICK;
        gamepad_evt.value[0] = horizontal;
        gamepad_evt.value[1] = vertical;

        gamepad_init.gamepad_handle(gamepad_evt);
    }
  }
}

void configure_stick(uint8_t stick_index, uint8_t vertical_pin, uint8_t horizontal_pin)
{

  NRF_SAADC->CH[stick_index].PSELP = vertical_pin << SAADC_CH_PSELP_PSELP_Pos;
  NRF_SAADC->CH[stick_index + 1].PSELP = horizontal_pin << SAADC_CH_PSELP_PSELP_Pos;
  
  uint32_t channel_config = (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) |
                            (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) |
                            (SAADC_CH_CONFIG_GAIN_Gain1_4 << SAADC_CH_CONFIG_GAIN_Pos) | 
                            (SAADC_CH_CONFIG_REFSEL_VDD1_4 << SAADC_CH_CONFIG_REFSEL_Pos) |
                            (SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos) |
                            (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) |
                            (SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos);

  NRF_SAADC->CH[stick_index].CONFIG = channel_config;
  NRF_SAADC->CH[stick_index + 1].CONFIG = channel_config;
}

void initialize_gpiote()
{
    NRF_GPIOTE->INTENSET = (1 << 31);

    NVIC_EnableIRQ(GPIOTE_IRQn);
    NVIC_SetPriority(GPIOTE_IRQn, 6);
}

void configure_buttons(uint8_t *p_buttons, uint8_t button_count)
{
    for (int i = 0; i < button_count; i++)
    {
        NRF_P0->PIN_CNF[*(p_buttons + i)] =     (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
                                                (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                                                (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) |
                                                (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                                                (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);
    }

    gamepad_init.p_buttons = p_buttons;
    gamepad_init.button_count = button_count;
}

void GPIOTE_IRQHandler()
{
    if (NRF_GPIOTE->EVENTS_PORT)
    {
        NRF_P0->LATCH = NRF_P0->LATCH;
        NRF_GPIOTE->EVENTS_PORT = 0;

        for (int i = 0; i < gamepad_init.button_count; i++)
        {
            uint8_t button_pin = *(gamepad_init.p_buttons + i);
            if (NRF_P0->LATCH & (1 << button_pin))
            {
              bool state = NRF_P0->IN & (1 << button_pin);
              NRF_P0->PIN_CNF[button_pin] = (NRF_P0->PIN_CNF[button_pin] & ~(1 << GPIO_PIN_CNF_SENSE_Pos)) | (state << GPIO_PIN_CNF_SENSE_Pos);
            }
        }
        gamepad_evt_t gamepad_evt;
        gamepad_evt.evt_type = GAMEPAD_BUTTONS;
        gamepad_evt.value[0] = 1;
        gamepad_evt.value[1] = 2;

        gamepad_init.gamepad_handle(gamepad_evt);

    }
}