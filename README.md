# libDrawOWM

This library is a port of the drawing code from Lmarzen's excellent [esp32-weather-epd](https://github.com/lmarzen/esp32-weather-epd/tree/main) project.

The Lmarzen's project is a complete implementation of a ESP32 based E-Paper Weather Display
targeting a 7.5" E-Paper display using weather data fetched from the 
OpenWeatherMap API.  

This library contains just the drawing code with the following modifications:
1. The library can be used with either [Bodmer's TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) or [Seeed's fork of TFT_eSPI](https://github.com/Seeed-Studio/Seeed_GFX) 
library rather than [ZinggJM's GxEPD2](https://github.com/ZinggJM/GxEPD2). <br>
Support for other libraries based on [Adafruit's GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library) can
probably be added fairly easily.

2. Most of the configuration preferences supported by the original project are 
now selected at run time rather than compile time.

3. Support for 400 x 300 (4.2") displays added thanks to 
RockBase's [PR#240](https://github.com/lmarzen/esp32-weather-epd/pull/240).

4. Most of the previously hard coded ICON bitmaps have been replaced by TrueType rendering.

## Why?

I think Lmarzen's screen design is one of the best that I've seen and I 
wanted to add it as an option to the [OpenEPaperLink](https://github.com/OpenEPaperLink/OpenEPaperLink/tree/master) project.

Additionally I wanted to see if the weather predications from OpenWeatherMap
were more accurate for the locations I'm interested in than open-metro.com.

OpenEPaperLink uses a client/server architecture which uses electronic shelf labels (price tags) as displays and
uses the TFT_eSPI library to render screen images to transmission to the tags.

Since most users of OpenEPaperLink flash their boards with release binaries
rather than building from source it is important to be able to configure
the language and measurement units at run time rather than compile time.

If you would like to help test the OpenEPaperLink OWM support a Beta release is available.  Installation instructions can be found [here](https://github.com/skiphansen/OpenEPaperLink/wiki/Installing-OpenWeatherMap-Beta).

## Example 800 x 480 (7.4") screen layout

<img width="1265" height="845" alt="image" src="https://github.com/user-attachments/assets/a369f6de-06bc-40f8-95a8-dbbfd61dfe6b" />

## Example 640 x 384 (7.4") screen layout

<img width="892" height="741" alt="image" src="https://github.com/user-attachments/assets/342e423f-7e79-422e-b4dd-8cd26ac5cb67" />

## Example 400 x 300 (4.2") screen layout

<img width="659" height="699" alt="image" src="https://github.com/user-attachments/assets/49ac2c3c-c850-4ec2-8616-9852a0bec047" />

## Compile time configuration

By default the library will embed the TrueType ICON data within the library.
If desired TTF data can be read from file(s) in the flash filesystem by setting
conditional compile variables 

| Environment Variable | |
| - | - |
| TTF_PATH_WEATHER_ICONS | full path to erikflower's weather Icons TTF file<br>For example /fonts/weathericons-regular-webfont.ttf|
| TTF_PATH_OWM_ICONS | full path to owm_icons.ttf |
| OWM_USE_BITMAPS | if set the original esp32-weaher-epd bitmaps are used rather than the TTF versions |





