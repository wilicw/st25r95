#include "st25r95.h"

/*
 * Some BSP functions
 */

__weak void st25r95_delay(uint32_t time) {}

__weak void st25r95_nss(uint8_t enable) {}

__weak void st25r95_tx(uint8_t *data, size_t len) {}

__weak void st25r95_rx(uint8_t *data, size_t len) {}

__weak void st25r95_irq_pulse() {}

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
  st25r95_delay(1);
  static uint8_t rx_data[256];
  st25r95_nss(1);
  st25r95_spi_byte(ST25_READ);
  st25r95_rx(rx_data, 1);
  if (rx_data[0] == ST25_ECHO) {
    st25r95_nss(0);
    return rx_data;
  }
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
  if (res[0] != 0) return ST25_INVALID_DEVICE;
  if (!(res[2] == 'N' && res[3] == 'F' && res[4] == 'C')) return ST25_INVALID_DEVICE;
  return ST25_OK;
}

st25r95_status_t st25r95_off() {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_PS;
  tx_len++;
  tx_buffer[tx_len++] = ST25_PROTOCOL_OFF;
  tx_buffer[tx_len++] = 0;
  tx_buffer[2] = tx_len - 3;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_14443A(st25r95_rate_t tx_rate, st25r95_rate_t rx_rate) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_PS;
  tx_len++;
  tx_buffer[tx_len++] = ST25_PROTOCOL_14443A;
  tx_buffer[tx_len++] = tx_rate << 6 | rx_rate << 4;
  tx_buffer[2] = tx_len - 3;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_read_reg(uint8_t address, uint8_t *data) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_RR;
  tx_buffer[tx_len++] = 0x3;
  tx_buffer[tx_len++] = address;
  tx_buffer[tx_len++] = 0x1;
  tx_buffer[tx_len++] = 0;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] == ST25_OK) *data = res[2];
  return res[0];
}

st25r95_status_t st25r95_write_timerw(uint8_t data) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_WR;
  tx_buffer[tx_len++] = 0x4;
  tx_buffer[tx_len++] = 0x3A;
  tx_buffer[tx_len++] = 0x0;
  tx_buffer[tx_len++] = data;
  tx_buffer[tx_len++] = 0x4;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_write_ARC_index(uint8_t index) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_WR;
  tx_buffer[tx_len++] = 0x3;
  tx_buffer[tx_len++] = 0x68;
  tx_buffer[tx_len++] = 0x0;
  tx_buffer[tx_len++] = index;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_write_ARC(uint8_t index, uint8_t data) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_WR;
  tx_buffer[tx_len++] = 0x4;
  tx_buffer[tx_len++] = 0x68;
  tx_buffer[tx_len++] = 0x1;
  tx_buffer[tx_len++] = index;
  tx_buffer[tx_len++] = data;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_echo() {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_ECHO;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

uint8_t st25r95_14443A_detect(uint8_t *ret_uid) {
  uint8_t data[10] = {0xff};
  st25r95_14443A_REQA(data);
  if (data[0] == 0xff) return 0;
  if (data[0] & 0b00100000) {
    return 0;
  }

  uint8_t UID[10];
  uint8_t UID_size = data[0] >> 6;

  for (uint8_t i = 0; i < UID_size + 1; i++) {
    data[0] = 0xff;
    st25r95_14443A_ANTICOLLISION(i, data);
    if (data[0] == 0xff) return 0;
    if (data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4]) return 0;
    UID[4 * i + 0] = data[0];
    UID[4 * i + 1] = data[1];
    UID[4 * i + 2] = data[2];
    UID[4 * i + 3] = data[3];
    st25r95_14443A_select(i, data, data[0], data[1], data[2], data[3]);
    if (data[0] & 0x4)
      continue;
    else
      break;
  }

  memset(ret_uid, 0, 10);
  memcpy(ret_uid, UID, 10);
  return 1;
}

void st25r95_14443A_REQA(uint8_t *data) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_SR;
  tx_buffer[tx_len++] = 0x2;
  tx_buffer[tx_len++] = REQA;
  tx_buffer[tx_len++] = 7; // REQA is a 7bits command.

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);

}

const static uint8_t cascade_level[] = {CL_1, CL_2, CL_3};

void st25r95_14443A_ANTICOLLISION(uint8_t level, uint8_t *data) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_SR;
  tx_buffer[tx_len++] = 0x03;
  tx_buffer[tx_len++] = cascade_level[level];
  tx_buffer[tx_len++] = 0x20; // NVB
  tx_buffer[tx_len++] = 0x08;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);
}

void st25r95_14443A_select(uint8_t level, uint8_t *data, uint8_t uid0, uint8_t uid1, uint8_t uid2, uint8_t uid3) {
  tx_len = 0;
  tx_buffer[tx_len++] = ST25_SEND;
  tx_buffer[tx_len++] = ST25_SR;
  tx_buffer[tx_len++] = 0x08;
  tx_buffer[tx_len++] = cascade_level[level];
  tx_buffer[tx_len++] = 0x70;
  tx_buffer[tx_len++] = uid0;
  tx_buffer[tx_len++] = uid1;
  tx_buffer[tx_len++] = uid2;
  tx_buffer[tx_len++] = uid3;
  tx_buffer[tx_len++] = uid0 ^ uid1 ^ uid2 ^ uid3;
  tx_buffer[tx_len++] = tx_flag_AppendCRC | 8;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);
}
