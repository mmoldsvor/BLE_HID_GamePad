void imu_begin(void);

void send_packet(uint8_t channel_number, uint8_t data_length);

void get_data(uint16_t data_length);

void receive_packet(void);

void soft_reset(void);

void enable_rotation_vector(void);

void get_feature(void);