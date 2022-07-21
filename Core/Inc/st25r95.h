#ifndef ST25R95_ST25R95_H
#define ST25R95_ST25R95_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  ST25_SEND = 0x0,
  ST25_RESET = 0x1,
  ST25_READ = 0x2,
  ST25_POLL = 0x3,
} st25r95_control;

typedef enum {
  ST25_IDN = 0x1,
  ST25_PS = 0x2,
  ST25_SR = 0x4,
  ST25_IDLE = 0x7,
  ST25_RR = 0x8,
  ST25_WR = 0x9,
  ST25_BR = 0xA,
  ST25_ECHO = 0x55,
} st25r95_command;

typedef enum {
  ST25_PROTOCOL_OFF = 0x0,
  ST25_PROTOCOL_15693 = 0x1,
  ST25_PROTOCOL_14443A = 0x2,
  ST25_PROTOCOL_14443B = 0x3,
  ST25_PROTOCOL_18092 = 0x4,
} st25r95_protocol;

typedef enum {
  ST25_OK,
  ST25_ERROR,
  ST25_INVALID_DEVICE,
} st25r95_status;

void st25r95_init();

void st25r95_reset();

st25r95_status st25r95_IDN();


#endif
