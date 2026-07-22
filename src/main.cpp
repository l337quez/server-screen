// ============================================================
// ESP32 CYD Server Screen — Main Application Shell
// Board: ESP32-2432S028R | Display: ILI9341 320x240
// ============================================================
#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include <ArduinoJson.h> // <-- Librería vital para parsear el JSON de la PC
#include "settings.h"
#include <DNSServer.h>
#include <WebServer.h>
#include "portal_html.h"

// --- CONFIGURACIÓN DE RED Y API ---
String global_ssid = "";
String global_pass = "";
String global_api_ip = "192.168.1.100";
#define WS_SERVER_PORT 8000
#define WS_SERVER_PATH "/ws"

WebSocketsClient webSocket;
DNSServer dnsServer;
WebServer webServer(80);
bool apModeActive = false;
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);

unsigned long connectionLedTimer = 0;
bool connectionLedActive = false;
unsigned long messageLedTimer = 0;
bool messageLedActive = false;

// --- FreeRTOS & Async Queue ---
TaskHandle_t TaskWebSockets;
String pending_json = "";
volatile bool has_pending_json = false;

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

// Componentes de la UI ahora globales para poder actualizarlos desde el puerto serie
lv_obj_t *img_inspector;
lv_obj_t *lbl_inspector_name;
lv_obj_t *lbl_inspector_msg;
lv_obj_t *cnt_inspector_msg;

lv_obj_t *img_lawyer;
lv_obj_t *lbl_lawyer_name;
lv_obj_t *lbl_lawyer_msg;
lv_obj_t *cnt_lawyer_msg;

// Main screen globals
lv_obj_t *img_main_avatar;
lv_obj_t *lbl_main_title;
lv_obj_t *lbl_main_msg;
lv_obj_t *cnt_main_msg;
bool is_ronaldo_active = false;
// --- CUSTOM FONTS ---
LV_FONT_DECLARE(lv_font_minecraftia_16);

// --- IMAGE DEFINITIONS ---
extern "C" const lv_img_dsc_t inspector_pixel_invert_only;
extern "C" const uint16_t sprite_lawyer[];

const lv_img_dsc_t img_lawyer_dsc = {
  {
    LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED, // cf
    0, // always_zero/magic
    0, // reserved
    111, // w
    200 // h
  },
  22200 * 2, // data_size
  (const uint8_t *)sprite_lawyer // data
};

extern "C" const uint16_t sprite_dupin_frontal[];
extern "C" const uint16_t sprite_ronaldo_frontal[];

const lv_img_dsc_t img_dupin_frontal_dsc = {
  {
    LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED,
    0, 0, 70, 70
  },
  70 * 70 * 2,
  (const uint8_t *)sprite_dupin_frontal
};

const lv_img_dsc_t img_ronaldo_frontal_dsc = {
  {
    LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED,
    0, 0, 70, 70
  },
  70 * 70 * 2,
  (const uint8_t *)sprite_ronaldo_frontal
};

// Agents state tracking

// Estados en RAM de los diálogos y nombres de los agentes
String name_inspector = "Dupin";
String msg_inspector = "I found new test\nin the investigation";
bool active_inspector = true;
String name_lawyer = "Jace";
String msg_lawyer = "We was winning\nthis case";
bool active_lawyer = true;
String name_ronaldo = "Ronaldo";
String msg_ronaldo = "Waiting new bug to fix";
bool active_ronaldo = true;


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

void actualizar_interfaz_inspector(const char* name, const char* message, bool is_active = true) {
  String name_text = String(name) + " Inspector";
  if (!is_active) {
    name_text += " [OFF]";
  }
  lv_label_set_text(lbl_inspector_name, name_text.c_str());

  if (!is_active) {
    lv_label_set_text(lbl_inspector_msg, "Agent is currently offline.");
  } else {
    lv_label_set_text(lbl_inspector_msg, message);
  }
  
  lv_obj_scroll_to_y(cnt_inspector_msg, 0, LV_ANIM_OFF);
  lv_obj_align_to(lbl_inspector_name, img_inspector, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  lv_obj_align_to(cnt_inspector_msg, img_inspector, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
}

void actualizar_interfaz_lawyer(const char* name, const char* message, bool is_active = true) {
  String name_text = String(name) + " Lawyer";
  if (!is_active) {
    name_text += " [OFF]";
  }
  lv_label_set_text(lbl_lawyer_name, name_text.c_str());

  if (!is_active) {
    lv_label_set_text(lbl_lawyer_msg, "Agent is currently offline.");
  } else {
    lv_label_set_text(lbl_lawyer_msg, message);
  }
  
  lv_obj_scroll_to_y(cnt_lawyer_msg, 0, LV_ANIM_OFF);
  lv_obj_align_to(lbl_lawyer_name, img_lawyer, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  lv_obj_align_to(cnt_lawyer_msg, img_lawyer, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
}

void actualizar_main_screen() {
  if (is_ronaldo_active) {
    lv_img_set_src(img_main_avatar, &img_ronaldo_frontal_dsc);
    String title = name_ronaldo + " Developer";
    lv_label_set_text(lbl_main_title, title.c_str());
    lv_label_set_text(lbl_main_msg, msg_ronaldo.c_str());
  } else {
    lv_img_set_src(img_main_avatar, &img_dupin_frontal_dsc);
    String title = name_inspector + " Inspector";
    lv_label_set_text(lbl_main_title, title.c_str());
    lv_label_set_text(lbl_main_msg, msg_inspector.c_str());
  }
}

// --- MAIN SCREEN USER INTERFACE ---
void create_ui() {
  // --- Main Screen Avatar Image (Top Left) ---
  img_main_avatar = lv_img_create(scr_main);
  lv_obj_align(img_main_avatar, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_flag(img_main_avatar, LV_OBJ_FLAG_CLICKABLE);

  // --- Main Screen Title (Right of Image) ---
  lbl_main_title = lv_label_create(scr_main);
  lv_obj_set_style_text_font(lbl_main_title, &lv_font_montserrat_18, 0); 
  lv_obj_set_style_text_color(lbl_main_title, lv_color_hex(0x000000), 0); 
  lv_obj_align(lbl_main_title, LV_ALIGN_TOP_LEFT, 100, 50);

  // --- Main Screen Text Box Container (Below Image) ---
  cnt_main_msg = lv_obj_create(scr_main);
  lv_obj_set_size(cnt_main_msg, 300, 140); // Enlarged to fill remaining space
  lv_obj_align(cnt_main_msg, LV_ALIGN_BOTTOM_MID, 0, -10);
  
  lv_obj_set_style_bg_opa(cnt_main_msg, 0, 0); 
  lv_obj_set_style_border_width(cnt_main_msg, 1, 0); // Light border
  lv_obj_set_style_border_color(cnt_main_msg, lv_color_hex(0x000000), 0);
  lv_obj_set_style_pad_all(cnt_main_msg, 8, 0); 
  lv_obj_clear_flag(cnt_main_msg, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_scrollbar_mode(cnt_main_msg, LV_SCROLLBAR_MODE_AUTO);

  // --- Main Screen Text Box Label ---
  lbl_main_msg = lv_label_create(cnt_main_msg);
  lv_obj_set_style_text_font(lbl_main_msg, &lv_font_minecraftia_16, 0);
  lv_obj_set_style_text_color(lbl_main_msg, lv_color_hex(0x000000), 0); 
  lv_obj_set_width(lbl_main_msg, LV_PCT(100));
  lv_obj_set_style_text_align(lbl_main_msg, LV_TEXT_ALIGN_LEFT, 0);

  // --- Image Click Callback to Swap ---
  lv_obj_add_event_cb(img_main_avatar, [](lv_event_t *e) {
    is_ronaldo_active = !is_ronaldo_active;
    actualizar_main_screen();
  }, LV_EVENT_CLICKED, NULL);

  // Settings button stays on scr_main (Moved to top right)
  lv_obj_t *btn_gear = lv_btn_create(scr_main);
  lv_obj_set_size(btn_gear, 32, 32);
  lv_obj_align(btn_gear, LV_ALIGN_TOP_RIGHT, -5, 5);
  lv_obj_set_style_bg_opa(btn_gear, 0, 0); 
  lv_obj_set_style_shadow_width(btn_gear, 0, 0); 
  
  lv_obj_t *lbl_gear = lv_label_create(btn_gear);
  lv_obj_set_style_text_font(lbl_gear, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_gear, LV_SYMBOL_SETTINGS);
  lv_obj_set_style_text_color(lbl_gear, lv_color_hex(0x000000), 0); 
  lv_obj_center(lbl_gear);

  lv_obj_add_event_cb(btn_gear, [](lv_event_t *e) {
    lv_tabview_set_act(tabview, 0, LV_ANIM_OFF);
    lv_scr_load_anim(scr_settings, LV_SCR_LOAD_ANIM_MOVE_TOP, 300, 0, false);
  }, LV_EVENT_CLICKED, NULL);

  // --- Inspector Container (Top) ---
  lv_obj_t *cont_insp = lv_obj_create(tab_agents);
  lv_obj_set_size(cont_insp, LV_PCT(100), 220);
  lv_obj_align(cont_insp, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_opa(cont_insp, 0, 0);
  lv_obj_set_style_border_width(cont_insp, 0, 0);
  lv_obj_clear_flag(cont_insp, LV_OBJ_FLAG_SCROLLABLE);

  img_inspector = lv_img_create(cont_insp);
  lv_img_set_src(img_inspector, &inspector_pixel_invert_only);
  lv_obj_align(img_inspector, LV_ALIGN_LEFT_MID, 5, -15);

  lbl_inspector_name = lv_label_create(cont_insp);
  lv_obj_set_style_text_color(lbl_inspector_name, lv_color_hex(0x000000), 0); 
  lv_label_set_text(lbl_inspector_name, "Dupin Inspector");
  lv_obj_set_style_text_align(lbl_inspector_name, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align_to(lbl_inspector_name, img_inspector, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  cnt_inspector_msg = lv_obj_create(cont_insp);
  lv_obj_set_size(cnt_inspector_msg, 160, 140);
  lv_obj_align_to(cnt_inspector_msg, img_inspector, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
  lv_obj_set_style_bg_opa(cnt_inspector_msg, 0, 0);
  lv_obj_set_style_border_width(cnt_inspector_msg, 0, 0);
  lv_obj_set_style_pad_all(cnt_inspector_msg, 0, 0);
  lv_obj_clear_flag(cnt_inspector_msg, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_scrollbar_mode(cnt_inspector_msg, LV_SCROLLBAR_MODE_AUTO);

  lbl_inspector_msg = lv_label_create(cnt_inspector_msg);
  lv_obj_set_style_text_font(lbl_inspector_msg, &lv_font_minecraftia_16, 0);
  lv_obj_set_style_text_color(lbl_inspector_msg, lv_color_hex(0x000000), 0); 
  lv_label_set_text(lbl_inspector_msg, "Waiting for data\nfrom PC...");
  lv_obj_set_width(lbl_inspector_msg, LV_PCT(100));
  lv_obj_set_style_text_align(lbl_inspector_msg, LV_TEXT_ALIGN_LEFT, 0);

  // --- Lawyer Container (Below) ---
  lv_obj_t *cont_lawy = lv_obj_create(tab_agents);
  lv_obj_set_size(cont_lawy, LV_PCT(100), 220);
  lv_obj_align(cont_lawy, LV_ALIGN_TOP_MID, 0, 230); // Position below the first
  lv_obj_set_style_bg_opa(cont_lawy, 0, 0);
  lv_obj_set_style_border_width(cont_lawy, 0, 0);
  lv_obj_clear_flag(cont_lawy, LV_OBJ_FLAG_SCROLLABLE);

  img_lawyer = lv_img_create(cont_lawy);
  lv_img_set_src(img_lawyer, &img_lawyer_dsc);
  lv_obj_align(img_lawyer, LV_ALIGN_LEFT_MID, 5, -15);

  lbl_lawyer_name = lv_label_create(cont_lawy);
  lv_obj_set_style_text_color(lbl_lawyer_name, lv_color_hex(0x000000), 0); 
  lv_label_set_text(lbl_lawyer_name, "Jace Lawyer");
  lv_obj_set_style_text_align(lbl_lawyer_name, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align_to(lbl_lawyer_name, img_lawyer, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  cnt_lawyer_msg = lv_obj_create(cont_lawy);
  lv_obj_set_size(cnt_lawyer_msg, 160, 140);
  lv_obj_align_to(cnt_lawyer_msg, img_lawyer, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
  lv_obj_set_style_bg_opa(cnt_lawyer_msg, 0, 0);
  lv_obj_set_style_border_width(cnt_lawyer_msg, 0, 0);
  lv_obj_set_style_pad_all(cnt_lawyer_msg, 0, 0);
  lv_obj_clear_flag(cnt_lawyer_msg, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_scrollbar_mode(cnt_lawyer_msg, LV_SCROLLBAR_MODE_AUTO);

  lbl_lawyer_msg = lv_label_create(cnt_lawyer_msg);
  lv_obj_set_style_text_font(lbl_lawyer_msg, &lv_font_minecraftia_16, 0);
  lv_obj_set_style_text_color(lbl_lawyer_msg, lv_color_hex(0x000000), 0); 
  lv_label_set_text(lbl_lawyer_msg, "Waiting for data\nfrom PC...");
  lv_obj_set_width(lbl_lawyer_msg, LV_PCT(100));
  lv_obj_set_style_text_align(lbl_lawyer_msg, LV_TEXT_ALIGN_LEFT, 0);
  
  // Set initial states based on variables
  actualizar_interfaz_inspector(name_inspector.c_str(), msg_inspector.c_str(), active_inspector);
  actualizar_interfaz_lawyer(name_lawyer.c_str(), msg_lawyer.c_str(), active_lawyer);
  actualizar_main_screen();
}

// --- WEBSOCKET EVENT HANDLER ---
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Desconectado del servidor!");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WS] Conectado a url: %s\n", payload);
      webSocket.sendTXT("Conectado desde ESP32 CYD");
      break;
    case WStype_TEXT:
      {
        // Extraemos el JSON en Core 0
        String json_string = "";
        for(size_t i = 0; i < length; i++) {
          json_string += (char)payload[i];
        }
        
        Serial.print("[WS] Recibido JSON en Core 0: ");
        Serial.println(json_string);

        // Lo guardamos para que Core 1 (UI) lo procese sin bloquearse
        pending_json = json_string;
        has_pending_json = true;
      }
      break;
  }
}

void startAPMode() {
  Serial.println("[AP Mode] Iniciando punto de acceso y servidor de configuracion...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Agent_Screen_Setup");
  
  delay(100);
  
  dnsServer.start(DNS_PORT, "*", apIP);
  
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html", PORTAL_HTML);
  });

  webServer.on("/scan", HTTP_GET, []() {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
      if (i > 0) json += ",";
      json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
    json += "]";
    WiFi.scanDelete();
    webServer.send(200, "application/json", json);
  });
  
  webServer.on("/connect", HTTP_POST, []() {
    String ssid = webServer.arg("ssid");
    String pass = webServer.arg("pass");
    String api_ip = webServer.arg("api_ip");

    if (api_ip.length() == 0) {
      api_ip = "192.168.1.2";
    }
    
    if (ssid.length() > 0) {
      Preferences p;
      p.begin("wifi", false);
      p.putString("ssid", ssid);
      p.putString("pass", pass);
      p.putString("api_ip", api_ip);
      p.end();
      
      webServer.send(200, "text/plain", "Guardado! Reiniciando...");
      delay(1500);
      ESP.restart();
    } else {
      webServer.send(400, "text/plain", "El SSID es requerido.");
    }
  });
  
  webServer.on("/generate_204", []() { webServer.sendHeader("Location", "http://192.168.4.1/"); webServer.send(302, "text/plain", ""); });
  webServer.onNotFound([]() { webServer.sendHeader("Location", "http://192.168.4.1/"); webServer.send(302, "text/plain", ""); });
  
  webServer.begin();
  apModeActive = true;
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.println("WIFI SETUP MODE");
  tft.println("");
  tft.setTextSize(1);
  tft.println("1. Conectate a Wi-Fi:");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("   Agent_Screen_Setup");
  tft.println("");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("2. Abre en el navegador:");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("   http://192.168.4.1");
}

// --- TASK SECUNDARIA PARA RED (CORE 0) ---
void TaskWebSocketsCode( void * pvParameters ) {
  for(;;) {
    if (!apModeActive) {
      webSocket.loop();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Prevenir que colapse el procesador
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setLED(false, false, false);

  pinMode(TFT_CS_PIN, OUTPUT);
  pinMode(XPT2046_CS, OUTPUT);
  digitalWrite(TFT_CS_PIN, HIGH);
  digitalWrite(XPT2046_CS, HIGH);
  
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  ledcSetup(BACKLIGHT_CHANNEL, 5000, 8);
  ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
  setBacklightBrightness(255); 

  Preferences p;
  p.begin("wifi", true);
  global_ssid = p.getString("ssid", "");
  global_pass = p.getString("pass", "");
  global_api_ip = p.getString("api_ip", "192.168.1.2");
  p.end();

  // Forzar la IP correcta en caso de que se haya guardado mal en el portal
  if (global_api_ip == "192.168.1.100") {
    global_api_ip = "192.168.1.2";
  }

  if (global_ssid == "") {
    setLED(true, false, false); // Rojo fijo si no hay WiFi configurado
    startAPMode();
    return; // Detener inicialización si entra en AP Mode
  }

  // WiFi Setup
  WiFi.begin(global_ssid.c_str(), global_pass.c_str());
  Serial.print("Connecting to WiFi...");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nError conectando al WiFi. Iniciando AP Mode.");
    setLED(true, false, false); // Rojo encendido por fallo de conexion
    startAPMode();
    return;
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Encender LED Verde indicando éxito
  setLED(false, true, false); 
  connectionLedActive = true;
  connectionLedTimer = millis();

  // WebSocket Setup
  webSocket.begin(global_api_ip.c_str(), WS_SERVER_PORT, WS_SERVER_PATH);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);


  
  pinMode(LDR_PIN, INPUT);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCR_W * 4);
  
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCR_W;
  disp_drv.ver_res = SCR_H;
  disp_drv.flush_cb = my_flush_cb;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  Preferences prefs;
  prefs.begin("settings", true);
  bool darkMode = prefs.getBool("dark", true);
  prefs.end();

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

  scr_main = lv_scr_act();
  lv_obj_clear_flag(scr_main, LV_OBJ_FLAG_SCROLLABLE);

  create_settings_ui();
  create_ui();

  lastTouchTime = millis();

  lv_timer_create(
      [](lv_timer_t *t) {
        int ldrVal = analogRead(LDR_PIN);
        int threshold = 15;
        if (ldrVal >= threshold) {
          if (millis() - lastTouchTime > 5000) {
            setBacklightBrightness(0);
          } else {
            setBacklightBrightness(255);
          }
        } else {
          setBacklightBrightness(255);
        }
      },
      1000, NULL);

  // Inicializar Tarea Secundaria en el Núcleo 0 (Para descargar la UI)
  xTaskCreatePinnedToCore(
      TaskWebSocketsCode,   /* Funcion de la tarea */
      "TaskWebSockets",     /* Nombre */
      10000,                /* Tamaño de pila */
      NULL,                 /* Parametros */
      1,                    /* Prioridad */
      &TaskWebSockets,      /* Handle */
      0);                   /* Núcleo 0 */
}

void loop() {
  if (apModeActive) {
    dnsServer.processNextRequest();
    webServer.handleClient();
    delay(10);
    return;
  }

  // --- Procesamiento de Mensajes en Core 1 (Seguro para LVGL) ---
  if (has_pending_json) {
    has_pending_json = false;
    String json_to_process = pending_json;
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json_to_process);

    if (!error) {
      JsonArray agentes = doc["agentes"];
      bool global_alarm = false;

      if (doc["hardware"]["is_alarm"] | false) {
        global_alarm = true;
      }

      if (!agentes.isNull() && agentes.size() > 0) {
        for (JsonObject agente : agentes) {
          const char* name = agente["name"] | "Unknown";
          const char* role = agente["role"] | "inspector";
          const char* message = agente["message"] | "";
          bool is_alarm = agente["is_alarm"] | false;
          bool is_active = agente["is_active"] | true;

          if (is_alarm) global_alarm = true;

          if (strcmp(role, "lawyer") == 0) {
            name_lawyer = name;
            msg_lawyer = message;
            active_lawyer = is_active;
          } else if (strcmp(role, "developer") == 0 || String(name).equalsIgnoreCase("Ronaldo")) {
            name_ronaldo = name;
            msg_ronaldo = message;
            active_ronaldo = is_active;
          } else {
            name_inspector = name;
            msg_inspector = message;
            active_inspector = is_active;
          }
        }

        if (global_alarm) {
          setLED(true, false, false); 
          Serial.println("[LED] Alarm detected! RED turned ON.");
        } else {
          setLED(true, true, false); // Naranja (Rojo + Verde)
          messageLedActive = true;
          messageLedTimer = millis();
          Serial.println("[LED] New message received. ORANGE turned ON.");
        }

        actualizar_interfaz_inspector(name_inspector.c_str(), msg_inspector.c_str(), active_inspector);
        actualizar_interfaz_lawyer(name_lawyer.c_str(), msg_lawyer.c_str(), active_lawyer);
        actualizar_main_screen();
      }
    } else {
      Serial.print(F("[JSON] Error en parseo: "));
      Serial.println(error.f_str());
    }
  }

  // Apagar LED Verde despues de 3 segundos de conexion
  if (connectionLedActive && (millis() - connectionLedTimer > 3000)) {
    setLED(false, false, false);
    connectionLedActive = false;
  }

  // Apagar LED Naranja despues de 1 segundo de mensaje
  if (messageLedActive && (millis() - messageLedTimer > 1000)) {
    setLED(false, false, false);
    messageLedActive = false;
  }

  lv_timer_handler(); // Atender los refrescos y animaciones de LVGL (Sin bloqueos de red)
  delay(5);
}