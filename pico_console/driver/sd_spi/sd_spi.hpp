#pragma once
// SD card (spi interface) lib

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// for using unicode
#include <wchar.h>
#include <locale.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#define sdSpi_SLOW 400*1000
#define sdSpi_FAST 10*1000*1000

#define sdSpi_CH spi0

#define SD_RX 16
#define SD_SCK 18
#define SD_TX 19

enum _sd_type_t {
  SD_TYPE_SDSC,
  SD_TYPE_SDHC,
//  SD_TYPE_SDXC,
//  SD_TYPE_SDUC,
  SD_TYPE_UNKNOWN = 0xFF
};

enum _sd_command_t {
  SD_CMD0     = 0,  // GO_IDLE_STATE -> R1
  SD_CMD1     = 1,  // SEND_OP_COND -> R1
  SD_CMD8     = 8,  // SEND_IF_COND -> R7
  SD_CMD9     = 9,  // SEND_CSD -> R1 + 0xFE + data[16]
  SD_CMD10    = 10, // SEND_CID -> R1 + 0xFE + data[16]
  SD_CMD12    = 12, // STOP_TRANSMISSION -> R1b
  SD_CMD13    = 13, // SEND_STATUS -> R2
  SD_CMD17    = 17, // READ_SINGLE_BLOCK -> R1 + 0xFE + data[512]
  SD_CMD18    = 18, // READ_MULTIPLE_BLOCK -> R1
  SD_CMD24    = 24, // WRITE_SINGLE_BLOCK -> R1 + 0xFE + data[512]
  SD_CMD25    = 25, // WRITE_MULTIPLE_BLOCK -> R1
  SD_CMD55    = 55, // APP_CMD -> R1
  SD_CMD58    = 58, // READ_OCR -> R3

  SD_ACMD41   = 41, // SD_SEND_OP_COND -> R1
  SD_ACMD22   = 22, // SEND_NUM_WR_BLOCKS -> R1
  SD_ACMD23   = 23, // SET_WR_BLK_ERASE_COUNT -> R1
  SD_ACMD42   = 42, // SET_CLR_CARD_DETECT -> R1
  SD_ACMD51   = 51, // SEND_SCR -> R1 + 0xFE + data[8]
};

enum _sd_response_t {
  SD_R1,
  SD_R1B,
  SD_R2,
  SD_R3,
  SD_R7,
  SD_R_UNKNOWN,
};

typedef struct _sd_info
{
  bool is_inited;
  uint8_t version;
  enum _sd_type_t type;
  uint32_t block_len;
  uint64_t size;
  
} sd_info;


class sdSpi {
  public:
  sdSpi(int pin_cs, int pin_cd);
  
  sd_info info;

  void init(void);
  bool card_check(void);
  int card_init(void);
  int card_deinit(void);

  int sector_read(size_t sector_num, void* buf);
  int sector_write(size_t sector_num, void* buf);

  private:
  int _pin_cs, _pin_cd;

  int send_cmd(enum _sd_command_t cmd, uint8_t* write_buf, uint8_t* read_buf, bool keep_cs_low);
  int read_data_block(uint8_t* out, size_t len);
};