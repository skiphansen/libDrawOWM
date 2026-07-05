/* Configuration option declarations for esp32-weather-epd.
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <cstdint>
#include <Arduino.h>

#define ACCENT_COLOR TFT_RED

// LOCALE
// If your locale is not here, you can add it by copying and modifying one of
// the files in src/locales. Please feel free to create a pull request to add
// official support for your locale.
//   Language (Territory)            code
//   German (Germany)                de_DE
//   English (United Kingdom)        en_GB
//   English (United States)         en_US
//   Estonian (Estonia)              et_EE
//   Finnish (Finland)               fi_FI
//   French (France)                 fr_FR
//   Italiano (Italia)               it_IT
//   Dutch (Belgium)                 nl_BE
//   Portuguese (Brazil)             pt_BR
//   Spanish (Spain)                 es_ES
#define LOCALE en_US

// WIND DIRECTION INDICATOR
// Choose whether the wind direction indicator should be an arrow, number, or
// expressed in Compass Point Notation (CPN).
// The arrow indicator can be combined with NUMBER or CPN.
//
//   PRECISION                  #     ERROR   EXAMPLE
//   Cardinal                   4  ±45.000°   E
//   Intercardinal (Ordinal)    8  ±22.500°   NE
//   Secondary Intercardinal   16  ±11.250°   NNE
//   Tertiary Intercardinal    32   ±5.625°   NbE
#define WIND_INDICATOR_ARROW
// #define WIND_INDICATOR_NUMBER
// #define WIND_INDICATOR_CPN_CARDINAL
// #define WIND_INDICATOR_CPN_INTERCARDINAL
// #define WIND_INDICATOR_CPN_SECONDARY_INTERCARDINAL
// #define WIND_INDICATOR_CPN_TERTIARY_INTERCARDINAL
// #define WIND_INDICATOR_NONE

// WIND DIRECTION ICON PRECISION
// The wind direction icon shown to the left of the wind speed can indicate wind
// direction with a minimum error of ±0.5°. This uses more flash storage because
// 360 24x24 wind direction icons must be stored, totaling ~25kB. For either
// preference or in case flash space becomes a concern there are a handful of
// selectable options listed below. 360 points seems excessive, but the option
// is there.
//
//   PRECISION                  #     ERROR  STORAGE
//   Cardinal                   4  ±45.000°     288B  E
//   Intercardinal (Ordinal)    8  ±22.500°     576B  NE
//   Secondary Intercardinal   16  ±11.250°   1,152B  NNE
//   Tertiary Intercardinal    32   ±5.625°   2,304B  NbE
//   (360)                    360   ±0.500°  25,920B  1°
// Uncomment your preferred wind level direction precision.
// #define WIND_ICONS_CARDINAL
// #define WIND_ICONS_INTERCARDINAL
#define WIND_ICONS_SECONDARY_INTERCARDINAL
// #define WIND_ICONS_TERTIARY_INTERCARDINAL
// #define WIND_ICONS_360

// Choose the style of moon phase icon you like
//   Primary     : dark color means where the moon is
//   Alternative : dark color means where the shadow is
// Uncomment your preferred moon phase style.
// #define MOONPHASE_PRIMARY
#define MOONPHASE_ALTERNATIVE


// FONTS
// A handful of popular Open Source typefaces have been included with this
// project for your convenience. Change the font by selecting its corresponding
// header file.
//
//   FONT           HEADER FILE              FAMILY          LICENSE
//   FreeMono       FreeMono.h               GNU FreeFont    GNU GPL v3.0
//   FreeSans       FreeSans.h               GNU FreeFont    GNU GPL v3.0
//   FreeSerif      FreeSerif.h              GNU FreeFont    GNU GPL v3.0
//   Lato           Lato_Regular.h           Lato            SIL OFL v1.1
//   Montserrat     Montserrat_Regular.h     Montserrat      SIL OFL v1.1
//   Open Sans      OpenSans_Regular.h       Open Sans       SIL OFL v1.1
//   Poppins        Poppins_Regular.h        Poppins         SIL OFL v1.1
//   Quicksand      Quicksand_Regular.h      Quicksand       SIL OFL v1.1
//   Raleway        Raleway_Regular.h        Raleway         SIL OFL v1.1
//   Roboto         Roboto_Regular.h         Roboto          Apache v2.0
//   Roboto Mono    RobotoMono_Regular.h     Roboto Mono     Apache v2.0
//   Roboto Slab    RobotoSlab_Regular.h     Roboto Slab     Apache v2.0
//   Ubuntu         Ubuntu_R.h               Ubuntu font     UFL v1.0
//   Ubuntu Mono    UbuntuMono_R.h           Ubuntu font     UFL v1.0
//
// Adding new fonts is relatively straightforward, see fonts/README.
//
// Note:
//   The layout of the display was designed around spacing and size of the GNU
//   FreeSans font, but this project supports the ability to modularly swap
//   fonts. Using a font other than FreeSans may result in undesired spacing or
//   other artifacts.
#define FONT_HEADER "fonts/FreeSans.h"

// FORECAST TEMPERATURE ORDER
// The order of temperture Hi|Lo can optionally be configured using
// the following options.
//   HL   : High | Low
//   LH   : Low | High
//
// #define TEMP_ORDER_HL
#define TEMP_ORDER_LH

// DAILY PRECIPITATION
// Daily precipitation indicated under Hi|Lo can optionally be configured using
// the following options.
//   0 : Disable (hide always)
//   1 : Enable (show always)
//   2 : Smart (show only when precipitation is forecasted)
#define DISPLAY_DAILY_PRECIP 2

// HOURLY WEATHER ICONS
// Weather icons to be displayed on the temperature and precipitation chart.
// They are drawn at the the x-axis tick marks just above the temperature line
//   0 : Disable
//   1 : Enable
#define DISPLAY_HOURLY_ICONS 0

// DISPLAY_ROTATION
//   Set rotation setting for display, 0 thru 3 corresponding to 4 
//   cardinal rotations
#define DISPLAY_ROTATION   1

// STATUS BAR EXTRAS
//   Extra information that can be displayed on the status bar. Set to 1 to
//   enable.
#define STATUS_BAR_EXTRAS_BAT_PERCENTAGE 0
#define STATUS_BAR_EXTRAS_BAT_VOLTAGE    1
#define STATUS_BAR_EXTRAS_WIFI_STRENGTH  1
#define STATUS_BAR_EXTRAS_WIFI_RSSI      0

// BATTERY MONITORING
//   You may choose to power your weather display with or without a battery.
//   Low power behavior can be controlled in config.cpp.
//   If you wish to disable battery monitoring set this macro to 0.
#define BATTERY_MONITORING 1

// DEBUG
//   If defined, enables increase verbosity over the serial port.
//   level 0: basic status information, assists troubleshooting (default)
//   level 1: increased verbosity for debugging
//   level 2: print api responses to serial monitor
#define DEBUG_LEVEL 0

// const char *TIME_FORMAT = "%H:%M";   // 24-hour ex: 01:23   23:00
// Time format used when displaying axis labels. (Max 11 characters)
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
#define TIME_FORMAT "%l:%M %P" // 12-hour ex: 1:23 am  11:00 pm

// const char *HOUR_FORMAT = "%H";      // 24-hour ex: 01   23
// Date format used when displaying date in top-right corner.
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
#define HOUR_FORMAT "%l%P" // 12-hour ex: 1am  11p

// Date/Time format used when displaying the last refresh time along the bottom
// of the screen.
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
#define REFRESH_TIME_FORMAT "%x %H:%M"

// HOURLY OUTLOOK GRAPH
// Number of hours to display on the outlook graph. (range: [8-48])
#define HOURLY_GRAPH_MAX   24

#define WARN_BATTERY_VOLTAGE        3535  // (millivolts) ~20%
#define WARN_BATTERY_VOLTAGE_COIN   2750  // (millivolts) ~20%

// Battery voltage calculations are based on a typical 3.7v LiPo.
#define MAX_BATTERY_VOLTAGE      4200  // (millivolts)
#define MIN_BATTERY_VOLTAGE      3000  // (millivolts)


#if !(defined(LOCALE))
  #error Invalid configuration. Locale not selected.
#endif
#if !(  defined(TEMP_ORDER_HL)      \
      ^ defined(TEMP_ORDER_LH))
  #error Invalid configuration. Exactly one temperature order must be selected.
#endif
#if !(  defined(WIND_INDICATOR_ARROW)                         \
      || (                                                    \
          defined(WIND_INDICATOR_NUMBER)                      \
        ^ defined(WIND_INDICATOR_CPN_CARDINAL)                \
        ^ defined(WIND_INDICATOR_CPN_INTERCARDINAL)           \
        ^ defined(WIND_INDICATOR_CPN_SECONDARY_INTERCARDINAL) \
        ^ defined(WIND_INDICATOR_CPN_TERTIARY_INTERCARDINAL)  \
      )                                                       \
      ^ defined(WIND_INDICATOR_NONE))
  #error Invalid configuration. Illegal selction of wind indicator(s).
#endif
#if defined(WIND_INDICATOR_ARROW)                   \
 && !(  defined(WIND_ICONS_CARDINAL)                \
      ^ defined(WIND_ICONS_INTERCARDINAL)           \
      ^ defined(WIND_ICONS_SECONDARY_INTERCARDINAL) \
      ^ defined(WIND_ICONS_TERTIARY_INTERCARDINAL)  \
      ^ defined(WIND_ICONS_360))
  #error Invalid configuration. Exactly one wind direction icon precision level must be selected.
#endif
#if !(defined(FONT_HEADER))
  #error Invalid configuration. Font not selected.
#endif
#if !(defined(DISPLAY_DAILY_PRECIP))
  #error Invalid configuration. DISPLAY_DAILY_PRECIP not defined.
#endif
#if !(defined(DISPLAY_HOURLY_ICONS))
  #error Invalid configuration. DISPLAY_HOURLY_ICONS not defined.
#endif
#if !(defined(BATTERY_MONITORING))
  #error Invalid configuration. BATTERY_MONITORING not defined.
#endif
#if !(defined(DEBUG_LEVEL))
  #error Invalid configuration. DEBUG_LEVEL not defined.
#endif
#if !(  defined(MOONPHASE_PRIMARY)  \
      ^ defined(MOONPHASE_ALTERNATIVE))
  #error Invalid configuration. Exactly one moon phase style must be selected.
#endif

#endif
