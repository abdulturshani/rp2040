#include <Arduino.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "logic_in.pio.h"  // auto-generated header

PIO pio = pio0;
uint sm = 0;
const uint INPUT_PIN = 2;  // GPIO to capture

void setup() {
  Serial.begin(115200);
  while(!Serial);

  // Prepare the PIO and pin
  pio_gpio_init(pio, INPUT_PIN);
  pio_sm_config c = pio_get_default_sm_config();
  
  // Set up PIO program parameters
  sm_config_set_in_pins(&c, INPUT_PIN);
  sm_config_set_in_shift(&c, true, 32, false); // autopush every 32 bits
  sm_config_set_clkdiv(&c, 100.0f);              // sample at full speed
  
  // Load and start the program
  uint offset = pio_add_program(pio, &logic_in_program);
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true);
}

void loop() {
  // Read data from FIFO (each word = 32 samples)
  if (!pio_sm_is_rx_fifo_empty(pio, sm)) {
    uint32_t word = pio_sm_get(pio, sm);
    Serial.print(">");
    Serial.print("var1:");
    Serial.print((word/(0xffffffff)));
    Serial.print(",");
    Serial.println(); // Writes \r\n
    delay(10);
  }
}