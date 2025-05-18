#pragma once
// uart log lib

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

void uartLog_init(uart_inst_t* _uart, int tx_pin, int rx_pin, int baudrate);
void uartLog_set_baudrate(int baudrate);
void on_uart_rx(void);