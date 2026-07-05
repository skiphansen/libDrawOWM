# libDrawOWM

This library is a port of the drawing code from Lmarzen's excellent [esp32-weather-epd](https://github.com/lmarzen/esp32-weather-epd/tree/main) project.

The original project was a complete implementation of ESP32 based E-Paper Weather Display
targeting a 7.5" E-Paper display using weather data is fetched from the 
OpenWeatherMap API.

<img width="1265" height="845" alt="image" src="https://github.com/user-attachments/assets/a369f6de-06bc-40f8-95a8-dbbfd61dfe6b" />

<img width="659" height="699" alt="image" src="https://github.com/user-attachments/assets/49ac2c3c-c850-4ec2-8616-9852a0bec047" />


## Why?

I wanted to add support for weather from OpenWeatherMap to the [OpenEPaperLink](https://github.com/OpenEPaperLink/OpenEPaperLink/tree/master) project
as an alternative to openmetro.  I also think Lmarzen's screen design is one
of the best weather displays I've found.

OpenEPaperLink uses a client/server architecture which uses electronic shelf labels (price tags) as displays and
uses the TFT_eSPI library to render screen images to transmission to the tags.

## Differences from esp32-weather-epd

1. Currently the library uses either [Bodmer's TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) or [Seeed's fork of TFT_eSPI](https://github.com/Seeed-Studio/Seeed_GFX) rather than 
[ZinggJM's GxEPD2](https://github.com/ZinggJM/GxEPD2) as an interface to E-paper 
displays.  Support for other libraries based on [https://github.com/adafruit/Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library) can
probably be added fairly easily.

2. Most of the configuration preferences supported by the original project are 
now selected at run time rather than compile time.

3. Supports 4.2" displays thanks to RockBase's [PR#240](https://github.com/lmarzen/esp32-weather-epd/pull/240).

