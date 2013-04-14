#include "num2words.h"

static const char* const ONES[] = {
  "",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine"
};

static const char* const TEENS[] ={
  "",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen"
};

static const char* const TENS[] = {
  "",
  "ten",
  "twenty",
  "thirty",
  "forty",
  "fifty",
  "sixty",
  "seventy",
  "eighty",
  "ninety"
};

static const char* STR_OH_CLOCK = "o'clock";
static const char* STR_NOON = "noon";
static const char* STR_MIDNIGHT = "midnight";
static const char* STR_OH = "o' ";

static size_t append_number(char* words, int num,int hours) {
  int tens_val = num / 10 % 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val > 0) {
    if (tens_val == 1 && num != 10) {
      strcat(words, TEENS[ones_val]);
      return strlen(TEENS[ones_val]);
    }
    strcat(words, TENS[tens_val]);
    len += strlen(TENS[tens_val]);
    if (ones_val > 0) {
      strcat(words, " ");
      len += 1;
    }
  }
  if(hours == 1){
   if (ones_val > 0 || num == 0) {
    strcat(words, ONES[ones_val]);
    len += strlen(ONES[ones_val]);
    }
  }
  return len;
}
static size_t append_minutes_number(char* words, int num) {
  int tens_val = num / 10;
  int ones_val = num % 10;

  size_t len = 0;

  if (tens_val > 0) {
    if (tens_val == 1 && num != 10) {
      return 0;
    }
  }

  if (ones_val > 0 || num == 0) {
    strcat(words, ONES[ones_val]);
    len += strlen(ONES[ones_val]);
  }
  return len;
}

static size_t append_string(char* buffer, const size_t length, const char* str) {
  strncat(buffer, str, length);

  size_t written = strlen(str);
  return (length > written) ? written : length;
}

void fuzzy_minutes_to_words(int hours, int minutes, char* words, size_t length) {
  int fuzzy_hours = hours;
  int fuzzy_minutes = minutes;

  size_t remaining = length;
  memset(words, 0, length);
  
  //Is it midnight? or noon
  if ((fuzzy_hours == 0 && fuzzy_minutes == 0) || (fuzzy_hours == 12 && fuzzy_minutes == 0)){
   //then do nothing
  } else {
    //is it the top of the hour?
    if(fuzzy_minutes == 0){
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, STR_OH_CLOCK);
    }else if(fuzzy_minutes < 10){
      //is it before ten minutes into the hour
      remaining -= append_string(words, remaining, " ");
      remaining -= append_string(words, remaining, STR_OH);
    }
      remaining -= append_number(words, fuzzy_minutes,0);
 
  }
}

void fuzzy_sminutes_to_words(int hours, int minutes, char* words, size_t length) {
  int fuzzy_hours = hours;
  int fuzzy_minutes = minutes;

  size_t remaining = length;
  memset(words, 0, length);
  
  //Is it midnight? or noon
  if ((fuzzy_hours == 0 && fuzzy_minutes == 0) || (fuzzy_hours == 12 && fuzzy_minutes == 0)){
   //then do nothing
  } else {
      remaining -= append_minutes_number(words, fuzzy_minutes);
 
  }
}

void fuzzy_hours_to_words(int hours, int minutes, char* words, size_t length) {
  int fuzzy_hours = hours;
  int fuzzy_minutes = minutes;

  size_t remaining = length;
  memset(words, 0, length);
  
  //Is it midnight?
  if (fuzzy_hours == 0 && fuzzy_minutes == 0) {
    remaining -= append_string(words, remaining, STR_MIDNIGHT);
  //is it noon?
  } else if (fuzzy_hours == 12 && fuzzy_minutes == 0) {
    remaining -= append_string(words, remaining, STR_NOON);
  } else if (fuzzy_hours == 0 || fuzzy_hours == 12){
    remaining -= append_number(words, 12,1);
  } else {
    //get hour
    remaining -= append_number(words, fuzzy_hours % 12,1);
  }
}
