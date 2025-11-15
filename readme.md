### RP2040 As Logic Analyzer - V1
---
This is an open source project that utilising the RP2040 as a basic logic analyzer, with a potential for further development. The exact used board is the Raspberry Pi Pico - with RP2040 chipset, but any other dev board with the same chipset will do. This project is planned to be the starting point to the AI-Assisted Logic Analyzer, in which the part that would handle the process of running a signal classifaier model will be implemented later.

Created By: Adam Momen | Abdulfatah Alturshani

### Overview
--- 
The main aim from this project is the use of the RP2040 PIO feature, and use it at its full potential to sample a given digital signal with a frequency up to 62.5 MHz (at full clock speed `125 MHz`). The PIO (Programmble Input/ Output) controller is a key feature that enables this chip to reach such high speed signaling. But understanding the low level mechanism and how this contoller is functioning is a little tricky. However using the Arduino with Pico SDK, it was easier to make this work.

![alt text](<Screenshot 2025-11-11 at 8.43.15 PM.png>)

### PIO Block (Programmable Input/ Output):
--- 
In short it is a programmable controller that could be automated. The RP2040 has 2 PIO blocks (0, 1). Each has 4 state machines. each state machine consists of In/Out Shift Registers, 2 Scratch Registers (X,Y), Program Counter, and Clock Div.

![alt text](<Screenshot 2025-11-11 at 8.44.52 PM.png>)


### PIO Programs
---
Each PIO unite has its own Instruction Memory which could hold up to 32 instructions, and read from 4 ports at the time. 

The PIO has a total of nine instructions: `JMP, WAIT, IN, OUT, PUSH, PULL, MOV, IRQ, and SET`. See Section 3.4 for details on these
instructions.

The PIO assembler is included with the SDK, and is called `pioasm`. This program processes a PIO assembly input text file,
which may contain multiple programs, and writes out the assembled programs ready for use.

```c
8 .program squarewave
9  set pindirs, 1 ; Set pin to output
10 again:
11 set pins, 1 [1] ; Drive pin high and then delay for one cycle
12 set pins, 0 ; Drive pin low
13 jmp again ; Set PC to label `again`
```
---
### Program Flow
---
1. Select the PIO block (pio0/ pio1)
2. State Machine variable, to hold the value of the active state machine [0,1,2,3]
3. Set in pin_base [GPIO_NUM] starting from [GPIO_0] & the `pin_count`, how many pins to use after the `pin_base`.
   For example, `pin_base = 0;` and `pin_count = 4`, this means the PIO would scan the GPIOs from [GPIO_0], [GPIO_1], [GPIO_2], and [GPIO_3].
4. Program is in direct machine code instead of the PIOASM, to avoid using the assembler. Here is an online PIOASM tool to compile the code directly.

* [pioasm Online | Wokwi](https://wokwi.com/tools/pioasm)

5. Program details, most important [length of the instructions, origin (usually -1)]
6. `pio_gpio_init` → to link the GPIO to be used with PIO.
7. Configurations are little advanced, next chapter.
8. Load the program to the Instruction memory, via the `pio_add_program` function. offset of the memory address will be returned.
9. Last init the PIO, State Machine via `pio_sm_init  & pio_sm_set_enabled.`

### Simple Logic Analyzer (1-bit stream)
---
In this experiment I have written a simple snippet to configure a GPIO to be input tied with PIO_0 to read the income bit stream. at the fastest frequency the sampling rate should reach up to 60MHz (not sure, needed to confirm it later). 
1. `pio_init` function is responible of setting up the IN reading program, which is consists of simply `in pins, pin_count` instruciotn.
2. Call  PIO instance and setup the `pin_base` & `pin_count` to initilize the PIO. Start sampling imedietely upon success PIO init.
3. in the `print_channels` function, read the `rx_fifo` register (32_bit) and store it’s data in.
4. Using the VS_Code extension [Serial Plotter](https://marketplace.visualstudio.com/items?itemName=badlogicgames.serial-plotter), a special serial printing should be used to plot the read bit stream.
5. To stream multiple channels [Pins] at the same time, we use the `rx_fifo` as 8-bit template by reading only the first 8-bits [a bit for each channel] where the `MUXIMUM_CHANNEL_NUM` is 8.

``` C
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
```
5. Further more the `micros()` function is used to transmit the actual time stamps. This will be useful when running my own [pico_logger.py](pico_logger.py) serial plotter.

6. We can use the assembler `PIOASM` to write the needed instructions as below:

```c
// assembly instructions for the PIO
.program logic_in
.wrap_target
    in pins, pin_count            ; sample pins 
.wrap
```
however, as long as we we need only 1 instruction to do the scanning, then the following method `pio_encode_in(pin_base, pin_count)` can just do it!
``` C
uint16_t prg_instr = pio_encode_in(pio_pins, pin_count);
struct pio_program prg = {
    .instructions = &prg_instr,
    .length=1,
    .origin=-1
};
```

### Useful Links:
---
* [RP2040 Technical Manual](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
* [RP2040 SDK Programming Manual{C/C++}](https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008354-DS-1-raspberry-pi-pico-c-sdk.pdf?disposition=inline)