#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define FW_UPDATE_OFFSET 0x100000
#define MAX_FW_SIZE (512 * 1024)

void __not_in_flash_func(receive_and_flash_firmware)() {
    uart_puts(UART_ID, "OK\n");

    uint8_t len_buf[4];
    for (int i = 0; i < 4; i++) {
        while (!uart_is_readable(UART_ID));
        len_buf[i] = uart_getc(UART_ID);
    }

    uint32_t fw_len = len_buf[0] | (len_buf[1] << 8) | (len_buf[2] << 16) | (len_buf[3] << 24);
    if (fw_len > MAX_FW_SIZE) {
        uart_puts(UART_ID, "ERR: too big\n");
        return;
    }

    uint8_t *buf = (uint8_t *)malloc(fw_len);
    if (!buf) {
        uart_puts(UART_ID, "ERR: malloc\n");
        return;
    }

    for (uint32_t i = 0; i < fw_len; i++) {
        while (!uart_is_readable(UART_ID));
        buf[i] = uart_getc(UART_ID);
    }

    uart_puts(UART_ID, "FLASH\n");

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FW_UPDATE_OFFSET, ((fw_len + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE);
    flash_range_program(FW_UPDATE_OFFSET, buf, fw_len);
    restore_interrupts(ints);

    free(buf);
    uart_puts(UART_ID, "JUMP\n");

    uart_deinit(UART_ID);
    void (*app)(void) = (void (*)(void))(XIP_BASE + FW_UPDATE_OFFSET);
    app();
}

int main() {
    stdio_init_all();
    sleep_ms(500);

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_puts(UART_ID, "BOOT");

    sleep_ms(500); // Optional wait

    // Simple polling loop for update command
    while (true) {
        if (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);
            static char cmd[16];
            static int pos = 0;
            if (pos < 15) cmd[pos++] = c;
            if (c == '\n') {
                cmd[pos] = '\0';
                if (strcmp(cmd, "FW_UPDATE\n") == 0) {
                    receive_and_flash_firmware();
                }
                pos = 0;
            }
        }
    }
}