#pragma once

#include "pebble_os.h"
#include "pebble_app.h"
#include "string.h"

#define SMALL 0
#define DAY 0
#define BUFFER_SIZE 43

#if SMALL
#define DATE_FORMAT "%B %e"
#elif DAY
#define DATE_FORMAT "%a %B %e"
#else
#define DATE_FORMAT "%b %e"
#endif

void fuzzy_minutes_to_words(PblTm *t, char* words);
void fuzzy_hours_to_words(PblTm *t, char* words);
void fuzzy_sminutes_to_words(PblTm *t, char* words);
void fuzzy_dates_to_words(PblTm *t, char* words);