
// SD card (spi interface) lib

#include "SD_SPI.hpp"

SD_SPI::SD_SPI(int pin_cs, int pin_cd) {
  this->_pin_cs = pin_cs;
  this->_pin_cd = pin_cd;
}

void SD_SPI::init(void) {
  gpio_init(_pin_cs);
  gpio_set_dir(_pin_cs, GPIO_OUT);
  gpio_put(_pin_cs, 1);

  gpio_init(_pin_cd);
  gpio_set_dir(_pin_cd, GPIO_IN);
  gpio_pull_up(_pin_cd);

  spi_init(SD_SPI_CH, SD_SPI_FAST);
  gpio_set_function(SD_RX, GPIO_FUNC_SPI);
  gpio_pull_up(SD_RX);
  gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
  gpio_set_function(SD_TX, GPIO_FUNC_SPI);

  this->info.is_inited = false;
  this->info.type = SD_TYPE_UNKNOWN;
}

bool SD_SPI::card_check(void) {
  return !gpio_get(_pin_cd);
}

int SD_SPI::card_init(void) {
  uint8_t write_buf[16];
  uint8_t read_buf[16];
  uint32_t timeout = 1000;

  if(!card_check()) {
    return -1;
  }

  // set low speed
  spi_init(SD_SPI_CH, SD_SPI_SLOW);

  // set SD card to SPI mode
  // CS high and send over 74 clocks
  memset(write_buf, 0xFF, 10);
  gpio_put(_pin_cs, 1);
  spi_write_blocking(SD_SPI_CH, write_buf, 10); // 8 * 10 = 80 > 74

  // send CMD0
  if(send_cmd(SD_CMD0, NULL, read_buf, false) < 0) {
    wprintf(L"CMD0 timeout\n");
    return -1;
  }

  // send CMD8
  write_buf[0] = 0x00;
  write_buf[1] = 0x00;
  write_buf[2] = 0x01;
  write_buf[3] = 0xAA;
  if (send_cmd(SD_CMD8, write_buf, read_buf, false) == 0) {
    if ((read_buf[0] & 0x04) == 0 && read_buf[3] == 0x01 && read_buf[4] == 0xAA) {
      this->info.version = 0x20;
    } else {
      this->info.version = 0x10;
    }
  } else {
    this->info.version = 0x10;
  }

  // ACMD41 loop
  for(; timeout > 0; timeout--) {
    // send CMD55
    if(send_cmd(SD_CMD55, NULL, read_buf, false) < 0) {
      wprintf(L"CMD55 timeout\n");
      return -55;
    }

    // send ACMD41
    if(this->info.version >= 0x20) {
      write_buf[0] = 0x40;
    } else {
      write_buf[0] = 0x00;
    }
    write_buf[1] = 0x00;
    write_buf[2] = 0x00;
    write_buf[3] = 0x00;
    if(send_cmd(SD_ACMD41, write_buf, read_buf, false) < 0) {
      wprintf(L"ACMD41 timeout\n");
      return -41;
    }

    if(read_buf[0] == 0x00) {
      break;
    }

    sleep_ms(1);
  }

  if(!timeout) {
    wprintf(L"ACMD41 loop timeout\n");
    return -41;
  }

  // send CMD58
  if(send_cmd(SD_CMD58, NULL, read_buf, false) < 0) {
    wprintf(L"CMD58 timeout\n");
    return -58;
  }

  if(read_buf[1] & 0x40) {
    this->info.type = SD_TYPE_SDHC;
  } else {
    this->info.type = SD_TYPE_SDSC;
  }

  // send CMD9
  if (send_cmd(SD_CMD9, NULL, read_buf, true) < 0 || read_buf[0] != 0x00) {
    wprintf(L"CMD9 failed\n");
    gpio_put(_pin_cs, 1);
    return -9;
  }
  
  uint8_t csd[16];
  if (read_data_block(csd, 16) < 0) {
    wprintf(L"CSD read failed\n");
    gpio_put(_pin_cs, 1);
    return -19;
  }
  
  uint64_t capacity = 0;
  if ((csd[0] >> 6) == 1) { // SDHC+
    uint32_t c_size = ((csd[7] & 0x3F) << 16) | (csd[8] << 8) | csd[9];
    capacity = (uint64_t)(c_size + 1) * 512 * 1024;
  } else { // SDSC
    uint32_t c_size = ((csd[6] & 0x03) << 10) | (csd[7] << 2) | ((csd[8] & 0xC0) >> 6);
    uint32_t c_size_mult = ((csd[9] & 0x03) << 1) | ((csd[10] & 0x80) >> 7);
    uint32_t block_len = 1 << (csd[5] & 0x0F);
    uint32_t mult = 1 << (c_size_mult + 2);
    capacity = (uint64_t)(c_size + 1) * mult * block_len;
  }

  this->info.size = capacity;

  // init complete, set high speed
  gpio_put(_pin_cs, 1);
  wprintf(L"SD init ok!\n");
  this->info.is_inited = true;
  spi_init(SD_SPI_CH, SD_SPI_FAST);

  wprintf(L"SD init complete!\n");
  return 0;
}

int SD_SPI::card_deinit(void) {
  gpio_put(_pin_cs, 1);
  spi_init(SD_SPI_CH, SD_SPI_FAST);

  this->info.is_inited = false;
  this->info.type == SD_TYPE_UNKNOWN;
  return 0;
}

int SD_SPI::send_cmd(enum _sd_command_t cmd, uint8_t* write_buf, uint8_t* read_buf, bool keep_cs_low) {
  uint8_t cmd_buf[6] = {0,};
  uint32_t timeout = 512;
  enum _sd_response_t response = SD_R_UNKNOWN;

  cmd_buf[0] = (0x40 | (cmd & 0x3F));
  gpio_put(_pin_cs, 0);

  switch (cmd)
  {
  case SD_CMD0:
    {
      cmd_buf[5] = 0x95;
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);

      // wait R1 response
      response = SD_R1;
    }
    break;
  case SD_CMD8:
    {
      memcpy(&cmd_buf[1], write_buf, 4);
      cmd_buf[5] = 0x87;
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);

      // wait R7 response
      response = SD_R7;
    }
    break;
  case SD_CMD9:
    {
      memcpy(&cmd_buf[1], write_buf, 4);
      cmd_buf[5] = 0xFF;
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);

      // wait R1 response
      response = SD_R1;
    }
    break;
  case SD_CMD17:
    {
      memcpy(&cmd_buf[1], write_buf, 4);
      cmd_buf[5] = 0xFF; // any valid CRC for non-CMD0/CMD8
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);
      response = SD_R1;
    }
    break;
  case SD_CMD24:
    {
      memcpy(&cmd_buf[1], write_buf, 4);
      cmd_buf[5] = 0xFF;
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);
      response = SD_R1;
    }
    break;
  case SD_CMD55:
    {
      cmd_buf[5] = 0xFF;
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);

      // wait R1 response
      response = SD_R1;
    }
    break;
  case SD_CMD58:
    {
      cmd_buf[5] = 0xFF;
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);

      // wait R3 response
      response = SD_R3;
    }
    break;
  case SD_ACMD41:
    {
      memcpy(&cmd_buf[1], write_buf, 4);
      cmd_buf[5] = 0xFF;
      spi_write_blocking(SD_SPI_CH, cmd_buf, 6);

      // wait R1 response
      response = SD_R1;
    }
    break;
    default: // unknown cmd
      gpio_put(_pin_cs, 1);
      return -1;
  }

  // wait response
  cmd_buf[0] = 0xFF;
  for(; timeout > 0; timeout--)
  {
    spi_write_read_blocking(SD_SPI_CH, cmd_buf, read_buf, 1);
    if((read_buf[0] & 0x80) == 0x00) {
      break;
    }
  }

  if(!timeout) {
    gpio_put(_pin_cs, 1);
    return -1;
  }

  // response
  switch (response)
  {
    case SD_R1:
    break;
    case SD_R1B:
    {
      uint8_t busy;
      do {
        uint8_t dummy = 0xFF;
        spi_write_read_blocking(SD_SPI_CH, &dummy, &busy, 1);
      } while (busy != 0xFF);
    }
    break;
    case SD_R2:
    {
      cmd_buf[2] = 0xFF;
      spi_write_read_blocking(SD_SPI_CH, cmd_buf, &read_buf[1], 1);
    }
    break;
    case SD_R3:
    case SD_R7:
    {
      cmd_buf[2] = 0xFF;
      cmd_buf[3] = 0xFF;
      cmd_buf[4] = 0xFF;
      spi_write_read_blocking(SD_SPI_CH, cmd_buf, &read_buf[1], 4);
    }
    break;
  }

  if (!keep_cs_low) {
    uint8_t dummy = 0xFF;
    spi_write_blocking(SD_SPI_CH, &dummy, 1); // flush
    gpio_put(_pin_cs, 1);
  }

  return 0;
}

int SD_SPI::sector_read(size_t sector_num, void* buf) {
  int res = 0;

  if(!this->info.is_inited || !buf) {
    return -1;
  }
  spi_init(SD_SPI_CH, SD_SPI_FAST);

  if(this->info.type == SD_TYPE_SDSC) {
    // todo
  } else if(this->info.type == SD_TYPE_SDHC) {
    uint8_t write_buf[4];
    uint8_t read_buf[5];

    write_buf[0] = (sector_num >> 24) & 0xFF;
    write_buf[1] = (sector_num >> 16) & 0xFF;
    write_buf[2] = (sector_num >> 8) & 0xFF;
    write_buf[3] = (sector_num) & 0xFF;
  
    if (send_cmd(SD_CMD17, write_buf, read_buf, true) < 0 || read_buf[0] != 0x00) {
      wprintf(L"CMD17 response error: 0x%02X\n", read_buf[0]);
      gpio_put(_pin_cs, 1);
      return -17; 
    }

    sleep_us(50);
    res = read_data_block((uint8_t*)buf, 512);
    gpio_put(_pin_cs, 1);
    uint8_t dummy = 0xFF;
    spi_write_blocking(SD_SPI_CH, &dummy, 1); // flush
  } else {
    return -1;
  }

  return res;
}

int SD_SPI::sector_write(size_t sector_num, void* buf) {
  if(!this->info.is_inited || !buf) {
    return -1;
  }
  spi_init(SD_SPI_CH, SD_SPI_FAST);

  if(this->info.type == SD_TYPE_SDSC) {
    // todo
  } else if(this->info.type == SD_TYPE_SDHC) {
    uint8_t write_buf[4];
    uint8_t read_buf[5];
    uint8_t dummy = 0xFF;
  
    write_buf[0] = (sector_num >> 24) & 0xFF;
    write_buf[1] = (sector_num >> 16) & 0xFF;
    write_buf[2] = (sector_num >> 8) & 0xFF;
    write_buf[3] = (sector_num) & 0xFF;
  
    if (send_cmd(SD_CMD24, write_buf, read_buf, true) < 0 || read_buf[0] != 0x00)
      return -24;
  
    gpio_put(_pin_cs, 0);
  
    spi_write_blocking(SD_SPI_CH, &dummy, 1);
  
    // data token
    uint8_t token = 0xFE;
    spi_write_blocking(SD_SPI_CH, &token, 1);
  
    // send data
    spi_write_blocking(SD_SPI_CH, (uint8_t*)buf, 512);
  
    // CRC dummy
    uint8_t crc[2] = {0xFF, 0xFF};
    spi_write_blocking(SD_SPI_CH, crc, 2);
  
    // check response byte
    uint8_t resp;
    spi_write_read_blocking(SD_SPI_CH, &dummy, &resp, 1);
    if ((resp & 0x1F) != 0x05) {
      gpio_put(_pin_cs, 1);
      return -25;
    }
  
    // Busy polling
    uint8_t busy;
    do {
      spi_write_read_blocking(SD_SPI_CH, &dummy, &busy, 1);
    } while (busy == 0x00);
  
    gpio_put(_pin_cs, 1);
    dummy = 0xFF;
    spi_write_blocking(SD_SPI_CH, &dummy, 1); // flush
  } else {
    return -1;
  }

  return 0;
}

int SD_SPI::read_data_block(uint8_t* out, size_t len) {
  uint8_t dummy = 0xFF;
  uint8_t token;
  uint32_t timeout = 65535;

  // wait data token(0xFE) 
  //gpio_put(_pin_cs, 0);
  do {
    spi_write_read_blocking(SD_SPI_CH, &dummy, &token, 1);
    timeout--;
  } while ((token == 0xFF || token == 0x00) && timeout > 0);
  
  if (token != 0xFE) {
    wprintf(L"Unexpected token: 0x%02X\n", token);
    //gpio_put(_pin_cs, 1);
    return -1;
  }

  // recv data
  memset(out, 0xFF, len);
  spi_write_read_blocking(SD_SPI_CH, out, out, len);

  // ignore CRC
  uint8_t crc[2] = {0xFF, 0xFF};
  spi_write_read_blocking(SD_SPI_CH, crc, crc, 2);

  //gpio_put(_pin_cs, 1);

  return 0;
}
