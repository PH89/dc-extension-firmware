#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/uart.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define FW_UPDATE_OFFSET 0x100000
#define MAX_FIRMWARE_SIZE (512 * 1024)
#define UART_ID uart0

void __not_in_flash_func(receive_and_flash_firmware)() {
    uart_puts(UART_ID, "OK\n");

    uint8_t len_buf[4];
    for (int i = 0; i < 4; i++) {
        while (!uart_is_readable(UART_ID));
        len_buf[i] = uart_getc(UART_ID);
    }

    uint32_t fw_len = len_buf[0] | (len_buf[1] << 8) | (len_buf[2] << 16) | (len_buf[3] << 24);
    if (fw_len > MAX_FIRMWARE_SIZE) {
        uart_puts(UART_ID, "ERR: size too large\n");
        return;
    }

    uart_puts(UART_ID, "RECV\n");
    uint8_t *buffer = (uint8_t *)malloc(fw_len);
    if (!buffer) {
        uart_puts(UART_ID, "ERR: malloc\n");
        return;
    }

    for (uint32_t i = 0; i < fw_len; i++) {
        while (!uart_is_readable(UART_ID));
        buffer[i] = uart_getc(UART_ID);
    }

    uart_puts(UART_ID, "FLASH\n");

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FW_UPDATE_OFFSET, ((fw_len + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE);
    flash_range_program(FW_UPDATE_OFFSET, buffer, fw_len);
    restore_interrupts(ints);

    free(buffer);
    uart_puts(UART_ID, "JUMP\n");

    uart_deinit(UART_ID);

    void (*new_app)(void) = (void (*)(void))(XIP_BASE + FW_UPDATE_OFFSET);
    new_app();
}