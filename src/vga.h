//
// Created by Zixiong Liu on 12/16/16.
//

#ifndef MYOS_VGA_H
#define MYOS_VGA_H

#include <stdint.h>

static const uint8_t COLOR_BLACK = 0;
static const uint8_t COLOR_BLUE = 1;
static const uint8_t COLOR_GREEN = 2;
static const uint8_t COLOR_CYAN = 3;
static const uint8_t COLOR_RED = 4;
static const uint8_t COLOR_MAGENTA = 5;
static const uint8_t COLOR_BROWN = 6;
static const uint8_t COLOR_LIGHT_GREY = 7;
static const uint8_t COLOR_DARK_GREY = 8;
static const uint8_t COLOR_LIGHT_BLUE = 9;
static const uint8_t COLOR_LIGHT_GREEN = 10;
static const uint8_t COLOR_LIGHT_CYAN = 11;
static const uint8_t COLOR_LIGHT_RED = 12;
static const uint8_t COLOR_LIGHT_MAGENTA = 13;
static const uint8_t COLOR_LIGHT_BROWN = 14;
static const uint8_t COLOR_WHITE = 15;

class vga {
    static constexpr int buffer_height = 25, buffer_width = 80;
    static constexpr uint16_t* terminal_buffer = (uint16_t*) 0xB8000;
    static int pos_x, pos_y;

public:
    typedef uint8_t vga_color_t;
    static void vga_init();
    static void print(const char* str, vga_color_t color);
    static void print(const char* str);
    static void putc(const char c, vga_color_t);
    static void new_line();
    static void scroll();
};



#endif //MYOS_VGA_H
