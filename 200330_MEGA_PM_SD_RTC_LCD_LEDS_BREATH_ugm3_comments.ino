// Proyecto NOTOS - Medición de partículas volátiles (PM10, PM2.5)
// Mediación-Investigación 2018-2020
// Febrero 2020 - Silvia Teixeira en Medialab-Prado

#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <Adafruit_NeoPixel.h>

// data pin for NeoPixel ring
#define PIN 49
// number of pixelns on the ring
#define NUMPIXELS 8
// Serial1 port on Arduino Mega TX1:18, RX1:19 
#define pmsSerial Serial1

unsigned long interval = 60000;
unsigned long previousMillis = 0;

// control pin for RTC
const int ChipSelect=10;

#define LOG_FILE_PREFIX "PMlog" // Name of the log file.
#define MAX_LOG_FILES 100 // Number of log files that can be made
#define LOG_FILE_SUFFIX "csv" // Suffix of the log file
char logFileName[13]; // Char string to store the log file name
// Data to be logged:
#define LOG_COLUMN_COUNT 9

char * log_col_names[LOG_COLUMN_COUNT] = {
  "1um (ug/m3)", "2.5um (ug/m3)", "10um (ug/m3)","1um", "2.5um", "10um", "Fecha","Hora", "Temp"
}; // log_col_names is printed at the top of the file.

File logFile;

RTC_DS3231 rtc;

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
 
  Serial.begin(9600); 
  pmsSerial.begin(9600);

  lcd.init();
  lcd.backlight();

  pixels.begin();
  
// SD Card Initialization
  if (!SD.begin(ChipSelect))
  {
    Serial.println("Error iniciando la tarjeta SD.");
    return;
  }
  Serial.println("Tarjeta SD iniciada.");

updateFileName();
printHeader();


  //open file
  logFile=SD.open("nombreArchivo", FILE_WRITE);

  // if the file opened ok, write to it:
  if (logFile) {
    Serial.println("Open File");
    // print the headings for our data
    logFile.println("Particle Concentration");
  }
  logFile.close();

//---- Set RTC ------//

  // ajuste manual
  //rtc.adjust(DateTime(2020, 2, 21, 16, 39, 0)); 

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

struct pms5003data data;

    
void loop() {
  
  if (readPMSdata(&pmsSerial)) {
    // reading data was successful!
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Unidades de Concentration (ug/m3)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_env);
    Serial.println("---------------------------------------");
    Serial.print("Particulas > 0.3um / 0.1L aire:"); Serial.println(data.particles_03um);
    Serial.print("Particulas > 2.5um / 0.1L aire:"); Serial.println(data.particles_25um);
    Serial.print("Particulas > 10.0 um / 0.1L aire:"); Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");

    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.print("Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("AIR_BOREAS");
    lcd.setCursor(0,1);
    lcd.print("Calidad del aire");
    //delay(3000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("PM 2.5: "); 
    lcd.setCursor(8,0);
    lcd.print(data.pm25_env );
    lcd.setCursor(10,0);
    lcd.print("ug/m3"); 
    lcd.setCursor(0,1);
    lcd.print("PM 10: "); 
    lcd.setCursor(7,1);
    lcd.println(data.pm100_env );
    lcd.setCursor(9,1);
    lcd.print(" ug/m3");
    //delay(1000);

//---- set NeoPixel lightning ----//

  if(data.pm25_env && data.pm100_env < 10)
  {
    for(int j=20;j>0;j--) // increases brightness
    {
      for(int i=0;i<NUMPIXELS;i++) // lights up one pixel at a time, unless delay (0);
      {
        pixels.setPixelColor(i, pixels.Color(0, j, 0)); // green
        pixels.show(); // This sends the updated pixel color to the hardware.
        delay (100);
      }
      delay(10);
    }

    for(int j=0;j<20;j++) // decreases brightness
    {
      for(int i=0;i<NUMPIXELS;i++) 
      {
        pixels.setPixelColor(i, pixels.Color(0, j, 0)); // green
        pixels.show();
        delay (100);
      }
      delay(10);
    }
  }

  else if(data.pm25_env && data.pm100_env >= 10 && data.pm10_env && data.pm100_env <= 20)
  {
    for(int j=0;j<15;j++)
    {
      for(int i=0;i<NUMPIXELS;i++)
      {
        pixels.setPixelColor(i, pixels.Color(3*j, j, 0)); // orange
        pixels.show(); 
        delay (100);
      }
      delay(1);
    } 
    
    for(int j=15;j>0;j--)
    {
      for(int i=0;i<NUMPIXELS;i++)
      {
        pixels.setPixelColor(i, pixels.Color(3*j, j, 0)); // orange
        pixels.show();
        delay (100);
      }
      delay(1);
    }

  }

  else if(data.pm25_env && data.pm100_env >= 20){
  
    for(int j=20;j>0;j--)
        {
          for(int i=0;i<NUMPIXELS;i++)
          {
            pixels.setPixelColor(i, pixels.Color(j, 0, 0)); // red
            pixels.show(); 
            delay (100);
          }
          delay(10);
        }
    
        for(int j=0;j<20;j++)
        {
          for(int i=0;i<NUMPIXELS;i++)
          {
            pixels.setPixelColor(i, pixels.Color(j, 0, 0)); // red
            pixels.show(); 
            delay (100);
          }
          delay(10);
        } 
    }
  GuardarPM(); // saves data on SD card
    
    //delay(3000);
  }
}

byte GuardarPM(){
  
    unsigned long currentMillis = millis(); // counts time to execute a timer for saving data on the SD card

    if (currentMillis - previousMillis > interval)
    {
      DateTime now = rtc.now();
      logFile = SD.open(logFileName, FILE_WRITE);
      
      if(logFile)
      {
        logFile.print(data.pm10_env);
        logFile.print(',');
        logFile.print(data.pm25_env);
        logFile.print(',');
        logFile.print(data.pm100_env);
        logFile.print(',');
        logFile.print(data.particles_03um);
        logFile.print(',');
        logFile.print(data.particles_25um);
        logFile.print(',');
        logFile.print(data.particles_100um);
        logFile.print(',');
        logFile.print(now.year(), DEC);
        logFile.print('-');
        logFile.print(now.month(), DEC);
        logFile.print('-');
        logFile.print(now.day(), DEC);
        logFile.print(',');
        logFile.print(now.hour(), DEC);
        logFile.print(':');
        logFile.print(now.minute(), DEC);
        logFile.print(',');
        logFile.print(rtc.getTemperature());
        logFile.println();
        logFile.close();
        return 1;
      }
      previousMillis = millis();
    }
    return 0;
  }
  
void printHeader()
{
  logFile = SD.open(logFileName, FILE_WRITE); // Open the log file

  if (logFile) // If the log file opened, print our column names to the file
  {
    for (int i=0; i < LOG_COLUMN_COUNT; i++)
    {
      logFile.print(log_col_names[i]);
      if (i < LOG_COLUMN_COUNT - 1) // If it's anything but the last column
        logFile.print(','); // print a comma
      else // If it's the last column
        logFile.println(); // print a new line
    }
    logFile.close(); // close the file
  }
}

// updateFileName() - Looks through the log files already present on a card,
// and creates a new file with an incremented file index.
void updateFileName()
{
  int i = 0;
  for (; i < MAX_LOG_FILES; i++)
  {
    memset(logFileName, 0, strlen(logFileName)); // Clear logFileName string
    // Set logFileName to "PMlogXX.csv":
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    if (!SD.exists(logFileName)) // If a file doesn't exist
    {
      break; // Break out of this loop. We found our index
    }
    else // Otherwise:
    {
      Serial.print(logFileName);
      Serial.println(" exists"); // Print a debug statement
    }
  }
  Serial.print("File name: ");
  Serial.println(logFileName); // Debug print the file name
}


boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }

  /* debugging
  for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
  }
  Serial.println();
  */
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}


