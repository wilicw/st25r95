# ST25R95/CR95HF SPI Driver

Currently, only supported CF95HF.

## Get Started

### Implement the board's driver code

The following example is for STM32.

```c
// Delay
void st25r95_delay(uint32_t time) {
  HAL_Delay(time);
}

// IRQ_IN pulse
void st25r95_irq_pulse() {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
  HAL_Delay(10);
}

// For SPI SS pin
void st25r95_nss(uint8_t enable) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, enable ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

// SPI Tx function
void st25r95_tx(uint8_t *data, size_t len) {
  HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
}

// SPI Rx function
void st25r95_rx(uint8_t *data, size_t len) {
  HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
}
```

### Main function

```c
#include "st25r95.h"

int main() {
  // Init NFC reader.
  st25r95_init();
  // Check the reader information.
  st25r95_IDN();
  // Set protocol to ISO/IEC 14443A
  st25r95_14443A(ST25_26K_106K, ST25_26K_106K);
  // Set Analog register value for Modulation Index and Receiver Gain.
  st25r95_write_ARC(1, 0xD3);
  // Fine-tuning the Timer Window (TimerW) register.
  st25r95_write_timerw(0x58);
  
  // Define a 10byte uid variable.
  uint8_t uid[10];
  while(1) {
    if (st25r95_14443A_detect(uid)) {
      debug_print(uid, 10);
    }
  }
  
  return 0;
}
```

## LICENSE

[Apache License 2.0](https://github.com/wilicw/st25r95/blob/main/LICENSE)

