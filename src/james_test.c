#include "pebble_fonts.h"
#include "num2words.h"

#define MY_UUID { 0xF6, 0x93, 0x61, 0x62, 0xCA, 0xC0, 0x40, 0xEC, 0xBB, 0x9B, 0x9C, 0xBA, 0x8F, 0x7B, 0xD4, 0xE6 }
PBL_APP_INFO(MY_UUID,
             "Words + Date", "James Hrisho",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer text_date_layer;

typedef struct CommonWordsData {
  TextLayer label;
  char buffer[BUFFER_SIZE];
  PropertyAnimation *in_animation;
  PropertyAnimation *out_animation;
  void (*update) (PblTm *t, char *words);
} CommonWordsData;

static CommonWordsData s_data;
static CommonWordsData s_data_minutes;
static CommonWordsData s_data_sminutes;
static CommonWordsData date;
static PropertyAnimation in_animation1;
static PropertyAnimation in_animation2;
static PropertyAnimation in_animation3;
static PropertyAnimation in_animation4;
static PropertyAnimation out_animation1;
static PropertyAnimation out_animation2;
static PropertyAnimation out_animation3;
static PropertyAnimation out_animation4;


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
  PblTm t;
  get_time(&t);
  layer->update(&t, layer->buffer);
  slide_in(layer->in_animation, layer);
  animation_schedule(&layer->in_animation->animation);
}

static void handle_minute_tick(AppContextRef app_ctx, PebbleTickEvent* e) {
  PblTm *t = e->tick_time;
  if((e->units_changed & MINUTE_UNIT) == MINUTE_UNIT) {
    if (!(t->tm_min == 14 || t->tm_min == 17 || t->tm_min == 18 || t->tm_min == 19)) {
      slide_out(s_data_sminutes.out_animation, &s_data_sminutes);
      animation_set_handlers(&s_data_sminutes.out_animation->animation, (AnimationHandlers){
        .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
      }, (void *) &s_data_sminutes);
      animation_schedule(&s_data_sminutes.out_animation->animation);
    }
    if (t->tm_min % 10 == 0 || (t->tm_min > 10 && t->tm_min < 20) || t->tm_min == 1) {
      slide_out(s_data_minutes.out_animation, &s_data_minutes);
      animation_set_handlers(&s_data_minutes.out_animation->animation, (AnimationHandlers){
        .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
      }, (void *) &s_data_minutes);
      animation_schedule(&s_data_minutes.out_animation->animation);
    }
  }
  if ((e->units_changed & HOUR_UNIT) == HOUR_UNIT) {
    slide_out(s_data.out_animation, &s_data);
    animation_set_handlers(&s_data.out_animation->animation, (AnimationHandlers){
      .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
    }, (void *) &s_data);
    animation_schedule(&s_data.out_animation->animation);
  }
  if ((e->units_changed & DAY_UNIT) == DAY_UNIT) {
    strcpy(date.buffer, "Xxxxxxxxx 00");
    string_format_time(date.buffer, sizeof(date.buffer), "%B %e", e->tick_time);
    slide_out(date.out_animation, &date);
    animation_set_handlers(&date.out_animation->animation, (AnimationHandlers){
      .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
    }, (void *) &date);
    animation_schedule(&date.out_animation->animation);
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
  s_data.update = &fuzzy_hours_to_words;
  s_data.in_animation = &in_animation3;
  s_data.out_animation = &out_animation3;
  text_layer_init(&s_data.label, GRect(0, 0, window.layer.frame.size.w, 45));
  text_layer_set_background_color(&s_data.label, GColorBlack);
  text_layer_set_text_color(&s_data.label, GColorWhite);
  text_layer_set_font(&s_data.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_40)));
  layer_add_child(&window.layer, &s_data.label.layer);

// 00 minutes
  s_data_minutes.update = &fuzzy_minutes_to_words;
  s_data_minutes.in_animation = &in_animation2;
  s_data_minutes.out_animation = &out_animation2;
  text_layer_init(&s_data_minutes.label,GRect(0, 46, window.layer.frame.size.w, 34));
  text_layer_set_background_color(&s_data_minutes.label, GColorBlack);
  text_layer_set_text_color(&s_data_minutes.label, GColorWhite);
  text_layer_set_font(&s_data_minutes.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_30)));
  layer_add_child(&window.layer, &s_data_minutes.label.layer);

// single digits
  s_data_sminutes.update = &fuzzy_sminutes_to_words;
  s_data_sminutes.in_animation = &in_animation1;
  s_data_sminutes.out_animation = &out_animation1;
  text_layer_init(&s_data_sminutes.label,GRect(0, 81, window.layer.frame.size.w, 35));
  text_layer_set_background_color(&s_data_sminutes.label, GColorBlack);
  text_layer_set_text_color(&s_data_sminutes.label, GColorWhite);
  text_layer_set_font(&s_data_sminutes.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_LIGHT_30)));
  layer_add_child(&window.layer, &s_data_sminutes.label.layer);

//Date
  date.update = &fuzzy_date;
  date.in_animation = &in_animation4;
  date.out_animation = &out_animation4;
  text_layer_init(&date.label, GRect(0, 120, window.layer.frame.size.w, 48));
  text_layer_set_text_color(&date.label, GColorWhite);
  text_layer_set_background_color(&date.label, GColorBlack);
  text_layer_set_font(&date.label, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  text_layer_set_text_alignment(&date.label, GTextAlignmentLeft);
  layer_add_child(&window.layer, &date.label.layer);

//show your face
  PblTm t;
  get_time(&t);

  fuzzy_sminutes_to_words(&t, s_data_sminutes.buffer);
  slide_in(s_data_sminutes.in_animation, &s_data_minutes);
  animation_schedule(&s_data_sminutes.in_animation->animation);

  fuzzy_minutes_to_words(&t, s_data_minutes.buffer);
  slide_in(s_data_minutes.in_animation, &s_data_sminutes);
  animation_schedule(&s_data_minutes.in_animation->animation);

  fuzzy_hours_to_words(&t, s_data.buffer);
  slide_in(s_data.in_animation, &s_data);
  animation_schedule(&s_data.in_animation->animation);

  fuzzy_date(&t, date.buffer);
  slide_in(date.in_animation, &date);
  animation_schedule(&date.in_animation->animation);

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
