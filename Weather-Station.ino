// things for temp/humidity
#include "DHT.h"
#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors
DHT dht(DHTPIN, DHTTYPE);

// things for lcd
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
// create an LCD object (Hex address, # characters, # rows)
// my LCD display in on Hex address 27 and is a 20x4 version
LiquidCrystal_I2C lcd(0x27, 20, 4); 

// things for air presure
#include <SFE_BMP180.h>
//#include <Wire.h>
SFE_BMP180 bmp180;
#define ALTITUDE 20.0 // Altitude in meters

void setup() {
  
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) {}
    
  dht.begin();
  
  lcd.init();  
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Initializing");

  if (bmp180.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}

int timeSinceLastRead = 0;
void loop(){

  char bmpStatus;
  double temp, _pressure, pressure, hum;
  
  // Report every 2 seconds.
  if(timeSinceLastRead > 2000) {

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float hum = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(hum) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, hum);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, hum, false);

    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F\n");

    // Start a temperature measurement:
    // If request is successful, the number of ms to wait is returned.
    // If request is unsuccessful, 0 is returned.
    
    bmpStatus = bmp180.startTemperature();
    if (bmpStatus != 0) {

      delay(bmpStatus);
      bmpStatus = bmp180.getTemperature(temp);
      if (bmpStatus != 0) {
          
        // Start a pressure measurement:
        // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
        // If request is successful, the number of ms to wait is returned.
        // If request is unsuccessful, 0 is returned.
    
        bmpStatus = bmp180.startPressure(3);
        if (bmpStatus != 0) {
        
          // Wait for the measurement to complete:
          delay(bmpStatus);
            // Retrieve the completed pressure measurement:
            // Note that the measurement is stored in the variable P.
            // Note also that the function requires the previous temperature measurement (T).
            // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
            // Function returns 1 if successful, 0 if failure.
    
            bmpStatus = bmp180.getPressure(_pressure, temp);
            if (bmpStatus != 0) {
    
              // The pressure sensor returns abolute pressure, which varies with altitude.
              // To remove the effects of altitude, use the sealevel function and your current altitude.
              // This number is commonly used in weather reports.
              // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
              // Result: p0 = sea-level compensated pressure in mb
    
              pressure = bmp180.sealevel(_pressure, ALTITUDE); // we're at 1655 meters (Boulder, CO)
              Serial.print("relative (sea-level) pressure: ");
              Serial.print(pressure,2);
              Serial.print(" mbar\n");
            }
        }
      }
    }

    lcd.setCursor(0, 0);
    lcd.print("T: " + String(temp) + " H: " + String(hum));
    lcd.setCursor(0, 1);
    lcd.print("P: " + String(pressure) + "mbar");

    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}
