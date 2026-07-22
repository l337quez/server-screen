#include "settings.h"
#include <Arduino.h>
#include <Preferences.h>

lv_obj_t *scr_settings;
lv_obj_t *tabview;
lv_obj_t *tab_agents;

void create_settings_ui() {
  // 1. Create the Settings Screen
  scr_settings = lv_obj_create(NULL);

  // 2. Create Tabview (Tabs at the top, height 40)
  tabview = lv_tabview_create(scr_settings, LV_DIR_TOP, 40);

  // 3. Add the "Agents" tab
  tab_agents = lv_tabview_add_tab(tabview, "Agents");

  // 4. Add the "System" tab
  lv_obj_t *tab_system = lv_tabview_add_tab(tabview, "System");

  // --- SYSTEM TAB CONTENT ---
  // Clean, elegant label and toggle for Dark Mode
  lv_obj_t *lbl_theme = lv_label_create(tab_system);
  lv_label_set_text(lbl_theme, "Dark Mode");
  lv_obj_align(lbl_theme, LV_ALIGN_TOP_LEFT, 15, 20);
  lv_obj_set_style_text_font(lbl_theme, &lv_font_montserrat_18, 0);

  lv_obj_t *sw_theme = lv_switch_create(tab_system);
  lv_obj_align(sw_theme, LV_ALIGN_TOP_RIGHT, -15, 18);

  // Load current theme setting from Preferences
  Preferences p;
  p.begin("settings", true);
  bool currentDark = p.getBool("dark", true);
  p.end();

  if (currentDark) {
    lv_obj_add_state(sw_theme, LV_STATE_CHECKED);
  } else {
    lv_obj_clear_state(sw_theme, LV_STATE_CHECKED);
  }

  // Event handler for toggling Dark Mode
  lv_obj_add_event_cb(
      sw_theme,
      [](lv_event_t *e) {
        lv_obj_t *obj = lv_event_get_target(e);
        bool dark = lv_obj_has_state(obj, LV_STATE_CHECKED);

        Preferences p;
        p.begin("settings", false);
        p.putBool("dark", dark);
        p.end();

        // Restart ESP32 to apply theme globally
        ESP.restart();
      },
      LV_EVENT_VALUE_CHANGED, NULL);

  // 4. Back / Home Button in bottom right
  lv_obj_t *btn_back = lv_btn_create(scr_settings);
  lv_obj_set_size(btn_back, 36, 36);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
  lv_obj_set_style_bg_opa(btn_back, 0, 0);       // Transparent background
  lv_obj_set_style_shadow_width(btn_back, 0, 0); // No shadow

  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_back, LV_SYMBOL_HOME);
  
  // Use color hex 0x000000 which shows as white on inverted CYD display
  lv_obj_set_style_text_color(lbl_back, lv_color_hex(0x000000), 0);
  lv_obj_center(lbl_back);

  lv_obj_add_event_cb(
      btn_back,
      [](lv_event_t *e) {
        lv_scr_load_anim(scr_main, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 300, 0, false);
      },
      LV_EVENT_CLICKED, NULL);
}
