//This code for the ID107 smartwatch using sandeep mistry's core in platformio
//(see https://github.com/najnesnaj/platformio-id107 -> readme.md)
//is a demo of a low power rtc application, it goes to sleep after some time
// and wakes up on front button touch. Time is kept in sleep mode via millis()/RTC1.
//Currently hacky implementations:
//The clock also wakes up every 512 seconds to update millis() (triggered by
//core functions) which activates the screen. Furthermore now() needs to be
//called at least every 50 days or so to prevent loosing track of time,
//because millis() overflows at that point. This is currently not assured in this code.
//Current consumption:
//I measured 9 milliamps when active and 14 microamps when sleeping.
//To really get the low microamps you need to power cycle the watch after programming
//(because of the debug logic still being active, ca 1.3 mA) and disconnect
//the debug adapter from it (because of leakage, ca 30 uA).
//Written by Arglurgl
/*
   CTSClient.ino

   Written by Chiara Ruggeri (chiara@arduino.org)

   This example shows the client capabilities of the
   BLEPeripheral library.
   It acts as a CTS client and recovers the current date and
   time from a central every time USER1 button is pressed.
   The informations are transmitted as a string. A parse to
   byte type is needed to read the correct values.
   You can use nRFConnect app to set up a CTS server or use
   another board with CTSServer example in
   File->Examples->BLE->Central menu.
   To have further informations about the CTS service please
   refer to the Bluetooth specifications:
https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.current_time.xml

This example code is in the public domain.

*/

#include <Arduino.h>
#include <SSD1306Spi.h>
#include <TimeLib.h>//get this lib here: https://github.com/PaulStoffregen/Time
#include <compile_time.h>//macro for build time as unix time, for setting clock,optional
#include <BLEPeripheral.h>
uint32_t last_wakeup = 0;

#define SLEEP_TIMEOUT_MS 15000

#define OLED_WIDTH 64
#define OLED_HEIGHT 32
SSD1306Spi oled(OLED_RST, OLED_DC, OLED_CS); // (pin_rst, pin_dc, pin_cs)

char * months [] = {"Unknown", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
char * days [] = {"Unknown", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};


// create peripheral instance
BLEPeripheral                    blePeripheral                            = BLEPeripheral();

// create remote service with UUID compliant to CTS service
BLERemoteService                 remoteCtsService            = BLERemoteService("1805");

// create remote characteristics with UUID and properties compliant to CTS service 
BLERemoteCharacteristic          remoteCtsCharacteristic           = BLERemoteCharacteristic("2a2b", BLERead | BLENotify);






void blePeripheralConnectHandler(BLECentral& central) { 
	Serial.print(F("Connected event, central: "));
	Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {

	Serial.print(F("Disconnected event, central: "));
	Serial.println(central.address());
}

void blePeripheralRemoteServicesDiscoveredHandler(BLECentral& central) {

	Serial.print(F("Remote services discovered event, central: "));
	Serial.println(central.address());

	if (remoteCtsCharacteristic.canRead()) {
		remoteCtsCharacteristic.read();
	}
}




void bleRemoteCtsCharacteristicValueUpdatedHandle(BLECentral& central, BLERemoteCharacteristic& characteristic) {
	// copy the time value in a local variable
	char currentTime[BLE_REMOTE_ATTRIBUTE_MAX_VALUE_LENGTH + 1];
	memset(currentTime, 0, sizeof(currentTime));
	memcpy(currentTime, remoteCtsCharacteristic.value(), remoteCtsCharacteristic.valueLength());
	// year is the first two bytes of the string
	byte first=currentTime[0];
	byte second=currentTime[1];
	uint16_t year=first | (second << 8);
	// month is the third byte
	byte month = currentTime[2];
	// day, hours, minutes and seconds follow
	byte day = currentTime[3];
	byte hours = currentTime[4];
	byte minutes = currentTime[5];
	byte seconds = currentTime[6];


	setTime(hours,minutes,seconds, day, month,year);

	// day of the week
	byte dow = currentTime[7];
	// 1/256th of a second
	byte fraction = currentTime[8];
	// Print current time
	  Serial.println("*** Current Time received ***");
	    Serial.print(day);
	    Serial.print(" ");
	    Serial.print(months[month]);
	    Serial.print(" ");
	    Serial.print(year);
	    Serial.print(", ");
	    Serial.print(hours);
	    Serial.print(":");
	    Serial.print(minutes);
	    Serial.print(":");
	    Serial.println(seconds);
	    Serial.print("Day of the week: ");
	    Serial.println(days[dow]);
	    Serial.print("Fractions: ");
	    Serial.print(fraction);
	    Serial.println(" / 256 s");
	    Serial.println("-------------------------------------");
}
	    

void readTime(){
	remoteCtsCharacteristic.read();
}

void draw_clock(){  //draw clock on OLED
	char buffer[11];
	time_t time_now = now();
	oled.clear();
	oled.drawString(0, 0, "CLOCK");
	sprintf(buffer,"%02d.%02d.%04d",day(time_now),month(time_now),year(time_now));
	oled.drawString(0, 10, buffer);
	sprintf(buffer,"%02d:%02d:%02d",hour(time_now),minute(time_now),second(time_now));
	oled.drawString(0,20,buffer);
	oled.display();
}

void setup() {
	pinMode(PIN_BUTTON2, INPUT);//front button
	pinMode(PIN_BUTTON1, INPUT_PULLUP);//side button

	setTime(__TIME_UNIX__);//set clock to build time

	blePeripheral.setLocalName("CTS-Client");

	// set device name and appearance
	blePeripheral.setDeviceName("CTS client");
	blePeripheral.setAppearance(BLE_APPEARANCE_GENERIC_CLOCK);

	blePeripheral.addRemoteAttribute(remoteCtsService);
	blePeripheral.addRemoteAttribute(remoteCtsCharacteristic);
	//
	// assign event handlers for connected, disconnected to peripheral
	blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
	blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
	blePeripheral.setEventHandler(BLERemoteServicesDiscovered, blePeripheralRemoteServicesDiscoveredHandler);

	// assign event handlers for characteristic
	remoteCtsCharacteristic.setEventHandler(BLEValueUpdated, bleRemoteCtsCharacteristicValueUpdatedHandle);


	// begin initialization
	blePeripheral.begin();
	oled.setScreenSize(OLED_WIDTH, OLED_HEIGHT);
	oled.init();
	oled.flipScreenVertically();
	oled.setTextAlignment(TEXT_ALIGN_LEFT);
	oled.setFont(ArialMT_Plain_10);

	// Configure and Enable interrupt on nRF side
	NRF_GPIO->PIN_CNF[PIN_BUTTON2] |= (GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos);
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Enabled << GPIOTE_INTENSET_PORT_Pos;
	// Configure ARM chip to wakeup on interrupt from nRF side, but no need to call ISR
	NVIC_DisableIRQ(GPIOTE_IRQn);
	SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
}



void loop() {

	while (1) {
		NRF_GPIOTE->EVENTS_PORT = 0;
		NVIC_ClearPendingIRQ(GPIOTE_IRQn);

		if ((millis()-last_wakeup) > SLEEP_TIMEOUT_MS) {//check if timeout occured
			oled.sleep();//Put display to sleep
			__WFE();   // sleep nrf51
//we can possiblly check somewhere here if we woke up from button or
//RTC1 overflow which triggers wakeup every 512 seconds...
//possibly also add call to now() here.
			oled.wake();//we woke up, re-enable display
			last_wakeup = millis();//save last wakeup
		}
			readTime();	
		if (digitalRead(PIN_BUTTON1)==0) // sidebutton pushed => readtime  
		{
			//use the bluetooth time service to set the time
			readTime();	
		}
		// poll peripheral and switch to dfu mode
		draw_clock();
		// either woken up or didn't sleep. Clear event register anyways.
		__SEV();        // Set event register
		__WFE();        // clear event register
	}
}
