#include "st25r95.h"

/*
 * Some BSP functions
 */

void __attribute__((weak)) st25r95_delay(uint32_t time) {}

void __attribute__((weak)) st25r95_nss(uint8_t enable) {}

void __attribute__((weak)) st25r95_tx(uint8_t *data, size_t len) {}

void __attribute__((weak)) st25r95_rx(uint8_t *data, size_t len) {}

void __attribute__((weak)) st25r95_irq_pulse() {}

volatile static uint8_t tx_buffer[256];
volatile static size_t tx_len;

void st25r95_spi_tx() {
  st25r95_tx(tx_buffer, tx_len);
  tx_len = 0;
}

void st25r95_spi_byte(uint8_t data) {
  tx_len = 1;
  tx_buffer[0] = data;
  st25r95_spi_tx();
}

uint8_t *st25r95_response() {
  static uint8_t rx_data[256];
  st25r95_nss(1);
  st25r95_spi_byte(ST25_READ);
  st25r95_rx(rx_data, 1);
  st25r95_rx(rx_data + 1, 1);
  st25r95_rx(rx_data + 2, *(rx_data + 1));
  st25r95_nss(0);
  return rx_data;
}

void st25r95_init() {
  st25r95_reset();
  st25r95_irq_pulse();
}

void st25r95_reset() {
  st25r95_nss(1);
  st25r95_spi_byte(ST25_RESET);
  st25r95_nss(0);
}

st25r95_status_t st25r95_IDN() {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_IDN;
  tx_buffer[tx_len++] = 0;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  st25r95_delay(10);
  uint8_t *res = st25r95_response();
  if (res[0] != 0) return ST25_ERROR;
  if (!(res[2] == 'N' && res[3] == 'F' && res[4] == 'C')) return ST25_INVALID_DEVICE;
  return ST25_OK;
}

st25r95_status_t st25r95_off() {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_PS;
  tx_len++;
  tx_buffer[tx_len++] = ST25_PROTOCOL_OFF;
  tx_buffer[tx_len++] = 0;
  tx_buffer[1] = tx_len - 2;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != 0) return ST25_ERROR;
  return ST25_OK;
}