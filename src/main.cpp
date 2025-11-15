#include <Arduino.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "logic_in.pio.h"  // auto-generated header


void pio_init(PIO pio, uint8_t sm, uint8_t pin_base, uint8_t pin_count, float freq_div){

    uint16_t prg_instr = pio_encode_in(pio_pins, pin_count);
    struct pio_program prg = {
        .instructions = &prg_instr,
        .length=1,
        .origin=-1
    };

    // add program offset
    uint offset = pio_add_program(pio, &prg);
    // configure the sm
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, pin_base);
    sm_config_set_wrap(&c, offset, offset);
    sm_config_set_clkdiv(&c, freq_div);

    // autopush from ISR to RX_FIFO
    sm_config_set_in_shift(&c, false, true, pin_count);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
    
}

void setup() {
    Serial.begin(921600);
    while(!Serial);

    PIO pio_0 = pio0;
    const uint8_t cap_pin_base = 0;
    const uint8_t cap_pin_count = 4;
    const uint8_t sm = pio_claim_unused_sm(pio_0, true);
    pio_init(pio_0, sm, cap_pin_base, cap_pin_count, 10.0f);

    while(true) {
    // Read data from FIFO (each word = 32 samples)
    uint32_t last_word = 0;
    if (!pio_sm_is_rx_fifo_empty(pio_0, sm)) {
        uint32_t word = pio_sm_get(pio_0, sm);
        for (uint8_t i = 0; i < 8; i++)
        {
            uint8_t bit = (word>>i)&0x01;
            Serial.print(">");
            Serial.print("ch");
            Serial.print(i);
            Serial.print(":");
            Serial.print(bit);
            Serial.print(",");
            Serial.print("ts:");
            Serial.print(micros());
            Serial.println();
            delay(100);
        }
        // Serial.print((uint8_t)(word&0x000000ff), BIN);
        // Serial.print(",");
        // Serial.println(micros());
    }
}
  
}

void loop() {

}