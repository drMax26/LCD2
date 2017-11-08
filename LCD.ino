#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
/*****************
 
 */
#include <OneWire.h>
OneWire ds(3);

/*****************************************************************************************
 * Подключение LCD-экрана к Arduino Mega осуществляется по этой схеме:
+5V:  +5V
MISO:   Вывод 50 на Mega 2560 (Miso на ADK)
SCK:  Вывод 52 на Mega 2560 (Sck на ADK)
MOSI:   Вывод 51 на Mega 2560 (Mosi на ADK)
LCD CS:   вывод 10
SD CS:  вывод 4
D/C:  вывод 9
RESET:  вывод 8
BL:   +5V
GND:  GND
 */

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to -1!
#define TFT_DC     8
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

RTC_DS1307 RTC;

char humidity[] = {193,234,223,229,236,237,240,241,251, 58, 32};
char temperature[] = {209,228,235,238,228,239,223,241,242,239,223, 58, 32};

                              //Вс      Пн          Вт          Ср        Чт          Пт        Сб
byte DayOfWeekShort[7][2]{{193, 240},{206, 236},{193, 241},{208, 239},{214, 241},{206, 241},{208, 240}};      

byte Last_Minutes = 255;

byte Hour = 255;
byte Minute = 255;
byte Day = 255;
byte Month = 255;
int Year = 255;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello! Adafruit ST7735 rotation test");

  Wire.begin(); // Start the I2C
  RTC.begin();  // Init RTC
  //RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date is expanded to date and time on your computer at compiletime
  time.settime(0,51,21,27,10,15,2); 
  /*
  Serial.println(__DATE__);
  Serial.println(__TIME__);

Serial.print( F("Compiled: "));
Serial.print( F(__DATE__));
Serial.print( F(", "));
Serial.print( F(__TIME__));
Serial.print( F(", "));
Serial.println( F(__VERSION__));
  */
//  dht.setup(DHTPIN);

  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  // Use this initializer (uncomment) if you're using a 1.44" TFT
  //tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab

  // Use this initializer (uncomment) if you're using a 0.96" 180x60 TFT
  //tft.initR(INITR_MINI160x80);   // initialize a ST7735S chip, mini display

    //Serial.println("init");

    tft.setTextWrap(false); // Allow text to run off right edge
    tft.fillScreen(ST7735_BLACK);

    //Serial.println("This is a test of the rotation capabilities of the TFT library!");
    //Serial.println("Press <SEND> (or type a character) to advance");

    tft.setTextColor(ST7735_WHITE);
    
    tft.setTextSize(1);
    /*
    tft.setRotation(tft.getRotation()+1);
    */

    /**
    for (byte i = 0; i < 20; i++)
    {
        tft.setCursor(0, i * 8);
        tft.print(F2(text1));
    }
    /**/

}

void loop() 
{
  // put your main code here, to run repeatedly:
     ShowTime(8,0);

//     delay (20000);
}

void ShowTime(byte x, byte y)
{

    DateTime now = RTC.now();
    
    if (Last_Minutes == now.minute())
    {
        return;
    }
    Last_Minutes = now.minute();

    tft.fillRect(x, y, 128, 8, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);

    tft.setCursor(x, y);
    if (now.day() < 10)
      tft.print("0");
    tft.print(now.day(), DEC);
    tft.print('.');
    if (now.month() < 10)
      tft.print("0");
    tft.print(now.month(), DEC);
    tft.print('.');
    tft.print(now.year(), DEC);
    
    tft.print("    ");

    if (now.hour() < 10)
      tft.print("0");
    tft.print(now.hour(), DEC);
    tft.print(':');
    if (now.minute() < 10)
      tft.print("0");
    tft.print(now.minute(), DEC);

    byte dow = DayOfWeek(now.year(), now.month(), now.day());

    //Serial.print("dow = ");
    //Serial.println(dow);
    
    uint16_t color;
    switch (dow)
    {
        case 0 :  
              color = ST7735_RED;
        break;
       case 6 :  
              color = ST7735_RED;
        break;
        default:
            color = ST7735_GREEN;
        break;
    }

    tft.drawChar(x + 65, 0, DayOfWeekShort[dow][0], color, ST7735_BLACK, 1);
    tft.drawChar(x + 70, 0, DayOfWeekShort[dow][1], color, ST7735_BLACK, 1);

    ShowTemp18b20(0,39);
    
    ShowTemp(0,15);
    
}

byte DayOfWeek(int y, byte m, byte d) 
{   // y > 1752, 1 <= m <= 12
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
 
  y -= m < 3;
  return ((y + y/4 - y/100 + y/400 + t[m-1] + d) % 7); // 00 - 06, 01 = Sunday
}

void ShowTemp(byte x, byte y)
{
    /*
    float DHT_humidity = dht.getHumidity();
    float DHT_temperature = dht.getTemperature();

    tft.fillRect(x, y, 128, 16, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);

    tft.setCursor(x, y);

    for (byte i = 0; i < sizeof(humidity); i++)
      tft.print(humidity[i]);

    if ((DHT_humidity < 40)||(DHT_humidity > 70))
    {
        tft.setTextColor(ST7735_RED);
    }
    else
    {
        tft.setTextColor(ST7735_GREEN);
    }
    tft.print(DHT_humidity, 2);

    tft.setTextColor(ST7735_WHITE);
    tft.println("%");

    for (byte i = 0; i < sizeof(temperature); i++)
      tft.print(temperature[i]);
    
    if (DHT_temperature < 19)
    {
        tft.setTextColor(ST7735_BLUE);
    }
    else
        if (DHT_temperature > 23)
        {
            tft.setTextColor(ST7735_RED);
        }
        else
        {
            tft.setTextColor(ST7735_GREEN);
        }
    tft.print(DHT_temperature, 2);
    tft.setTextColor(ST7735_WHITE);
    tft.print(char(128));
    /**/
}

void ShowTemp18b20(byte x, byte y)
{
    tft.fillRect(x, y, 128, 16, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(x, y);
 
k:
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float DHT_temperature, fahrenheit;

    if ( !ds.search(addr)) 
    {
        //Serial.println("No more addresses.");
        //Serial.println();
        ds.reset_search();
        //delay(250);
        return;
    }

    //Serial.print("ROM =");
    for( i = 0; i < 8; i++) 
    {
        //Serial.write(' ');
        //Serial.print(addr[i], HEX);
    }

    if (OneWire::crc8(addr, 7) != addr[7]) 
    {
        //Serial.println("CRC is not valid!");
        return;
    }

    //Serial.println();

    // первый байт определяет чип
    switch (addr[0]) 
    {
        case 0x10:
            //Serial.println(" Chip = DS18S20"); // или более старый DS1820
            type_s = 1;
        break;

        case 0x28:
            //Serial.println(" Chip = DS18B20");
            type_s = 0;
        break;

        case 0x22:
            //Serial.println(" Chip = DS1822");
            type_s = 0;
        break;
      
        default:
            //Serial.println("Device is not a DS18x20 family device.");
            return;
      }

      ds.reset();
      ds.select(addr);

      ds.write(0x44); // начинаем преобразование, используя ds.write(0x44,1) с "паразитным" питанием
      delay(1000); // 750 может быть достаточно, а может быть и не хватит

      // мы могли бы использовать тут ds.depower(), но reset позаботится об этом
      present = ds.reset();
      ds.select(addr);
      ds.write(0xBE);
      //Serial.print(" Data = ");
      //Serial.print(present, HEX);
      //Serial.print(" ");
      
      for ( i = 0; i < 9; i++) 
      { // нам необходимо 9 байт
          data[i] = ds.read();
          //Serial.print(data[i], HEX);
          //Serial.print(" ");
      }
      //Serial.print(" CRC=");
      //Serial.print(OneWire::crc8(data, 8), HEX);
      //Serial.println();
      // конвертируем данный в фактическую температуру
      // так как результат является 16 битным целым, его надо хранить в
      // переменной с типом данных "int16_t", которая всегда равна 16 битам,
      // даже если мы проводим компиляцию на 32-х битном процессоре

      int16_t raw = (data[1] << 8) | data[0];
      if (type_s) 
      {
          raw = raw << 3; // разрешение 9 бит по умолчанию
          if (data[7] == 0x10) 
          {
              raw = (raw & 0xFFF0) + 12 - data[6];
          }
      }
      else 
      {
          byte cfg = (data[4] & 0x60);
          // при маленьких значениях, малые биты не определены, давайте их обнулим
          if (cfg == 0x00) 
              raw = raw & ~7; // разрешение 9 бит, 93.75 мс
          else 
              if (cfg == 0x20) 
                  raw = raw & ~3; // разрешение 10 бит, 187.5 мс
              else 
                  if (cfg == 0x40) 
                      raw = raw & ~1; // разрешение 11 бит, 375 мс

              //// разрешение по умолчанию равно 12 бит, время преобразования - 750 мс
        }
    DHT_temperature = (float)raw / 16.0;
//    fahrenheit = celsius * 1.8 + 32.0;
  
    //Serial.print(" Temperature = ");
    //Serial.print(celsius);
    //Serial.print(" Celsius, ");
    //Serial.print(fahrenheit);
    //Serial.println(" Fahrenheit");

    for (byte i = 0; i < sizeof(temperature); i++)
      tft.print(temperature[i]);
    
    
    if (DHT_temperature < 19)
    {
        tft.setTextColor(ST7735_BLUE);
    }
    else
        if (DHT_temperature > 23)
        {
            tft.setTextColor(ST7735_RED);
        }
        else
        {
            tft.setTextColor(ST7735_GREEN);
        }
    tft.print(DHT_temperature, 2);
    tft.setTextColor(ST7735_WHITE);
    tft.println(char(128));

    goto k;
}
