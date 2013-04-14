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

typedef struct CommonWordsData {
  TextLayer label;
  char buffer[BUFFER_SIZE];
} CommonWordsData;

static CommonWordsData s_data;
static CommonWordsData s_data_minutes;
static CommonWordsData s_data_sminutes;
static CommonWordsData date;

#define TIME_SLOT_ANIMATION_DURATION 700

void slide_out(PropertyAnimation *animation, CommonWordsData *layer) {
  GRect from_frame = layer_get_frame(&layer->label.layer);

  GRect to_frame = GRect(-window.layer.frame.size.w, from_frame.origin.y,
                          window.layer.frame.size.w, from_frame.size.h);

  property_animation_init_layer_frame(animation, &layer->label.layer, NULL, &to_frame);
  animation_set_duration(&animation->animation, TIME_SLOT_ANIMATION_DURATION);
  animation_set_curve(&animation->animation, AnimationCurveEaseIn);
}

void slide_in(PropertyAnimation *animation, CommonWordsData *layer) {
  GRect to_frame = layer_get_frame(&layer->label.layer);
  GRect from_frame = GRect(2*window.layer.frame.size.w, to_frame.origin.y,
                          window.layer.frame.size.w, to_frame.size.h);

  layer_set_frame(&layer->label.layer, from_frame);
  text_layer_set_text(&layer->label, layer->buffer);
  property_animation_init_layer_frame(animation, &layer->label.layer, NULL, &to_frame);
  animation_set_duration(&animation->animation, TIME_SLOT_ANIMATION_DURATION);
  animation_set_curve(&animation->animation, AnimationCurveEaseOut);
}

void slide_out_animation_stopped(Animation *slide_out_animation, bool finished, void *context) {
  CommonWordsData *layer = (CommonWordsData *)context;
  layer->label.layer.frame.origin.x = 0;
  static PropertyAnimation animation;
  slide_in(&animation, layer);
  animation_schedule(&animation.animation);
}

static void handle_minute_tick(AppContextRef app_ctx, PebbleTickEvent* e) {
  PblTm *t = e->tick_time;
  if((e->units_changed & MINUTE_UNIT) == MINUTE_UNIT) {
    fuzzy_sminutes_to_words(t->tm_hour, t->tm_min, s_data_sminutes.buffer, BUFFER_SIZE);
    static PropertyAnimation animation2;
    slide_out(&animation2, &s_data_sminutes);
    animation_set_handlers(&animation2.animation, (AnimationHandlers){
      .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
    }, (void *) &s_data_sminutes);
    animation_schedule(&animation2.animation);
    if (t->tm_min % 10 == 0 || t->tm_min < 20) {
      fuzzy_minutes_to_words(t->tm_hour, t->tm_min, s_data_minutes.buffer, BUFFER_SIZE);
      static PropertyAnimation animation1;
      slide_out(&animation1, &s_data_minutes);
      animation_set_handlers(&animation1.animation, (AnimationHandlers){
        .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
      }, (void *) &s_data_minutes);
      animation_schedule(&animation1.animation);
    }
  }
  if ((e->units_changed & HOUR_UNIT) == HOUR_UNIT) {
    fuzzy_hours_to_words(t->tm_hour, t->tm_min, s_data.buffer, BUFFER_SIZE);
    static PropertyAnimation animation3;
    slide_out(&animation3, &s_data);
    animation_set_handlers(&animation3.animation, (AnimationHandlers){
      .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
    }, (void *) &s_data);
    animation_schedule(&animation3.animation);
  }
  if ((e->units_changed & DAY_UNIT) == DAY_UNIT) {
    strcpy(date.buffer,"Xxxxxxxxx 00");
    string_format_time(date.buffer, sizeof(date.buffer), "%B %e", e->tick_time);
    static PropertyAnimation animation4;
    slide_out(&animation4, &date);
    animation_set_handlers(&animation4.animation, (AnimationHandlers){
      .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
    }, (void *) &date);
    animation_schedule(&animation4.animation);
  }
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

  text_layer_init(&date.label, GRect(0, 120, window.layer.frame.size.w, 48));
  text_layer_set_text_color(&date.label, GColorWhite);
  text_layer_set_background_color(&date.label, GColorBlack);
  text_layer_set_font(&date.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  text_layer_set_text_alignment(&date.label, GTextAlignmentLeft);
  layer_add_child(&window.layer, &date.label.layer);

//show your face
  PblTm t;
  get_time(&t);

  fuzzy_sminutes_to_words(t.tm_hour, t.tm_min, s_data_sminutes.buffer, BUFFER_SIZE);
  static PropertyAnimation animation1;
  slide_in(&animation1, &s_data_minutes);
  animation_schedule(&animation1.animation);
  fuzzy_minutes_to_words(t.tm_hour, t.tm_min, s_data_minutes.buffer, BUFFER_SIZE);
  static PropertyAnimation animation2;
  slide_in(&animation2, &s_data_sminutes);
  animation_schedule(&animation2.animation);
  fuzzy_hours_to_words(t.tm_hour, t.tm_min, s_data.buffer, BUFFER_SIZE);
  static PropertyAnimation animation3;
  slide_in(&animation3, &s_data);
  animation_schedule(&animation3.animation);
  strcpy(date.buffer,"Xxxxxxxxx 00");
  string_format_time(date.buffer, sizeof(date.buffer), "%B %e", &t);
  static PropertyAnimation animation4;
  slide_in(&animation4, &date);
  animation_schedule(&animation4.animation);

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
