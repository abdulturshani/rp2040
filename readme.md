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
3. `LED_PIN = 6,` GPIO 6.
4. Program is in direct machine code instead of the PIOASM, to avoid using the assembler. Here is an online PIOASM tool to compile the code directly.

* [pioasm Online | Wokwi](https://wokwi.com/tools/pioasm)

5. Program details, most important [length of the instructions, origin (usually -1)]
2. `pio_gpio_init` → to link the GPIO to be used with PIO.
3. Configurations are little advanced, next chapter.
4. Load the program to the Instruction memory, via the `pio_add_program` function. offset of the memory address will be returned.
5. Last init the PIO, State Machine via `pio_sm_init  & pio_sm_set_enabled.`

### Simple Logic Analyzer (1-bit stream)
---
In this experiment I have written a simple snippet to configure a GPIO to be input tied with PIO_0 to read the income bit stream. at the fastest frequency the sampling rate should reach up to 60MHz (not sure, needed to confirm it later). 
1. Usual initialization procedure.
2. Configure the INPUT_PIN as PIO input.
3. in the `loop()` function, read the `rx_fifo` register (32_bit) and store it’s data in.
4. Using the VS_Code extension [Serial Plotter](https://marketplace.visualstudio.com/items?itemName=badlogicgames.serial-plotter), a special serial printing should be used to plot the read bit stream.

``` C
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
```
5. Using the online `PIOASM`, the assembly code was automatically generated.

```c
// assembly instructions for the PIO
.program logic_in
    set pindirs, 0        ; all pins input (assumes 1 pin)
loop:
    in pins, 1            ; sample one bit
    jmp loop
```

``` C
static const uint16_t logic_in_program_instructions[] = {
            //     .wrap_target
    0xe080, //  0: set    pindirs, 0                 
    0x4001, //  1: in     pins, 1                    
    0x0001, //  2: jmp    1                          
            //     .wrap
};
```

### PIO + DMA (Auto Buffering)


### Links
---
[RP2040 Technical Manual](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)