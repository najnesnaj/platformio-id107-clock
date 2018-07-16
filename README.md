# platformio-id107
nrf51822 ID107 smartwatch arduino clock 

this is work in progress, that piggy backs on previous work by many people, mainly : @goran-mahovlic, @rogerclarkmelbourne, @curtpw, @Gordon, @micooke and in this case @arglurgl


included is  a  HOWTO on setting up platformio for the smartwatch
included is  a  HOWTO on using scripts to upload firmware 

if you just want to test, you can flash output.hex to your smartwatch

some backgroundinfo can be found here : https://github.com/najnesnaj/smartband
and here: https://github.com/najnesnaj/ota-dfu-smartband


----------------------------------------------------------------------

initially the time is set to the compile time (which would not be correct)

the program acts as a CTS (current time service) client and recovers the current date and time from a central every time the side button is pressed.

You can use "nordic nRFConnect app" to set up a CTS server.

----------------------------------------------------------------------
furthermore, there is a "led service" 
BLEService("19b10010e8f2537e4f6cd104768a1214");

if you write something to that service, it does not switch a led, but puts the watch in DFU mode   (buttonless DFU(device firmware update) mode)

once in dfu-mode, you can use the uploadblue script to upload firmware via bluetooth, afterwards the watch goes back to normal bluetooth mode

-----------------------------------------------------------------------



under the "src" directory you'll find the "arduino" program with some "nordic" flavours
-----------------------------------------------------------------------

important Remark!
===============================
the normal state of the watch is sleep-mode, you can wake it up by touching the front touch button
it will remain alive for 15 seconds (only during this time bluetooth will work)

so touch it : and then update it!




