#ifndef __FONTS_FREESANS_TTF_H__
#define __FONTS_FREESANS_TTF_H__
// TrueType equivalents of the GFX fonts in FreeSans.h, used when
// OWM_USE_TTF_TEXT is defined.
//
// The GFX fonts were created by Adafruit's fontconvert which renders at
// 141 DPI, so an N point GFX font has an em size of N * 141 / 72 pixels.
// The value is the TrueType em size in pixels, rounded to the nearest pixel.
static const OwmFont_t FONT_5pt8b  = {10};
static const OwmFont_t FONT_6pt8b  = {12};
static const OwmFont_t FONT_7pt8b  = {14};
static const OwmFont_t FONT_8pt8b  = {16};
static const OwmFont_t FONT_11pt8b = {22};
static const OwmFont_t FONT_12pt8b = {24};
static const OwmFont_t FONT_14pt8b = {27};
static const OwmFont_t FONT_16pt8b = {31};
static const OwmFont_t FONT_18pt8b = {35};
static const OwmFont_t FONT_26pt8b = {51};
static const OwmFont_t FONT_48pt8b_temperature = {94};
#endif
