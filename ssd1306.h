/**
 * Modified from pico-examples ssd1306_i2c.c which is
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "hardware/i2c.h"
#include "ssd1306_font.h"

#define SSD1306_ADDRESS 0x3C

#define SSD1306_I2C_PORT    i2c1
#define SSD1306_I2C_SDA     26
#define SSD1306_I2C_SCL     27
#define SSD1306_BAUDRATE    1000000

#define SSD1306_LCDWIDTH            128
#define SSD1306_LCDHEIGHT           32
#define SSD1306_LINE_HEIGHT         8
#define SSD1306_FRAMEBUFFER_SIZE    (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / SSD1306_LINE_HEIGHT)

// Commands
#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22
#define SSD1306_DEACTIVATE_SCROLL   0x2E
#define SSD1306_ACTIVATE_SCROLL     0x2F
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_SEGREMAP0           0xA0
#define SSD1306_SEGREMAP127         0xA1
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON        0xA5
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETVCOMDETECT       0xDB

uint8_t fb[SSD1306_FRAMEBUFFER_SIZE + 1] = { 0x40 };
uint8_t *framebuffer = fb + 1;

static void ssd1306_send_command(uint8_t cmd)
{
    uint8_t buf[] = { 0x00, cmd };
    i2c_write_blocking(SSD1306_I2C_PORT, SSD1306_ADDRESS, buf, 2, false);
}

static void ssd1306_send_commands(uint8_t* cmds, int len)
{
    i2c_write_blocking(SSD1306_I2C_PORT, SSD1306_ADDRESS, cmds, len, false);
}

static void ssd1306_init()
{
    // Setup I2C
    i2c_init(SSD1306_I2C_PORT, SSD1306_BAUDRATE);
    gpio_set_function(SSD1306_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_I2C_SDA);
    gpio_pull_up(SSD1306_I2C_SCL);

    uint8_t init_cmds[] =
    {
        0x00,
        SSD1306_DISPLAYOFF,
        SSD1306_SETMULTIPLEX, 0x1F,
        SSD1306_SETDISPLAYOFFSET, 0x00,
        SSD1306_SETSTARTLINE,
        SSD1306_SEGREMAP127,
        SSD1306_COMSCANDEC,
        SSD1306_SETCOMPINS, 0x02, // 0x02 = 32, 0x12 = 64
        SSD1306_SETCONTRAST, 0xFF,
        SSD1306_DISPLAYALLON_RESUME,
        SSD1306_NORMALDISPLAY,
        SSD1306_SETDISPLAYCLOCKDIV, 0x80,
        SSD1306_CHARGEPUMP, 0x14,
        SSD1306_DISPLAYON,
        SSD1306_MEMORYMODE, 0x00,   // 0 = horizontal, 1 = vertical, 2 = page
        SSD1306_COLUMNADDR, 0, SSD1306_LCDWIDTH - 1,  // Set the screen wrapping points
        SSD1306_PAGEADDR, 0, 3 // 3 = 32, 7 = 64
    };

    ssd1306_send_commands(init_cmds, sizeof(init_cmds));
}

static void ssd1306_invert_display(bool invert)
{
    ssd1306_send_command(invert ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
}

static void ssd1306_update_display()
{
    i2c_write_blocking(SSD1306_I2C_PORT, SSD1306_ADDRESS, fb, sizeof(fb), false);
}

static void ssd1306_clear_display()
{
    memset(framebuffer, 0, SSD1306_FRAMEBUFFER_SIZE);
    ssd1306_update_display();
}

static void ssd1306_set_pixel(int x, int y, bool on)
{
    if (x < 0 && x >= SSD1306_LCDWIDTH && y < 0 && y >= SSD1306_LCDHEIGHT)
    {
        return;
    }

    // The calculation to determine the correct bit to set depends on which address
    // mode we are in. This code assumes horizontal

    // The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
    // Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
    // byte 1 is x = 1, y=0->7 etc

    // This code could be optimised, but is like this for clarity. The compiler
    // should do a half decent job optimising it anyway.

    // 128 pixels, 1bpp, but each row is 8 pixel high, so (128 / 8) * 8

    int byte_idx = (y / SSD1306_LINE_HEIGHT) * SSD1306_LCDWIDTH + x;
    uint8_t byte = framebuffer[byte_idx];

    if (on)
    {
        byte |=  1 << (y % SSD1306_LINE_HEIGHT);
    }
    else
    {
        byte &= ~(1 << (y % SSD1306_LINE_HEIGHT));
    }

    framebuffer[byte_idx] = byte;
}

static void ssd1306_draw_line(int x0, int y0, int x1, int y1, bool on = true)
{
    int dx =  abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (true)
    {
        ssd1306_set_pixel(x0, y0, on);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

static void ssd1306_write_char(uint8_t x, uint8_t y, char ch)
{
    if (x > SSD1306_LCDWIDTH - SSD1306_FONT_WIDTH || y > SSD1306_LCDHEIGHT - SSD1306_FONT_WIDTH)
    {
        return;
    }

    // Skip non-printable characters
    if (ch < ' ' && ch > '}')
    {
        return;
    }

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y / SSD1306_LINE_HEIGHT;

    int idx = SSD1306_FONT_WIDTH * (ch - ' ');
    int fb_idx = y * SSD1306_LCDWIDTH + x;

    for (int i = 0; i < SSD1306_FONT_WIDTH; ++i)
    {
        framebuffer[fb_idx++] = font[idx + i];
    }
}

static void ssd1306_write_str(uint8_t x, uint8_t y, const char* str)
{
    while (*str)
    {
        ssd1306_write_char(x, y, *str++);
        x += SSD1306_FONT_WIDTH + SSD1306_FONT_PADDING;
    }
}

#endif /* SSD1306_H */