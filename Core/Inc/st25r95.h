#ifndef ST25R95_ST25R95_H
#define ST25R95_ST25R95_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __weak
#define __weak   __attribute__((weak))
#endif

typedef enum {
  ST25_SEND = 0x0,
  ST25_RESET = 0x1,
  ST25_READ = 0x2,
  ST25_POLL = 0x3,
} st25r95_control_t;

typedef enum {
  ST25_IDN = 0x1,
  ST25_PS = 0x2,
  ST25_SR = 0x4,
  ST25_IDLE = 0x7,
  ST25_RR = 0x8,
  ST25_WR = 0x9,
  ST25_BR = 0xA,
  ST25_ECHO = 0x55,
} st25r95_command_t;

typedef enum {
  ST25_PROTOCOL_OFF = 0x0,
  ST25_PROTOCOL_15693 = 0x1,
  ST25_PROTOCOL_14443A = 0x2,
  ST25_PROTOCOL_14443B = 0x3,
  ST25_PROTOCOL_18092 = 0x4,
} st25r95_protocol_t;

typedef enum {
  ST25_26K_106K,
  ST25_52K_212K,
  ST25_6K_424K,
  ST25_848K,
} st25r95_rate_t;

typedef enum {
  ST25_OK,
  ST25_INVALID_DEVICE,
  ST25_PASS,
  ST25_BCC_ERROR,

  ST25_EEmdSOFerror23 = 0x63,
  ST25_EEmdSOFerror10 = 0x65,
  ST25_EEmdEgterror = 0x66,
  ST25_ETr1TooBigTooLong = 0x67,
  ST25_ETr1TooSmall = 0x68,
  ST25_EinternalError = 0x7,
  ST25_EFrameRecvOK = 0x80,
  ST25_EUserStop = 0x85,
  ST25_ECommError = 0x86,
  ST25_EFrameWaitTOut = 0x87,
  ST25_EInvalidSof = 0x88,
  ST25_EBufOverflow = 0x89,
  ST25_EFramingError = 0x8A,
  ST25_EEgtError = 0x8B,
  ST25_EInvalidLen = 0x8C,
  ST25_ECrcError = 0x8D,
  ST25_ERecvLost = 0x8E,
  ST25_ENoField = 0x8F,
  ST25_EUnintByte = 0x90,
} st25r95_status_t;

typedef enum {
  ISO14443A_SINGLE = 0x0,
  ISO14443A_DOUBLE = 0x1,
  ISO14443A_TRIPLE = 0x2,
} UID_size_t;

typedef enum {
  REQA = 0x26,
  WUPA = 0x52,
  CL_1 = 0x93,
  CL_2 = 0x95,
  CL_3 = 0x97,
} ISO14443_command;

typedef enum {
  tx_flag_Topaz = 0x80,
  tx_flag_SplitFrame = 0x40,
  tx_flag_AppendCRC = 0x20,
  tx_flag_ParityFraming = 0x08,
} ISO14443_tx_flag;

typedef enum {
  ST25_WU_SRC_Timeout = 0x01,
  ST25_WU_SRC_TagDetection = 0x02,
  ST25_WU_SRC_IRQ = 0x08,
  ST25_WU_SRC_SS = 0x10,
  ST25_WU_SRC_32K = 0x00 << 6,
  ST25_WU_SRC_16K = 0x01 << 6,
  ST25_WU_SRC_8K = 0x10 << 6,
  ST25_WU_SRC_4K = 0x11 << 6,
} st25r95_wu_source;

typedef enum {
  ST25_EC_Hibernate = 0x0400,
  ST25_EC_Sleep = 0x0200,
  ST25_EC_TagDetectorCalibration = 0xA100,
  ST25_EC_TagDetection = 0x2100,
} st25r95_enter_ctrl;

typedef enum {
  ST25_WU_CTRL_Hibernate = 0x0400,
  ST25_WU_CTRL_Sleep = 0x3800,
  ST25_WU_CTRL_TagDetectorCalibration = 0xF801,
  ST25_WU_CTRL_TagDetection = 0x7901,
} st25r95_wu_ctrl;

typedef enum {
  ST25_LEAVE_CTRL_Hibernate = 0x1800,
  ST25_LEAVE_CTRL_Sleep = 0x1800,
  ST25_LEAVE_CTRL_TagDetectorCalibration = 0x1800,
  ST25_LEAVE_CTRL_TagDetection = 0x1800,
} st25r95_leave_ctrl;

typedef enum {
  ST25_STATE_NORMAL,
  ST25_STATE_IDLE,
} st25r95_state_t;

typedef void (*st25r95_nss)(uint8_t);

typedef void (*st25r95_tx)(uint8_t *, size_t);

typedef void (*st25r95_rx)(uint8_t *, size_t);

typedef void (*st25r95_irq_pulse)();

typedef void (*st25r95_callback)(uint8_t *);

typedef struct {
  /* Reader state and variables */
  st25r95_state_t state;
  st25r95_protocol_t protocol;
  uint8_t DACRef;
  st25r95_rate_t tx_speed;
  st25r95_rate_t rx_speed;
  uint8_t timerw;
  uint8_t ARC;
  uint8_t uid[10];
  volatile uint8_t irq_flag;
  /* BSP Functions */
  st25r95_callback callback;
  st25r95_nss nss;
  st25r95_tx tx;
  st25r95_rx rx;
  st25r95_irq_pulse irq_pulse;
} st25r95_handle;

void st25r95_init(st25r95_handle *);

void st25r95_reset(st25r95_handle *);

st25r95_status_t st25r95_IDN(st25r95_handle *);

st25r95_status_t st25r95_off(st25r95_handle *);

st25r95_status_t st25r95_14443A(st25r95_handle *);

st25r95_status_t st25r95_read_reg(st25r95_handle *, uint8_t, uint8_t *);

st25r95_status_t st25r95_write_timerw(st25r95_handle *, uint8_t);

st25r95_status_t st25r95_write_ARC_index(st25r95_handle *, uint8_t);

st25r95_status_t st25r95_write_ARC(st25r95_handle *, uint8_t, uint8_t);

st25r95_status_t st25r95_echo(st25r95_handle *);

void st25r95_14443A_REQA(st25r95_handle *, uint8_t *);

void st25r95_14443A_ANTICOLLISION(st25r95_handle *, uint8_t, uint8_t *);

void st25r95_14443A_select(st25r95_handle *, uint8_t, uint8_t *, uint8_t, uint8_t, uint8_t, uint8_t);

uint8_t st25r95_14443A_detect(st25r95_handle *);

void st25r95_idle(st25r95_handle *);

void st25r95_calibrate(st25r95_handle *);

void st25r95_service(st25r95_handle *);

#endif
