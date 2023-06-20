# ESP32_Odjezdova_tabule

Goal: Displays a table of train departures/arrivals stylized to look like an Elektročas board but at a smaller scale. Uses the ESP32's Wi-Fi capabilities to fetch up-to-date data from m.vitamin.spravazeleznic.cz (API also used by SŽDC's [official Android app](https://play.google.com/store/apps/details?id=cz.cdis.vitamin&hl=en_US)). Settings are adujsted in a HTML page from a Wi-Fi-enabled device.

### Hardware:

- ESP32 Wrover Kit (most ESP32 boards should work with this repository if they have enough FLASH)
- [1.47" 320x172 IPS LCD with rounded corners](https://www.aliexpress.com/item/1005003835721393.html)
- Photoresistor and resistor
- Buttons? (Not implemented yet)

## This is a work in progress!

The project does not work yet.

So far, the capabilities of the Wi-Fi client (clock sync over NTP and querying the API), ADC/DAC and LCD library are being tested: the current version will only

- display strings that test diacritics for relevant languages (CZ/SK/DE/PL/HU) on the LCD
- fetch time over NTP and show a digital clock as a marquee (tests how much the LCD's lack of vsync & rotated VRAM (diagonal tearing) are an issue, as well as the response time of the screen (unfortunately excellent unlike the IRL STN's but the proof-of-concept animation shows how a transition effect can kinda overcome this) 
- automatically adjust screen brightness based on how much light is hitting the photoresistor
- request the departures of a hard-coded station and send the raw response over serial

### TODO:

- Parse the incoming JSON using the ArduinoJSON library.
- Process the received data (adjust capitalization, shorten strings, extract marquee text...).
- Display the data in a neat table with animation (flashing time, cycling through via stations...)
- Update the displayed information every 60 seconds. Ideally, the marquee should keep scrolling smoothly while large parts of the display are being updated.
- Implement the WatchDog, multithreading and low-energy mode for high stability and low energy consumption.
- Detect and show/recover from basic errors (invalid settings, Wi-Fi disconnected, failed to obtain time...)
- Add Wi-Fi server capabilities.
- Create a HTML page for configuration and store it on the device.
- Store settings in vEEPROM or SPIFFS and load them on boot. 
- Add OSD that shows the device's SSID and configuration URL on boot / button press / touch detection, perhaps also some useful status information.

### Long-term goals outside the [MVP](https://en.wikipedia.org/wiki/Minimum_viable_product):

- Support for more realistic and/or useful displays ([STN](https://www.aliexpress.com/item/2052440680.html)? *VGA*?) There seems to be enough RAM for a 1-bit 1024x768 frame buffer.
- Support for departure boards by ELEN (LED) or Mobatime
- Support for high-res boards (possibly scaled down (with subpixel antialiasing?) because no affordable display has sufficient pixel-density to display them at native resolution close to 1:87 scale), maybe even split-flap Pragotron when we're at it - might be lucrative because fans LOVE them
- Support for other data sources (using SŽDC's API without prior consultation might be illegal, also model railways with their own schedules have custom controllers that could be interfaced with via serial)
