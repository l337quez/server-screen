#ifndef CYD_COMPAT_H
#define CYD_COMPAT_H

#include <stdint.h>

// LVGL v8 compatibility defines for the v9-generated claude.c
#ifndef LV_COLOR_FORMAT_RGB565
#define LV_COLOR_FORMAT_RGB565 6
#endif

#ifndef LV_IMAGE_HEADER_MAGIC
#define LV_IMAGE_HEADER_MAGIC 0
#endif

typedef struct {
    struct {
        uint32_t cf : 5;
        uint32_t magic : 3;
        uint32_t reserved : 2;
        uint32_t w : 11;
        uint32_t h : 11;
    } header;
    uint32_t data_size;
    const uint8_t * data;
} lv_image_dsc_t;

#endif // CYD_COMPAT_H
