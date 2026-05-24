// ============================================================
// ESP32 CYD Server Screen — Main Application Shell
// Board: ESP32-2432S028R | Display: ILI9341 320x240
// ============================================================
#include <Arduino.h>
#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include "settings.h"

// --- HARDWARE PINS ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
#define TFT_CS_PIN 15
#define LED_R 4
#define LED_G 16
#define LED_B 17
#define SCR_W 320
#define SCR_H 240
#define LDR_PIN 34
#define BACKLIGHT_PIN 21
#define BACKLIGHT_CHANNEL 6

// --- GLOBAL OBJECTS ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCR_W * 4];
static lv_color_t buf2[SCR_W * 4];

lv_obj_t *scr_main;
volatile unsigned long lastTouchTime = 0;

// --- INSPECTOR IMAGE DEFINITION ---
extern "C" const lv_img_dsc_t inspector_pixel;

// RGB LED Control
void setLED(bool red, bool green, bool blue) {
  digitalWrite(LED_R, red ? LOW : HIGH);
  digitalWrite(LED_G, green ? LOW : HIGH);
  digitalWrite(LED_B, blue ? LOW : HIGH);
}

// Backlight Brightness Control
void setBacklightBrightness(uint8_t brightness) {
  ledcWrite(BACKLIGHT_CHANNEL, brightness);
}

// --- DISPLAY FLUSH CALLBACK ---
void my_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *px) {
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)px, w * h, false);
  tft.endWrite();
  lv_disp_flush_ready(drv);
}

// --- TOUCH CALLBACK ---
void my_touch_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  if (ts.touched()) {
    lastTouchTime = millis();
    TS_Point p = ts.getPoint();
    if (p.z < 200) {
      data->state = LV_INDEV_STATE_REL;
      return;
    }
    int16_t x = ::map(p.x, 200, 3700, 0, SCR_W);
    int16_t y = ::map(p.y, 240, 3800, 0, SCR_H);
    data->point.x = constrain(x, 0, SCR_W - 1);
    data->point.y = constrain(y, 0, SCR_H - 1);
    data->state = LV_INDEV_STATE_PR;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// --- MAIN SCREEN USER INTERFACE ---
void create_ui() {
  scr_main = lv_scr_act();
  
  // Disable scrolling on main screen to prevent scroll gestures / scrollbars
  lv_obj_clear_flag(scr_main, LV_OBJ_FLAG_SCROLLABLE);

  // 1. Render the Inspector image on the left side of the screen
  lv_obj_t *img_inspector = lv_img_create(scr_main);
  lv_img_set_src(img_inspector, &inspector_pixel);
  lv_obj_align(img_inspector, LV_ALIGN_LEFT_MID, 20, 0);

  // 2. Settings button at bottom-left in the exact same position as reference project
  lv_obj_t *btn_gear = lv_btn_create(scr_main);
  lv_obj_set_size(btn_gear, 32, 32);
  lv_obj_align(btn_gear, LV_ALIGN_BOTTOM_LEFT, 5, -5);
  lv_obj_set_style_bg_opa(btn_gear, 0, 0);       // Transparent background
  lv_obj_set_style_shadow_width(btn_gear, 0, 0); // No shadow
  
  lv_obj_t *lbl_gear = lv_label_create(btn_gear);
  lv_obj_set_style_text_font(lbl_gear, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_gear, LV_SYMBOL_SETTINGS);
  
  // Use color hex 0x000000 which shows as white on inverted CYD display
  lv_obj_set_style_text_color(lbl_gear, lv_color_hex(0x000000), 0); 
  lv_obj_center(lbl_gear);

  lv_obj_add_event_cb(
      btn_gear,
      [](lv_event_t *e) {
        // Switch to the Settings screen created in settings.cpp
        lv_tabview_set_act(tabview, 0, LV_ANIM_OFF);
        lv_scr_load_anim(scr_settings, LV_SCR_LOAD_ANIM_MOVE_TOP, 300, 0, false);
      },
      LV_EVENT_CLICKED, NULL);
}

void setup() {
  Serial.begin(115200);

  // RGB LED configuration
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setLED(false, false, false); // All LEDs off

  // Display initialization
  pinMode(TFT_CS_PIN, OUTPUT);
  pinMode(XPT2046_CS, OUTPUT);
  digitalWrite(TFT_CS_PIN, HIGH);
  digitalWrite(XPT2046_CS, HIGH);
  
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Backlight configuration using PWM
  ledcSetup(BACKLIGHT_CHANNEL, 5000, 8);
  ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
  setBacklightBrightness(255); // Full brightness by default
  
  pinMode(LDR_PIN, INPUT);
  
  // Touch screen initialization
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCR_W * 4);
  
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCR_W;
  disp_drv.ver_res = SCR_H;
  disp_drv.flush_cb = my_flush_cb;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Read dark mode preferences to apply theme
  Preferences prefs;
  prefs.begin("settings", true);
  bool darkMode = prefs.getBool("dark", true);
  prefs.end();

  // Initialize default theme (Inverted to compensate for CYD screen)
  lv_theme_t *theme = lv_theme_default_init(NULL,
                                            lv_palette_main(LV_PALETTE_BLUE),
                                            lv_palette_main(LV_PALETTE_RED),
                                            !darkMode, 
                                            LV_FONT_DEFAULT);
  lv_disp_set_theme(NULL, theme);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_cb;
  lv_indev_drv_register(&indev_drv);

  // Create user interfaces
  create_ui();
  create_settings_ui();

  lastTouchTime = millis();

  // Create a 1-second interval timer for auto-brightness / LDR management
  lv_timer_create(
      [](lv_timer_t *t) {
        static bool isScreenOn = true;
        int ldrVal = analogRead(LDR_PIN);
        
        // Simple auto-dimming logic: if LDR value is high (darkness), dim the backlight
        // Using a default threshold of 15 like the reference project
        int threshold = 15;
        
        if (ldrVal >= threshold) {
          // Dark environment: turn off backlight after 5 seconds of inactivity
          if (millis() - lastTouchTime > 5000) {
            setBacklightBrightness(0);
            isScreenOn = false;
          } else {
            setBacklightBrightness(255);
          }
        } else {
          // Bright environment or recent touch: keep backlight full
          setBacklightBrightness(255);
          isScreenOn = true;
        }
      },
      1000, NULL);
}

void loop() {
  lv_timer_handler();
  delay(5);
}
