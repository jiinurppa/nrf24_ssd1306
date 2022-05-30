#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/divider.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"
#include "ssd1306.h"
#include "nrf24l01.h"

// UART defines
#define UART_ID uart0
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

bool radio_error = false;

constexpr bool enable_debug_logging = true;
constexpr bool enable_programming_mode = true;

constexpr uint16_t programming_delay_ms = 10000;

// Top-right corner animation for programming mode
void update_animation()
{
    uint8_t animation[] = { 0xC0, 0x30, 0x00, 0x00 };
    static int8_t animation_counter = 0;

    framebuffer[124] = animation[3 - animation_counter];
    framebuffer[125] = animation[3 - animation_counter];
    framebuffer[126] = animation[animation_counter];
    framebuffer[127] = animation[animation_counter];

    animation_counter = animation_counter == 3 ? 0 : animation_counter + 1;
}

// 4 lines, 21 characters per line with padding
void display_write_line(const char* text, uint8_t line = 1)
{
    uint8_t line_y = SSD1306_FONT_LINE1_Y;

    if (line == 2)
    {
        line_y = SSD1306_FONT_LINE2_Y;
    }
    else if (line == 3)
    {
        line_y = SSD1306_FONT_LINE3_Y;
    }
    else if (line == 4)
    {
        line_y = SSD1306_FONT_LINE4_Y;
    }

    ssd1306_write_str(SSD1306_FONT_PADDING, line_y, text);
}

void display_init_message()
{
    ssd1306_clear_display();
    display_write_line("    Device ready", 2);
    ssd1306_update_display();
}

void display_update()
{
    ssd1306_clear_display();
    display_write_line("Received message:", 1);
    display_write_line(nrf24_message_buffer, 2);

    if (radio_error)
    {
        display_write_line("Radio not responding!", 4);
    }

    ssd1306_update_display();
}

// Source: https://github.com/ghubcoder/PicoSleepRtc
void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig)
{
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_LSB);

    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    clocks_init();
    stdio_init_all();
}

void debug_log(const char* message)
{
    if (enable_debug_logging && message != nullptr)
    {
        uart_puts(UART_ID, message);
        uart_tx_wait_blocking(UART_ID);
    }
}

void debug_log_line(const char* message)
{
    if (enable_debug_logging && message != nullptr)
    {
        uart_puts(UART_ID, message);
        uart_puts(UART_ID, "\r\n");
        uart_tx_wait_blocking(UART_ID);
    }
}

void programming_mode()
{
    if (enable_programming_mode)
    {
        debug_log_line("Entering programming mode...");

        constexpr uint16_t loop_sleep_ms = 200;
        constexpr uint16_t loops = programming_delay_ms / loop_sleep_ms;
        constexpr uint8_t progress_tick = SSD1306_LCDWIDTH / loops;

        for (uint16_t i = loops; i > 0; --i)
        {
            ssd1306_clear_display();
            ssd1306_draw_line(0, SSD1306_LCDHEIGHT - 1, progress_tick * i, SSD1306_LCDHEIGHT - 1);
            display_write_line("In programming mode!", 3);
            update_animation();
            ssd1306_update_display();
            sleep_ms(loop_sleep_ms);
        }

        debug_log_line("Exiting programming mode.");
    }
}

int main()
{
    stdio_init_all();
    ssd1306_init();

    // Init UART for debug_log
    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    programming_mode();

    spi.begin(NRF23_SPI_PORT, NRF23_SPI_SCK, NRF23_SPI_TX, NRF23_SPI_RX);

    if (!radio.begin(&spi))
    {
        debug_log_line("Radio hardware is not responding!");
        radio_error = true;
    }

    nrf24_init();

    uint scb_orig = scb_hw->scr;
    uint clock0_orig = clocks_hw->sleep_en0;
    uint clock1_orig = clocks_hw->sleep_en1;

    display_init_message();
    debug_log_line("Init done.");

    while(1)
    {
        sleep_run_from_xosc();
        sleep_goto_dormant_until_pin(NRF24_IRQ_PIN, true, false);
        recover_from_sleep(scb_orig, clock0_orig, clock1_orig);

        bool tx_ok, tx_fail, rx_ready;
        radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (rx_ready)
        {
            nrf24_receive_message();
            debug_log("Received message: ");
            debug_log_line(nrf24_message_buffer);

            if (strcmp(nrf24_message_buffer, "!PING") == 0)
            {
                radio.stopListening();
                sleep_ms(50);
                debug_log_line("Sending !PONG");
                radio.writeFast("!PONG", NRF24_PAYLOAD_SIZE);
                bool tx_success = radio.txStandBy(500);

                if (tx_success)
                {
                    debug_log_line("Sent !PONG");
                }
                else
                {
                    debug_log_line("No response to !PONG");
                }

                radio.startListening();
                debug_log_line("Continuing listening...");
            }
        }

        display_update();
    }

    return 0;
}
