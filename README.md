# libDrawOWM

This library is a port of the drawing code from Lmarzen's excellent [esp32-weather-epd](https://github.com/lmarzen/esp32-weather-epd/tree/main) project.

The Lmarzen's project is a complete implementation of ESP32 based E-Paper Weather Display
targeting a 7.5" E-Paper display using weather data is fetched from the 
OpenWeatherMap API.  This library contains just the drawing code plus some minor changes to allow configuration at run time.

## Why?

I wanted to add support for weather from OpenWeatherMap to the [OpenEPaperLink](https://github.com/OpenEPaperLink/OpenEPaperLink/tree/master) project
as an alternative to open-metro.com.

Additionally I think Lmarzen's screen design is one of the best weather displays I've seen.

OpenEPaperLink uses a client/server architecture which uses electronic shelf labels (price tags) as displays and
uses the TFT_eSPI library to render screen images to transmission to the tags.

Since most users of OpenEPaperLink flash their boards with release binaries
rather than building from source it is important to be able to configure
the language and measurement units at run time rather than compile time.


<img width="1265" height="845" alt="image" src="https://github.com/user-attachments/assets/a369f6de-06bc-40f8-95a8-dbbfd61dfe6b" />

<img width="659" height="699" alt="image" src="https://github.com/user-attachments/assets/49ac2c3c-c850-4ec2-8616-9852a0bec047" />

<img width="892" height="741" alt="image" src="https://github.com/user-attachments/assets/342e423f-7e79-422e-b4dd-8cd26ac5cb67" />


## Differences from esp32-weather-epd

1. Currently the library can use [Bodmer's TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) and [Seeed's fork of TFT_eSPI](https://github.com/Seeed-Studio/Seeed_GFX) rather than 
[ZinggJM's GxEPD2](https://github.com/ZinggJM/GxEPD2) for the interface to E-paper display.  Support for other libraries based on [Adafruit's GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library) can
probably be added fairly easily.

2. Most of the configuration preferences supported by the original project are 
now selected at run time rather than compile time.

3. Support for 400 x 300 (4.2") displays added thanks to 
RockBase's [PR#240](https://github.com/lmarzen/esp32-weather-epd/pull/240).

