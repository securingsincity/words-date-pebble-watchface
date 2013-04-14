#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "num2words.h"

#define MY_UUID { 0xF6, 0x93, 0x61, 0x62, 0xCA, 0xC0, 0x40, 0xEC, 0xBB, 0x9B, 0x9C, 0xBA, 0x8F, 0x7B, 0xD4, 0xE6 }
PBL_APP_INFO(MY_UUID,
             "Words + Date", "James Hrisho",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);
#define BUFFER_SIZE 43

Window window;
TextLayer text_date_layer;

static struct CommonWordsData {
  TextLayer label;
  char buffer[BUFFER_SIZE];
} s_data;

static struct CommonWordsMinutesData {
  TextLayer label;
  char buffer[BUFFER_SIZE];
} s_data_minutes;

static struct CommonWordsSMinutesData {
  TextLayer label;
  char buffer[BUFFER_SIZE];
} s_data_sminutes;

static void update_time(PblTm* t) {
  fuzzy_hours_to_words(t->tm_hour, t->tm_min, s_data.buffer, BUFFER_SIZE);
  fuzzy_minutes_to_words(t->tm_hour, t->tm_min, s_data_minutes.buffer, BUFFER_SIZE);
  fuzzy_sminutes_to_words(t->tm_hour, t->tm_min, s_data_sminutes.buffer, BUFFER_SIZE);
  text_layer_set_text(&s_data.label, s_data.buffer);
  text_layer_set_text(&s_data_minutes.label, s_data_minutes.buffer);
  text_layer_set_text(&s_data_sminutes.label, s_data_sminutes.buffer);
}

static void handle_minute_tick(AppContextRef app_ctx, PebbleTickEvent* e) {
  update_time(e->tick_time);
  static char date_text[] = "Xxxxxxxxx 00";
  string_format_time(date_text, sizeof(date_text), "%B %e", e->tick_time);
  text_layer_set_text(&text_date_layer, date_text);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Words + Date");
  const bool animated = true;
  window_stack_push(&window, animated);
  window_set_background_color(&window, GColorBlack);
  resource_init_current_app(&APP_RESOURCES);

//hours 

  text_layer_init(&s_data.label, GRect(0, 0, window.layer.frame.size.w, 45));
  text_layer_set_background_color(&s_data.label, GColorBlack);
  text_layer_set_text_color(&s_data.label, GColorWhite);
  text_layer_set_font(&s_data.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_40)));
  layer_add_child(&window.layer, &s_data.label.layer);

// 00 minutes
  text_layer_init(&s_data_minutes.label,GRect(0, 46, window.layer.frame.size.w, 34));
  text_layer_set_background_color(&s_data_minutes.label, GColorBlack);
  text_layer_set_text_color(&s_data_minutes.label, GColorWhite);
  text_layer_set_font(&s_data_minutes.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_30)));
  layer_add_child(&window.layer, &s_data_minutes.label.layer);
// single digits
  text_layer_init(&s_data_sminutes.label,GRect(0, 81, window.layer.frame.size.w, 35));
  text_layer_set_background_color(&s_data_sminutes.label, GColorBlack);
  text_layer_set_text_color(&s_data_sminutes.label, GColorWhite);
  text_layer_set_font(&s_data_sminutes.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_30)));
  layer_add_child(&window.layer, &s_data_sminutes.label.layer);

//Date

  text_layer_init(&text_date_layer, GRect(7, 120, 144-7, 168-120));
  text_layer_set_text_color(&text_date_layer, GColorWhite);
  text_layer_set_background_color(&text_date_layer, GColorBlack);
  text_layer_set_font(&text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&window.layer, &text_date_layer.layer);

  PblTm t;
  get_time(&t);
  update_time(&t);
}


void pbl_main(void *params) {
 PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}
