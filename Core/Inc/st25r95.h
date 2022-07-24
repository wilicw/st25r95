#ifndef ST25R95_ST25R95_H
#define ST25R95_ST25R95_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void st25r95_init();

void st25r95_reset();

st25r95_status_t st25r95_IDN();

st25r95_status_t st25r95_off();

st25r95_status_t st25r95_14443A(st25r95_rate_t, st25r95_rate_t);

st25r95_status_t st25r95_read_reg(uint8_t, uint8_t*);

st25r95_status_t st25r95_write_timerw(uint8_t);

st25r95_status_t st25r95_write_ARC_index(uint8_t);

st25r95_status_t st25r95_write_ARC(uint8_t, uint8_t);

st25r95_status_t st25r95_echo();

void st25r95_14443A_REQA(uint8_t *);

st25r95_status_t st25r95_14443A_detect();

#endif
