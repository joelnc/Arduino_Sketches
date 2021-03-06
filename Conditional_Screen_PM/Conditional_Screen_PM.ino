// Goal here is to not always have screen on.
// So maybe screen off at PM < 5? Random green pixel 5-10
// Then at higher values, display PM 2.5 value using purpleair color scheme

// Still no RTC. Relaunched in kitchen at 11:37:30 on 13 June, keep with 9:30 PM resets

// AQ Lib
#include "Adafruit_PM25AQI.h"

// Screen SD Libs
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_seesaw.h>
#include <Adafruit_TFTShield18.h>

Adafruit_TFTShield18 ss; // Start Seesaw (whatever that means)

// TFT disp and SD share hardware SPI interface. Pins are board specific,
// can't remap. For Uno, others, pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
#define SD_CS    4  // Chip select line for SD card on Shield
#define TFT_CS  10  // Chip select line for TFT display on Shield
#define TFT_DC   8  // Data/command line for TFT on Shield
#define TFT_RST  -1  // Reset line for TFT is handled by seesaw!

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Set AQ to run
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();


//// Attempt SD setup ////
File logfile;
const int chipSelect = 4;

/* // Data header  (these lines get written to the beginning of a file when it's created) */
/* const char *dataHeader = "SampleNumber, PM1_S, PM25_S, PM10_S, PM1_E, PM25_E, PM10_E"; */
int samplenum = 1;      // declare the variable "samplenum" and start with 1

// Init x y locations for 5-10 ppm green squares
int xAdj = 0;
int yAdj = 0;

// Colors via http://www.barth-dev.de/online/rgb565-color-picker/
#define ST7735_ORANGE 0xFCE0

void setup() {

  ///
  // start by disabling both SD and TFT
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Start seesaw helper chip
  if (!ss.begin()){
    Serial.println("seesaw could not be initialized!");
    while(1);
  }
  Serial.println("seesaw started");
  Serial.print("Version: "); Serial.println(ss.getVersion(), HEX);

  // Start set the backlight off
  ss.setBacklight(TFTSHIELD_BACKLIGHT_OFF);
  // Reset the TFT
  ss.tftReset();

  // Initialize 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST77XX_BLACK);

  // Set backlight on fully
  ss.setBacklight(TFTSHIELD_BACKLIGHT_ON/2);

	tft.setRotation(1); // orient rel. buttons and joystick
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.print("Initializing....");

	///////    AQ setup up, currently serial based /////////////////
	// Wait one second for sensor to boot up!
  delay(1000);

  // There are 3 options for connectivity!
  if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }
	/* dispAQ(); */

	pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
		tft.setCursor(0, 60);
		tft.print("Card NOT READ");
		delay(3000);
		tft.setTextSize(3);
  } else {
		tft.setCursor(0, 60);
		tft.print("Card initialed....");
		delay(2000);
		tft.setTextSize(3);
	}
	
	// create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
	logfile.println("SampleNumber, PM1_S, PM25_S, PM10_S, PM1_E, PM25_E, PM10_E");    

}


void loop() {

  PM25_AQI_Data data;
  
  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(1000);  // try again in a bit!
    return;
  }
	// Reset the TFT
	ss.tftReset();

	// Initialize 1.8" TFT
	/* tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab */
	tft.fillScreen(ST77XX_BLACK);

	/* tft.setRotation(1); */
	/* tft.setTextColor(ST77XX_WHITE); */
	/* tft.setCursor(18, 33); tft.print("PM 1:"); */
	/* tft.setCursor(85, 33); tft.print(data.pm10_standard); */
	/* tft.setCursor(18, 60); tft.print(" 2.5:"); */
	/* tft.setCursor(85, 60); tft.print(data.pm25_standard); */
	/* tft.setCursor(18, 87); tft.print("  10:"); */
	/* tft.setCursor(85, 87); tft.print(data.pm100_standard); */

	if (data.pm25_standard < 5) {
		// Do nothing testdrawrects(ST7735_GREEN);
		// Reset the TFT
		ss.setBacklight(TFTSHIELD_BACKLIGHT_OFF);
		ss.tftReset();
	} else if (data.pm25_standard < 10){
		// Reset the TFT
		ss.tftReset();
		ss.setBacklight(TFTSHIELD_BACKLIGHT_ON/2);
		tft.fillScreen(ST77XX_BLACK);
		// Draw spec
		greenspec(ST7735_GREEN);
	} else if (data.pm25_standard < 50){
		// Reset the TFT
		ss.tftReset();
		ss.setBacklight(TFTSHIELD_BACKLIGHT_ON/2);
		tft.fillScreen(ST77XX_BLACK);
		// Print PM 25
		tft.setCursor(18, 25); tft.print("PM 2.5:");
		tft.setCursor(45, 65); tft.print(data.pm25_standard);
		// Draw rect
		testdrawrects(ST7735_YELLOW);
	} else if (data.pm25_standard < 100){
		// Reset the TFT
		ss.tftReset();
		ss.setBacklight(TFTSHIELD_BACKLIGHT_ON/2);
		tft.fillScreen(ST77XX_BLACK);
		// Print PM 25
		tft.setCursor(18, 25); tft.print("PM 2.5:");
		tft.setCursor(45, 65); tft.print(data.pm25_standard);
		// Draw rect
		testdrawrects(ST7735_ORANGE);
	} else {
		// Reset the TFT
		ss.tftReset();
		ss.setBacklight(TFTSHIELD_BACKLIGHT_ON/2);
		tft.fillScreen(ST77XX_BLACK);
		// Print PM 25
		tft.setCursor(18, 25); tft.print("PM 2.5:");
		tft.setCursor(45, 65); tft.print(data.pm25_standard);
		// Draw rect
		testdrawrects(ST7735_RED);
	}

	// SD logging
	logfile.print(samplenum);
	logfile.print(",");
	logfile.print(data.pm10_standard);
	logfile.print(",");
	logfile.print(data.pm25_standard);
	logfile.print(",");
	logfile.print(data.pm100_standard);
	logfile.print(",");
	logfile.print(data.pm10_env);
	logfile.print(",");
	logfile.print(data.pm25_env);
	logfile.print(",");
	logfile.print(data.pm100_env);
	logfile.println();
	logfile.flush();
	samplenum++;   //increment the sample number
	
  //Save the data record to the log file
  /* logData(dataRec); */

	delay(5000);
}

/* void getFileName() { */
/* 	/\* DateTime now = rtc.now(); *\/ */
/* 	// Turns file_today_name into date.txt format */
/*   sprintf(filename, "%02d%02d%02d.csv", now.year(), now.month(), now.date()); */
/* } */


void testdrawrects(uint16_t color) {
	// Draw rect, x0, y0, w, h, color
	// Bottom
	tft.drawRect(0, 110, 160, 10, color);
	tft.fillRect(0, 110, 160, 10, color);
	// Left
	tft.drawRect(0, 0, 10, 120, color);
	tft.fillRect(0, 0, 10, 120, color);
	// top
	tft.drawRect(0, 0, 160, 10, color);
	tft.fillRect(0, 0, 160, 10, color);
	// Right
	tft.drawRect(150, 0, 10, 120, color);
	tft.fillRect(150, 0, 10, 120, color);	
}

void greenspec(uint16_t color) {
	// X adj
	xAdj = rand() % 100 + 50;
	yAdj = rand() % 100 + 10;
	
	// Draw rect, x0, y0, w, h, color
	// Bottom
	tft.drawRect(xAdj, yAdj, 10, 10, color);
	tft.fillRect(xAdj, yAdj, 10, 10, color);
}





void dispAQ() {
	while (true) {
		PM25_AQI_Data data;
		
		// Reset the TFT
		ss.tftReset();

		// Initialize 1.8" TFT
		/* tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab */
		tft.fillScreen(ST77XX_BLACK);

		tft.setRotation(1);
		tft.setTextColor(ST77XX_WHITE);
		tft.setCursor(5, 30); tft.print("PM 1.0: ");
		tft.setCursor(100, 30); tft.print(data.pm10_standard);
		tft.setCursor(5, 60); tft.print("PM 2.5: ");
		tft.setCursor(100, 60); tft.print(data.pm25_standard);


		delay(4000);
	}
}

