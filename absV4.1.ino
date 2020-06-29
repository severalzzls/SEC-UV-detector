// Codes for controlling and monitoring the home-built chromatography system
// Version 4.1
// Copyrights to Zhilun Grant Zhao
// 2019

#include <UTFT.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

RTC_DS3231 rtc;

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
// Set the pins to the correct ones for your development shield
// ------------------------------------------------------------
// Arduino Uno / 2009:
// -------------------
// Standard Arduino Uno/2009 shield            : <display model>,A5,A4,A3,A2
// DisplayModule Arduino Uno TFT shield        : <display model>,A5,A4,A3,A2
//
// Arduino Mega:
// -------------------
// Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
// CTE TFT LCD/SD Shield for Arduino Mega      : <display model>,38,39,40,41
//
// Remember to change the model parameter to suit your display module!
UTFT myGLCD(ITDB32S_V2,38,39,40,41);
int x=1;
int buf[320];
int buf2[177];
int readingBuf[10];
int readingBuf2[15];
int counter = 0;
int counterNum = 1;
int masterCounter = 1;
//int c = 1;
int y;
int y1;
uint32_t timeBuf = 1;
uint32_t iniTime = 1;

File myFile;
String fileName;


void setup()
{
  
// Setup the LCD
  myGLCD.InitLCD();  
  
  initializeScr();
  // Draw x label
  myGLCD.setFont(SmallFont);  
  for (int i =40; i<=320; i+=40) {
    myGLCD.printNumI(i/4,i-4,225);
  }

  myGLCD.setFont(SevenSegNumFont);  
  
  // initialize buffer arrays
  for (int i = 0; i <= 319; i++) {
    buf[i]=221;
  };

  for (int i = 0; i <= 176; i++) {
    buf2[i]=0;
  };
  
  for (int i = 0; i <= 14; i++) {
    readingBuf2[i]=221;
  };
  
  rtc.begin();
  DateTime now = rtc.now();
  timeBuf = now.unixtime();
  iniTime = now.unixtime();
  
  if (SD.begin(53))
  {
    myGLCD.setColor(251,251,251);
    myGLCD.setBackColor(255,255,255);
    myGLCD.setFont(SmallFont);
    myGLCD.print("SD card initialized", 155, 155);
    fileName = String(String(now.month()) + String(now.day()) + String(now.hour()) + String(now.minute()) + ".txt");
    if (SD.exists(fileName)) 
    {
      myGLCD.print("File name exists!!", 155, 175);
      myGLCD.print("File overwrote!!", 155, 190);
    } 
    myFile = SD.open(fileName, FILE_WRITE);    
    myGLCD.print(fileName, 155, 175);
    myFile.close();
    
    
  } else
  {
    myGLCD.setColor(251,251,251);
    myGLCD.setBackColor(255,255,255);
    myGLCD.setFont(SmallFont);
    myGLCD.print("SD card init. failed", 155, 155);
    return;
  }  
}

void loop()
{  
  x++;
  
  if (x==177)
    {
      x=2;    
    }
  if (x==1)
    {
      myGLCD.setColor(0,0,0);
    }
  else
    {
      myGLCD.setColor(255,255,255);          
      myGLCD.drawPixel(x+140,buf2[x-1]);// delete previous data point
    }  

  for (int i=0; i<=9; i++) {
    readingBuf[i] = analogRead(A0);  
  }
  float b = average(readingBuf,10);
  if (b>260.0)
    {
      b=260.0;
    }
  y = (float)(b/260)*56+3;
  
  myGLCD.setColor(0,0,0);
  myGLCD.setFont(SevenSegNumFont);
  myGLCD.printNumI((int) b,4,6);
  myGLCD.setColor(255,0,0);
  //float b=(sin(((c*1.1)*3.14)/180));
  //y=56*abs(b)+3;
  myGLCD.drawPixel(x+140,y);
  buf2[x-1]=y;

  DateTime now = rtc.now();
  

  if (now.unixtime() - timeBuf >= 1)
  {
    myGLCD.setFont(SmallFont);
    myGLCD.printNumI(now.unixtime()-iniTime,2,225);
    for (int i=0; i<=9; i++) {
      readingBuf[i] = analogRead(A0);        
    }
    
    float d = average(readingBuf,10);
    if (d>260)
      {
        d=260;
      }
    y1 = (float)(d/260)*150+71;
    readingBuf2[counter] = y1;
    timeBuf = now.unixtime();
    counter++;
  }
  
  if (counterNum == 320){
    initializeScr();
    // Draw x label
    myGLCD.setFont(SmallFont);  
    for (int i =40; i<=320; i+=40) {
      myGLCD.printNumI(i/4+40*masterCounter,i-4,225);
    }
    for (int i = 0; i < 160; i++) {
      buf[i] = buf[i+159];
    }
    for (int i = 0; i < 159; i++) {
      myGLCD.setColor(255,0,0);
      myGLCD.drawLine(i,buf[i],i+1,buf[i+1]);
    }
    masterCounter++;
    counterNum = 160;
  }

  
  if (counter == 15){
    counter = 0;    
    buf[counterNum]=getMin(readingBuf2,15);    
    myGLCD.drawLine(counterNum-1,buf[counterNum-1],counterNum,buf[counterNum]);       

    // record data
    myFile = SD.open(fileName, FILE_WRITE);
    if (myFile) {    
      myFile.print(now.unixtime()-iniTime);
      myFile.print(";");    
      myFile.println(260 - buf[counterNum]);
      myFile.close(); // close the file
    }    
    counterNum++;
  }

  
  //delay(10);
}

float average (int * array, int len)  // assuming array is int.
{
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  for (int i = 0 ; i < len ; i++)
    sum += array [i] ;
  return  ((float) sum) / len ;  // average will be fractional, so float may be appropriate.
}

int getMin(int * array, int len)
{
  int minNum = array[0];
  for (int i=0; i<len; i++)
    if (array[i] < minNum) {
      minNum = array[i];
    }
  return minNum;  
}

void initializeScr()
{
  // Draw background
  myGLCD.clrScr();
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRect(0, 0, 319, 239);
  
  // header
  myGLCD.setColor(0,0,0);
  myGLCD.drawRect(2,2,318,60);
 
  // credit
  myGLCD.setColor(251,251,251);
  myGLCD.setBackColor(255,255,255);
  myGLCD.setFont(SmallFont);
  myGLCD.print("@Zhilun G. Zhao 2020 Ver.4", 145, 205);
  
  // Draw crosshairs
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.drawLine(0, 224, 319, 224);    
  for (int i=1; i<=320; i+=20) {
    myGLCD.drawLine(i, 220, i, 224);    
  };
  
}
