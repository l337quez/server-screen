/**
 * @file lv_conf.h
 * Configuration file for v8.3.11
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/*Color depth: 1 (1 bit per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define LV_COLOR_DEPTH 16

/*Swap the 2 bytes of RGB565 color. Useful if the display has a 8-bit interface (e.g. SPI)*/
#define LV_COLOR_16_SWAP 1

/*Chroma key color of images (color to be transparent)*/
#define LV_COLOR_CHROMA_KEY lv_color_hex(0xFFFFFF)

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*1: use custom malloc/free, 0: use the built-in `lv_mem_alloc` and `lv_mem_free`*/
#define LV_MEM_CUSTOM 0

#if LV_MEM_CUSTOM == 0
    /*Size of the memory pool in bytes for `lv_mem_alloc` in bytes [kB]*/
    #define LV_MEM_SIZE (48U * 1024U)          /*[bytes]*/

    /*Compiler prefix for a big array to be used as memory pool*/
    #define LV_MEM_ATTR

    /*Set an address for the memory pool instead of allocating it as an array.
     * Can be in external SRAM with special attributes*/
    #define LV_MEM_ADR 0
#endif

/*====================
   HAL SETTINGS
 *====================*/

/*Use a custom tick source without calling `lv_tick_inc` in an interrupt/task.
 * 1: Use custom tick, 0: Use system tick*/
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

/*=======================
 * FEATURE CONFIGURATIONS
 *=======================*/

/*-------------
 * Drawing
 *-----------*/

/*Enable complex shadows, gradients, etc. Note: 0 makes drawing faster*/
#define LV_DRAW_COMPLEX 1

/*========================
 * THEMES AND TEMPLATES
 *========================*/

/*A simple, generic theme*/
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    /*1: Allow dark mode, 0: Light mode only*/
    #define LV_THEME_DEFAULT_DARK 1

    /*1: Enable grow on hover/focus, 0: Disable transition*/
    #define LV_THEME_DEFAULT_GROW 1

    /*Default transition time in [ms]*/
    #define LV_THEME_DEFAULT_TRANSIT_TIME 300
#endif

/*==================
 *  FONT USAGE
 *==================*/

/*Montserrat fonts*/
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_36 1

/*Set default font*/
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*==================
 *   LV_OBJ SETTINGS
 *==================*/

/*Declare symbols*/
#define LV_USE_FONT_SUBPX 0

#endif /*LV_CONF_H*/
