#include QMK_KEYBOARD_H

static void sr_init(void) {
    gpio_set_pin_output(SHIFT_REG_PL);
    gpio_set_pin_output(SHIFT_REG_CLK);
    gpio_set_pin_input(SHIFT_REG_DATA);
    gpio_write_pin_high(SHIFT_REG_PL);
    gpio_write_pin_low(SHIFT_REG_CLK);
}

static uint8_t sr_read(void) {
    gpio_write_pin_low(SHIFT_REG_PL);
    __asm__ volatile("nop\nnop\nnop\nnop");
    gpio_write_pin_high(SHIFT_REG_PL);

    uint8_t data = 0;
    for (int i = 7; i >= 0; i--) {
        if (gpio_read_pin(SHIFT_REG_DATA)) data |= (1u << i);
        gpio_write_pin_high(SHIFT_REG_CLK);
        __asm__ volatile("nop\nnop\nnop\nnop");
        gpio_write_pin_low(SHIFT_REG_CLK);
        __asm__ volatile("nop\nnop\nnop\nnop");
    }
    return data;
}

static uint8_t enc_state[NUM_ENCODERS];

static const int8_t gray_lut[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0,
};

static void poll_encoders(void) {
    uint8_t sr = sr_read();
    for (uint8_t e = 0; e < NUM_ENCODERS; e++) {
        uint8_t raw    = (sr >> (6 - e * 2)) & 0x03;
        uint8_t new_st = ((raw >> 1) & 1) << 1 | (raw & 1);
        uint8_t old_st = enc_state[e];
        if (new_st != old_st) {
            int8_t dir = gray_lut[(old_st << 2) | new_st];
            if (dir ==  1) encoder_queue_event(e, true);
            if (dir == -1) encoder_queue_event(e, false);
            enc_state[e] = new_st;
        }
    }
}

void matrix_init_custom(void) {
    const pin_t rows[] = MATRIX_ROW_PINS;
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        gpio_set_pin_output(rows[r]);
        gpio_write_pin_high(rows[r]);
    }
    sr_init();

    uint8_t sr = sr_read();
    for (uint8_t e = 0; e < NUM_ENCODERS; e++) {
        uint8_t raw = (sr >> (6 - e * 2)) & 0x03;
        enc_state[e] = ((raw >> 1) & 1) << 1 | (raw & 1);
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;
    const pin_t rows[] = MATRIX_ROW_PINS;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        gpio_write_pin_low(rows[row]);
        wait_us(2);

        uint8_t sr   = sr_read();
        matrix_row_t cols = (~sr) & 0x0F;

        if (current_matrix[row] != cols) {
            current_matrix[row] = cols;
            changed = true;
        }

        gpio_write_pin_high(rows[row]);
        wait_us(1);
    }

    poll_encoders();
    return changed;
}
