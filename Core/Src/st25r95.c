#include "st25r95.h"

volatile static uint8_t tx_buffer[256];
volatile static size_t tx_len;

void st25r95_spi_tx(st25r95_handle *handler) {
  handler->tx(tx_buffer, tx_len);
  tx_len = 0;
}

void st25r95_spi_byte(st25r95_handle *handler, uint8_t data) {
  tx_len = 1;
  tx_buffer[0] = data;
  st25r95_spi_tx(handler);
}

void st25r95_service(st25r95_handle *handler) {
  if (handler->irq_flag == 1) {
    handler->irq_flag = 0;
    if (handler->state == ST25_STATE_IDLE) {
      st25r95_init(handler);
      if (st25r95_14443A_detect(handler)) {
        handler->callback(handler->uid);
      }
      st25r95_idle(handler);
    }
  }
}

uint8_t *st25r95_response(st25r95_handle *handler) {
  while (handler->irq_flag == 0);
  handler->irq_flag = 0;
  static uint8_t rx_data[256];
  handler->nss(1);
  st25r95_spi_byte(handler, ST25_READ);
  handler->rx(rx_data, 1);
  if (rx_data[0] == ST25_ECHO) {
    handler->nss(0);
    return rx_data;
  }
  handler->rx(rx_data + 1, 1);
  handler->rx(rx_data + 2, *(rx_data + 1));
  handler->nss(0);
  return rx_data;
}

void st25r95_init(st25r95_handle *handler) {
  st25r95_reset(handler);
  handler->irq_pulse();
  handler->state = ST25_STATE_NORMAL;
  switch (handler->protocol) {
    case ST25_PROTOCOL_14443A:
      st25r95_14443A(handler);
      st25r95_write_timerw(handler, handler->timerw);
      st25r95_write_ARC(handler, 1, handler->ARC);
      break;
    default:
      st25r95_off(handler);
      break;
  }
}

void st25r95_reset(st25r95_handle *handler) {
  handler->nss(1);
  st25r95_spi_byte(handler, ST25_RESET);
  handler->nss(0);
}

st25r95_status_t st25r95_IDN(st25r95_handle *handler) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_IDN;
  tx_buffer[2] = 0;
  tx_len = 3;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  if (res[0] != 0) return ST25_INVALID_DEVICE;
  if (!(res[2] == 'N' && res[3] == 'F' && res[4] == 'C')) return ST25_INVALID_DEVICE;
  return ST25_OK;
}

st25r95_status_t st25r95_off(st25r95_handle *handler) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_PS;
  tx_buffer[2] = 2;
  tx_buffer[3] = ST25_PROTOCOL_OFF;
  tx_buffer[4] = 0;
  tx_len = 5;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  return res[0];
}

st25r95_status_t st25r95_14443A(st25r95_handle *handler) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_PS;
  tx_buffer[2] = 2;
  tx_buffer[3] = ST25_PROTOCOL_14443A;
  tx_buffer[4] = handler->tx_speed << 6 | handler->rx_speed << 4;
  tx_len = 5;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  handler->protocol = ST25_PROTOCOL_14443A;

  uint8_t *res = st25r95_response(handler);
  return res[0];
}

st25r95_status_t st25r95_read_reg(st25r95_handle *handler, uint8_t address, uint8_t *data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_RR;
  tx_buffer[2] = 0x3;
  tx_buffer[3] = address;
  tx_buffer[4] = 0x1;
  tx_buffer[5] = 0;
  tx_len = 6;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  if (res[0] == ST25_OK) *data = res[2];
  return res[0];
}

st25r95_status_t st25r95_write_timerw(st25r95_handle *handler, uint8_t data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_WR;
  tx_buffer[2] = 0x4;
  tx_buffer[3] = 0x3A;
  tx_buffer[4] = 0x0;
  tx_buffer[5] = data;
  tx_buffer[6] = 0x4;
  tx_len = 7;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  return res[0];
}

st25r95_status_t st25r95_write_ARC_index(st25r95_handle *handler, uint8_t index) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_WR;
  tx_buffer[2] = 0x3;
  tx_buffer[3] = 0x68;
  tx_buffer[4] = 0x0;
  tx_buffer[5] = index;
  tx_len = 6;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  return res[0];
}

st25r95_status_t st25r95_write_ARC(st25r95_handle *handler, uint8_t index, uint8_t data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_WR;
  tx_buffer[2] = 0x4;
  tx_buffer[3] = 0x68;
  tx_buffer[4] = 0x1;
  tx_buffer[5] = index;
  tx_buffer[6] = data;
  tx_len = 7;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  return res[0];
}

st25r95_status_t st25r95_echo(st25r95_handle *handler) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_ECHO;
  tx_len = 2;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  return res[0];
}

uint8_t st25r95_14443A_detect(st25r95_handle *handler) {
  uint8_t data[10] = {0xff};
  st25r95_14443A_REQA(handler, data);
  if (data[0] == 0xff) return 0;
  if (data[0] & 0b00100000) {
    return 0;
  }

  uint8_t UID[10];
  uint8_t UID_size = data[0] >> 6;

  for (uint8_t i = 0; i < UID_size + 1; i++) {
    data[0] = 0xff;
    st25r95_14443A_ANTICOLLISION(handler, i, data);
    if (data[0] == 0xff) return 0;
    if (data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4]) return 0;
    if (data[5] & 0x80) {
    }
    UID[4 * i + 0] = data[0];
    UID[4 * i + 1] = data[1];
    UID[4 * i + 2] = data[2];
    UID[4 * i + 3] = data[3];
    st25r95_14443A_select(handler, i, data, data[0], data[1], data[2], data[3]);
    if (data[0] & 0x4)
      continue;
    else
      break;
  }

  memset(handler->uid, 0, 10);
  memcpy(handler->uid, UID, 10);
  return 1;
}

void st25r95_14443A_REQA(st25r95_handle *handler, uint8_t *data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_SR;
  tx_buffer[2] = 0x2;
  tx_buffer[3] = REQA;
  tx_buffer[4] = 7; // REQA is a 7bits command.
  tx_len = 5;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);

}

const static uint8_t cascade_level[] = {CL_1, CL_2, CL_3};

void st25r95_14443A_ANTICOLLISION(st25r95_handle *handler, uint8_t level, uint8_t *data) {
  tx_buffer[0] = ST25_SEND;
  tx_buffer[1] = ST25_SR;
  tx_buffer[2] = 0x03;
  tx_buffer[3] = cascade_level[level];
  tx_buffer[4] = 0x20; // NVB
  tx_buffer[5] = 0x08;
  tx_len = 6;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);
}

void
st25r95_14443A_select(st25r95_handle *handler, uint8_t level, uint8_t *data, uint8_t uid0, uint8_t uid1, uint8_t uid2,
                      uint8_t uid3) {
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

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  uint8_t *res = st25r95_response(handler);
  if (res[0] != ST25_EFrameRecvOK) return;

  memcpy(data, res + 2, res[1]);
}

void st25r95_idle(st25r95_handle *handler) {
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
  tx_buffer[13] = handler->DACRef - 8;
  tx_buffer[14] = handler->DACRef + 8;
  tx_buffer[15] = 0x3F;
  tx_buffer[16] = 0x01;
  tx_len = 17;

  handler->nss(1);
  st25r95_spi_tx(handler);
  handler->nss(0);

  handler->state = ST25_STATE_IDLE;
}

void st25r95_calibrate(st25r95_handle *handler) {
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
    handler->nss(1);
    st25r95_spi_tx(handler);
    handler->nss(0);

    res = st25r95_response(handler);
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

  handler->DACRef = calibrate_data[14];
}