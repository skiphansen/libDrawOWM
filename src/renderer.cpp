/* Renderer for esp32-weather-epd.
 * Copyright (C) 2022-2026  Luke Marzen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "locale_en_US.inc"
#include "_strftime.h"
#include "api_response.h"
#include "config.h"
#include "conversions.h"
#include "display_utils.h"
#include <DrawOWM.h>

#define ENABLE_LOGGING  0
#if ENABLE_LOGGING && __has_include("logging.h") 
#include "logging.h"
#else
#define LOG(format, ...)
#define ELOG(format, ...)
#define LOG_RAW(format, ...)
#endif


// fonts
#include FONT_HEADER

// icon header files
#include "icons/icons.h"

#ifndef ACCENT_COLOR
  #define ACCENT_COLOR TFT_BLACK
#endif

/* Returns the string width in pixels
 */
uint16_t DrawOWM::getStringWidth(const String &text)
{
  return display.textWidth(text);
}

/* Returns the string height in pixels
 */
uint16_t DrawOWM::getStringHeight(const String &text)
{
  return display.fontHeight();
}

void DrawOWM::setCursor(int16_t x, int16_t y)
{
   display.setCursor(x + config.xOffset,y + config.yOffset);
}

int16_t DrawOWM::getCursorX(void)
{
   return display.getCursorX() - config.xOffset;
}

int16_t DrawOWM::getCursorY(void)
{
   return display.getCursorY() - config.yOffset;
}

void DrawOWM::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
{
   display.drawLine(x0 + config.xOffset,y0 + config.yOffset,x1 + config.xOffset,y1 + config.yOffset,color);
}

void DrawOWM::drawPixel(int32_t x, int32_t y, uint32_t color)
{
   display.drawPixel(x + config.xOffset,y + config.yOffset,color);
}

/* Draws a string with alignment
 */
void DrawOWM::drawString(int16_t x, int16_t y, const String &text, alignment_t alignment,
                uint16_t color)
{
  uint16_t w = display.textWidth(text);
  if (alignment == RIGHT)
  {
    x = x - w;
  }
  if (alignment == CENTER)
  {
    x = x - w / 2;
  }
  setCursor(x, y);
  display.setTextColor(color);
  display.print(text);
  x = getCursorX();
  if(MaxX < x) {
     MaxX = x;
     LOG("New MaxX \"%s\" %d\n",text.c_str(),MaxX);
  }
  y += display.fontHeight();
  if(MaxY < y) {
     MaxY = y;
     LOG("New MaxY \"%s\" %d\n",text.c_str(),MaxY);
  }
} // end drawString

/* Draws a string that will flow into the next line when max_width is reached.
 * If a string exceeds max_lines an ellipsis (...) will terminate the last word.
 * Lines will break at spaces(' ') and dashes('-').
 *
 * Note: max_width should be big enough to accommodate the largest word that
 *       will be displayed. If an unbroken string of characters longer than
 *       max_width exist in text, then the string will be printed beyond
 *       max_width.
 */
void DrawOWM::drawMultiLnString(int16_t x, int16_t y, const String &text,
                       alignment_t alignment, uint16_t max_width,
                       uint16_t max_lines, int16_t line_spacing,
                       uint16_t color)
{
  uint16_t current_line = 0;
  String textRemaining = text;
  // print until we reach max_lines or no more text remains
  while (current_line < max_lines && !textRemaining.isEmpty())
  {
    uint16_t w = display.textWidth(textRemaining);

    int endIndex = textRemaining.length();
    // check if remaining text is to wide, if it is then print what we can
    String subStr = textRemaining;
    int splitAt = 0;
    int keepLastChar = 0;
    while (w > max_width && splitAt != -1)
    {
      if (keepLastChar)
      {
        // if we kept the last character during the last iteration of this while
        // loop, remove it now so we don't get stuck in an infinite loop.
        subStr.remove(subStr.length() - 1);
      }

      // find the last place in the string that we can break it.
      if (current_line < max_lines - 1)
      {
        splitAt = std::max(subStr.lastIndexOf(" "),
                           subStr.lastIndexOf("-"));
      }
      else
      {
        // this is the last line, only break at spaces so we can add ellipsis
        splitAt = subStr.lastIndexOf(" ");
      }

      // if splitAt == -1 then there is an unbroken set of characters that is
      // longer than max_width. Otherwise if splitAt != -1 then we can continue
      // the loop until the string is <= max_width
      if (splitAt != -1)
      {
        endIndex = splitAt;
        subStr = subStr.substring(0, endIndex + 1);

        char lastChar = subStr.charAt(endIndex);
        if (lastChar == ' ')
        {
          // remove this char now so it is not counted towards line width
          keepLastChar = 0;
          subStr.remove(endIndex);
          --endIndex;
        }
        else if (lastChar == '-')
        {
          // this char will be printed on this line and removed next iteration
          keepLastChar = 1;
        }

        if (current_line < max_lines - 1)
        {
          // this is not the last line
          w = display.textWidth(subStr);
        }
        else
        {
          // this is the last line, we need to make sure there is space for
          // ellipsis
          w = display.textWidth(subStr + "...");
          if (w <= max_width)
          {
            // ellipsis fit, add them to subStr
            subStr = subStr + "...";
          }
        }

      } // end if (splitAt != -1)
    } // end inner while

    drawString(x, y + (current_line * line_spacing), subStr, alignment, color);

    // update textRemaining to no longer include what was printed
    // +1 for exclusive bounds, +1 to get passed space/dash
    textRemaining = textRemaining.substring(endIndex + 2 - keepLastChar);

    ++current_line;
  } // end outer while

  return;
} // end drawMultiLnString

/* Initialize e-paper display
 */
void DrawOWM::initDisplay()
{
  display.setRotation(DISPLAY_ROTATION);
  display.setTextSize(1);
  display.setTextColor(TFT_BLACK);
  display.setTextWrap(false);
} // end initDisplay

/* Power-off e-paper display
 */
void DrawOWM::powerOffDisplay()
{
} // end initDisplay


/* These functions are responsible for drawing the current conditions and
 * associated icons on the left panel.
 */

// drawCurrentSunrise
void DrawOWM::drawCurrentSunrise(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = config.PosSunrise % 2;
  int PosY = static_cast<int>(config.PosSunrise / 2);
  const unsigned char *IconBitmap = NULL;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_sunrise_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_sunrise_48x48;
        break;
  }

  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,IconBitmap,
                     WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_SUNRISE, LEFT);

  // sunrise
  setFreeFont(ValueFont);
  char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
  time_t ts = current.sunrise;
  tm *timeInfo = localtime(&ts);
  _strftime(timeBuffer, sizeof(timeBuffer),config.TimeFormat, timeInfo);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, timeBuffer, LEFT);
}
// end drawCurrentSunrise

// drawCurrentWind
void DrawOWM::drawCurrentWind(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = (config.PosWind % 2);
  int PosY = static_cast<int>(config.PosWind / 2);
  const unsigned char *IconBitmap = NULL;

  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_strong_wind_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_strong_wind_48x48;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap, WI_SZ, WI_SZ, TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_WIND, LEFT);

  // wind
  setFreeFont(ValueFont);
#ifdef WIND_INDICATOR_ARROW
  if(config.DisplayWidth >= 640) {
     drawInvertedBitmap(WI_LOFF + (WI_COL * PosX),
                        WI_Y0 + 24 / 2 + WI_DY * PosY,
                        getWindBitmap24(current.wind_deg),24, 24, TFT_BLACK);
  }
#endif
   switch (config.WindSpeed) {
      case UNITS_SPEED_METERSPERSECOND:
         dataStr = String(static_cast<int>(std::round(current.wind_speed)));
         unitStr = String(" ") + TXT_UNITS_SPEED_METERSPERSECOND;
         break;
      case UNITS_SPEED_FEETPERSECOND:
         dataStr = String(static_cast<int>(std::round(
                                                     meterspersecond_to_feetpersecond(current.wind_speed) )));
         unitStr = String(" ") + TXT_UNITS_SPEED_FEETPERSECOND;
         break;
      case UNITS_SPEED_KILOMETERSPERHOUR:
         dataStr = String(static_cast<int>(std::round(
                                                     meterspersecond_to_kilometersperhour(current.wind_speed) )));
         unitStr = String(" ") + TXT_UNITS_SPEED_KILOMETERSPERHOUR;
         break;
      case UNITS_SPEED_MILESPERHOUR:
         dataStr = String(static_cast<int>(std::round(
                                                     meterspersecond_to_milesperhour(current.wind_speed) )));
         unitStr = String(" ") + TXT_UNITS_SPEED_MILESPERHOUR;
         break;
      case UNITS_SPEED_KNOTS:
         dataStr = String(static_cast<int>(std::round(
                                                     meterspersecond_to_knots(current.wind_speed) )));
         unitStr = String(" ") + TXT_UNITS_SPEED_KNOTS;
         break;
      case UNITS_SPEED_BEAUFORT:
         dataStr = String(meterspersecond_to_beaufort(current.wind_speed));
         unitStr = String(" ") + TXT_UNITS_SPEED_BEAUFORT;
         break;
   }

#ifdef WIND_INDICATOR_ARROW
   if(config.DisplayWidth >= 640) {
      drawString((WI_LOFF + 24) + (WI_COL * PosX),
                 WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
   }
   else  {
      drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
   }
#else
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
#endif
  const GFXfont *temp;
  temp = config.DisplayWidth >= 640 ? &FONT_8pt8b : &FONT_5pt8b;
  setFreeFont(temp);
  drawString(getCursorX(),WI_Y0 + WI_DDY + WI_DY * PosY,unitStr,LEFT);

#if defined(WIND_INDICATOR_NUMBER)
  dataStr = String(current.wind_deg) + "\260";
  setFreeFont(&FONT_12pt8b);
  drawString(getCursorX() + 6, 204 + 17 / 2 + (48 + 8) * PosY + 48 / 2,
             dataStr, LEFT);
#endif
#if defined(WIND_INDICATOR_CPN_CARDINAL)                \
 || defined(WIND_INDICATOR_CPN_INTERCARDINAL)           \
 || defined(WIND_INDICATOR_CPN_SECONDARY_INTERCARDINAL) \
 || defined(WIND_INDICATOR_CPN_TERTIARY_INTERCARDINAL)
  dataStr = getCompassPointNotation(current.wind_deg);
  temp = config.DisplayWidth >= 640 ? &FONT_12pt8b: &FONT_6pt8b;
  setFreeFont(temp);
  int16_t x_offset = config.DisplayWidth >= 640 ? 6 : 3;
  drawString(getCursorX() + x_offset,
             WI_Y0 + WI_DDY + WI_DY * PosY,dataStr, LEFT);
#endif

  return;
}
// end drawCurrentWind

// drawCurrentUVI
void DrawOWM::drawCurrentUVI(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = (config.PosUvi % 2);
  int PosY = static_cast<int>(config.PosUvi / 2);
  const unsigned char *IconBitmap = NULL;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_day_sunny_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_day_sunny_48x48;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_UV_INDEX, LEFT);

  // uv index
  setFreeFont(ValueFont);
  unsigned int uvi = static_cast<unsigned int>(
                                std::max(std::round(current.uvi), 0.0f));
  dataStr = String(uvi);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
  const GFXfont *temp = config.DisplayWidth >= 640 ? &FONT_7pt8b : &FONT_5pt8b;
  // spacing between end of index value and start of descriptor text
  const int sp = config.DisplayWidth >= 640 ? 8 : 2;
  setFreeFont(temp);

  dataStr = String(getUVIdesc(uvi));
  int max_w = (162 + (PosX * 162) - sp) - (getCursorX() + sp);
  if (getStringWidth(dataStr) <= max_w)
  { // Fits on a single line, draw along bottom
     drawString(getCursorX() + sp, WI_Y0 + WI_DDY + WI_DY * PosY,
                dataStr, LEFT);
  }
  else
  { // use smaller font
    setFreeFont(&FONT_5pt8b);
    if (getStringWidth(dataStr) <= max_w)
    { // Fits on a single line with smaller font, draw along bottom
       drawString(getCursorX() + sp,
                  WI_Y0 + WI_DDY + WI_DY * PosY,
                  dataStr, LEFT);
    }
    else
    { // Does not fit on a single line, draw higher to allow room for 2nd line
       drawMultiLnString(getCursorX() + sp,
                         WI_Y0 + WI_DDY + WI_DY * PosY - 10,
                         dataStr, LEFT, max_w, 2, 10);
    }
  }
}
// end drawCurrentUVI

// drawCurrentAirQuality
void DrawOWM::drawCurrentAirQuality(const owm_resp_air_pollution_t &owm_air_pollution)
{
  String dataStr, unitStr;
  int PosX = (config.PosAirQuality % 2);
  int PosY = static_cast<int>(config.PosAirQuality / 2);
  const unsigned char *IconBitmap = NULL;
  const GFXfont *TempFont;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = air_filter_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = air_filter_48x48;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);

  const char *air_quality_index_label;
  if (aqi_desc_type(AQI_SCALE) == AIR_QUALITY_DESC)
  {
    air_quality_index_label = TXT_AIR_QUALITY;
  }
  else // (aqi_desc_type(AQI_SCALE) == AIR_POLLUTION_DESC)
  {
    air_quality_index_label = TXT_AIR_POLLUTION;
  }
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, air_quality_index_label, LEFT);

  // spacing between end of index value and start of descriptor text
  const int sp = 8;

  // air quality index
  const owm_components_t &c = owm_air_pollution.components;
  // OpenWeatherMap does not provide pb (lead) conentrations, so we pass NULL.
  int aqi = calc_aqi(AQI_SCALE, c.co, c.nh3, c.no, c.no2, c.o3, NULL, c.so2,
                                c.pm10, c.pm2_5);
  int aqi_max = aqi_scale_max(AQI_SCALE);
  if (aqi > aqi_max)
  {
    dataStr = "> " + String(aqi_max);
  }
  else
  {
    dataStr = String(aqi);
  }
  setFreeFont(ValueFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);

  TempFont = config.DisplayWidth >= 640 ? &FONT_7pt8b: &FONT_5pt8b;
  setFreeFont(TempFont);
  dataStr = String(aqi_desc(AQI_SCALE, aqi));
  int max_w = (162 + (PosX * 162) - sp) - (getCursorX() + sp);
  if (getStringWidth(dataStr) <= max_w)
  { // Fits on a single line, draw along bottom
     drawString(getCursorX() + sp, WI_Y0 + WI_DDY + WI_DY * PosY,
                dataStr, LEFT);
  }
  else
  { // use smaller font
    setFreeFont(&FONT_5pt8b);
    if (getStringWidth(dataStr) <= max_w)
    { // Fits on a single line with smaller font, draw along bottom
       drawString(getCursorX() + sp,
                  WI_Y0 + WI_DDY + WI_DY * PosY,
                  dataStr, LEFT);
    }
    else
    { // Does not fit on a single line, draw higher to allow room for 2nd line
       drawMultiLnString(getCursorX() + sp,
                         WI_Y0 + WI_DDY + WI_DY * PosY - 10,
                         dataStr, LEFT, max_w, 2, 10);
    }
  }
}
// end drawCurrentAirQuality

// drawCurrentInTemp
void DrawOWM::drawCurrentInTemp(float inTemp)
{
  String dataStr, unitStr;
  int PosX = (config.PosIntemp % 2);
  int PosY = static_cast<int>(config.PosIntemp / 2);
  const unsigned char *IconBitmap = NULL;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = house_thermometer_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = house_thermometer_48x48;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX),WI_Y0 + WI_LDY + WI_DY * PosY,
             TXT_INDOOR_TEMPERATURE,LEFT);

  // indoor temperature
  setFreeFont(ValueFont);
  if (!std::isnan(inTemp))
  {
    if(config.bMetric) {
       dataStr = String(std::round(inTemp * 10) / 10.0f, 1);
    }
    else {
       dataStr = String(static_cast<int>(
                 std::round(celsius_to_fahrenheit(inTemp))));
    }
  }
  else
  {
    dataStr = "--";
  }
  dataStr += "\260";
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
}
// end drawCurrentInTemp

// drawCurrentSunset
void DrawOWM::drawCurrentSunset(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = (config.PosSunset % 2);
  int PosY = static_cast<int>(config.PosSunset / 2);
  const unsigned char *IconBitmap = NULL;

  // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_sunset_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_sunset_48x48;
        break;
  }
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,IconBitmap,
                     WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_SUNSET, LEFT);

  // sunset
  setFreeFont(ValueFont);
  char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
  time_t ts = current.sunset;
  tm *timeInfo = localtime(&ts);
  _strftime(timeBuffer, sizeof(timeBuffer),config.TimeFormat, timeInfo);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, timeBuffer, LEFT);
}
// end drawCurrentSunset

// drawCurrentHumidity
void DrawOWM::drawCurrentHumidity(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = (config.PosHumidity % 2);
  int PosY = static_cast<int>(config.PosHumidity / 2);
  const unsigned char *IconBitmap = NULL;
  const GFXfont *TempFont;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_humidity_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_humidity_48x48;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  TempFont = config.DisplayWidth >= 640 ? &FONT_7pt8b : &FONT_6pt8b;
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_HUMIDITY, LEFT);

  // humidity
  setFreeFont(ValueFont);
  dataStr = String(current.humidity);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
  TempFont = config.DisplayWidth >= 640 ? &FONT_8pt8b : &FONT_5pt8b;
  setFreeFont(TempFont);
  drawString(getCursorX(), WI_Y0 + WI_DDY + WI_DY * PosY, "%", LEFT);
}
// end drawCurrentHumidity

// drawCurrentPressure
void DrawOWM::drawCurrentPressure(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = (config.PosPressure % 2);
  int PosY = static_cast<int>(config.PosPressure / 2);
  const GFXfont *TempFont;
  const unsigned char *IconBitmap = NULL;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_barometer_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_barometer_48x48;
        break;
  }
  //  icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  //  labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_PRESSURE, LEFT);

  // pressure
  switch(config.PressureType) {
     case UNITS_PRES_HECTOPASCALS:
        dataStr = String(current.pressure);
        unitStr = String(" ") + TXT_UNITS_PRES_HECTOPASCALS;
        break;

      case UNITS_PRES_PASCALS:
        dataStr = String(static_cast<int>(std::round(
                         hectopascals_to_pascals(current.pressure) )));
        unitStr = String(" ") + TXT_UNITS_PRES_PASCALS;
        break;

      case UNITS_PRES_MILLIMETERSOFMERCURY:
        dataStr = String(static_cast<int>(std::round(
                         hectopascals_to_millimetersofmercury(current.pressure) )));
        unitStr = String(" ") + TXT_UNITS_PRES_MILLIMETERSOFMERCURY;
        break;

      case UNITS_PRES_INCHESOFMERCURY:
        dataStr = String(std::round(1e1f *
                         hectopascals_to_inchesofmercury(current.pressure)
                         ) / 1e1f, 1);
        unitStr = String(" ") + TXT_UNITS_PRES_INCHESOFMERCURY;
        break;

      case UNITS_PRES_MILLIBARS:
        dataStr = String(static_cast<int>(std::round(
                         hectopascals_to_millibars(current.pressure) )));
        unitStr = String(" ") + TXT_UNITS_PRES_MILLIBARS;
        break;

      case UNITS_PRES_ATMOSPHERES:
        dataStr = String(std::round(1e3f *
                         hectopascals_to_atmospheres(current.pressure) )
                         / 1e3f, 3);
        unitStr = String(" ") + TXT_UNITS_PRES_ATMOSPHERES;
        break;

      case UNITS_PRES_GRAMSPERSQUARECENTIMETER:
        dataStr = String(static_cast<int>(std::round(
                         hectopascals_to_gramspersquarecentimeter(current.pressure)
                         )));
        unitStr = String(" ") + TXT_UNITS_PRES_GRAMSPERSQUARECENTIMETER;
        break;

     case UNITS_PRES_POUNDSPERSQUAREINCH:
        dataStr = String(std::round(1e2f *
                         hectopascals_to_poundspersquareinch(current.pressure)
                         ) / 1e2f, 2);
        unitStr = String(" ") + TXT_UNITS_PRES_POUNDSPERSQUAREINCH;
        break;
  }

  setFreeFont(ValueFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
  TempFont = config.DisplayWidth >= 640 ? &FONT_8pt8b: &FONT_5pt8b;
  setFreeFont(TempFont);
  drawString(getCursorX(), WI_Y0 + WI_DDY + WI_DY * PosY,unitStr, LEFT);
}
// end drawCurrentPressure

// drawCurrentVisibility
void DrawOWM::drawCurrentVisibility(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = (config.PosVisibility % 2);
  int PosY = static_cast<int>(config.PosVisibility / 2);
  const unsigned char *IconBitmap = NULL;
  const GFXfont *TempFont;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = visibility_icon_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = visibility_icon_48x48;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_VISIBILITY, LEFT);

  // visibility
  setFreeFont(ValueFont);

  float vis;
  if(config.DistanceType == UNITS_DIST_KILOMETERS) {
     vis = meters_to_kilometers(current.visibility);
     unitStr = String(" ") + TXT_UNITS_DIST_KILOMETERS;
  }
  else {
     vis = meters_to_miles(current.visibility);
     unitStr = String(" ") + TXT_UNITS_DIST_MILES;
  }

  // if visibility is less than 1.95, round to 1 decimal place
  // else round to int
  if (vis < 1.95)
  {
    dataStr = String(std::round(10 * vis) / 10.0, 1);
  }
  else
  {
    dataStr = String(static_cast<int>(std::round(vis)));
  }
  if ((config.DistanceType == UNITS_DIST_KILOMETERS && vis >= 10) ||
      (config.DistanceType == UNITS_DIST_MILES && vis >= 6))
  {
    dataStr = "> " + dataStr;
  }
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
  TempFont = config.DisplayWidth >= 640 ? &FONT_8pt8b : &FONT_5pt8b;
  setFreeFont(TempFont);
  drawString(getCursorX(), WI_Y0 + WI_DDY + WI_DY * PosY,unitStr, LEFT);
}
// end drawCurrentVisibility

// drawCurrentInHumidit
void DrawOWM::drawCurrentInHumidity(float inHumidity)
{
  String dataStr, unitStr;
  int PosX = (config.PosInhumidity % 2);
  int PosY = static_cast<int>(config.PosInhumidity / 2);
  const unsigned char *IconBitmap = NULL;
  const GFXfont *TempFont;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = house_humidity_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = house_humidity_48x48;
        break;
  }

  // current weather data icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // current weather data labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_INDOOR_HUMIDITY, LEFT);

  // indoor humidity
  if (!std::isnan(inHumidity))
  {
    dataStr = String(static_cast<int>(std::round(inHumidity)));
  }
  else
  {
    dataStr = "--";
  }
  setFreeFont(ValueFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);

  TempFont = config.DisplayWidth >= 640 ? &FONT_8pt8b: &FONT_5pt8b;
  setFreeFont(TempFont);
  drawString(getCursorX(), WI_Y0 + WI_DDY + WI_DY * PosY, "%", LEFT);
}
// end drawCurrentInHumidity

// drawCurrentMoonrise
void DrawOWM::drawCurrentMoonrise(const owm_daily_t &today)
{
  String dataStr, unitStr;
  int PosX = config.PosMoonrise % 2;
  int PosY = static_cast<int>(config.PosMoonrise / 2);
  const unsigned char *IconBitmap = NULL;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_moonrise_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_moonrise_48x48;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_MOONRISE, LEFT);

  // moonrise
  char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
  time_t ts = today.moonrise;
  tm *timeInfo = localtime(&ts);
  _strftime(timeBuffer, sizeof(timeBuffer), TIME_FORMAT, timeInfo);
  setFreeFont(ValueFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, timeBuffer, LEFT);
}
// end drawCurrentMoonrise

// drawCurrentMoonset
void DrawOWM::drawCurrentMoonset(const owm_daily_t &today)
{
  String dataStr, unitStr;
  int PosX = (config.PosMoonset % 2);
  int PosY = static_cast<int>(config.PosMoonset / 2);
  const unsigned char *IconBitmap = NULL;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_moonset_24x24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_moonset_48x48;
        break;
  }
  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                      IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);

  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_MOONSET, LEFT);

  // moonset
  char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
  time_t ts = today.moonset;
  tm *timeInfo = localtime(&ts);
  _strftime(timeBuffer, sizeof(timeBuffer), TIME_FORMAT, timeInfo);
  setFreeFont(ValueFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, timeBuffer, LEFT);
}
// end drawCurrentMoonset

// drawCurrentMoonphase
void DrawOWM::drawCurrentMoonphase(const owm_daily_t &daily)
{
  String dataStr, unitStr;
  int PosX = (config.PosMoonphase % 2);
  int PosY = static_cast<int>(config.PosMoonphase / 2);
  const unsigned char *IconBitmap = NULL;
  const GFXfont *TempFont;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = getMoonPhaseBitmap24(daily);
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = getMoonPhaseBitmap48(daily);
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);
  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_MOONPHASE, LEFT);

  // moonphase
  const int sp = 8;
  dataStr = String(getMoonPhaseStr(daily));
  int max_w = (162 + (PosX * 162) - sp) - (48 + (PosX * 162));
  TempFont = config.DisplayWidth >= 640 ? &FONT_7pt8b: &FONT_5pt8b;
  setFreeFont(TempFont);
  if (getStringWidth(dataStr) <= max_w)
  { // Fits on a single line, draw along bottom
     drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY,
                dataStr, LEFT);
  }
  else
  { // use smaller font
    setFreeFont(&FONT_5pt8b);
    if (getStringWidth(dataStr) <= max_w)
    { // Fits on a single line with smaller font, draw along bottom
       drawString(WI_LOFF + (WI_COL * PosX),
                  WI_Y0 + WI_DDY + WI_DY * PosY,
                  dataStr, LEFT);
    }
    else
    { // Does not fit on a single line, draw higher to allow room for 2nd line
       drawMultiLnString(WI_LOFF + (WI_COL * PosX),
                         WI_Y0 + WI_DDY + WI_DY * PosY - 10,
                         dataStr, LEFT, max_w, 2, 10);
    }
  }
}
// end drawCurrentMoonphase

// drawCurrentDewpoint
void DrawOWM::drawCurrentDewpoint(const owm_current_t &current)
{
  String dataStr, unitStr;
  int PosX = (config.PosDewpoint % 2);
  int PosY = static_cast<int>(config.PosDewpoint / 2);
  const unsigned char *IconBitmap = NULL;
  const unsigned char *IconBitmap1 = NULL;
  uint16_t WI_SZ1 = 0;

    // icons
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_thermometer_24x24;
        IconBitmap1 = wi_raindrops_16x16;
        WI_SZ1 = 16;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_thermometer_48x48;
        IconBitmap1 = wi_raindrops_24x24;
        WI_SZ1 = 24;
        break;
  }

  // icons
  drawInvertedBitmap(WI_COL * PosX, WI_Y0 + WI_DY * PosY,
                     IconBitmap,WI_SZ,WI_SZ,TFT_BLACK);
  
  // icons
  drawInvertedBitmap(WI_COL * PosX + 24 - 12, WI_Y0 + WI_DY * PosY + 4,
                     IconBitmap1,WI_SZ1,WI_SZ1,TFT_BLACK);
  
  // labels
  setFreeFont(LabelFont);
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_LDY + WI_DY * PosY, TXT_DEWPOINT, LEFT);

  // Dew point
  setFreeFont(ValueFont);
  if (!std::isnan(current.dew_point))
  {
     if(config.bMetric) {
        dataStr = String(std::round(kelvin_to_celsius(current.dew_point) * 10) / 10.0f, 1);
     }
     else {
     dataStr = String(static_cast<int>(
               std::round(kelvin_to_fahrenheit(current.dew_point))));
     }
  }
  else
  {
    dataStr = "--";
  }
  dataStr += "\260";
  drawString(WI_LOFF + (WI_COL * PosX), WI_Y0 + WI_DDY + WI_DY * PosY, dataStr, LEFT);
} 
// end drawCurrentDewpoint

//End defining functions for left panel.

void DrawOWM::drawInit()
{
   MaxX = 65535;
   MaxY = 65535;

   switch (config.DisplayFormat) {
      case FORMAT_400X300:
         LabelFont = &FONT_6pt8b;
         ValueFont = &FONT_7pt8b;
         UnitFont = &FONT_7pt8b;
         WI_COL  = 85;   // column width (px)
         WI_Y0   = 104;  // first-row base y
         WI_DY   = 40;   // row stride (icon 24 + gap 10)
         WI_SZ   = 24;   // icon size
         WI_LOFF = 24;   // label/data x offset from col start
         WI_LDY  = 5;    // label baseline delta from row base
         WI_DDY  = 17;   // data baseline delta from row base (5 + 24/2)
         break;

      case FORMAT_640X384:
      case FORMAT_800X480:
         LabelFont = &FONT_7pt8b;
         ValueFont = &FONT_12pt8b;
         UnitFont = &FONT_14pt8b;
         WI_COL  = 162;  // column width (px)
         WI_Y0   = 204;  // first-row base y
         WI_DY   = 56;   // row stride (icon 48 + gap 8)
         WI_SZ   = 48;   // icon size
         WI_LOFF = 48;   // label/data x offset from col start
         WI_LDY  = 10;   // label baseline delta from row base
         WI_DDY  = 32;   // data baseline delta from row base (8 + 48/2)
         break;
   }
}

/* This function is responsible for drawing the current conditions and
 * associated icons.
 */
void DrawOWM::drawCurrentConditions(const owm_current_t &current,
                           const owm_daily_t &today,
                           const owm_resp_air_pollution_t &owm_air_pollution,
                           float inTemp, float inHumidity)
{
   String dataStr, unitStr;
   // current weather icon
   const GFXfont *TemperatureFont = NULL;
   const uint8_t *IconBitmap = NULL;
   uint16_t TempX = 0;
   uint16_t TempY = 0;
   uint16_t FeelsLikeX = 0;
   uint16_t FeelsLikeY = 0;
   uint16_t UnitY = 0;
   uint16_t LargeIconSize = 0;
   float CurrentTemp;
   float FeelsLike;

   LOG("\n");
   switch (config.DisplayFormat) {
      case FORMAT_400X300:
         LargeIconSize = 96;
         TemperatureFont = &FONT_18pt8b;
         IconBitmap = getCurrentConditionsBitmap96(current, today);
         TempX = 96 + 30 - 10;
         TempY = 96 / 2 + 25 / 2;
         UnitY = 96 / 2 - 69 / 2 + 20;
         FeelsLikeX = 96 + 60 / 2;
         FeelsLikeY = 96 / 2 + 25 / 2 + 12 / 2 + 17 / 2;
         break;

      case FORMAT_640X384:
      case FORMAT_800X480:
         LargeIconSize = 196;
         TemperatureFont = &FONT_48pt8b_temperature;
         IconBitmap = getCurrentConditionsBitmap196(current, today);
         if(config.DisplayFormat == FORMAT_640X384) {
            TempX = 156 + 164 / 2 - 20;
            FeelsLikeX = 156 + 164 / 2 - 20;
         }
         else {
            TempX = 196 + 164 / 2 - 20;
            FeelsLikeX = 196 + 164 / 2;
         }
         TempY = 196 / 2 + 69 / 2;
         UnitY = 196 / 2 - 69 / 2 + 20;
         FeelsLikeY = 98 + 69 / 2 + 12 + 17;
         break;
   }

   drawInvertedBitmap(0, 0,IconBitmap,LargeIconSize,LargeIconSize,TFT_BLACK);

   // current temp
   if (config.bMetric) {
      CurrentTemp = kelvin_to_celsius(current.temp);
      FeelsLike = kelvin_to_celsius(current.feels_like);
      unitStr = TXT_UNITS_TEMP_CELSIUS;
   }
   else {
      CurrentTemp = kelvin_to_fahrenheit(current.temp);
      FeelsLike = kelvin_to_fahrenheit(current.feels_like);
      unitStr = TXT_UNITS_TEMP_FAHRENHEIT;
   }
   dataStr = String(static_cast<int>(std::round(CurrentTemp)));
   setFreeFont(TemperatureFont);
   drawString(TempX,TempY,dataStr, CENTER);
   setFreeFont(UnitFont);
   drawString(getCursorX(), UnitY, unitStr, LEFT);
   // current feels like
   dataStr = String(TXT_FEELS_LIKE) + ' '
             + String(static_cast<int>(std::round(FeelsLike)))
             + '\260';
   setFreeFont(ValueFont);
   drawString(FeelsLikeX,FeelsLikeY, dataStr, CENTER);

   // line dividing top and bottom display areas
   // display.drawLine(0, 196, config.DisplayWidth - 1, 196, TFT_BLACK);

   // draw current data of the left panel
   if (config.PosSunrise >= 0) {
      LOG("Calling drawCurrentSunrise\n");
      drawCurrentSunrise(current);
   }

   if (config.PosSunset>= 0) {
      LOG("Calling drawCurrentSunset\n");
      drawCurrentSunset(current);
   }

   if (config.PosWind>= 0) {
      LOG("Calling drawCurrentWind\n");
      drawCurrentWind(current);
   }

   if (config.PosHumidity>= 0) {
      LOG("Calling drawCurrentHumidity\n");
      drawCurrentHumidity(current);
   }

   if (config.PosUvi >= 0) {
      LOG("Calling drawCurrentUVI\n");
      drawCurrentUVI(current);
   }

   if (config.PosPressure >= 0) {
      LOG("Calling drawCurrentPressure\n");
      drawCurrentPressure(current);
   }

   if (config.PosVisibility >= 0) {
      LOG("Calling drawCurrentVisibility\n");
      drawCurrentVisibility(current);
   }

   if (config.PosAirQuality >= 0) {
      LOG("Calling drawCurrentAirQuality\n");
      drawCurrentAirQuality(owm_air_pollution);
   }

   if (config.PosIntemp >= 0) {
      LOG("Calling drawCurrentInTemp\n");
      drawCurrentInTemp(inTemp);
   }

   if (config.PosInhumidity >= 0) {
      LOG("Calling drawCurrentInHumidity\n");
      drawCurrentInHumidity(inHumidity);
   }

   if (config.PosMoonrise >= 0) {
      LOG("Calling drawCurrentMoonrise\n");
      drawCurrentMoonrise(today);
   }

   if (config.PosMoonset >= 0) {
      LOG("Calling drawCurrentMoonset\n");
      drawCurrentMoonset(today);
   }

   if (config.PosMoonphase >= 0) {
      LOG("Calling drawCurrentMoonphase\n");
      drawCurrentMoonphase(today);
   }

   if (config.PosDewpoint >= 0) {
      LOG("Calling drawCurrentDewpoint\n");
      drawCurrentDewpoint(current);
   }
   // end drawing left panel
} // end drawCurrentConditions

/* This function is responsible for drawing the five day forecast.
 */
void DrawOWM::drawForecast(const owm_daily_t *daily, tm timeInfo) {
   // 5 day, forecast
   String hiStr, loStr;
   String dataStr, unitStr;
   const unsigned char *IconBitmap = NULL;
   uint16_t DailyWI_SZ = 0;
   const GFXfont *TempFont;
#ifdef TEMP_ORDER_HL
   #define LeftStr  hiStr
   #define RightStr loStr
#else
   #define LeftStr  loStr
   #define RightStr hiStr
#endif

   LOG("\n");
// icons
   for (int i = 0; i < 5; ++i) {
      int x = 0;
      int y = 0;
      switch (config.DisplayFormat) {
         case FORMAT_400X300:
            DailyWI_SZ = 32;
            IconBitmap = getDailyForecastBitmap32(daily[i]);
            x = 178 + (i * 44);
            y = 49 + 69 / 4 - 32 / 2 - 6 / 2;
            break;

         case FORMAT_640X384:
            DailyWI_SZ = 64;
            IconBitmap = getDailyForecastBitmap64(daily[i]);
            x = 318 + (i * 64);
            y = 98 + 69 / 2 - 32 - 6;
            break;

         case FORMAT_800X480:
            DailyWI_SZ = 64;
            IconBitmap = getDailyForecastBitmap64(daily[i]);
            x = 398 + (i * 82);
            y = 98 + 69 / 2 - 32 - 6;
            break;
      }
      // icons
      drawInvertedBitmap(x,y,IconBitmap,DailyWI_SZ,DailyWI_SZ,TFT_BLACK);

      // day of week label
      char dayBuffer[8] = {};
      _strftime(dayBuffer, sizeof(dayBuffer), "%a", &timeInfo); // abbrv'd day
      if (config.DisplayFormat == FORMAT_400X300) {
         setFreeFont(&FONT_6pt8b);
         drawString(x + 15, 49 + 69 / 4 - 32 / 2 - 26 / 2 - 6 / 2 + 16 / 2,
                    dayBuffer,CENTER);
      }
      else {
         setFreeFont(&FONT_11pt8b);
         drawString(x + 31 - 2, 98 + 69 / 2 - 32 - 26 - 6 + 16,dayBuffer,CENTER);
      }
      timeInfo.tm_wday = (timeInfo.tm_wday + 1) % 7; // increment to next day

      // high | low
      if (config.DisplayFormat == FORMAT_400X300) {
         setFreeFont(&FONT_5pt8b);
         drawString(x + 15, 49 + 69 / 4 + 38 / 2 - 6 / 2 + 12 / 2, "|", CENTER);
      }
      else {
         setFreeFont(&FONT_8pt8b);
         drawString(x + 31, 98 + 69 / 2 + 38 - 6 + 12, "|", CENTER);
      }
      if (config.bMetric) {
         hiStr = String(static_cast<int>(
                     std::round(kelvin_to_celsius(daily[i].temp.max)))) +
                     "\260";
         loStr = String(static_cast<int>(
                     std::round(kelvin_to_celsius(daily[i].temp.min)))) +
                     "\260";
      }
      else {
         hiStr = String(static_cast<int>(
                     std::round(kelvin_to_fahrenheit(daily[i].temp.max)))) +
                     "\260";
         loStr = String(static_cast<int>(
                     std::round(kelvin_to_fahrenheit(daily[i].temp.min)))) +
                     "\260";
      }

      uint16_t RightX;
      uint16_t LeftX;
      uint16_t LowHighY;
      if (config.DisplayFormat == FORMAT_400X300) {
         LeftX  = x + 15 - 3;
         RightX = x + 15 + 3;
         LowHighY = 49 + 69 / 4 + 38 / 2 - 6 / 2 + 12 / 2;
      }
      else {
         LeftX  = x + 31 - 4;
         RightX = x + 31 + 5;
         LowHighY = 98 + 69 / 2 + 38 - 6 + 12;
      }
      drawString(RightX,LowHighY,RightStr,LEFT);
      drawString(LeftX,LowHighY,LeftStr,RIGHT);

// daily forecast precipitation
#if DISPLAY_DAILY_PRECIP
      float dailyPrecip = daily[i].snow + daily[i].rain;
      TempFont = config.DisplayWidth >= 640 ? &FONT_6pt8b : &FONT_5pt8b;
      setFreeFont(TempFont);
      switch (config.PrecipType) {
      case UNITS_DAILY_PRECIP_POP:
         dailyPrecip = daily[i].pop * 100;
         dataStr = String(static_cast<int>(dailyPrecip));
         unitStr = "%";
         break;

      case UNITS_DAILY_PRECIP_MILLIMETERS:
      // Round up to nearest mm
         dailyPrecip = std::round(dailyPrecip);
         dataStr = String(static_cast<int>(dailyPrecip));
         unitStr = String(" ") + TXT_UNITS_PRECIP_MILLIMETERS;
         break;

      case UNITS_DAILY_PRECIP_CENTIMETERS:
      // Round up to nearest 0.1 cm
         dailyPrecip = millimeters_to_centimeters(dailyPrecip);
         dailyPrecip = std::round(dailyPrecip * 10) / 10.0f;
         dataStr = String(dailyPrecip, 1);
         unitStr = String(" ") + TXT_UNITS_PRECIP_CENTIMETERS;
         break;

      case UNITS_DAILY_PRECIP_INCHES:
      // Round up to nearest 0.01 inch
         dailyPrecip = millimeters_to_inches(dailyPrecip);
         dailyPrecip = std::round(dailyPrecip * 100) / 100.0f;
         dataStr = String(dailyPrecip, 2);
         unitStr = String(" ") + TXT_UNITS_PRECIP_INCHES;
         break;
      }
#if (DISPLAY_DAILY_PRECIP == 2) // smart
      if (dailyPrecip > 0.0f) {
#endif
         if (config.DisplayFormat == FORMAT_400X300) {
            String FullString = dataStr + unitStr;
            uint16_t y = LowHighY + getTextHeight(FullString.c_str()) + 2;
            drawString(x + 15,y,FullString,CENTER);
         }
         else {
            drawString(x + 31, 98 + 69 / 2 + 38 - 6 + 26,
                       dataStr + unitStr, CENTER);
         }
#if (DISPLAY_DAILY_PRECIP == 2) // smart
      }
#endif
#endif // DISPLAY_DAILY_PRECIP
   }
} // end drawForecast

/* This function is responsible for drawing the current alerts if any.
* Up to 2 alerts can be drawn.
*/
void DrawOWM::drawAlerts(std::vector<owm_alerts_t> & alerts,
               const String &city, const String &date)
{
#if DEBUG_LEVEL >= 1
  Serial.println("[debug] alerts.size()    : " + String(alerts.size()));
#endif
  LOG("\n");
  if (alerts.size() == 0)
  { // no alerts to draw
    return;
  }

  int *ignore_list = (int *) calloc(alerts.size(), sizeof(*ignore_list));
  int *alert_indices = (int *) calloc(alerts.size(), sizeof(*alert_indices));
  if (!ignore_list || !alert_indices)
  {
    Serial.println("Error: Failed to allocate memory while handling alerts.");
    free(ignore_list);
    free(alert_indices);
    return;
  }

  // Converts all event text and tags to lowercase, removes extra information,
  // and filters out redundant alerts of lesser urgency.
  filterAlerts(alerts, ignore_list);

  // limit alert text width so that is does not run into the location or date
  // strings
  setFreeFont(&FONT_16pt8b);
  int city_w = getStringWidth(city);
  setFreeFont(&FONT_12pt8b);
  int date_w = getStringWidth(date);
  int max_w = config.DisplayWidth - 2 - std::max(city_w, date_w) - (196 + 4) - 8;

  // find indices of valid alerts
  int num_valid_alerts = 0;
#if DEBUG_LEVEL >= 1
  Serial.print("[debug] ignore_list      : [ ");
#endif
  for (int i = 0; i < alerts.size(); ++i)
  {
#if DEBUG_LEVEL >= 1
    Serial.print(String(ignore_list[i]) + " ");
#endif
    if (!ignore_list[i])
    {
      alert_indices[num_valid_alerts] = i;
      ++num_valid_alerts;
    }
  }
#if DEBUG_LEVEL >= 1
  Serial.println("]\n[debug] num_valid_alerts : " + String(num_valid_alerts));
#endif

  if (num_valid_alerts == 1)
  { // 1 alert
    // adjust max width to for 48x48 icons
    max_w -= 48;

    owm_alerts_t &cur_alert = alerts[alert_indices[0]];
    drawInvertedBitmap(196, 8, getAlertBitmap48(cur_alert), 48, 48,
                               ACCENT_COLOR);
    // must be called after getAlertBitmap
    toTitleCase(cur_alert.event);

    setFreeFont(&FONT_14pt8b);
    if (getStringWidth(cur_alert.event) <= max_w)
    { // Fits on a single line, draw along bottom
      drawString(196 + 48 + 4, 24 + 8 - 12 + 20 + 1, cur_alert.event, LEFT);
    }
    else
    { // use smaller font
      setFreeFont(&FONT_12pt8b);
      if (getStringWidth(cur_alert.event) <= max_w)
      { // Fits on a single line with smaller font, draw along bottom
        drawString(196 + 48 + 4, 24 + 8 - 12 + 17 + 1, cur_alert.event, LEFT);
      }
      else
      { // Does not fit on a single line, draw higher to allow room for 2nd line
        drawMultiLnString(196 + 48 + 4, 24 + 8 - 12 + 17 - 11,
                          cur_alert.event, LEFT, max_w, 2, 23);
      }
    }
  } // end 1 alert
  else
  { // 2 alerts
    // adjust max width to for 32x32 icons
    max_w -= 32;

    setFreeFont(&FONT_12pt8b);
    for (int i = 0; i < 2; ++i)
    {
      owm_alerts_t &cur_alert = alerts[alert_indices[i]];

      drawInvertedBitmap(196, (i * 32), getAlertBitmap32(cur_alert),
                                 32, 32, ACCENT_COLOR);
      // must be called after getAlertBitmap
      toTitleCase(cur_alert.event);

      drawMultiLnString(196 + 32 + 3, 5 + 17 + (i * 32),
                        cur_alert.event, LEFT, max_w, 1, 0);
    } // end for-loop
  } // end 2 alerts

  free(ignore_list);
  free(alert_indices);

  return;
} // end drawAlerts

/* This function is responsible for drawing the city string and date
 * information in the top right corner.
 */
void DrawOWM::drawLocationDate(const String &city, const String &date)
{
  // location, date
   uint16_t x = config.DisplayWidth - 2;
   uint16_t y;

   LOG("\n");
  if(config.DisplayWidth >= 640) {
     setFreeFont(&FONT_16pt8b);
     drawString(x, 23, city, RIGHT, ACCENT_COLOR);
     setFreeFont(&FONT_12pt8b);
     drawString(x, 30 + 4 + 17, date, RIGHT);
  }
  else {
  // NB: this code is VERY specific to Adafruit_GFX fonts!
     uint16_t BelowLine;
     uint16_t BelowLast;
     setFreeFont(&FONT_11pt8b);
     y = getTextHeight(city,&BelowLine) - BelowLine;
     drawString(x,y,city,RIGHT,ACCENT_COLOR);

     setFreeFont(&FONT_8pt8b);
     BelowLast = BelowLine;
     y += BelowLast + getTextHeight(date,&BelowLine) - BelowLine + 1;
     drawString(x,y,date,RIGHT);
  }
} // end drawLocationDate

/* The % operator in C++ is not a true modulo operator but it instead a
 * remainder operator. The remainder operator and modulo operator are equivalent
 * for positive numbers, but not for negatives. The follow implementation of the
 * modulo operator works for +/-a and +b.
 */
inline int modulo(int a, int b)
{
  const int result = a % b;
  return result >= 0 ? result : result + b;
}

/* Convert temperature in kelvin to the display y coordinate to be plotted.
 */
int DrawOWM::kelvin_to_plot_y(float kelvin, int tempBoundMin, float yPxPerUnit,
                     int yBoundMin)
{
  if(config.bMetric) {
     return static_cast<int>(std::round(
       yBoundMin - (yPxPerUnit * (kelvin_to_celsius(kelvin) - tempBoundMin)) ));
  }
  else {
     return static_cast<int>(std::round(
       yBoundMin - (yPxPerUnit * (kelvin_to_fahrenheit(kelvin) - tempBoundMin)) ));
  }
}

/* This function is responsible for drawing the outlook graph for the specified
 * number of hours(up to 48).
 */
void DrawOWM::drawOutlookGraph(
   const owm_hourly_t *hourly,
   const owm_daily_t *daily,
   tm timeInfo)
{
  int xPos0 = 0;
  int xPos1 = config.DisplayWidth;
  int yPos0 = 0;
  int yPos1 = 0;
  const char *HourFormat = HOUR_FORMAT;

  LOG("\n");
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        xPos0 = 198;
        yPos0 = 108;
        yPos1 = config.DisplayHeight - 19 - 23;
     // no room for AM/PM
        HourFormat = "%l";
        break;

     case FORMAT_640X384:
     // no room for AM/PM
        HourFormat = "%l";
     // Intentional fall though to FORMAT_800X480
     case FORMAT_800X480:
        xPos0 = 350;
        yPos0 = 216;
        yPos1 = config.DisplayHeight - 46;
        break;
  }

  // draw x tick marks
  // calculate y max/min and intervals
  int yMajorTicks = 5;
  float tempMin;
  if(config.bMetric) {
     tempMin = kelvin_to_celsius(hourly[0].temp);
  }
  else {
     tempMin = kelvin_to_fahrenheit(hourly[0].temp);
  }
  float tempMax = tempMin;

  float precipMax;
  if(config.PrecipHrType == UNITS_HOURLY_PRECIP_POP) {
     precipMax = hourly[0].pop;
  }
  else {
     precipMax = hourly[0].rain_1h + hourly[0].snow_1h;
  }
  int yTempMajorTicks = 5;
  float newTemp = 0;
  for (int i = 1; i < HOURLY_GRAPH_MAX; ++i)
  {
     if(config.bMetric) {
        newTemp = kelvin_to_celsius(hourly[i].temp);
     }
     else {
        newTemp = kelvin_to_fahrenheit(hourly[i].temp);
     }
    tempMin = std::min(tempMin, newTemp);
    tempMax = std::max(tempMax, newTemp);
    if(config.PrecipHrType == UNITS_HOURLY_PRECIP_POP) {
       precipMax = std::max<float>(precipMax, hourly[i].pop);
    }
    else {
       precipMax = std::max<float>(
                   precipMax, hourly[i].rain_1h + hourly[i].snow_1h);
    }
  }
  int tempBoundMin = static_cast<int>(tempMin - 1)
                      - modulo(static_cast<int>(tempMin - 1), yTempMajorTicks);
  int tempBoundMax = static_cast<int>(tempMax + 1)
   + (yTempMajorTicks - modulo(static_cast<int>(tempMax + 1), yTempMajorTicks));

  // while we have to many major ticks then increase the step
  while ((tempBoundMax - tempBoundMin) / yTempMajorTicks > yMajorTicks)
  {
    yTempMajorTicks += 5;
    tempBoundMin = static_cast<int>(tempMin - 1)
                      - modulo(static_cast<int>(tempMin - 1), yTempMajorTicks);
    tempBoundMax = static_cast<int>(tempMax + 1) + (yTempMajorTicks
                      - modulo(static_cast<int>(tempMax + 1), yTempMajorTicks));
  }
  // while we have not enough major ticks, add to either bound
  while ((tempBoundMax - tempBoundMin) / yTempMajorTicks < yMajorTicks)
  {
    // add to whatever bound is closer to the actual min/max
    if (tempMin - tempBoundMin <= tempBoundMax - tempMax)
    {
      tempBoundMin -= yTempMajorTicks;
    }
    else
    {
      tempBoundMax += yTempMajorTicks;
    }
  }

  int yPrecipMajorTickDecimals;
  float precipBoundMax = 0.0f;
  switch(config.PrecipHrType) {
      case UNITS_HOURLY_PRECIP_POP:
        xPos1 = config.DisplayWidth - 23;
        if (precipMax > 0)
        {
          precipBoundMax = 100.0f;
        }
        break;

      case UNITS_HOURLY_PRECIP_MILLIMETERS:
        xPos1 = config.DisplayWidth - 24;
        precipBoundMax = std::ceil(precipMax); // Round up to nearest mm
        yPrecipMajorTickDecimals = (precipBoundMax < 10);
        break;

      case UNITS_HOURLY_PRECIP_CENTIMETERS:
        xPos1 = config.DisplayWidth - 25;
        precipMax = millimeters_to_centimeters(precipMax);
        // Round up to nearest 0.1 cm
        precipBoundMax = std::ceil(precipMax * 10) / 10.0f;
        if (precipBoundMax < 1)
        {
          yPrecipMajorTickDecimals = 2;
          if (precipBoundMax > 0)
          {
            xPos1 -= 6; // needs extra room
          }
        }
        else if (precipBoundMax < 10)
        {
          yPrecipMajorTickDecimals = 1;
        }
        else
        {
          yPrecipMajorTickDecimals = 0;
        }
        break;

      case UNITS_HOURLY_PRECIP_INCHES:
        xPos1 = config.DisplayWidth - 25;
        precipMax = millimeters_to_inches(precipMax);
        // Round up to nearest 0.1 inch
        precipBoundMax = std::ceil(precipMax * 10) / 10.0f;
        if (precipBoundMax < 1)
        {
          yPrecipMajorTickDecimals = 2;
        }
        else if (precipBoundMax < 10)
        {
          yPrecipMajorTickDecimals = 1;
        }
        else
        {
          yPrecipMajorTickDecimals = 0;
        }
        break;
  }

  float yPrecipMajorTickValue = precipBoundMax / yMajorTicks;
  float precipRoundingMultiplier = std::pow(10.f, yPrecipMajorTickDecimals);

  if (precipBoundMax > 0)
  { // fill need extra room for labels
    xPos1 -= 23;
  }

  // draw x axis
  drawLine(xPos0, yPos1    , xPos1, yPos1    , TFT_BLACK);
  drawLine(xPos0, yPos1 - 1, xPos1, yPos1 - 1, TFT_BLACK);

  // draw y axis
  float yInterval = (yPos1 - yPos0) / static_cast<float>(yMajorTicks);
  for (int i = 0; i <= yMajorTicks; ++i)
  {
    String dataStr;
    String precipUnit;
    int yTick = static_cast<int>(yPos0 + (i * yInterval));
    setFreeFont(&FONT_8pt8b);
    // Temperature
    dataStr = String(tempBoundMax - (i * yTempMajorTicks));
    dataStr += "\260";
    drawString(xPos0 - 8, yTick + 4, dataStr, RIGHT, ACCENT_COLOR);

    float precipTick = precipBoundMax - (i * yPrecipMajorTickValue);
    // Precipitation volume
    precipTick = std::round(precipTick * precipRoundingMultiplier)
                            / precipRoundingMultiplier;
    dataStr = String(precipTick, yPrecipMajorTickDecimals);
    if (precipBoundMax > 0)
    { // don't labels if precip is 0
       switch (config.PrecipHrType) {
         case UNITS_HOURLY_PRECIP_POP:
            // PoP
            dataStr = String(100 - (i * 20));
            precipUnit = "%";
            break;

         case UNITS_HOURLY_PRECIP_MILLIMETERS:
            precipUnit = String(" ") + TXT_UNITS_PRECIP_MILLIMETERS;
            break;

         case UNITS_HOURLY_PRECIP_CENTIMETERS:
            precipUnit = String(" ") + TXT_UNITS_PRECIP_CENTIMETERS;
            break;
         case UNITS_HOURLY_PRECIP_INCHES:
            precipUnit = String(" ") + TXT_UNITS_PRECIP_INCHES;
            break;
       }
      drawString(xPos1 + 8, yTick + 4, dataStr, LEFT);
      setFreeFont(&FONT_5pt8b);
      drawString(getCursorX(), yTick + 4, precipUnit, LEFT);
    } // end draw labels if precip is >0

    // draw dotted line
    if (i < yMajorTicks)
    {
      for (int x = xPos0; x <= xPos1 + 1; x += 3)
      {
        drawPixel(x, yTick + (yTick % 2), TFT_BLACK);
      }
    }
  }

  int xMaxTicks = 8;
  int hourInterval = static_cast<int>(ceil(HOURLY_GRAPH_MAX
                                           / static_cast<float>(xMaxTicks)));
  float xInterval = (xPos1 - xPos0 - 1) / static_cast<float>(HOURLY_GRAPH_MAX);
  setFreeFont(&FONT_8pt8b);

  // precalculate all x and y coordinates for temperature values
  float yPxPerUnit = (yPos1 - yPos0)
                     / static_cast<float>(tempBoundMax - tempBoundMin);
  std::vector<int> x_t;
  std::vector<int> y_t;
  x_t.reserve(HOURLY_GRAPH_MAX);
  y_t.reserve(HOURLY_GRAPH_MAX);
    for (int i = 0; i < HOURLY_GRAPH_MAX; ++i)
  {
    y_t[i] = kelvin_to_plot_y(hourly[i].temp, tempBoundMin, yPxPerUnit, yPos1);
    x_t[i] = static_cast<int>(std::round(xPos0 + (i * xInterval)
                                          + (0.5 * xInterval) ));
  }

#if DISPLAY_HOURLY_ICONS
  int day_idx = 0;
#endif
  setFreeFont(&FONT_8pt8b);
  for (int i = 0; i < HOURLY_GRAPH_MAX; ++i)
  {
    int xTick = static_cast<int>(xPos0 + (i * xInterval));
    int x0_t, x1_t, y0_t, y1_t;

    if (i > 0)
    {
      // temperature
      x0_t = x_t[i - 1];
      x1_t = x_t[i    ];
      y0_t = y_t[i - 1];
      y1_t = y_t[i    ];
      // graph temperature
      drawLine(x0_t    , y0_t    , x1_t    , y1_t    , ACCENT_COLOR);
      drawLine(x0_t    , y0_t + 1, x1_t    , y1_t + 1, ACCENT_COLOR);
      drawLine(x0_t - 1, y0_t    , x1_t - 1, y1_t    , ACCENT_COLOR);

      // draw hourly bitmap
#if DISPLAY_HOURLY_ICONS
      if (daily[day_idx].dt + 86400 <= hourly[i].dt) {
        ++day_idx;
      }
      if ((i % hourInterval) == 0) // skip first and last tick
      {
        int y_b = INT_MAX;
        // find the highest (lowest in coordinate value) temperature point that
        // exists within the width of the icon.
        // find closest point above the temperature line where the icon won't
        // interect the temperature line.
        // y = mx + b
        int span = static_cast<int>(std::round(16 / xInterval));
        int l_idx = std::max(i - 1 - span, 0);
        int r_idx = std::min(i + span, HOURLY_GRAPH_MAX - 1);
        // left intersecting slope
        float m_l = (y_t[l_idx + 1] - y_t[l_idx]) / xInterval;
        int x_l = xTick - 16 - x_t[l_idx];
        int y_l = static_cast<int>(std::round(m_l * x_l + y_t[l_idx]));
        y_b = std::min(y_l, y_b);
        // right intersecting slope
        float m_r = (y_t[r_idx] - y_t[r_idx - 1]) / xInterval;
        int x_r = xTick + 16 - x_t[r_idx - 1];
        int y_r = static_cast<int>(std::round(m_r * x_r + y_t[r_idx - 1]));
        y_b = std::min(y_r, y_b);
        // any peaks in between
        for (int idx = l_idx + 1; idx < r_idx; ++idx)
        {
          y_b = std::min(y_t[idx], y_b);
        }
        const uint8_t *bitmap = getHourlyForecastBitmap32(hourly[i],
                                                          daily[day_idx]);
        drawInvertedBitmap(xTick - 16, y_b - 32,
                                   bitmap, 32, 32, TFT_BLACK);
      }
#endif
    }

    float precipVal = hourly[i].rain_1h + hourly[i].snow_1h;

    switch(config.PrecipHrType) {
       case UNITS_HOURLY_PRECIP_POP:
          precipVal = hourly[i].pop * 100;
          break;

       case UNITS_HOURLY_PRECIP_CENTIMETERS:
          precipVal = millimeters_to_centimeters(precipVal);
          break;

       case UNITS_HOURLY_PRECIP_INCHES:
          precipVal = millimeters_to_inches(precipVal);
          break;

       case UNITS_HOURLY_PRECIP_MILLIMETERS:
          break;
    }

    x0_t = static_cast<int>(std::round( xPos0 + 1 + (i * xInterval)));
    x1_t = static_cast<int>(std::round( xPos0 + 1 + ((i + 1) * xInterval) ));
    yPxPerUnit = (yPos1 - yPos0) / precipBoundMax;
    y0_t = static_cast<int>(std::round( yPos1 - (yPxPerUnit * (precipVal)) ));
    y1_t = yPos1;

    // graph Precipitation
    for (int y = y1_t - 1; y > y0_t; y -= 2)
    {
      for (int x = x0_t + (x0_t % 2); x < x1_t; x += 2)
      {
        drawPixel(x, y, TFT_BLACK);
      }
    }

    if ((i % hourInterval) == 0)
    {
      // draw x tick marks
      drawLine(xTick    , yPos1 + 1, xTick    , yPos1 + 4, TFT_BLACK);
      drawLine(xTick + 1, yPos1 + 1, xTick + 1, yPos1 + 4, TFT_BLACK);
      // draw x axis labels
      char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
      time_t ts = hourly[i].dt;
      tm *timeInfo = localtime(&ts);
      _strftime(timeBuffer, sizeof(timeBuffer), HourFormat, timeInfo);
      drawString(xTick, yPos1 + 1 + 12 + 4 + 3, timeBuffer, CENTER);
    }

  }

  // draw the last tick mark
  if ((HOURLY_GRAPH_MAX % hourInterval) == 0)
  {
    int xTick = static_cast<int>(
                std::round(xPos0 + (HOURLY_GRAPH_MAX * xInterval)));
    // draw x tick marks
    drawLine(xTick    , yPos1 + 1, xTick    , yPos1 + 4, TFT_BLACK);
    drawLine(xTick + 1, yPos1 + 1, xTick + 1, yPos1 + 4, TFT_BLACK);
    // draw x axis labels
    char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
    time_t ts = hourly[HOURLY_GRAPH_MAX - 1].dt + 3600;
    tm *timeInfo = localtime(&ts);
    _strftime(timeBuffer, sizeof(timeBuffer), HourFormat, timeInfo);
    drawString(xTick, yPos1 + 1 + 12 + 4 + 3, timeBuffer, CENTER);
  }
} // end drawOutlookGraph

/* This function is responsible for drawing the status bar along the bottom of
 * the display.
 */
void DrawOWM::drawStatusBar(const String &statusStr, const String &refreshTimeStr,
                   int rssi, uint32_t batVoltage)
{
  String dataStr;
  uint16_t dataColor = TFT_BLACK;
  setFreeFont(&FONT_6pt8b);
  int pos = config.DisplayWidth - 2;
  const int sp = 2;
  const unsigned char *IconBitmap = NULL;
  uint16_t RefreshWI_SZ = 0;

  LOG("\n");
  switch (config.DisplayFormat) {
     case FORMAT_400X300:
        IconBitmap = wi_refresh_24x24;
        RefreshWI_SZ = 24;
        break;

     case FORMAT_640X384:
     case FORMAT_800X480:
        IconBitmap = wi_refresh_32x32;
        RefreshWI_SZ = 32;
        break;
  }

#if BATTERY_MONITORING
  uint32_t batPercent = calcBatPercent(batVoltage,MIN_BATTERY_VOLTAGE,
                                       MAX_BATTERY_VOLTAGE,config.bLiPo);
  uint32_t WarnBattV = config.bLiPo ? WARN_BATTERY_VOLTAGE : 
                                      WARN_BATTERY_VOLTAGE_COIN;
  if (batVoltage < WarnBattV)
  {
    dataColor = ACCENT_COLOR;
  }
#if STATUS_BAR_EXTRAS_BAT_PERCENTAGE || STATUS_BAR_EXTRAS_BAT_VOLTAGE
  dataStr = "";
#if STATUS_BAR_EXTRAS_BAT_PERCENTAGE
  dataStr += String(batPercent) + "%";
#endif
#if STATUS_BAR_EXTRAS_BAT_VOLTAGE
  dataStr += " (" + String( std::round(batVoltage / 10.f) / 100.f, 2 ) + "v)";
#endif
  drawString(pos, config.DisplayHeight - 1 - 2, dataStr, RIGHT, dataColor);
  pos -= getStringWidth(dataStr) + 1;
#endif
  pos -= 24;
  drawInvertedBitmap(pos, config.DisplayHeight - 1 - 17,
                     getBatBitmap24(batPercent), 24, 24, dataColor);
  pos -= sp + 9;
#endif

  // WiFi
  dataColor = rssi >= -70 ? TFT_BLACK : ACCENT_COLOR;
#if STATUS_BAR_EXTRAS_WIFI_STRENGTH || STATUS_BAR_EXTRAS_WIFI_RSSI
  dataStr = "";
#if STATUS_BAR_EXTRAS_WIFI_STRENGTH
  dataStr += String(getWiFidesc(rssi));
#endif
#if STATUS_BAR_EXTRAS_WIFI_RSSI
  if (rssi != 0)
  {
    dataStr += " (" + String(rssi) + "dBm)";
  }
#endif
  drawString(pos, config.DisplayHeight - 1 - 2, dataStr, RIGHT, dataColor);
  pos -= getStringWidth(dataStr) + 1;
#endif
  pos -= 18;
  drawInvertedBitmap(pos, config.DisplayHeight - 1 - 13, getWiFiBitmap16(rssi),
                             16, 16, dataColor);
  pos -= sp + 8;

  // last refresh
  dataColor = TFT_BLACK;
  drawString(pos, config.DisplayHeight - 1 - 2, refreshTimeStr, RIGHT, dataColor);
  pos -= getStringWidth(refreshTimeStr) + 25;
  drawInvertedBitmap(pos,config.DisplayHeight - 1 - 21,IconBitmap,
                     RefreshWI_SZ,RefreshWI_SZ,dataColor);
  pos -= sp;

  // status
  dataColor = ACCENT_COLOR;
  if (!statusStr.isEmpty())
  {
    drawString(pos, config.DisplayHeight - 1 - 2, statusStr, RIGHT, dataColor);
    pos -= getStringWidth(statusStr) + 24;
    drawInvertedBitmap(pos, config.DisplayHeight - 1 - 18, error_icon_24x24,
                               24, 24, dataColor);
  }

  return;
} // end drawStatusBar

/* This function is responsible for drawing prominent error messages to the
 * screen.
 *
 * If error message line 2 (errMsgLn2) is empty, line 1 will be automatically
 * wrapped.
 */
void DrawOWM::drawError(const uint8_t *bitmap_196x196,
               const String &errMsgLn1, const String &errMsgLn2)
{
  setFreeFont(&FONT_26pt8b);
  if (!errMsgLn2.isEmpty())
  {
    drawString(config.DisplayWidth / 2,
               config.DisplayHeight / 2 + 196 / 2 + 21,
               errMsgLn1, CENTER);
    drawString(config.DisplayWidth / 2,
               config.DisplayHeight / 2 + 196 / 2 + 21 + 55,
               errMsgLn2, CENTER);
  }
  else
  {
    drawMultiLnString(config.DisplayWidth / 2,
                      config.DisplayHeight / 2 + 196 / 2 + 21,
                      errMsgLn1, CENTER, config.DisplayWidth - 200, 2, 55);
  }
  drawInvertedBitmap(config.DisplayWidth / 2 - 196 / 2,
                             config.DisplayHeight / 2 - 196 / 2 - 21,
                             bitmap_196x196, 196, 196, ACCENT_COLOR);
  return;
} // end drawError

void DrawOWM::drawInvertedBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) 
{
// taken from Adafruit_GFX.cpp, modified
   int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
   uint8_t byte = 0;
   for (int16_t j = 0; j < h; j++) {
      for (int16_t i = 0; i < w; i++) {
         if (i & 7) byte <<= 1;
         else {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
            byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
#else
            byte = bitmap[j * byteWidth + i / 8];
#endif
         }
         if (!(byte & 0x80)) {
            drawPixel(x + i, y + j, color);
         }
      }
   }
}

// #define DEBUG_GET_HEIGHT
uint16_t DrawOWM::getTextHeight(const String &str,uint16_t *pBelow)
{
   uint8_t glyph_ab = 0;
   uint8_t glyph_bb = 0;
   const char *pStr = str.c_str();
   uint8_t c;
#ifdef DEBUG_GET_HEIGHT
   uint8_t ab_c = '?';
   uint8_t bb_c = '?';
#endif
   uint16_t Ret;

   // Find the biggest above and below baseline offsets
   while((c = *pStr++)) {
     if(c < CurrentFont->first) {
        ELOG(" 0x%x < 0x%x, \"%s\"\n",c,CurrentFont->first,str.c_str());
        break;
     }
     if(c > CurrentFont->last) {
        ELOG(" 0x%x > 0x%x, \"%s\"\n",c,CurrentFont->last,str.c_str());
        break;
     }

     GFXglyph *glyph1  = &CurrentFont->glyph[c - CurrentFont->first];
     int8_t ab = -pgm_read_byte(&glyph1->yOffset);
     if (ab > glyph_ab) {
        glyph_ab = ab;
#ifdef DEBUG_GET_HEIGHT
        ab_c = c;
#endif
     }
     int8_t bb = pgm_read_byte(&glyph1->height) - ab;
     if (bb > glyph_bb) {
        glyph_bb = bb;
#ifdef DEBUG_GET_HEIGHT
        bb_c = c;
#endif
     }
   }
   if(pBelow != NULL) {
      *pBelow = glyph_bb;
   }
   Ret =  glyph_ab + glyph_bb;
#ifdef DEBUG_GET_HEIGHT
   LOG("'%c' is %d above base,'%c' is %d below base\n",ab_c,glyph_ab,bb_c,
   glyph_bb);
   LOG("Height of \"%s\" is %d\n",str.c_str(),Ret);
#endif
   return Ret;
}

/**************************************************************************/
/*!
    @brief  Helper to determine size of a string with current font/size.
            Pass string and a cursor position, returns UL corner and W,H.
    @param  str  The ASCII string to measure
    @param  x    The current cursor X
    @param  y    The current cursor Y
    @param  x1   The boundary X coordinate, returned by function
    @param  y1   The boundary Y coordinate, returned by function
    @param  w    The boundary width, returned by function
    @param  h    The boundary height, returned by function
*/
/**************************************************************************/
void DrawOWM::getTextBounds(const String &str,int16_t x,int16_t y,int16_t *x1,
                             int16_t *y1,uint16_t *w,uint16_t *h)
{
   *w =  display.textWidth(str);
   *h =  display.fontHeight();
   *x1 = x + *w;
   *y1 = y + *h;
//   LOG("\"%s\" is %d x %d\n",str.c_str(),*w,*h);
}

void DrawOWM::setFreeFont(const GFXfont *f)
{
   CurrentFont = f;
   display.setFreeFont(f);
}

