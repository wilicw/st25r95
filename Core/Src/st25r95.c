#include "st25r95.h"

/*
 * Some BSP functions
 */

__weak void st25r95_nss(uint8_t enable) {}

__weak void st25r95_tx(uint8_t *data, size_t len) {}

__weak void st25r95_rx(uint8_t *data, size_t len) {}

__weak void st25r95_irq_pulse() {}

volatile static uint8_t tx_buffer[256];
volatile static size_t tx_len;
volatile uint8_t irq_flag = 0;
static st25r95_state_t reader_state = ST25_STATE_NORMAL;
static st25r95_protocol_t reader_protocol;
static uint8_t DACRef = 0x6C;

void st25r95_spi_tx() {
  st25r95_tx(tx_buffer, tx_len);
  tx_len = 0;
}

void st25r95_spi_byte(uint8_t data) {
  tx_len = 1;
  tx_buffer[0] = data;
  st25r95_spi_tx();
}

void st25r95_irq_callback() {
  irq_flag = 1;
}

void st25r95_service(st25_callback callback) {
  if (irq_flag == 1) {
    irq_flag = 0;
    static uint8_t uid[10];
    if (reader_state == ST25_STATE_IDLE) {
      st25r95_init();
      st25r95_IDN();
      switch (reader_protocol) {
        case ST25_PROTOCOL_14443A:
          st25r95_14443A(ST25_26K_106K, ST25_26K_106K);
          st25r95_write_timerw(0x58);
          st25r95_write_ARC(1, 0xD1);

          if (st25r95_14443A_detect(uid)) {
            callback(uid);
          }

          st25r95_idle();
          break;
        default:
          break;
      }
    }
    irq_flag = 0;
  }
}

uint8_t *st25r95_response() {
  while (irq_flag == 0);
  irq_flag = 0;
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
  reader_state = ST25_STATE_NORMAL;
}

void st25r95_reset() {
  st25r95_nss(1);
  st25r95_spi_byte(ST25_RESET);
  st25r95_nss(0);
}

st25r95_status_t st25r95_IDN() {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_IDN;
  tx_buffer[2] = 0;
  tx_len = 3;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != 0) return ST25_INVALID_DEVICE;
  if (!(res[2] == 'N' && res[3] == 'F' && res[4] == 'C')) return ST25_INVALID_DEVICE;
  return ST25_OK;
}

st25r95_status_t st25r95_off() {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_PS;
  tx_buffer[2] = 2;
  tx_buffer[3] = ST25_PROTOCOL_OFF;
  tx_buffer[4] = 0;
  tx_len = 5;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_14443A(st25r95_rate_t tx_rate, st25r95_rate_t rx_rate) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_PS;
  tx_buffer[2] = 2;
  tx_buffer[3] = ST25_PROTOCOL_14443A;
  tx_buffer[4] = tx_rate << 6 | rx_rate << 4;
  tx_len = 5;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  reader_protocol = ST25_PROTOCOL_14443A;

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_read_reg(uint8_t address, uint8_t *data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_RR;
  tx_buffer[2] = 0x3;
  tx_buffer[3] = address;
  tx_buffer[4] = 0x1;
  tx_buffer[5] = 0;
  tx_len = 6;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] == ST25_OK) *data = res[2];
  return res[0];
}

st25r95_status_t st25r95_write_timerw(uint8_t data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_WR;
  tx_buffer[2] = 0x4;
  tx_buffer[3] = 0x3A;
  tx_buffer[4] = 0x0;
  tx_buffer[5] = data;
  tx_buffer[6] = 0x4;
  tx_len = 7;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_write_ARC_index(uint8_t index) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_WR;
  tx_buffer[2] = 0x3;
  tx_buffer[3] = 0x68;
  tx_buffer[4] = 0x0;
  tx_buffer[5] = index;
  tx_len = 6;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_write_ARC(uint8_t index, uint8_t data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_WR;
  tx_buffer[2] = 0x4;
  tx_buffer[3] = 0x68;
  tx_buffer[4] = 0x1;
  tx_buffer[5] = index;
  tx_buffer[6] = data;
  tx_len = 7;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  return res[0];
}

st25r95_status_t st25r95_echo() {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_ECHO;
  tx_len = 2;

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
    if (data[5] & 0x80) {
    }
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
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_SR;
  tx_buffer[2] = 0x2;
  tx_buffer[3] = REQA;
  tx_buffer[4] = 7; // REQA is a 7bits command.
  tx_len = 5;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);

}

const static uint8_t cascade_level[] = {CL_1, CL_2, CL_3};

void st25r95_14443A_ANTICOLLISION(uint8_t level, uint8_t *data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_SR;
  tx_buffer[2] = 0x03;
  tx_buffer[3] = cascade_level[level];
  tx_buffer[4] = 0x20; // NVB
  tx_buffer[5] = 0x08;
  tx_len = 6;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);
}

void st25r95_14443A_select(uint8_t level, uint8_t *data, uint8_t uid0, uint8_t uid1, uint8_t uid2, uint8_t uid3) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_SR;
  tx_buffer[2] = 0x08;
  tx_buffer[3] = cascade_level[level];
  tx_buffer[4] = 0x70;
  tx_buffer[5] = uid0;
  tx_buffer[6] = uid1;
  tx_buffer[7] = uid2;
  tx_buffer[8] = uid3;
  tx_buffer[9] = uid0 ^ uid1 ^ uid2 ^ uid3;
  tx_buffer[10] = tx_flag_AppendCRC | 8;
  tx_len = 11;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  uint8_t *res = st25r95_response();
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);
}

st25r95_status_t st25r95_idle() {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_IDLE;
  tx_buffer[2] = 0x0E;
  tx_buffer[3] = ST25_WU_SRC_TagDetection;
  tx_buffer[4] = ST25_EC_TagDetection >> 8;
  tx_buffer[5] = ST25_EC_TagDetection & 0xFF;
  tx_buffer[6] = ST25_WU_CTRL_TagDetection >> 8;
  tx_buffer[7] = ST25_WU_CTRL_TagDetection & 0xFF;
  tx_buffer[8] = ST25_LEAVE_CTRL_TagDetection >> 8;
  tx_buffer[9] = ST25_LEAVE_CTRL_TagDetection & 0xFF;
  tx_buffer[10] = 0x20;
  tx_buffer[11] = 0x60;
  tx_buffer[12] = 0x60;
  tx_buffer[13] = DACRef - 8;
  tx_buffer[14] = DACRef + 8;
  tx_buffer[15] = 0x3F;
  tx_buffer[16] = 0x01;
  tx_len = 17;

  st25r95_nss(1);
  st25r95_spi_tx();
  st25r95_nss(0);

  reader_state = ST25_STATE_IDLE;

//  uint8_t *res = st25r95_response();
//  return res[0];
}

void st25r95_calibrate() {
  static uint8_t calibrate_data[] = {
    ST25_SEND,
    ST25_IDLE,
    0x0E,
    ST25_WU_SRC_TagDetection | ST25_WU_SRC_Timeout,
    ST25_EC_TagDetectorCalibration >> 8,
    ST25_EC_TagDetectorCalibration & 0xFF,
    ST25_WU_CTRL_TagDetectorCalibration >> 8,
    ST25_WU_CTRL_TagDetectorCalibration & 0xFF,
    ST25_LEAVE_CTRL_TagDetectorCalibration >> 8,
    ST25_LEAVE_CTRL_TagDetectorCalibration & 0xFF,
    0x20,
    0x60,
    0x60,
    0x00,
    0x00,
    0x3F,
    0x01,
  };

  static uint8_t *res;

  for (uint8_t i = 0; i < 9; i++) {
    tx_len = 17;

    memcpy(tx_buffer, calibrate_data, sizeof(calibrate_data));
    st25r95_nss(1);
    st25r95_spi_tx();
    st25r95_nss(0);

    res = st25r95_response();
    if (res[0] == 0x00 && res[1] == 0x01) {
      if (res[2] == 0x02) {
        switch (i - 1) {
          case 0:
            calibrate_data[14] = 0xFC;
            break;
          case 1:
            break;
          case 2:
            calibrate_data[14] += 0x40;
            break;
          case 3:
            calibrate_data[14] += 0x20;
            break;
          case 4:
            calibrate_data[14] += 0x10;
            break;
          case 5:
            calibrate_data[14] += 0x08;
            break;
          case 6:
            calibrate_data[14] += 0x04;
            break;
          case 7:
            break;
        }
      } else if (res[2] == 0x01) {
        switch (i - 1) {
          case 0:
            break;
          case 1:
            calibrate_data[14] -= 0x80;
            break;
          case 2:
            calibrate_data[14] -= 0x40;
            break;
          case 3:
            calibrate_data[14] -= 0x20;
            break;
          case 4:
            calibrate_data[14] -= 0x10;
            break;
          case 5:
            calibrate_data[14] -= 0x08;
            break;
          case 6:
            calibrate_data[14] -= 0x04;
            break;
          case 7:
            calibrate_data[14] -= 0x04;
            break;
        }
      }
    }
  }

  DACRef = calibrate_data[14];
}