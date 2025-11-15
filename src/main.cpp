#include <Arduino.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "logic_in.pio.h"  // auto-generated header

#define MAX_CHANNEL_NUM 8

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
    sm_config_set_in_shift(&c, false, true, (pin_count>MAX_CHANNEL_NUM? MAX_CHANNEL_NUM: pin_count));
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
    
}

void print_channels (PIO pio, uint8_t sm )
{
    while(true) {
    // Read data from FIFO (each word = 32 samples)
    if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
        uint32_t word = pio_sm_get(pio, sm);
        for (uint8_t i = 0; i < MAX_CHANNEL_NUM; i++){
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
        }
    }
}

void setup() {
    Serial.begin(921600);
    while(!Serial);

    // inint a PIO instance of PIO-0 
    PIO pio_0 = pio0;
    // define the pin base [starting GPIO Num] & the count [how many GPIO after the first one to be used]
    // Max channel number is 8 for now.
    const uint8_t cap_pin_base = 0;
    const uint8_t cap_pin_count = 8;
    const uint8_t sm = pio_claim_unused_sm(pio_0, true);
    pio_init(pio_0, sm, cap_pin_base, cap_pin_count, 10.0f);

    print_channels(pio_0, sm);
  
}

void loop() {

}