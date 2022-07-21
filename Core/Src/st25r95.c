#include "st25r95.h"

/*
 * Some BSP functions
 */

void __attribute__((weak)) st25r95_delay(uint32_t time) {}

void __attribute__((weak)) st25r95_nss(uint8_t enable) {}

void __attribute__((weak)) st25r95_tx(uint8_t *data, size_t len) {}

void __attribute__((weak)) st25r95_rx(uint8_t *data, size_t len) {}

void __attribute__((weak)) st25r95_irq_pulse() {}

static const uint8_t tx_read = ST25_READ;
static const uint8_t tx_reset = ST25_RESET;

uint8_t *st25r95_response() {
  static uint8_t rx_data[256];
  st25r95_nss(1);
  st25r95_tx(&tx_read, 1);
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
  st25r95_tx(&tx_reset, 1);
  st25r95_nss(0);
}

st25r95_status st25r95_IDN() {
  static const uint8_t idn_tx[] = {ST25_SEND, ST25_IDN, 0};
  st25r95_nss(1);
  st25r95_tx(idn_tx, sizeof(idn_tx));
  st25r95_nss(0);
  st25r95_delay(10);
  uint8_t *res = st25r95_response();
  if (res[0] != 0) return ST25_ERROR;
  if (!(res[2] == 'N' && res[3] == 'F' && res[4] == 'C')) return ST25_INVALID_DEVICE;
  return ST25_OK;
}