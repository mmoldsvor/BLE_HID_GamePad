#include "gamepad_driver.h"

static gamepad_evt_handler_t gamepad_evt_handler_instance;
static int16_t saadc_buffer[4] = {0};

void initialize_gamepad(gamepad_evt_handler_t gamepad_evt_handler)
{
  gamepad_evt_handler_instance = gamepad_evt_handler;
}

void initialize_saadc(uint8_t resolution)
{
  NRF_SAADC->RESOLUTION = resolution << SAADC_RESOLUTION_VAL_Pos;
  
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
    gamepad_evt_t gamepad_evt;

    gamepad_evt.evt_type = GAMEPAD_LEFT_STICK;
    gamepad_evt.evt_value[0] = fmax(0, saadc_buffer[0]);
    gamepad_evt.evt_value[1] = fmax(0, saadc_buffer[1]);

    gamepad_evt_handler_instance(gamepad_evt);

    gamepad_evt.evt_type = GAMEPAD_RIGHT_STICK;
    gamepad_evt.evt_value[0] = fmax(0, saadc_buffer[2]);
    gamepad_evt.evt_value[1] = fmax(0, saadc_buffer[3]);

    gamepad_evt_handler_instance(gamepad_evt);

    NRF_SAADC->EVENTS_END = 0;
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
                            (SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos);

  NRF_SAADC->CH[stick_index].CONFIG = channel_config;
  NRF_SAADC->CH[stick_index + 1].CONFIG = channel_config;
}