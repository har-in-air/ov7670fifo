# ESP32 camera web server connected to ov7670+fifo camera module

Functions as a web server to periodically serve camera image frames from an OV7670 camera module with AL422 FIFO interfaced to an ESP32 module. 

The esp32 first tries to initialize as a station connecting to an existing wifi network. If it succeeds, you can then open a web browser with the url specified as the IP address of the station (you will see the IP address assigned to the ESP32 in the console debug monitor). e.g. in my case it was  http://192.168.1.108

When you connect, the server will transmit a javascript web page from the SPIFFS flash file system and your browser will execute it. The javascript code requests an image, the ESP32 responds by triggering a single camera frame capture to the on-board FIFO, and then reads out and transmits the binary data of the QVGA (320x240) YUV image. The javascript code translates the data into RGB pixels and paints an HTML canvas on your browser page. The whole process is repeated every few seconds to update the image.

If unable to connect to an existing WIFI network, the esp32 will set up as an access point with SSID 'ESP32Cam'. In this case, first connect to this network, then open your browser and set the url to http://192.168.4.1. Everything else follows as above.

Pulls together various bits of code from the net that I modified for this project 

1. Spiffs code from https://github.com/loboris/ESP32_spiffs_example
2. HTTP server code from https://github.com/tekker/esp32-ov7670-hacking
3. Setting up as access point or station https://github.com/nkolban/esp32-snippets/tree/master/networking/bootwifi
4. Debugging the ov7670 camera setup with a python script http://www.rpg.fi/desaster/blog/2012/10/20/ov7670-fifo-msp430-launchpad/

### OV7670 + AL422 camera module
Referring to 

http://wiki.beyondlogic.org/index.php/OV7670_Camera_Module_with_AL422_FIFO_Theory_of_Operation

it appears I have a module set up for a negative VSYNC. And it has a 12MHz oscillator for the OV7670 pixel clock. So it appears to be different than the common examples I've seen. Maybe that was why it was so much cheaper (~ $10) ! Anyways, for this module you need to ground the PWDN and OE pins, connect WRST to VSYNC, pullup RST to 3V3 with a resistor (I used 22K) and pullup SCL/SDA pins to 3v3 (I used 4.7K). The STR pin is unconnected. You can find the pin connections to the ESP32 in OV7670.h. I used only pin #s lower than 32 to optimize the set and clear gpio pin functions. 

There are lots of examples of code on the net with OV7670 register setups (it's mostly undocumented registers :-( ). I was only able to get QVGA YUV and RGB565 images working - no luck with VGA. Maybe something to do with the peculiarities of this particular module's schematic and clock frequency.

#### How to build
Execute 'make menuconfig' and set serial flasher com port and baudrate. Set your existing wifi access point SSID name and password. If you are planning to use the esp32 camera as an AP, this is not required.

Execute 'make flashfs' to build and flash the SPIFFS image containing the javascript html pages. This needs to be done only once, unless you decide to modify the javascript code to do something different - e.g. display monochrome images, do edge detection etc. 

Execute 'make flash monitor' to build and flash the application code. You can verify the camera initialization and station/access point initialization on the serial debug monitor terminal. The console baudrate is set up as 230400 baud to speedup transmitting of camera frames over the serial port when checking for correct OV7670 register setup. Uncomment #define CAMERA_DEBUG in main.c for this purpose. You will find appropriate python scripts in the toplevel directory for debugging. 
Example usage : 'python camview_qvgayuv.py com7 230400'

Comment out #define CAMERA_DEBUG to run in normal webserver mode. The LED connected to ESP32 pin 21 will flash everytime a new camera frame is captured by the browser.

I was not able to get this working with ESP32 clock frequencies higher than 80MHz, despite slowing the i2c clock and slowing the fifo pixel readout clock. Probably something to do with the wiring. My ESP32 module is soldered to an adapter board connected to an intermediate adapter board connected to the OV7670. I have seen references to problems seen with OV7670 wiring - noise or no picture at all with longer interface wires, maybe the sub-optimal wiring is the issue.


