# Door-Sign
Electronic WiFi door sign to show when you're busy

The hardware is as follows;
A Wemos D1 mini (or clone) or lite x 1
https://www.wemos.cc/en/latest/d1/d1_mini.html

A Wemos D1 mini RGB LED shield x 1
https://www.wemos.cc/en/latest/d1_mini_shield/rgb_led.html

A Wemos D1 mini battery shield x 1
https://www.wemos.cc/en/latest/d1_mini_shield/battery.html

A 'tripler' board for the above x 1
https://www.wemos.cc/en/latest/d1_mini_shield/tripler_base.html

A flat 3.7v li-po battery of your choice.  I used this 4000mah one and it lasts for around 3 days;
https://aliexpress.com/item/4001359252535.html

The battery (if you're using the case I designed below) should not exceed these dimensions (it's recommended not to get too close to them to allow heat dissipation);
66mm x 72mm x 95mm

The D1 mini and the two shields should be installed onto the tripler.  I soldered the little header stands that come with it, so I can quickly replace any of the three.  I also removed the plug from the battery (be careful not to short it) and soldered it onto the battery shield underneath.

The 3D printed case is in three parts.  The base has slots for the tripler to slide into, along with a hole for a screw to mount the sign.  There is another smaller hole and notch to route the battery cable in one of two ways, depending on whether you want the battery shield on the left or right.  The filter is optional and can be used with white/crystal filament to add a screen to the LEDs.  You may want to glue this in each corner and around the main case lid hole, but it's designed to be a press fit.  The lid drops over the base entirely once installation of the board and battery are complete and tested.  The lid drops over the base entirely once installation of the board and battery are complete and tested.  Make sure the lid goes on the right way around by checking the USB port holes left and right at the bottom.  There are two friction strips to give a tighter fit, you may need to sand these slightly to allow easier removal of the lid.
https://www.thingiverse.com/thing:5343020

The code is in this repo.  It's an Arduino sketch that you just need to open in Arduino IDE etc and update the user config section at the top.  Be sure to download/install the libraries shown at the top.  Then compile and flash to the Wemos D1 mini.
The code is 200 lines long and simply connects to your WiFi and an MQTT broker to listen for messages on a topic **OfficeSign/meeting** and then send the battery level back on **OfficeSign/status**
The messages that it will recognise are;

meeting - you're busy and this will make the sign flash a red cross
nomeeting - this will return the sign to the default green cycling circle
sleep - switch off the LEDs
awake - switch the LEDs back on

How you want to trigger the sign is the fun part, but for instance you can use a button on your desk, timers or have something like Homeassistant detect the mic on your PC via the companion app.

The battery status is read from the Wemos A0 pin, so solder across the J2 jumper on the battery sheild to enable to built-in resistor to enable this correctly.  The sketch reads the value and maps it to a %. Batteries and climates differ, so you may find that this is not totally accurate in your case.  If so, set the first and second numbers in the map statement on line 136 as required to give a more accurate conversion.  By default these are 600 and 970.  I found these matched the 4000mah battery pretty well most of the time. You could change it to output just the value as read and post-process somewhere else to save altering the sketch.
