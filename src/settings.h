#ifndef SETTINGS_H
#define SETTINGS_H

#include <lvgl.h>

// Declare global screens and functions
void create_settings_ui();
extern lv_obj_t *scr_settings;
extern lv_obj_t *scr_main;
extern lv_obj_t *tabview;
extern lv_obj_t *tab_agents;

#endif
