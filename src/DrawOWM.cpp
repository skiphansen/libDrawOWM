#include <Arduino.h>
#include <ArduinoJson.h>

#include "TFT_eSPI.h"
#include "config.h"
#include <DrawOWM.h>
#include "display_utils.h"

#define ENABLE_LOGGING  1
#if ENABLE_LOGGING && __has_include("logging.h") 
#include "logging.h"
#else
#define LOG(format, ...)
#define LOG_RAW(format, ...)
#endif

#define SET_LOCALE_STRING(x) if(p->x) x = p->x

#ifdef SEEED_GFX
DrawOWM::DrawOWM(EPaper &epaper,OwmConfig &Config)
#else
DrawOWM::DrawOWM(TFT_eSprite &epaper,OwmConfig &Config)
#endif
   : display(epaper),config(Config)
{
   LocaleStrings_t LocaleStrings = {
      .LC_DAY = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday", 
                 "Saturday"},
      .LC_ABDAY = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
      .LC_MON = {"January", "February", "March","April","May","June",
                 "July","August","September","October","November","December"},
      .LC_ABMON = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct",
                   "Nov","Dec"},
      .LC_D_T_FMT     = "%a %d %b %Y %r %Z",
      .LC_D_FMT       = "%m/%d/%y",
      .LC_T_FMT       = "%r",
      .LC_T_FMT_AMPM  = "%I:%M:%S %p",
      .LC_AM_STR      = "AM",
      .LC_PM_STR      = "PM",

      .LC_ERA         = "",
      .LC_ERA_D_FMT   = "",
      .LC_ERA_D_T_FMT = "",
      .LC_ERA_T_FMT   = "",

   // MISCELLANEOUS MESSAGES
   // Title Case
      .TXT_UNKNOWN = "Unknown",

   // CURRENT CONDITIONS
      .TXT_FEELS_LIKE         = "Feels Like",
      .TXT_SUNRISE            = "Sunrise",
      .TXT_SUNSET             = "Sunset",
      .TXT_MOONRISE           = "Moonrise",
      .TXT_MOONSET            = "Moonset",
      .TXT_WIND               = "Wind",
      .TXT_HUMIDITY           = "Humidity",
      .TXT_UV_INDEX           = "UV Index",
      .TXT_PRESSURE           = "Pressure",
      .TXT_AIR_QUALITY        = "Air Quality",
      .TXT_AIR_POLLUTION      = "Air Pollution",
      .TXT_VISIBILITY         = "Visibility",
      .TXT_INDOOR_TEMPERATURE = "Temperature",
      .TXT_INDOOR_HUMIDITY    = "Humidity",
      .TXT_DEWPOINT           = "Dew Point",

   // MOON PHASE
      .TXT_MOONPHASE       = "Moon Phase",
      .TXT_NEW_MOON        = "New Moon",
      .TXT_WAXING_CRESCENT = "Waxing Crescent",
      .TXT_FIRST_QUARTER   = "First Quarter",
      .TXT_WAXING_GIBBOUS  = "Waxing Gibbous",
      .TXT_FULL_MOON       = "Full Moon",
      .TXT_WANING_GIBBOUS  = "Waning Gibbous",
      .TXT_THIRD_QUARTER   = "Last Quarter",
      .TXT_WANING_CRESCENT = "Waning Crescent",

   // UV INDEX
      .TXT_UV_LOW       = "Low",
      .TXT_UV_MODERATE  = "Moderate",
      .TXT_UV_HIGH      = "High",
      .TXT_UV_VERY_HIGH = "Very High",
      .TXT_UV_EXTREME   = "Extreme",

   // WIFI
      .TXT_WIFI_EXCELLENT     = "Excellent",
      .TXT_WIFI_GOOD          = "Good",
      .TXT_WIFI_FAIR          = "Fair",
      .TXT_WIFI_WEAK          = "Weak",
      .TXT_WIFI_NO_CONNECTION = "No Connection",

   // UNIT SYMBOLS - TEMPERATURE
      .TXT_UNITS_TEMP_KELVIN     = "K",
      .TXT_UNITS_TEMP_CELSIUS    = "\260C",
      .TXT_UNITS_TEMP_FAHRENHEIT = "\260F",
   // UNIT SYMBOLS - WIND SPEED
      .TXT_UNITS_SPEED_METERSPERSECOND   = "m/s",
      .TXT_UNITS_SPEED_FEETPERSECOND     = "ft/s",
      .TXT_UNITS_SPEED_KILOMETERSPERHOUR = "km/h",
      .TXT_UNITS_SPEED_MILESPERHOUR      = "mph",
      .TXT_UNITS_SPEED_KNOTS             = "kt",
      .TXT_UNITS_SPEED_BEAUFORT          = "",
   // UNIT SYMBOLS - PRESSURE
      .TXT_UNITS_PRES_HECTOPASCALS             = "hPa",
      .TXT_UNITS_PRES_PASCALS                  = "Pa",
      .TXT_UNITS_PRES_MILLIMETERSOFMERCURY     = "mmHg",
      .TXT_UNITS_PRES_INCHESOFMERCURY          = "inHg",
      .TXT_UNITS_PRES_MILLIBARS                = "mbar",
      .TXT_UNITS_PRES_ATMOSPHERES              = "atm",
      .TXT_UNITS_PRES_GRAMSPERSQUARECENTIMETER = "g/cm\262",
      .TXT_UNITS_PRES_POUNDSPERSQUAREINCH      = "lb/in\262",
   // UNITS SYMBOLS - VISIBILITY DISTANCE
      .TXT_UNITS_DIST_KILOMETERS = "km",
      .TXT_UNITS_DIST_MILES      = "mi",
   // UNITS SYMBOLS - PRECIPITATION
      .TXT_UNITS_PRECIP_MILLIMETERS = "mm",
      .TXT_UNITS_PRECIP_CENTIMETERS = "cm",
      .TXT_UNITS_PRECIP_INCHES      = "in"
   };
   SetLocale(&LocaleStrings);
}

void DrawOWM::DrawIt()
{
   tm timeInfo = {};
   String dateStr;
   String refreshTimeStr;
   time_t CurrentTime;
   String statusStr = {};

   deserializeOneCall(config.ForecastApiResponse,owm_onecall,config.bDisplayAlerts);
   CurrentTime = (time_t) owm_onecall.current.dt;
   localtime_r(&CurrentTime, &timeInfo);
   getRefreshTimeStr(refreshTimeStr,true,&timeInfo);
   getDateStr(dateStr, &timeInfo);
   deserializeAirQuality(config.AirPollutionApiResponse,owm_air_pollution);
   drawInit();
   drawCurrentConditions(owm_onecall.current, owm_onecall.daily[0],
                         owm_air_pollution, config.inTemp,config.inHumidity);
   drawOutlookGraph(owm_onecall.hourly, owm_onecall.daily, timeInfo);
   drawForecast(owm_onecall.daily, timeInfo);
   drawLocationDate(config.City,dateStr);
   if(config.bDisplayAlerts ) {
      drawAlerts(owm_onecall.alerts,config.City,dateStr);
   }
   drawStatusBar(statusStr,refreshTimeStr,config.Rssi,config.batteryVoltage);
}

void DrawOWM::SetLocale(LocaleStrings_t *p)
{
// if LocaleStrs is provided override the english strings with the provided
// strings 
   if(p != NULL) {
      for(int i = 0; i < 7; i++) {
         SET_LOCALE_STRING(LC_DAY[i]);
         SET_LOCALE_STRING(LC_ABDAY[i]);
      }
      for(int i = 0; i < 12; i++) {
         SET_LOCALE_STRING(LC_MON[i]);
         SET_LOCALE_STRING(LC_ABMON[i]);
      }
      SET_LOCALE_STRING(LC_D_T_FMT);
      SET_LOCALE_STRING(LC_D_FMT);
      SET_LOCALE_STRING(LC_T_FMT);
      SET_LOCALE_STRING(LC_T_FMT_AMPM);
      SET_LOCALE_STRING(LC_AM_STR);
      SET_LOCALE_STRING(LC_PM_STR);
      SET_LOCALE_STRING(LC_ERA);
      SET_LOCALE_STRING(LC_ERA_D_FMT);
      SET_LOCALE_STRING(LC_ERA_D_T_FMT);
      SET_LOCALE_STRING(LC_ERA_T_FMT);
      SET_LOCALE_STRING(TXT_UNKNOWN);
      SET_LOCALE_STRING(TXT_FEELS_LIKE);
      SET_LOCALE_STRING(TXT_SUNRISE);
      SET_LOCALE_STRING(TXT_SUNSET);
      SET_LOCALE_STRING(TXT_MOONRISE);
      SET_LOCALE_STRING(TXT_MOONSET);
      SET_LOCALE_STRING(TXT_WIND);
      SET_LOCALE_STRING(TXT_HUMIDITY);
      SET_LOCALE_STRING(TXT_UV_INDEX);
      SET_LOCALE_STRING(TXT_PRESSURE);
      SET_LOCALE_STRING(TXT_AIR_QUALITY);
      SET_LOCALE_STRING(TXT_AIR_POLLUTION);
      SET_LOCALE_STRING(TXT_VISIBILITY);
      SET_LOCALE_STRING(TXT_INDOOR_TEMPERATURE);
      SET_LOCALE_STRING(TXT_INDOOR_HUMIDITY);
      SET_LOCALE_STRING(TXT_DEWPOINT);
      SET_LOCALE_STRING(TXT_MOONPHASE);
      SET_LOCALE_STRING(TXT_NEW_MOON);
      SET_LOCALE_STRING(TXT_WAXING_CRESCENT);
      SET_LOCALE_STRING(TXT_FIRST_QUARTER);
      SET_LOCALE_STRING(TXT_WAXING_GIBBOUS);
      SET_LOCALE_STRING(TXT_FULL_MOON);
      SET_LOCALE_STRING(TXT_WANING_GIBBOUS);
      SET_LOCALE_STRING(TXT_THIRD_QUARTER);
      SET_LOCALE_STRING(TXT_WANING_CRESCENT);
      SET_LOCALE_STRING(TXT_UV_LOW);
      SET_LOCALE_STRING(TXT_UV_MODERATE);
      SET_LOCALE_STRING(TXT_UV_HIGH);
      SET_LOCALE_STRING(TXT_UV_VERY_HIGH);
      SET_LOCALE_STRING(TXT_UV_EXTREME);
      SET_LOCALE_STRING(TXT_WIFI_EXCELLENT);
      SET_LOCALE_STRING(TXT_WIFI_GOOD);
      SET_LOCALE_STRING(TXT_WIFI_FAIR);
      SET_LOCALE_STRING(TXT_WIFI_WEAK);
      SET_LOCALE_STRING(TXT_WIFI_NO_CONNECTION);
      SET_LOCALE_STRING(TXT_UNITS_TEMP_KELVIN);
      SET_LOCALE_STRING(TXT_UNITS_TEMP_CELSIUS);
      SET_LOCALE_STRING(TXT_UNITS_TEMP_FAHRENHEIT);
      SET_LOCALE_STRING(TXT_UNITS_SPEED_METERSPERSECOND);
      SET_LOCALE_STRING(TXT_UNITS_SPEED_FEETPERSECOND);
      SET_LOCALE_STRING(TXT_UNITS_SPEED_KILOMETERSPERHOUR);
      SET_LOCALE_STRING(TXT_UNITS_SPEED_MILESPERHOUR);
      SET_LOCALE_STRING(TXT_UNITS_SPEED_KNOTS);
      SET_LOCALE_STRING(TXT_UNITS_SPEED_BEAUFORT);
      SET_LOCALE_STRING(TXT_UNITS_PRES_HECTOPASCALS);
      SET_LOCALE_STRING(TXT_UNITS_PRES_PASCALS);
      SET_LOCALE_STRING(TXT_UNITS_PRES_MILLIMETERSOFMERCURY);
      SET_LOCALE_STRING(TXT_UNITS_PRES_INCHESOFMERCURY);
      SET_LOCALE_STRING(TXT_UNITS_PRES_MILLIBARS);
      SET_LOCALE_STRING(TXT_UNITS_PRES_ATMOSPHERES);
      SET_LOCALE_STRING(TXT_UNITS_PRES_GRAMSPERSQUARECENTIMETER);
      SET_LOCALE_STRING(TXT_UNITS_PRES_POUNDSPERSQUAREINCH);
      SET_LOCALE_STRING(TXT_UNITS_DIST_KILOMETERS);
      SET_LOCALE_STRING(TXT_UNITS_DIST_MILES);
      SET_LOCALE_STRING(TXT_UNITS_PRECIP_MILLIMETERS);
      SET_LOCALE_STRING(TXT_UNITS_PRECIP_CENTIMETERS);
      SET_LOCALE_STRING(TXT_UNITS_PRECIP_INCHES);
   }
}

