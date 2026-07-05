#ifndef _DRAW_OWM_H_
#define _DRAW_OWM_H_

#include <TFT_eSPI.h>
#include "api_response.h"
#include "config.h"

typedef enum alignment
{
  LEFT,
  RIGHT,
  CENTER
} alignment_t;

// UNITS - WIND SPEED
//   Metric   : Kilometers per Hour
//   Imperial : Miles per Hour
typedef enum {
   UNITS_SPEED_METERSPERSECOND,
   UNITS_SPEED_FEETPERSECOND,
   UNITS_SPEED_KILOMETERSPERHOUR,
   UNITS_SPEED_MILESPERHOUR,
   UNITS_SPEED_KNOTS,
   UNITS_SPEED_BEAUFORT
} ConfigWindSpeed_t;

// UNITS - VISIBILITY DISTANCE
//   Metric   : Kilometers
//   Imperial : Miles
typedef enum {
   UNITS_DIST_MILES,
   UNITS_DIST_KILOMETERS
} ConfigDistance_t;

// UNITS - PRECIPITATION (DAILY)
// Measure of precipitation.
// This can either be Probability of Precipitation (PoP) or daily volume.
//   Metric   : Millimeters
//   Imperial : Inches
typedef enum {
   UNITS_DAILY_PRECIP_POP,
   UNITS_DAILY_PRECIP_MILLIMETERS,
   UNITS_DAILY_PRECIP_CENTIMETERS,
   UNITS_DAILY_PRECIP_INCHES
} ConfigPrecip_t;

// UNITS - PRECIPITATION (HOURLY)
// Measure of precipitation.
// This can either be Probability of Precipitation (PoP) or hourly volume.
//   Metric   : Millimeters
//   Imperial : Inches
typedef enum {
   UNITS_HOURLY_PRECIP_POP,
   UNITS_HOURLY_PRECIP_MILLIMETERS,
   UNITS_HOURLY_PRECIP_CENTIMETERS,
   UNITS_HOURLY_PRECIP_INCHES
} ConfigPrecipHr_t;

// UNITS - PRESSURE
//   Metric   : Millibars
//   Imperial : Inches of Mercury
typedef enum {
   UNITS_PRES_HECTOPASCALS,
   UNITS_PRES_PASCALS,
   UNITS_PRES_MILLIMETERSOFMERCURY,
   UNITS_PRES_INCHESOFMERCURY,
   UNITS_PRES_MILLIBARS,
   UNITS_PRES_ATMOSPHERES,
   UNITS_PRES_GRAMSPERSQUARECENTIMETER,
   UNITS_PRES_POUNDSPERSQUAREINCH
} ConfigPressure_t;

#define FORMAT_COUNT 3
typedef enum {
   FORMAT_800X480,
   FORMAT_640X384,
   FORMAT_400X300,
} ConfigDisplayFormat_t;

typedef struct {
   const char *City;
   const char *TimeFormat;
   const char *DateFormat;
   const char *ForecastApiResponse;
   const char *AirPollutionApiResponse;
// bMetric false: Fahrenheit / MPH / inches
// bMetric true: Celcius / Beaufort / millimeters
   bool bMetric;
   bool bLiPo;
   ConfigDisplayFormat_t DisplayFormat;
   ConfigWindSpeed_t WindSpeed;
   ConfigDistance_t DistanceType;
   ConfigPrecip_t PrecipType;
   ConfigPrecipHr_t PrecipHrType;
   ConfigPressure_t PressureType;

// ALERTS
//   The handling of alerts is complex. Each country has a unique national alert
//   system that receives alerts from many different government agencies. This
//   results is huge variance in the formatting of alerts. OpenWeatherMap
//   provides alerts in English only. Any combination of these factors may make
//   it undesirable to display alerts in some regions.
//   Disable alerts by changing the DISPLAY_ALERTS macro to 0.
   bool bDisplayAlerts;

// WIDGET POSITIONS
// Set the order of current condition you want to display
// in the following order
//  0   1
//  2   3
//  4   5
//  6   7
//  8   9
// if a 640 x 384 display is used, then positions 6,7,8,9 are not available
   int8_t PosSunrise;
   int8_t PosSunset;
   int8_t PosWind;
   int8_t PosHumidity;
   int8_t PosUvi;
   int8_t PosPressure;
   int8_t PosAirQuality;
   int8_t PosVisibility;
   int8_t PosIntemp;
   int8_t PosInhumidity;
   int8_t PosMoonrise;
   int8_t PosMoonset;
   int8_t PosMoonphase;
   int8_t PosDewpoint;
   uint16_t DisplayWidth;
   uint16_t DisplayHeight;
   int16_t xOffset;
   int16_t yOffset;

   float inTemp;
   float inHumidity;
   uint16_t batteryVoltage;
   int Rssi;
} OwmConfig;

typedef struct {
   const char *LC_DAY[7];
   const char *LC_ABDAY[7];
   const char *LC_MON[12];
   const char *LC_ABMON[12];
   const char *LC_D_T_FMT;
   const char *LC_D_FMT;
   const char *LC_T_FMT;
   const char *LC_T_FMT_AMPM;
   const char *LC_AM_STR;
   const char *LC_PM_STR;
   const char *LC_ERA;
   const char *LC_ERA_D_FMT;
   const char *LC_ERA_D_T_FMT;
   const char *LC_ERA_T_FMT;
   const char *TXT_UNKNOWN;
   const char *TXT_FEELS_LIKE;
   const char *TXT_SUNRISE;
   const char *TXT_SUNSET;
   const char *TXT_MOONRISE;
   const char *TXT_MOONSET;
   const char *TXT_WIND;
   const char *TXT_HUMIDITY;
   const char *TXT_UV_INDEX;
   const char *TXT_PRESSURE;
   const char *TXT_AIR_QUALITY;
   const char *TXT_AIR_POLLUTION;
   const char *TXT_VISIBILITY;
   const char *TXT_INDOOR_TEMPERATURE;
   const char *TXT_INDOOR_HUMIDITY;
   const char *TXT_DEWPOINT;
   const char *TXT_MOONPHASE;
   const char *TXT_NEW_MOON;
   const char *TXT_WAXING_CRESCENT;
   const char *TXT_FIRST_QUARTER;
   const char *TXT_WAXING_GIBBOUS;
   const char *TXT_FULL_MOON;
   const char *TXT_WANING_GIBBOUS;
   const char *TXT_THIRD_QUARTER;
   const char *TXT_WANING_CRESCENT;
   const char *TXT_UV_LOW;
   const char *TXT_UV_MODERATE;
   const char *TXT_UV_HIGH;
   const char *TXT_UV_VERY_HIGH;
   const char *TXT_UV_EXTREME;
   const char *TXT_WIFI_EXCELLENT;
   const char *TXT_WIFI_GOOD;
   const char *TXT_WIFI_FAIR;
   const char *TXT_WIFI_WEAK;
   const char *TXT_WIFI_NO_CONNECTION;
   const char *TXT_UNITS_TEMP_KELVIN;
   const char *TXT_UNITS_TEMP_CELSIUS;
   const char *TXT_UNITS_TEMP_FAHRENHEIT;
   const char *TXT_UNITS_SPEED_METERSPERSECOND;
   const char *TXT_UNITS_SPEED_FEETPERSECOND;
   const char *TXT_UNITS_SPEED_KILOMETERSPERHOUR;
   const char *TXT_UNITS_SPEED_MILESPERHOUR;
   const char *TXT_UNITS_SPEED_KNOTS;
   const char *TXT_UNITS_SPEED_BEAUFORT;
   const char *TXT_UNITS_PRES_HECTOPASCALS;
   const char *TXT_UNITS_PRES_PASCALS;
   const char *TXT_UNITS_PRES_MILLIMETERSOFMERCURY;
   const char *TXT_UNITS_PRES_INCHESOFMERCURY;
   const char *TXT_UNITS_PRES_MILLIBARS;
   const char *TXT_UNITS_PRES_ATMOSPHERES;
   const char *TXT_UNITS_PRES_GRAMSPERSQUARECENTIMETER;
   const char *TXT_UNITS_PRES_POUNDSPERSQUAREINCH;
   const char *TXT_UNITS_DIST_KILOMETERS;
   const char *TXT_UNITS_DIST_MILES;
   const char *TXT_UNITS_PRECIP_MILLIMETERS;
   const char *TXT_UNITS_PRECIP_CENTIMETERS;
   const char *TXT_UNITS_PRECIP_INCHES;
} LocaleStrings_t;

class DrawOWM {
public:
#ifdef SEEED_GFX
   DrawOWM(EPaper &spr,OwmConfig &Config);
#else
   DrawOWM(TFT_eSprite &spr,OwmConfig &Config);
#endif
   void SetLocale(LocaleStrings_t *pStrings);
   void DrawIt();

private:
#ifdef SEEED_GFX
      EPaper &display;
#else
      TFT_eSprite &display;
#endif
      OwmConfig &config;

      void drawInit();
      uint16_t getStringWidth(const String &text);
      uint16_t getStringHeight(const String &text);
      void drawString(int16_t x, int16_t y, const String &text,
                      alignment_t alignment,uint16_t color=TFT_BLACK);
      void drawMultiLnString(int16_t x, int16_t y, const String &text,
                       alignment_t alignment, uint16_t max_width,
                       uint16_t max_lines, int16_t line_spacing,
                       uint16_t color=TFT_BLACK);
      void initDisplay();
      void powerOffDisplay();
      void drawCurrentSunrise(const owm_current_t &current);
      void drawCurrentWind(const owm_current_t &current);
      void drawCurrentUVI(const owm_current_t &current);
      void drawCurrentAirQuality(const owm_resp_air_pollution_t &owm_air_pollution);
      void drawCurrentInTemp(float inTemp);
      void drawCurrentSunset(const owm_current_t &current);
      void drawCurrentHumidity(const owm_current_t &current);
      void drawCurrentPressure(const owm_current_t &current);
      void drawCurrentVisibility(const owm_current_t &current);
      void drawCurrentInHumidity(float inHumidity);
      void drawCurrentMoonrise(const owm_daily_t &today);
      void drawCurrentMoonset(const owm_daily_t &today);
      void drawCurrentMoonphase(const owm_daily_t &daily);
      void drawCurrentDewpoint(const owm_current_t &current);
      void drawCurrentConditions(const owm_current_t &current,
                                 const owm_daily_t &today,
                                 const owm_resp_air_pollution_t &owm_air_pollution,
                                 float inTemp, float inHumidity);
      void drawForecast(const owm_daily_t *daily, tm timeInfo);
      void drawAlerts(std::vector<owm_alerts_t> & alerts,const String &city,
                      const String &date);
      void drawLocationDate(const String &city, const String &date);
      void drawOutlookGraph(const owm_hourly_t *hourly,
                            const owm_daily_t *daily,tm timeInfo);
      void drawStatusBar(const String &statusStr, const String &refreshTimeStr,
                         int rssi, uint32_t batVoltage);
      void drawError(const uint8_t *bitmap_196x196,const String &errMsgLn1,
                     const String &errMsgLn2);
      void drawInvertedBitmap(int16_t x, int16_t y, const uint8_t bitmap[], 
                              int16_t w, int16_t h, uint16_t color);
      void getDateStr(String &s, tm *timeInfo);
      uint16_t getTextHeight(const String &str,uint16_t *pBelow = NULL);
      void getTextBounds(const String &str,int16_t x,int16_t y,int16_t *x1,
                         int16_t *y1,uint16_t *w,uint16_t *h);
      int kelvin_to_plot_y(float kelvin, int tempBoundMin, float yPxPerUnit,
                           int yBoundMin);
      void setFreeFont(const GFXfont *f);
      void setCursor(int16_t x, int16_t y);
      int16_t getCursorX(void);
      int16_t getCursorY(void);
      void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
      void drawPixel(int32_t x, int32_t y, uint32_t color);
      void getRefreshTimeStr(String &s, bool timeSuccess, tm *timeInfo);
      const char *getUVIdesc(unsigned int uvi);
      const char *getWiFidesc(int rssi);
      const char *getMoonPhaseStr(const owm_daily_t &daily);
      size_t _strftime(char *s, size_t maxsize, const char *format,
                       const struct tm *timeptr);


      owm_resp_onecall_t       owm_onecall;
      owm_resp_air_pollution_t owm_air_pollution;

// Widget layout constants — different for 4.2in (400x300) vs 7.5in (800x480)
//   FORMAT_800X480, FORMAT_640X384: 2 cols × 5 rows, icon=48×48, col=162px, base_y=204
//   FORMAT_400X300: 2 cols × 5 rows, icon=24×24, col=85px, base_y=104

// column width (px)
  uint16_t WI_COL;
// first-row base y
  uint16_t WI_Y0;
// row stride
  uint16_t WI_DY;
// icon size
  uint16_t WI_SZ;
// label/data x offset from col start
  uint16_t WI_LOFF;
// label baseline delta from row base
  uint16_t WI_LDY;
// data baseline delta from row base (5 + 24/2)
  uint16_t WI_DDY;

  const GFXfont *LabelFont;
  const GFXfont *ValueFont;
  const GFXfont *UnitFont;
  uint16_t MaxX;
  uint16_t MaxY;
  const GFXfont *CurrentFont;

// LC_TIME
// locale-based information,
// see https://man7.org/linux/man-pages/man3/nl_langinfo.3.html for more info.
// Note to Translators:
//   The LC_TIME definitions are included in the localedata/locales directory of
//   the glibc source tree, in files named after the locale codes and encoding
//   schemes. For example, the en_US locale data is stored in the
//   localedata/locales/en_US file, which contains the definitions for date and
//   time formats, month and day names, and other time-related settings for the
//   LC_TIME category.
//   https://www.gnu.org/software/libc/sources.html
// D_T_FMT                 string for formatting date and time
  const char *LC_D_T_FMT;
// LC_D_FMT                date format string
  const char *LC_D_FMT;
// T_FMT                   time format string
  const char *LC_T_FMT;
// T_FMT_AMPM              a.m. or p.m. time format string
  const char *LC_T_FMT_AMPM;
// AM_STR                  Ante Meridian affix
  const char *LC_AM_STR;
// PM_STR                  Post Meridian affix
  const char *LC_PM_STR;
// DAY_{1-7}               name of the n-th day of the week
  const char *LC_DAY[7];
// ABDAY_{1-7}             abbreviated name of the n-th day of the week
  const char *LC_ABDAY[7];
// MON_{1-12}              name of the n-th month of the year
   const char *LC_MON[12];
// ABMON_{1-12}            abbreviated name of the n-th month of the year
   const char *LC_ABMON[12];
// ERA                     era description segments
   const char *LC_ERA;
// ERA_D_FMT               era date format string
   const char *LC_ERA_D_FMT;
// ERA_D_T_FMT             era date and time format string
   const char *LC_ERA_D_T_FMT;
// ERA_T_FMT               era time format string
   const char *LC_ERA_T_FMT;

// Title Case
   const char *TXT_FEELS_LIKE;
   const char *TXT_SUNRISE;
   const char *TXT_SUNSET;
   const char *TXT_MOONRISE;
   const char *TXT_MOONSET;
   const char *TXT_WIND;
   const char *TXT_HUMIDITY;
   const char *TXT_UV_INDEX;
   const char *TXT_PRESSURE;
   const char *TXT_AIR_QUALITY;
   const char *TXT_AIR_POLLUTION;
   const char *TXT_VISIBILITY;
   const char *TXT_INDOOR_TEMPERATURE;
   const char *TXT_INDOOR_HUMIDITY;
   const char *TXT_DEWPOINT;
// MOON PHASE
   const char *TXT_MOONPHASE;
   const char *TXT_NEW_MOON;
   const char *TXT_WAXING_CRESCENT;
   const char *TXT_FIRST_QUARTER;
   const char *TXT_WAXING_GIBBOUS;
   const char *TXT_FULL_MOON;
   const char *TXT_WANING_GIBBOUS;
   const char *TXT_THIRD_QUARTER;
   const char *TXT_WANING_CRESCENT;

// UV INDEX
   const char *TXT_UV_LOW;
   const char *TXT_UV_MODERATE;
   const char *TXT_UV_HIGH;
   const char *TXT_UV_VERY_HIGH;
   const char *TXT_UV_EXTREME;

// WIFI
   const char *TXT_WIFI_EXCELLENT;
   const char *TXT_WIFI_GOOD;
   const char *TXT_WIFI_FAIR;
   const char *TXT_WIFI_WEAK;
   const char *TXT_WIFI_NO_CONNECTION;

// UNIT SYMBOLS - TEMPERATURE
   const char *TXT_UNITS_TEMP_KELVIN;
   const char *TXT_UNITS_TEMP_CELSIUS;
   const char *TXT_UNITS_TEMP_FAHRENHEIT;
// UNIT SYMBOLS - WIND SPEED
   const char *TXT_UNITS_SPEED_METERSPERSECOND;
   const char *TXT_UNITS_SPEED_FEETPERSECOND;
   const char *TXT_UNITS_SPEED_KILOMETERSPERHOUR;
   const char *TXT_UNITS_SPEED_MILESPERHOUR;
   const char *TXT_UNITS_SPEED_KNOTS;
   const char *TXT_UNITS_SPEED_BEAUFORT;
// UNIT SYMBOLS - PRESSURE
   const char *TXT_UNITS_PRES_HECTOPASCALS;
   const char *TXT_UNITS_PRES_PASCALS;
   const char *TXT_UNITS_PRES_MILLIMETERSOFMERCURY;
   const char *TXT_UNITS_PRES_INCHESOFMERCURY;
   const char *TXT_UNITS_PRES_MILLIBARS;
   const char *TXT_UNITS_PRES_ATMOSPHERES;
   const char *TXT_UNITS_PRES_GRAMSPERSQUARECENTIMETER;
   const char *TXT_UNITS_PRES_POUNDSPERSQUAREINCH;
// UNITS SYMBOLS - VISIBILITY DISTANCE
   const char *TXT_UNITS_DIST_KILOMETERS;
   const char *TXT_UNITS_DIST_MILES;
// UNITS SYMBOLS - PRECIPITATION
   const char *TXT_UNITS_PRECIP_MILLIMETERS;
   const char *TXT_UNITS_PRECIP_CENTIMETERS;
   const char *TXT_UNITS_PRECIP_INCHES;
// MISCELLANEOUS MESSAGES
   const char *TXT_UNKNOWN;
};

#endif   // _DRAW_OWM_H_
