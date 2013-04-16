#include "pebble_fonts.h"
#include "num2words.h"

#define MY_UUID { 0xF6, 0x93, 0x61, 0x62, 0xCA, 0xC0, 0x40, 0xEC, 0xBB, 0x9B, 0x9C, 0xBA, 0x8F, 0x7B, 0xD4, 0xE6 }
#define TIME_SLOT_ANIMATION_DURATION 700
#define NUM_LAYERS 4

PBL_APP_INFO(MY_UUID,
             "Words + Date", "Daniel Hertz + James Hrisho",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer text_date_layer;

typedef struct CommonWordsData {
  TextLayer label;
  PropertyAnimation in_animation;
  PropertyAnimation out_animation;
  void (*update) (PblTm *t, char *words);
  char buffer[BUFFER_SIZE];
} CommonWordsData;

static struct CommonWordsData layers[NUM_LAYERS] =
{{ .update = &fuzzy_sminutes_to_words },
 { .update = &fuzzy_minutes_to_words },
 { .update = &fuzzy_hours_to_words },
 { .update = &fuzzy_dates_to_words}};

static GFont light;
static GFont bold;
static GFont small;

void slide_out(PropertyAnimation *animation, CommonWordsData *layer) {
  GRect from_frame = layer_get_frame(&layer->label.layer);

  GRect to_frame = GRect(-window.layer.frame.size.w, from_frame.origin.y,
                          window.layer.frame.size.w, from_frame.size.h);

  property_animation_init_layer_frame(animation, &layer->label.layer, NULL,
                                        &to_frame);
  animation_set_duration(&animation->animation, TIME_SLOT_ANIMATION_DURATION);
  animation_set_curve(&animation->animation, AnimationCurveEaseIn);
}

void slide_in(PropertyAnimation *animation, CommonWordsData *layer) {
  GRect to_frame = layer_get_frame(&layer->label.layer);
  GRect from_frame = GRect(2*window.layer.frame.size.w, to_frame.origin.y,
                            window.layer.frame.size.w, to_frame.size.h);

  layer_set_frame(&layer->label.layer, from_frame);
  text_layer_set_text(&layer->label, layer->buffer);
  property_animation_init_layer_frame(animation, &layer->label.layer, NULL,
                                        &to_frame);
  animation_set_duration(&animation->animation, TIME_SLOT_ANIMATION_DURATION);
  animation_set_curve(&animation->animation, AnimationCurveEaseOut);
}

void slide_out_animation_stopped(Animation *slide_out_animation, bool finished,
                                  void *context) {
  CommonWordsData *layer = (CommonWordsData *)context;
  layer->label.layer.frame.origin.x = 0;
  PblTm t;
  get_time(&t);
  layer->update(&t, layer->buffer);
  slide_in(&layer->in_animation, layer);
  animation_schedule(&layer->in_animation.animation);
}

void update_layer(CommonWordsData *layer) {
  slide_out(&layer->out_animation, layer);
  animation_set_handlers(&layer->out_animation.animation, (AnimationHandlers){
    .stopped = (AnimationStoppedHandler)slide_out_animation_stopped
  }, (void *) layer);
  animation_schedule(&layer->out_animation.animation);
}

static void handle_minute_tick(AppContextRef app_ctx, PebbleTickEvent* e) {
  PblTm *t = e->tick_time;
  if((e->units_changed & MINUTE_UNIT) == MINUTE_UNIT) {
    if (!(t->tm_min == 14 || t->tm_min == 17 || t->tm_min == 18
          || t->tm_min == 19)) {
      update_layer(&layers[0]);
    }
    if (t->tm_min % 10 == 0 || (t->tm_min > 10 && t->tm_min < 20)
          || t->tm_min == 1) {
      update_layer(&layers[1]);
    }
  }
  if ((e->units_changed & HOUR_UNIT) == HOUR_UNIT ||
        ((t->tm_hour == 00 || t->tm_hour == 12) && t->tm_min == 1)) {
    update_layer(&layers[2]);
  }
  if ((e->units_changed & DAY_UNIT) == DAY_UNIT) {
    update_layer(&layers[3]);
  }
}

void init_layer(CommonWordsData *layer, GRect rect, GFont font) {
  text_layer_init(&layer->label, rect);
  text_layer_set_background_color(&layer->label, GColorClear);
  text_layer_set_text_color(&layer->label, GColorWhite);
  text_layer_set_font(&layer->label, font);
  layer_add_child(&window.layer, &layer->label.layer);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Words + Date");
  const bool animated = true;
  window_stack_push(&window, animated);
  window_set_background_color(&window, GColorBlack);
  resource_init_current_app(&APP_RESOURCES);
  light = fonts_load_custom_font(resource_get_handle(
                      RESOURCE_ID_FONT_ROBOTO_LIGHT_30));
  bold = fonts_load_custom_font(resource_get_handle(
                      RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_40));
  small = fonts_load_custom_font(resource_get_handle(
                      RESOURCE_ID_FONT_ROBOTO_CONDENSED_21));
// single digits
  init_layer(&layers[0], GRect(0, 81, window.layer.frame.size.w, 35), light);

// 00 minutes
  init_layer(&layers[1], GRect(0, 46, window.layer.frame.size.w, 34), light);

//hours
  init_layer(&layers[2], GRect(0, 0, window.layer.frame.size.w, 45), bold);

//Date
  init_layer(&layers[3], GRect(0, 120, window.layer.frame.size.w, 48), small);

//show your face
  PblTm t;
  get_time(&t);

  for (int i = 0; i < NUM_LAYERS; ++i)
  {
    layers[i].update(&t, layers[i].buffer);
    slide_in(&layers[i].in_animation, &layers[i]);
    animation_schedule(&layers[i].in_animation.animation);
  }
}

void handle_deinit(AppContextRef ctx) {
  fonts_unload_custom_font(light);
  fonts_unload_custom_font(bold);
  fonts_unload_custom_font(small);
}

void pbl_main(void *params) {
 PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }

  };
  app_event_loop(params, &handlers);
}


