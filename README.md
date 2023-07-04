# ESP32_Odjezdova_tabule

**Goal:** A tabletop or wall-mounted display for a railway fan's room or train station model. A small screen shows a table of train departures/arrivals stylized to look like an Elektročas board but at a smaller scale. Uses the ESP32's Wi-Fi capabilities to fetch up-to-date data from m.vitamin.spravazeleznic.cz (API also used by SŽDC's [official Android app](https://play.google.com/store/apps/details?id=cz.cdis.vitamin&hl=en_US)). Settings are adujsted in a HTML page from a Wi-Fi-enabled device.
    
     (                                                                             
     )\ )                (                        (                             )  
    (()/(    (   )     ( )\            (  (     ( )\   (  (               (  ( /(  
     /(_))  ))\ /((   ))((_)(  `  )   ))\ )(    )((_) ))\ )(   (     (   ))\ )\()) 
    (_))_  /((_|_))\ /((_)  )\ /(/(  /((_|()\  ((_)_ /((_|()\  )\ )  )\ /((_|_))/  
     |   \(_)) _)((_|_))| |((_|(_)_\(_))  ((_)  | _ |_))( ((_)_(_/( ((_|_))(| |_   
     | |) / -_)\ V // -_) / _ \ '_ \) -_)| '_|  | _ \ || | '_| ' \)) _ \ || |  _|  
     |___/\___| \_/ \___|_\___/ .__/\___||_|    |___/\_,_|_| |_||_|\___/\_,_|\__|  
                              |_|                                                  
    Sorry, please wait before the developer sorts some of his life issues before
    code for version 0.1 is committed. It is mostly done but needs some testing.

Version 1.0, the minimum viable product, should imitate this screen at Uherské Hradiště railway station  
![](https://d34-a.sdn.cz/d_34/c_img_QQ_y/th6ID0.jpeg) ![](https://raw.githubusercontent.com/ChaoticNeutralCzech/myRandomImages/main/ytyGax.jpg)  
by showing the following on the display:  
![](https://raw.githubusercontent.com/ChaoticNeutralCzech/myRandomImages/main/Odjezdy_UH_PoC3.gif)  
("DEMO" because this very much does not actually work yet.)

### Hardware:

- ESP32 Wrover Kit (most ESP32 boards should work with this repository if they have enough FLASH)
- [1.47" 320x172 IPS LCD with rounded corners](https://www.aliexpress.com/item/1005003835721393.html)
- Photoresistor and resistor
- Buttons? (Not implemented yet)

## This is a work in progress!

The project does not work yet. Really, not at all. It's been a nightmare developing this for effing 6 weeks and seeing so little progress while facing countless errors and issues. Burnout, yeah!


### TODO:

- [X] Parse the incoming JSON using the ArduinoJSON library.
- [X] Process the received data (adjust capitalization, shorten strings, extract marquee text...).
- [X] Display the data in a neat table with animation (flashing time, cycling through via stations...)
- [X] Update the displayed information every 60 seconds. Ideally, the marquee should keep scrolling smoothly while large parts of the display are being updated.
- [X] Implement the WatchDog, multithreading and low-energy mode for high stability and low energy consumption.
- [X] Detect and show/recover from basic errors (invalid settings, Wi-Fi disconnected, failed to obtain time...)
- [X] Add Wi-Fi server capabilities.
- [X] Create a HTML page for configuration and store it on the device.
- [X] Store settings in vEEPROM or SPIFFS and load them on boot. 
- [X] Add OSD that shows the device's SSID and configuration URL on boot / button press / touch detection, perhaps also some useful status information.

### Long-term goals outside the [MVP](https://en.wikipedia.org/wiki/Minimum_viable_product):

- Support for more realistic and/or useful displays ([STN](https://www.aliexpress.com/item/2052440680.html)? *VGA*?) There seems to be enough RAM for a 1-bit 1024x768 frame buffer.
- Support for departure boards by ELEN (LED) or Mobatime
- Support for high-res boards (possibly scaled down (with subpixel antialiasing?) because no affordable display has sufficient pixel-density to display them at native resolution close to 1:87 scale), maybe even split-flap Pragotron when we're at it - might be lucrative because fans LOVE them
- Support for other data sources (using SŽDC's API without prior consultation might be illegal, also model railways with their own schedules have custom controllers that could be interfaced with via serial; considering [crws.cz](https://crws.docs.apiary.io/#reference/odjezdy-a-odjezdove-tabule/odjezdove-tabule), which is  also proprietary and needs keys but at least we have documentation)
