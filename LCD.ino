#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
/*****************
GND  GND
VCC   +5V
SDA   A4
SCL   A5 
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
//                  В   л   а   ж   н   о   с   т   ь    :
char humidity[] = {193,234,223,229,236,237,240,241,251, 58, 32};
//                     Т   е   м   п   е   р   а   т   у   р   а    :
char temperature[] = {209,228,235,238,228,239,223,241,242,239,223, 58, 32};
//                Р    е    ж    и    м
char tRegim[] = {207, 228, 229, 231, 235, 32};

byte CurrentRegim = 1;

                              //Вс      Пн          Вт          Ср        Чт          Пт        Сб
byte DayOfWeekShort[7][2]{{193, 240},{206, 236},{193, 241},{208, 239},{214, 241},{206, 241},{208, 224}};      

byte Last_Minutes = 255;

byte Hour = 255;
byte Minute = 255;
byte Day = 255;
byte Month = 255;
int Year = 255;

float Temperatures[] = {0,0};

void setup() 
{
    Serial.begin(9600);
    Serial.println("Hello! Adafruit ST7735 rotation test");

    Wire.begin(); // Start the I2C
    RTC.begin();  // Init RTC
    RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date is expanded to date and time on your computer at compiletime
    
    // Use this initializer if you're using a 1.8" TFT
    tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  
    tft.setTextWrap(false); // Allow text to run off right edge
    tft.fillScreen(ST7735_BLACK);

    tft.setTextColor(ST7735_WHITE);
    
    tft.setTextSize(1);

    //GetTemperatures();
}

void loop() 
{
    DateTime now = RTC.now();

    ShowMenu1();
    
    if (Last_Minutes == now.minute())
    {
        return;
    }
    
    Last_Minutes = now.minute();  

    GetTemperatures();
    ShowTemperatures(0,15);
    ShowTime(8,0, now);

    delay (5000);

}

void ShowTime(byte x, byte y, DateTime now)
{
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
/*
    ShowTemp18b20(0,39);
    ShowTemp(0,15);
*/
    
}


byte DayOfWeek(int y, byte m, byte d) 
{   // y > 1752, 1 <= m <= 12
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
 
  y -= m < 3;
  return ((y + y/4 - y/100 + y/400 + t[m-1] + d) % 7); // 00 - 06, 01 = Sunday
}


void GetTemperatures()
{
    byte index = 0;
k:
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float DHT_temperature, fahrenheit;

    if ( !ds.search(addr)) 
    {
         ds.reset_search();
        return;
    }

    Serial.print("ROM =");
    for( i = 0; i < 8; i++) 
    {
        Serial.write(' ');
        Serial.print(addr[i], HEX);
    }
    Serial.println("");

    if (OneWire::crc8(addr, 7) != addr[7]) 
    {
        return;
    }

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
        
    Temperatures[index] = (float)raw / 16.0;
    Serial.println(index);
    index++; 

    goto k;
}





void ShowTemperatures(byte x, byte y)
{
    tft.fillRect(x, y, 128, 16, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(x, y);
 

    for (byte TCount = 0; TCount < 2; TCount++)
    {
        for (byte i = 0; i < sizeof(temperature); i++)
            tft.print(temperature[i]);

        if (Temperatures[TCount] < 19)
        {
            tft.setTextColor(ST7735_BLUE);
        }
        else
            if (Temperatures[TCount] > 23)
            {
                tft.setTextColor(ST7735_RED);
            }
            else
            {
                tft.setTextColor(ST7735_GREEN);
            }
        tft.print(Temperatures[TCount], 1);
        tft.setTextColor(ST7735_WHITE);
        tft.println(char(128));
    }
}


void ShowMenu1()
{
    //tft.fillRect(0, 32, 128, 320, ST7735_GREEN);
    //tft.setTextColor(ST7735_WHITE);
    

    for (byte Regim = 1; Regim <= 5; Regim++)
    {
        tft.setCursor(0, 48 + (Regim - 1) * 9);    
        if (Regim == CurrentRegim)
            tft.setTextColor(ST7735_GREEN);
        else
            tft.setTextColor(ST7735_BLUE);

        for (byte i = 0; i < sizeof(tRegim); i++)
            tft.print(tRegim[i]);

        tft.print(Regim);
        
    }

/*
    tRegim[] = {208, 228, 229, 231, 235, 32};
    byte CurrentRegim
*/
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
