#pragma once

#include "string.h"

void fuzzy_minutes_to_words(int hours, int minutes, char* words, size_t length);
void fuzzy_hours_to_words(int hours, int minutes, char* words, size_t length);
void fuzzy_sminutes_to_words(int hours, int minutes, char* words, size_t length);
