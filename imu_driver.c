void imu_begin(void)
{
    ret_code_t err_code;
      
    shtpData[0] = 0xF9;
    shtpData[1] = 0;
    nrf_delay_ms(1000);

    send_packet(2, 2);
    receive_packet();
}

void send_packet(uint8_t channel_number, uint8_t data_length)
{
    ret_code_t err_code;

    uint8_t packet_length = data_length + 4;

    uint8_t write_register[packet_length];

    write_register[0] = packet_length & 0xFF;
    write_register[1] = packet_length >> 8;
    write_register[2] = channel_number;
    write_register[3] = m_sequence_number[channel_number]++;
    
    for(uint8_t i = 0; i < data_length; i++)
    {
        write_register[i + 4] = shtpData[i];
    }

    m_xfer_done = false;
    err_code = nrf_drv_twi_tx(&m_twi, TWI_ADDRESS, write_register, packet_length, false);
    while (m_xfer_done == false);
    
    APP_ERROR_CHECK(err_code);
}

void get_data(uint16_t data_length)
{   
    ret_code_t err_code;
    uint8_t data_spot = 0;

    uint16_t bytes_remaining = data_length;
    while (bytes_remaining > 0)
    {
        while(m_interrupt_received == false);
        m_interrupt_received = false;

        uint16_t number_of_bytes_to_read = bytes_remaining;

        if (number_of_bytes_to_read > (I2C_BUFFER_LENGTH - 4))
            number_of_bytes_to_read = I2C_BUFFER_LENGTH - 4;

        uint8_t packet_data[number_of_bytes_to_read + 4];
        m_xfer_done = false;
        err_code = nrf_drv_twi_rx(&m_twi, TWI_ADDRESS, &packet_data, number_of_bytes_to_read + 4);
        while (m_xfer_done == false);
        APP_ERROR_CHECK(err_code);

        for(uint16_t i = 0; i < number_of_bytes_to_read; i++)
        {
            if (data_spot < MAX_PACKET_SIZE)
            {
                shtpData[data_spot++] = packet_data[i+4];
            }
        }
        bytes_remaining -= number_of_bytes_to_read;
    }
}

void receive_packet(void)
{
    ret_code_t err_code;

    uint8_t header_data[4];
    
    while(m_interrupt_received == false);
    m_interrupt_received = false;

    m_xfer_done = false;
    err_code = nrf_drv_twi_rx(&m_twi, TWI_ADDRESS, &header_data, 4);
    while (m_xfer_done == false);

    uint8_t packet_lsb = header_data[0];
    uint8_t packet_msb = header_data[1];
    uint8_t channel_number = header_data[2];
    uint8_t sequence_number = header_data[3];
    
    uint16_t data_length = (((uint16_t)packet_msb) << 8) | ((uint16_t)packet_lsb);

    if (data_length == 0)
        return;

    data_length -= 4;
    
    get_data(data_length);
}

void soft_reset(void)
{
    nrf_gpio_pin_clear(14);
    nrf_delay_ms(1000);
    nrf_gpio_pin_set(14);
    nrf_delay_ms(1000);

    shtpData[0] = 1;

    while(m_interrupt_received == false);
    m_interrupt_received = false;
    send_packet(1, 1);

    receive_packet();
    receive_packet();
}

void enable_rotation_vector()
{
    uint16_t microsBetweenReports = 0x7A120;
  
    shtpData[0] = 0xFD;	 //Set feature command. Reference page 55
    shtpData[1] = 0x05;							   //Feature Report ID. 0x01 = Accelerometer, 0x05 = Rotation vector
    shtpData[2] = 0;								   //Feature flags
    shtpData[3] = 0;								   //Change sensitivity (LSB)
    shtpData[4] = 0;								   //Change sensitivity (MSB)
    shtpData[5] = (microsBetweenReports >> 0) & 0xFF;  //Report interval (LSB) in microseconds. 0x7A120 = 500ms
    shtpData[6] = (microsBetweenReports >> 8) & 0xFF;  //Report interval
    shtpData[7] = (microsBetweenReports >> 16) & 0xFF; //Report interval
    shtpData[8] = (microsBetweenReports >> 24) & 0xFF; //Report interval (MSB)
    shtpData[9] = 0;								   //Batch Interval (LSB)
    shtpData[10] = 0;								   //Batch Interval
    shtpData[11] = 0;								   //Batch Interval
    shtpData[12] = 0;								   //Batch Interval (MSB)
    shtpData[13] = 0;	   //Sensor-specific config (LSB)
    shtpData[14] = 0;	   //Sensor-specific config
    shtpData[15] = 0;	  //Sensor-specific config
    shtpData[16] = 0;	  //Sensor-specific config (MSB)

    //Transmit packet on channel 2, 17 bytes
    send_packet(2, 17);
}

void get_feature()
{
    shtpData[0] = 0xFE;	 //Set feature command. Reference page 55
    shtpData[1] = 0x05;

    nrf_delay_ms(1000);
    send_packet(2, 2);

    receive_packet();

}