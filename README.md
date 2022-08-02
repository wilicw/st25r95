# ST25R95/CR95HF SPI Driver

Currently, only supported ISO/IEC 14443A protocol.

## Get Started

### Define a reader handler

```c
volatile st25r95_handle reader_handler;
```

### Implement the board's driver code

The following example is for STM32.

```c
// IRQ_IN pulse
void reader_irq_pulse() {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
  HAL_Delay(8);
}

// For SPI SS pin
void reader_nss(uint8_t enable) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, enable ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

// SPI Tx function
void reader_tx(uint8_t *data, size_t len) {
  HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
}

// SPI Rx function
void reader_rx(uint8_t *data, size_t len) {
  HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
}

// Interrupt callback function
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
  if (pin == GPIO_YOUR_IRQ_OUT_INTERRUPT_PIN) {
    reader_handler.irq_flag = 1;
  }    
}

// Define a callback function when tag detected
void st25_card_callback(uint8_t* uid) {
  debug_print(uid, 10);
}
```

### Main function

```c
#include "st25r95.h"

int main() {
  /* Setup all protocol parameters */
  reader_handler.protocol = ST25_PROTOCOL_14443A;
  reader_handler.tx_speed = ST25_26K_106K;
  reader_handler.rx_speed = ST25_26K_106K;
  reader_handler.timerw = 0x58;
  reader_handler.ARC = 0xD1;
  reader_handler.irq_flag = 0;

  /* Bind BSP Functions */
  reader_handler.nss = reader_nss;
  reader_handler.tx = reader_tx;
  reader_handler.rx = reader_rx;
  reader_handler.irq_pulse = reader_irq_pulse;
  
  /* Bind the callback function when tag detected */
  reader_handler.callback = st25_card_callback;

  /* Init the board */
  st25r95_init(&reader_handler);
  /* Calibration */
  st25r95_calibrate(&reader_handler);
  /* Switches the board into low consumption mode */
  st25r95_idle(&reader_handler);
  
  while(1) {
    st25r95_service(&reader_handler);
  }
  
  return 0;
}
```

## LICENSE

[Apache License 2.0](https://github.com/wilicw/st25r95/blob/main/LICENSE)

