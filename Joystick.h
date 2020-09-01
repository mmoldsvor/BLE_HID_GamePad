#include "nrf.h"

void initialize_saadc(uint16_t *saadc_buffer_ptr, uint8_t resolution);

void configure_stick(uint8_t stick_index, uint8_t vertical_pin, uint8_t horizontal_pin);
