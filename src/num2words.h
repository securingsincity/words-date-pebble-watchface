#pragma once

#include "pebble_os.h"
#include "pebble_app.h"
#include "string.h"

#define DAY 0
#define BUFFER_SIZE 43

#if DAY
#define DATE_FORMAT "%a %B %e"
#else
#define DATE_FORMAT "%B %e"
#endif

void fuzzy_minutes_to_words(PblTm *t, char* words);
void fuzzy_hours_to_words(PblTm *t, char* words);
void fuzzy_sminutes_to_words(PblTm *t, char* words);
void fuzzy_dates_to_words(PblTm *t, char* words);