
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_seesaw.h>
#include <Adafruit_TFTShield18.h>

Adafruit_TFTShield18 ss;


// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
#define SD_CS    4  // Chip select line for SD card on Shield
#define TFT_CS  10  // Chip select line for TFT display on Shield
#define TFT_DC   8  // Data/command line for TFT on Shield
#define TFT_RST  -1  // Reset line for TFT is handled by seesaw!

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

/* void setRotation(uint8_t rotation) */

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
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

  Serial.println("TFT OK!");
  tft.fillScreen(ST77XX_BLACK);

  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
  } else {
    Serial.println("OK!");
    File root = SD.open("/");
    printDirectory(root, 0);
    root.close();
  }

  // Set backlight on fully
  ss.setBacklight(TFTSHIELD_BACKLIGHT_ON);

	tft.setRotation(1);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.print("Air Quality");
  tft.setCursor(10, 20);
	tft.print("Other words");
}


uint8_t buttonhistory = 0;

void loop() {
  uint32_t buttons = ss.readButtons();
  tft.setTextSize(3);
  if(! (buttons & TFTSHIELD_BUTTON_DOWN)){
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(0, 10);
    tft.print("Down ");
    buttonhistory |= 1;
  }
  if(! (buttons & TFTSHIELD_BUTTON_LEFT)){
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(0, 35);
     tft.print("Left ");
    buttonhistory |= 2;
  }
  if(! (buttons & TFTSHIELD_BUTTON_UP)){
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(0, 60);
    tft.print("Up"); 
    buttonhistory |= 4;
  }
  if(! (buttons & TFTSHIELD_BUTTON_RIGHT)){
    tft.setTextColor(ST77XX_BLUE);
    tft.setCursor(0, 85);
    tft.print("Right");
    buttonhistory |= 8;
  }
  if(! (buttons & TFTSHIELD_BUTTON_1)){
    tft.setTextColor(ST77XX_BLUE);
    tft.setCursor(0, 140);
    tft.print("1");
    buttonhistory |= 16;
  }
  if(! (buttons & TFTSHIELD_BUTTON_2)){
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(50, 140);
    tft.print("2");
    buttonhistory |= 32;
  }
  if(! (buttons & TFTSHIELD_BUTTON_3)){
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(100, 140);
    tft.print("3");
    buttonhistory |= 64;
  }
  if (! (buttons & TFTSHIELD_BUTTON_IN)) {
    tft.setTextColor(ST77XX_MAGENTA);
    tft.setCursor(0, 110);
    tft.print("SELECT");
  }
  if (buttonhistory == 0x7F) {
    tft.print("Tacos!");
    while (1) {
      tft.invertDisplay(true);
      delay(500);
      tft.invertDisplay(false);
      delay(500);
    }
  }
  delay(100);
}


void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
