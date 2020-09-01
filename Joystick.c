#include "Joystick.h"

void initialize_saadc(uint16_t *saadc_buffer_ptr, uint8_t resolution)
{
  NRF_SAADC->RESOLUTION = resolution << SAADC_RESOLUTION_VAL_Pos;
  
  NRF_SAADC->RESULT.PTR = (uint32_t *) saadc_buffer_ptr;
  NRF_SAADC->RESULT.MAXCNT = 2;
  
  NRF_PPI->CHEN |= (1 << 0);
  NRF_PPI->CH[0].EEP = (uint32_t) &NRF_SAADC->EVENTS_END;
  NRF_PPI->CH[0].TEP = (uint32_t) &NRF_SAADC->TASKS_START;

  NRF_SAADC->ENABLE = 1;
  NRF_SAADC->TASKS_START = 1;
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