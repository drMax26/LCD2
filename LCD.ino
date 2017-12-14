#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"
/*****************
GND  GND
VCC   +5V
SDA   SDA
SCL   SCL
 */
#include <OneWire.h>
//датчики температуры
//к контакту сигнал 4,7кОм на +5В
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

bool Start = true;

#define LeftButton  4
#define UPButton  5
#define DownButton  6
#define RightButton  7

boolean BLeft     = true;
boolean BUP   = true;
boolean BDown   = true;
boolean BRight    = true;

unsigned long KeyboardDelay = 50;
unsigned long timeUp = 0;
unsigned long timeDown = 0;
unsigned long timeLeft = 0;
unsigned long timeRight = 0;

#define Heat1 22
#define Heat2 23
#define Heat3 24
#define Fun1 25
#define Fun2 26


bool Heat1State = false;
bool Heat2State = false;
bool Heat3State = false;

bool Fun1State = false;
bool Fun2State = false;

//                  В   л   а   ж   н   о   с   т   ь    :
char txtHumidity[] = {193,234,223,229,236,237,240,241,251, 58, 32};
//                      Т   е   м   п   е   р   а   т   у   р   а    :
char txtTemperature[] = {209,228,235,238,228,239,223,241,242,239,223, 58, 32};


//                Р    е    ж    и    м
//char tRegim = {207, 228, 229, 231, 235, 32};


byte AllRegim = 5;
char tRegim[][10] = {
//                    З    и    м    а        А    В    Т    О      
                {32, 198, 231, 235, 223, 32, 191, 193, 209, 205},
//                    З    и    м    а        П    О    C    Т   
                {32, 198, 231, 235, 223, 32, 206, 205, 208, 209},
//                    Л    е    т    о        А    В    Т    О   
                {32, 202, 228, 241, 237, 32, 191, 193, 209, 205},
//                    Л    е    т    о        П    О    C    Т   
                {32, 202, 228, 241, 237, 32, 206, 205, 208, 209},                                        
//                    Р    у    ч    н    о    й    
                {32, 207, 242, 246, 236, 237, 232, 32, 32, 32},                    
                    };

char tRegim2[][4] {
            {32, '$', '1', 32},
            {32, '$', '2', 32},
            {32, '$', '3', 32},
            {32, '*', '1', 32},
            {32, '*', '2', 32}
                  };
//Текущий режим в первом уровне
byte CurrentRegim = 1;
//Режим при переборе с клавиатуры в первом уровне
byte CurrentRegimKeyboard = 1;

//В каком режиме меню мы сейчас находимся
byte MenuLevel = 1;


//Режим при переборе с клавиатуры
byte CurrentRegimKeyboard2 = 1;

//Симовл для отображения перебора режимов с клавиатуры
char RegimShow[] = {16,17};

                              //Вс      Пн          Вт          Ср        Чт          Пт        Сб
byte DayOfWeekShort[7][2]{{193, 240},{206, 236},{193, 241},{208, 239},{214, 241},{206, 241},{208, 224}};      

//Дни в которые нужно включать
//                  ВС     ПН    ВТ    СР    ЧТ    ПТ     СБ
boolean WorkDay[7]{false, true, true, true, true, true, false};   



//Время пар
//                           1             2             3             4             5             6                 7                 8
int WorkTime[8][2] = {{ 510 , 590 },{ 600 , 680 },{ 710 , 790 },{ 800 , 880 },{ 890 , 970 },{ 980 , 1060  },{ 1070  , 1150  },{ 1160  , 1240  }};

//Время работы вентилятора после пары
byte FanWorkTime = 5;

//Поселнее обновление экрана
byte Last_Minutes = 255;

//Поселнее обновление температурі
byte LastSeconds = 80;
//Через сколько секунд обновлять температуру и производить включения и выключения нагревателей и вентиляторов
byte SecondsRefreshTemperature = 15;


float Temperatures[] = {0,0};

#define MinTemp  15.0
#define MaxTemp  20.0

void setup() 
{
    //Serial.begin(9600);
    //Serial.println("Hello! Adafruit ST7735 rotation test");

    Wire.begin(); // Start the I2C
    RTC.begin();  // Init RTC
    //RTC.adjust(DateTime(__DATE__, __TIME__));  // Time and date is expanded to date and time on your computer at compiletime
    
    // Use this initializer if you're using a 1.8" TFT
    tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  
    tft.setTextWrap(false); // Allow text to run off right edge
    tft.fillScreen(ST7735_BLACK);

    tft.setTextColor(ST7735_WHITE);
    
    tft.setTextSize(1);

    pinMode(LeftButton, INPUT); 
    pinMode(UPButton, INPUT); 
    pinMode(DownButton, INPUT); 
    pinMode(RightButton, INPUT);

    pinMode(Heat1, OUTPUT);
    pinMode(Heat2, OUTPUT);
    pinMode(Heat3, OUTPUT);
    
    pinMode(Fun1, OUTPUT);
    pinMode(Fun2, OUTPUT);

    digitalWrite(Heat1, LOW);
    digitalWrite(Heat2, LOW);
    digitalWrite(Heat3, LOW);

    digitalWrite(Fun1, LOW);
    digitalWrite(Fun2, LOW);
/*
    ShowMenu1();
    GetTemperatures();
    ShowTemperatures(0,30);
    ShowState();
/**/

    tft.setTextColor(ST7735_GREEN);
    tft.setTextSize(2);

    tft.setCursor(0, 64);
    tft.print("Starting...");

    tft.setTextSize(1);
}

void loop() 
{
    DateTime now = RTC.now();

    KeyboardWork();
    

    if (LastSeconds != (now.second() / SecondsRefreshTemperature))
    {
        //Serial.print("Now seconds = ");
        //Serial.print(now.second());
        //Serial.print(" ");
        //Serial.println(now.second() / SecondsRefreshTemperature);

      
        LastSeconds = (now.second() / SecondsRefreshTemperature);
        GetTemperatures();
        ShowTemperatures(0,30);
        WorkWithFunHeat(DayOfWeek(now.year(), now.month(), now.day()), now.hour() * 60 + now.minute());
        ShowState();
    }

    
    
    if (Last_Minutes == now.minute())
    {
        return;
    }
    
    Last_Minutes = now.minute();  

    ShowTime(8,0, now);
    if (Start)
      ShowMenu1();

    Start = false;
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

    byte dow = DayOfWeek(now.year(), now.month(), now.day());
    tft.setTextColor(ST7735_RED);
    if (WorkDay[dow])
    {
        int nowTimeMinutes = now.hour() * 60 + now.minute();
        for (byte i = 0; i < 8; i++)
        {
            if ((WorkTime[i][0] <= nowTimeMinutes) && (nowTimeMinutes <= WorkTime[i][1]))
            {
                tft.setTextColor(ST7735_GREEN);
            }
            else
              if ((WorkTime[i][0] <= nowTimeMinutes) && (nowTimeMinutes <= WorkTime[i][1] + FanWorkTime))
                  tft.setTextColor(ST7735_BLUE);
        }        
    }
    
    if (now.hour() < 10)
      tft.print("0");
    tft.print(now.hour(), DEC);
    tft.print(':');
    if (now.minute() < 10)
      tft.print("0");
    tft.print(now.minute(), DEC);

   

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
    tft.fillRect(0, 152, 128, 8, ST7735_BLACK);
    tft.setTextColor(ST7735_BLUE);

    tft.setCursor(2, 152);
    tft.print("Reading sensors");
    
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
        tft.fillRect(0, 152, 128, 8, ST7735_BLACK);
        return;
    }

    //Serial.print("ROM =");
    /*
    for( i = 0; i < 8; i++) 
    {
        //Serial.write(' ');
        //Serial.print(addr[i], HEX);
    }
    */
    //Serial.println("");

    if (OneWire::crc8(addr, 7) != addr[7]) 
    {
        tft.fillRect(0, 152, 128, 8, ST7735_BLACK);
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
    //Serial.println(index);
    index++; 

    goto k;

    //tft.fillRect(0, 152, 128, 8, ST7735_BLACK);
    tft.setTextColor(ST7735_GREEN);

    tft.setCursor(2, 152);
    tft.print("Reading sensors OK");
}





void ShowTemperatures(byte x, byte y)
{
    tft.fillRect(x, y, 128, 16, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor(x, y);
 

    for (byte TCount = 0; TCount < 2; TCount++)
    {
        for (byte i = 0; i < sizeof(txtTemperature); i++)
            tft.print(txtTemperature[i]);

        if (Temperatures[TCount] < MinTemp)
        {
            tft.setTextColor(ST7735_BLUE);
        }
        else
            if (Temperatures[TCount] > MaxTemp)
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
    tft.fillRect(0, 60, 128, 320, ST7735_BLACK);
    //tft.setTextColor(ST7735_WHITE);
    
    //Serial.print(sizeof(tRegim));
    for (byte Regim = 1; Regim <= AllRegim; Regim++)
    {
        tft.setCursor(0, 60 + (Regim - 1) * 9);
        tft.setTextColor(ST7735_WHITE);
        if (Regim == CurrentRegimKeyboard)
        {
            tft.print(RegimShow[0]);
        }
        else
        {
            tft.print(char(32));  
        }
        
        if (Regim == CurrentRegim)
            tft.setTextColor(ST7735_GREEN);
        else
            tft.setTextColor(ST7735_BLUE);

        for (byte i = 0; i < sizeof(tRegim[Regim - 1]); i++)
            tft.print(tRegim[Regim - 1][i]);

        //tft.print(Regim);

        tft.setTextColor(ST7735_WHITE);
        if (Regim == CurrentRegimKeyboard)
        {
            tft.print(RegimShow[1]);
        }
        
    }

/*
    tRegim[] = {208, 228, 229, 231, 235, 32};
    byte CurrentRegim
*/
}
//Обработка клавиш
void KeyboardWork()
{
  /*
#define LeftButton  4
#define UPButton  5
#define DownButton  6
#define RightButton  7

boolean BLeft     = true;
boolean BUP   = true;
boolean BDown   = true;
boolean BRight    = true;
*/
    boolean change = false; 
    if (HIGH == digitalRead(LeftButton))
        BLeft  = true;
    if (HIGH == digitalRead(UPButton))
        BUP  = true;
    if (HIGH == digitalRead(DownButton))
        BDown  = true;
    if (HIGH == digitalRead(RightButton))
        BRight  = true;

    if (1 == MenuLevel)
    {
        if (((BLeft)&&(LOW == digitalRead(LeftButton)))&&(KeyboardDelay < abs((millis() - timeLeft))))
        {
            //Serial.println("1 - Left");
        }

        if (((BUP)&&(LOW == digitalRead(UPButton)))&&(KeyboardDelay < abs((millis() - timeUp))))
        {
            //Serial.println("1 - Up");
            timeUp = millis();
            BUP = false;
            CurrentRegimKeyboard--;
            if (1 > CurrentRegimKeyboard)
              CurrentRegimKeyboard = AllRegim;
            change = true;
        }

        if (((BDown)&&(LOW == digitalRead(DownButton)))&&(KeyboardDelay < abs((millis() - timeDown))))
        {
            //Serial.println("1 - Down");
            timeDown = millis();
            BDown = false;
            CurrentRegimKeyboard++;
            if (AllRegim < CurrentRegimKeyboard)
              CurrentRegimKeyboard = 1;
            change = true;
        }

        if (((BRight)&&(LOW == digitalRead(RightButton)))&&(KeyboardDelay < abs((millis() - timeRight))))
        {
            /**/
            //Serial.println("1 - Right");
            timeRight = millis();
            BRight = false;
            CurrentRegim =  CurrentRegimKeyboard;
            if (5 == CurrentRegim)
            {
                MenuLevel = 2;
                CurrentRegimKeyboard2 = 1;
                ShowMenu2();
            }
            else
                change = true;
            LastSeconds = 80;
            /**/
        }
        if (change)
            ShowMenu1();
         
    }
    else
    {
        if (((BLeft)&&(LOW == digitalRead(LeftButton)))&&(KeyboardDelay < abs((millis() - timeLeft))))
        {
            //Serial.println("2 - Left");
            timeLeft = millis();
            BLeft = false;
            MenuLevel = 1;
            ShowMenu1();
        }

        if (((BUP)&&(LOW == digitalRead(UPButton)))&&(KeyboardDelay < abs((millis() - timeUp))))
        {
            //Serial.println("2 - Up");
            timeUp = millis();
            BUP = false;
            CurrentRegimKeyboard2--;
            if (1 > CurrentRegimKeyboard2)
              CurrentRegimKeyboard2 = 5;
            change = true;
        }

        if (((BDown)&&(LOW == digitalRead(DownButton)))&&(KeyboardDelay < abs((millis() - timeDown))))
        {
            //Serial.println("2 - Down");
            timeDown = millis();
            BDown = false;
            CurrentRegimKeyboard2++;
            if (5 < CurrentRegimKeyboard2)
              CurrentRegimKeyboard2 = 1;
            change = true;
        }


        if (((BRight)&&(LOW == digitalRead(RightButton)))&&(KeyboardDelay < abs((millis() - timeRight))))
        {
            /**/
            //Serial.println("2 - Right");
            timeRight = millis();
            BRight = false;

            switch (CurrentRegimKeyboard2)
            {
                case (1) :
                    Heat1State = !Heat1State;
                break;

                case (2) :
                    Heat2State = !Heat2State;
                break;

                case (3) :
                    Heat3State = !Heat3State;
                break;

                case (4) :
                    Fun1State = !Fun1State;
                break;

                case (5) :
                    Fun2State = !Fun2State;
                break;
            } 
  
            change = true;
            /**/
        }
        
        if (change)
        {
            ShowMenu2();
        }
    }

     
    //ShowMenu1();
    //ButtonState;
}


void ShowState()
{
    tft.setCursor(6, 16);

    if (Heat1State)
        tft.setTextColor(ST7735_GREEN);
    else
        tft.setTextColor(ST7735_RED);
   tft.print("$1 ");

   if (Heat2State)
        tft.setTextColor(ST7735_GREEN);
    else
        tft.setTextColor(ST7735_RED);
   tft.print("$2 ");

   if (Heat3State)
        tft.setTextColor(ST7735_GREEN);
    else
        tft.setTextColor(ST7735_RED);
   tft.print("$3 ");
    
   if (Fun1State)
        tft.setTextColor(ST7735_GREEN);
    else
        tft.setTextColor(ST7735_RED);
   tft.print("*1 ");

   if (Fun2State)
        tft.setTextColor(ST7735_GREEN);
    else
        tft.setTextColor(ST7735_RED);
   tft.print("*2 ");
}

void WorkWithFunHeat(byte _dow, int _nowTimeMinutes)
{
/*
    digitalWrite(Heat1, LOW);
    digitalWrite(Heat2, LOW);
    digitalWrite(Heat3, LOW);

    digitalWrite(Fun1, LOW);
    digitalWrite(Fun2, LOW);
 */

/*
    Heat1State = false;
    Heat2State = false;
    Heat3State = false;

    Fun1State = false;
    Fun2State = false;
 */
    bool AllStop = true;
 
    switch (CurrentRegim)
    {
        case (1) :
                  if (WorkDay[_dow])
                  {
                      for (byte i = 0; i < 8; i++)
                      {
                          if ((WorkTime[i][0] <= _nowTimeMinutes) && (_nowTimeMinutes <= WorkTime[i][1] + FanWorkTime))
                          {
                                AllStop = false;
                                Heat1State = true;
                                Fun1State = true;
                                Fun2State = false;
                                if (_nowTimeMinutes >= WorkTime[i][1])
                                  Fun2State = true;

                                if (Temperatures[0] < MinTemp)
                                    if (!Heat2State)
                                        Heat2State = true;
                                    else
                                        Heat3State = true;

                                if (Temperatures[0] > MaxTemp)
                                    if (Heat3State)
                                        Heat3State = false;
                                    else
                                        Heat2State = false;
                          }
                      }        
                  }
                  else
                  {
                       AllStop = false;
                  }
        break;


        case (2) :
                  AllStop = false;
        
                  Heat1State = true;
                  Heat2State = true;
                  Heat3State = true;
                                
                  Fun1State = true;
                  Fun2State = false;
        break;

        case (3) :
                  Heat1State = false;
                  Heat2State = false;
                  Heat3State = false;

                  Fun1State = false;
                  Fun2State = false;
                  
                  if (WorkDay[_dow])
                  {
                      for (byte i = 0; i < 8; i++)
                      {
                          if ((WorkTime[i][0] <= _nowTimeMinutes) && (_nowTimeMinutes <= WorkTime[i][1] + FanWorkTime))
                          {
                                AllStop = false;
                                Fun1State = true;
                                if (_nowTimeMinutes >= WorkTime[i][1])
                                  Fun2State = true;
                          }
                      }        
                  }
                  else
                  {
                       AllStop = false;
                  }
        break;

        case (4) :
                  AllStop = false;
        
                  Heat1State = false;
                  Heat2State = false;
                  Heat3State = false;
                  
                  Fun1State = true;
                  Fun2State = false;
        break;

        case (5) :
                AllStop = false;
        break;
    }

    if (AllStop)
    {
        Heat1State = false;
        Heat2State = false;
        Heat3State = false;

        Fun1State = false;
        Fun2State = false;
    }

    if (Heat1State)
        digitalWrite(Heat1, HIGH);
    else
        digitalWrite(Heat1, LOW);       

   if (Heat2State)
        digitalWrite(Heat2, HIGH);
    else
        digitalWrite(Heat2, LOW);

    if (Heat3State)
        digitalWrite(Heat3, HIGH);
    else
        digitalWrite(Heat3, LOW);

    if (Fun1State)
        digitalWrite(Fun1, HIGH);
    else
        digitalWrite(Fun1, LOW);

    if (Fun2State)
        digitalWrite(Fun2, HIGH);
    else
        digitalWrite(Fun2, LOW);

}

void ShowMenu2()
{
    tft.fillRect(0, 60, 128, 320, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);
    
    /**/
    //Serial.print(sizeof(tRegim));
    for (byte Regim = 1; Regim <= 5; Regim++)
    {
        tft.setCursor(0, 64 + (Regim - 1) * 9);
        tft.setTextColor(ST7735_WHITE);
        if (Regim == CurrentRegimKeyboard2)
        {
            tft.print(RegimShow[0]);
        }
        else
        {
            tft.print(char(32));  
        }

        switch (Regim)
        {
            case (1) :
                    if (Heat1State)
                         tft.setTextColor(ST7735_GREEN);                           
                    else
                        tft.setTextColor(ST7735_RED);
            break;

            case (2) :
                    if (Heat2State)
                         tft.setTextColor(ST7735_GREEN);                           
                    else
                        tft.setTextColor(ST7735_RED);
            break;

            case (3) :
                    if (Heat3State)
                         tft.setTextColor(ST7735_GREEN);                           
                    else
                        tft.setTextColor(ST7735_RED);
            break;

            case (4) :
                    if (Fun1State)
                         tft.setTextColor(ST7735_GREEN);                           
                    else
                        tft.setTextColor(ST7735_RED);
            break;

            case (5) :
                    if (Fun2State)
                         tft.setTextColor(ST7735_GREEN);                           
                    else
                        tft.setTextColor(ST7735_RED);
            break;
        }
        for (byte i = 0; i < sizeof(tRegim2[Regim - 1]); i++)
            tft.print(tRegim2[Regim - 1][i]);

        tft.setTextColor(ST7735_WHITE);
        if (Regim == CurrentRegimKeyboard2)
        {
            tft.print(RegimShow[1]);
        }
        
    }
  /**/
}
