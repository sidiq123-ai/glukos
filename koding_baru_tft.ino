#include "logo.h"
#include <JPEGDecoder.h>   // JPEG decoder library
#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

#include <Effortless_SPIFFS.h>

eSPIFFS fileSystem;

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData2"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL true

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

// Create 4 key for set Date screen
char setDateKeyLabel[4][12] = {"<", ">", "Lanjut", "Kembali"};
uint16_t setDateKeyColor = TFT_RED;
// Invoke the TFT_eSPI button class and create button object
TFT_eSPI_Button setDateKey[4];

// Create 2 key for main menu screen
char mainKeyLabel[2][12] = {"Pengukuran", "Data"};
uint16_t mainKeyColor = TFT_RED;
// Invoke the TFT_eSPI button class and create button object
TFT_eSPI_Button mainKey[2];

// Create 6 key for set screen
char setKeyLabel[6][12] = {"<", ">", "Lanjut", "Kembali", "Pria", "Wanita"};
uint16_t setKeyColor = TFT_BLUE;
// Invoke the TFT_eSPI button class and create button object
TFT_eSPI_Button setKey[6];

// Create 2 key for measurement screen
char measKeyLabel[2][12] = {"Ukur", "Kembali"};
uint16_t measKeyColor[2] = {TFT_BLUE, TFT_RED};
// Invoke the TFT_eSPI button class and create button object
TFT_eSPI_Button measKey[2];

// Create 3 key for data screen
char dataKeyLabel[3][12] = {"<", ">", "Kembali"};
uint16_t dataKeyColor = TFT_BLUE;
// Invoke the TFT_eSPI button class and create button object
TFT_eSPI_Button dataKey[3];

// Keypad start position, key sizes and spacing
#define KEY_X 18 // Centre of key
#define KEY_Y 107
#define KEY_W 30 // Width and height
#define KEY_H 30
#define KEY_SPACING_X 4 // X and Y gap
#define KEY_SPACING_Y 17
#define KEY_TEXTSIZE 1   // Font size multiplier

// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 32
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Number length, buffer for storing it and character index
#define NUM_LEN 9
char nameBuffer[NUM_LEN + 1] = "";
uint8_t nameIndex = 0;

char keyLabel[30][8] = {"A", "B", "C", "D", "E", "F", "G",
                        "H", "I", "J", "K", "L", "M", "N", 
                        "O", "P", "Q", "R", "S", "T", "U", 
                        "V", "W", "X", "Y", "Z", "_", "<", 
                        "Kembali", "Lanjut" };
uint16_t keyColor = TFT_BLUE;
TFT_eSPI_Button key[30];

int pageIndex = -3;
uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
bool pressed;
int dayDate = 1;
int monthDate = 1;
int yearDate = 2022;
int patientAge = 0;
int patientGenderSelected = 0;
int dataNumber = 1;
int dataShow = 1;

int adcsensor;
float fix;
int datafix;
int cacah;
float kalibrasi;
float guladarah;
float kolesterol;

char fileName[9] = "/data";
char patientData[18];

boolean newData = false;
const byte numChars = 10;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

char sendTreatBuf[50];

void setup() {
    // Initialise the TFT screen
  tft.init();

  tft.setSwapBytes(true);

  // Set the rotation before we calibrate
  tft.setRotation(2);

  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();

  // Draw set date menu
  drawSetDateMenu(1);
  
  Serial.begin(115200);
  pinMode(26,OUTPUT);
  digitalWrite(26,HIGH);

}

void loop() {
    screenRoutine();
/*    datafix = 0;
    fix = 0;
    adcsensor = analogRead(32);
    datafix = adcsensor + datafix;
    fix = datafix / 20.0;
   // guladarah = (fix - 1022.1) / 0.0967;
   // guladarah = 1.0065*(adcsensor+69)+12.762;
  //  if (guladarah < 0) guladarah = 0;
  //if (GLU >200) GLU = 200;

    //kolesterol = (fix - 896.1) / 0.0542;
   // kolesterol = 3.4232*(adcsensor+143)+392.11; 
   // if (kolesterol < 0) kolesterol = 0;
    // if (kolesterol > 300) kolesterol = 300;

   /* Serial.print("Glukosa : ");
    Serial.println(guladarah);
    Serial.println("kolesterol : ");
    Serial.println(kolesterol);
    Serial.println(adcsensor);
    delay(500);*/
}

void screenRoutine(){
  pressed = tft.getTouch(&t_x, &t_y);
  
  switch (pageIndex) {
    case -3:
      //set day date menu
      setDateScreenRoutine(1);
      break;
    case -2:
      //set month date menu
      setDateScreenRoutine(2);
      break;
    case -1:
      //set year date menu
      setDateScreenRoutine(3);
      break;
    case 0:
      //main menu
      mainScreenRoutine();
      break;
    case 1:
      //key menu
      keypadScreenRoutine();
      break;
    case 2:
      //set menu
      setScreenRoutine();
      break;
    case 3:
      //meas menu
      measScreenRoutine();
      break;
    case 4:
      //data menu
      dataScreenRoutine();
      break;
  }
}

void setDateScreenRoutine(int setDate){
  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 4; b++) {
    if (pressed && setDateKey[b].contains(t_x, t_y)) {
      setDateKey[b].press(true);  // tell the button it is pressed
    } else {
      setDateKey[b].press(false);  // tell the button it is NOT pressed
    }
  }

  tft.setFreeFont(LABEL1_FONT);

  // Check if any current key has changed state
  for (uint8_t b = 0; b < 4; b++) {
    if (setDateKey[b].justPressed()) {
      setDateKey[b].drawButton(true);
    }
  }
  for (uint8_t b = 0; b < 4; b++) {
    if (setDateKey[b].justReleased()) {
      setDateKey[b].drawButton(false);
      if(b == 0){
        if(setDate == 1){
          if(dayDate > 1){
            dayDate--;
          }
        }else if(setDate == 2){
          if(monthDate > 1){
            monthDate--;
          }
        }else if(setDate == 3){
          if(yearDate > 2022){
            yearDate--;
          }
        }
        drawSetDateMenuNow(setDate);
      }else if(b == 1){
        if(setDate == 1){
          if(dayDate < 31){
            dayDate++;
          }
        }else if(setDate == 2){
          if(monthDate < 12){
            monthDate++;
          }
        }else if(setDate == 3){
          if(yearDate < 2050){
            yearDate++;
          }
        }
        drawSetDateMenuNow(setDate);
      }else if(b == 2){
        if(setDate == 1){
          pageIndex = -2; // to the set month menu
          drawSetDateMenu(2);
        }else if(setDate == 2){
          pageIndex = -1; // to the set year menu
          drawSetDateMenu(3);
        }else if(setDate == 3){
          pageIndex = 0; // to the main menu
          drawMainMenu();
        }
      }else if(b == 3){
        pageIndex = -3; // to the set day menu
        drawSetDateMenu(1);
      }
    }
  } 
}

void mainScreenRoutine(){
  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 2; b++) {
    if (pressed && mainKey[b].contains(t_x, t_y)) {
      mainKey[b].press(true);  // tell the button it is pressed
    } else {
      mainKey[b].press(false);  // tell the button it is NOT pressed
    }
  }

  tft.setFreeFont(LABEL1_FONT);

  // Check if any current key has changed state
  for (uint8_t b = 0; b < 2; b++) {
    if (mainKey[b].justPressed()) {
      mainKey[b].drawButton(true);
    }
  }
  for (uint8_t b = 0; b < 2; b++) {
    if (mainKey[b].justReleased()) {
      mainKey[b].drawButton(false);
      if(b == 0){
        pageIndex = 1; // to the key menu
        nameIndex = 0; // Reset index to 0
        nameBuffer[nameIndex] = 0; // Place null in buffer
        patientAge = 0;
        patientGenderSelected = 0;
        guladarah = 0;
        kolesterol = 0;
        drawKeypad();
      }else if(b == 1){
        pageIndex = 4; // to the data menu
        dataShow = 1;
        drawDataMenu();
      }
    }
  }

}

  void keypadScreenRoutine(){
  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 30; b++) {
    if (pressed && key[b].contains(t_x, t_y)) {
      key[b].press(true);  // tell the button it is pressed
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  tft.setFreeFont(LABEL2_FONT);

  // Check if any current key has changed state
  for (uint8_t b = 0; b < 30; b++) {
    if (key[b].justPressed()) {
      key[b].drawButton(true);
    }
  }
  for (uint8_t b = 0; b < 30; b++) {
    if (key[b].justReleased()) {
      key[b].drawButton(false);

      if(b < 27){
        if (nameIndex < NUM_LEN) {
          nameBuffer[nameIndex] = keyLabel[b][0];
          nameIndex++;
          nameBuffer[nameIndex] = 0; // zero terminate
        }
        drawKeypadNow();
      }else if(b == 27){
        nameBuffer[nameIndex] = 0;
        if (nameIndex > 0) {
          nameIndex--;
          nameBuffer[nameIndex] = 0;//' ';
        }
        drawKeypadNow();
      }else if(b == 28){
        pageIndex = 0; // to the main menu
        nameIndex = 0; // Reset index to 0
        nameBuffer[nameIndex] = 0; // Place null in buffer
        drawMainMenu();
      }else{
        pageIndex = 2; // to the key menu
        drawSetMenu();
      }
    }
  }
}

void setScreenRoutine(){
  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 6; b++) {
    if (pressed && setKey[b].contains(t_x, t_y)) {
      setKey[b].press(true);  // tell the button it is pressed
    } else {
      setKey[b].press(false);  // tell the button it is NOT pressed
    }
  }

  tft.setFreeFont(LABEL1_FONT);

  // Check if any current key has changed state
  for (uint8_t b = 0; b < 4; b++) {
    if (setKey[b].justPressed()) {
      setKey[b].drawButton(true);
    }
  }
  for (uint8_t b = 0; b < 4; b++) {
    if (setKey[b].justReleased()) {
      setKey[b].drawButton(false);
      if(b == 0){
        if(patientAge > 0){
          patientAge--;
        }
        drawSetMenuNow();
      }else if(b == 1){
        if(patientAge < 100){
          patientAge++;
        }
        drawSetMenuNow();
      }else if(b == 2){
        pageIndex = 3; // to the meas menu
        if(patientGenderSelected == 0){
          patientGenderSelected = 1;
        }
        drawMeasMenu();
      }else if(b == 3){
        pageIndex = 0; // to the main menu
        drawMainMenu();
      }
    }
  }

    for (uint8_t b = 4; b < 6; b++) {
    if (setKey[b].justReleased()) {
      setKey[4].drawButton(b == 4);
      setKey[5].drawButton(b == 5);
      if(b == 4){
        patientGenderSelected = 1;
      }else if(b == 5){
        patientGenderSelected = 2;
      }
    }
  }
  
}

void measScreenRoutine(){
  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 2; b++) {
    if (pressed && measKey[b].contains(t_x, t_y)) {
      measKey[b].press(true);  // tell the button it is pressed
    } else {
      measKey[b].press(false);  // tell the button it is NOT pressed
    }
  }

  tft.setFreeFont(LABEL1_FONT);

  // Check if any current key has changed state
  for (uint8_t b = 0; b < 2; b++) {
    if (measKey[b].justPressed()) {
      measKey[b].drawButton(true);
    }
  }
  for (uint8_t b = 0; b < 2; b++) {
    if (measKey[b].justReleased()) {
      measKey[b].drawButton(false);
      if(b == 0){
        //measure
        drawWaitScreen();
        readgluchol();
        drawMeasMenu();
        dataNumber++;
        if(dataNumber > 100){
          dataNumber = 1;
        }
        saveThisData(dataNumber);
        saveDataIndex();
      }else if(b == 1){
        pageIndex = 0; // to the main menu
        drawMainMenu();
      }
    }
  }
}

void dataScreenRoutine(){
  // Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 3; b++) {
    if (pressed && dataKey[b].contains(t_x, t_y)) {
      dataKey[b].press(true);  // tell the button it is pressed
    } else {
      dataKey[b].press(false);  // tell the button it is NOT pressed
    }
  }

  tft.setFreeFont(LABEL1_FONT);

  // Check if any current key has changed state
  for (uint8_t b = 0; b < 3; b++) {
    if (dataKey[b].justPressed()) {
      dataKey[b].drawButton(true);
    }
  }
  for (uint8_t b = 0; b < 3; b++) {
    if (dataKey[b].justReleased()) {
      dataKey[b].drawButton(false);
      if(b == 0){
        if(dataShow > 1){
          dataShow--;
        }else{
          dataShow = 100;
        }
        drawDataStatNow();
      }else if(b == 1){
        if(dataShow < 100){
          dataShow++;
        }else{
          dataShow = 1;
        }
        drawDataStatNow();
      }else if(b == 2){
        pageIndex = -3; // to the set day menu
        drawSetDateMenu(1);
      }
    }
  }
  
}

void createFileName(char* fileN, int num){
  if(num > 0 && num <=100){
    sprintf(fileN, "/data%d", num);
  }else{
    sprintf(fileN, "/data%d", 404);
  }
}

void combineData(char* result, char* nameB, uint8_t patAge, uint8_t patGen, 
                uint8_t dayDat, uint8_t monthDat, uint8_t yearDat){
  sprintf(result, "%-9s%c%c%c%c%c%c%c%c", nameB, patAge, patGen, dayDat, monthDat, yearDat, 0);
}

void parseData(char *source, char *nameB, int *patAge, int *patGen, 
                int *dayDat, int *monthDat, int *yearDat){
  strncpy(nameB, source, 9);
  *patAge = source[9];
  *patGen = source[10];
  *dayDat = source[11];
  *monthDat = source[12];
  *yearDat = source[13];
//  *hbFront = source[14];
//  *hbBack = source[15];
//  *guladarah = source[14];
//  *kolesterol = source[15];
}

void saveDataIndex(){
  fileSystem.saveToFile("/dataIndex", dataNumber);
}

void getDataIndex(){
  fileSystem.openFromFile("/dataIndex", dataNumber);
}

void saveThisData(int dataNum){
  int yearDateConv = yearDate-2021;
  combineData(patientData, nameBuffer, patientAge, patientGenderSelected, dayDate, monthDate, yearDateConv);
  createFileName(fileName,dataNum);
  fileSystem.saveToFile(fileName, patientData);
}

void getThisData(int dataNum){
  const char* tempBuffer;
  createFileName(fileName,dataNum);
  fileSystem.openFromFile(fileName, tempBuffer);
  strncpy(patientData, tempBuffer, 18);
  int yearDateConv = 0;
  parseData(patientData, nameBuffer, &patientAge, &patientGenderSelected, &dayDate, &monthDate, &yearDateConv);
  yearDate = yearDateConv + 2021;
//  hb = (double)hbFront + (double)hbBack/100.0;
}

void readgluchol(){
  adcsensor = analogRead(32);
  guladarah = 1.0065*(adcsensor+69)+12.762;
  if (guladarah < 0) guladarah = 0;

  kolesterol = 3.4232*(adcsensor+143)+392.11; 
  if (kolesterol < 0) kolesterol = 0;
  if (kolesterol > 300) kolesterol = 300;
}

void drawBox()
{
  tft.fillScreen(TFT_BLACK);
  tft.fillRoundRect(0, 0, 240, 320, 10, TFT_WHITE);
  tft.fillRoundRect(5, 5, 230, 310, 10, TFT_DARKGREY);
  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setFreeFont(LABEL2_FONT);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawString("Smart Glukol", 120, 10, 1);
}

void drawSetDateMenu(int setDate)
{
  drawBox();

  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextDatum(TC_DATUM);
  
  if(setDate == 1){
    tft.drawString("Tanggal :", 120, 100);
  }else if(setDate == 2){
    tft.drawString("Bulan :", 120, 100);
  }else if(setDate == 3){
    tft.drawString("Tahun :", 120, 100);
  }

  // Draw the key
  tft.setFreeFont(LABEL1_FONT);
  for (uint8_t row = 0; row < 2; row++) {
    setDateKey[row].initButton(&tft, 30 + row * (40 + 140), // x, y, w, h, outline, fill, text
                        150,
                        40, 35, TFT_WHITE, setDateKeyColor, 
                        TFT_WHITE, setDateKeyLabel[row], 1);
    setDateKey[row].drawButton(false, setDateKeyLabel[row]);
  }
  for (uint8_t row = 0; row < 2; row++) {
    setDateKey[row+2].initButton(&tft, 120, // x, y, w, h, outline, fill, text
                        240 + row * (35 + 10),
                        160, 35, TFT_WHITE, setDateKeyColor, 
                        TFT_WHITE, setDateKeyLabel[row+2], 1);
    setDateKey[row+2].drawButton(false, setDateKeyLabel[row+2]);
  }

  drawSetDateMenuNow(setDate);
}

void drawSetDateMenuNow(int setDate)
{
  char charBuffer[10];

  tft.fillRect(70, 140, 100, 30, TFT_DARKGREY);
  
  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextSize(1);

  tft.setTextDatum(TC_DATUM);
  
  if(setDate == 1){
    sprintf(charBuffer, "%d", dayDate);
  }else if(setDate == 2){
    sprintf(charBuffer, "%d", monthDate);
  }else if(setDate == 3){
    sprintf(charBuffer, "%d", yearDate);
  }
  
  tft.drawString(charBuffer, 120, 140);
}

void drawMainMenu()
{
  drawBox();
 // drawArrayJpeg(logoImage, sizeof(logoImage), 34, 40); //wxh 172x167

  tft.pushImage(36,42,165,165,logopena);
  // Draw the key
  tft.setFreeFont(LABEL1_FONT);
  for (uint8_t row = 0; row < 2; row++) {
    mainKey[row].initButton(&tft, 120, // x, y, w, h, outline, fill, text
                        240 + row * (35 + 10),
                        160, 35, TFT_WHITE, mainKeyColor, 
                        TFT_WHITE, mainKeyLabel[row], 1);
    mainKey[row].drawButton(false, mainKeyLabel[row]);
  }
}

void drawKeypad(){
  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  // Draw keypad background
  tft.fillRect(0, 0, 240, 320, TFT_DARKGREY);

  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Nama :", 5, 5);

  // Draw number display area and frame
  tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
  tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);

  // Draw the keys
  for (uint8_t row = 0; row < 4; row++) {
    for (uint8_t col = 0; col < 7; col++) {
      uint8_t b = col + row * 7;

      tft.setFreeFont(LABEL2_FONT);

      if(b < 27){
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                          KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                          KEY_W, KEY_H, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                          keyLabel[b], KEY_TEXTSIZE);
        key[b].drawButton();
      }else{
        key[b].initButton(&tft, KEY_X + col * (KEY_W + KEY_SPACING_X),
                        KEY_Y + row * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        KEY_W, KEY_H, TFT_WHITE, TFT_RED, TFT_WHITE,
                        keyLabel[b], KEY_TEXTSIZE);
        key[b].drawButton();
      }
    }
  }
  key[28].initButton(&tft, 60,
                        KEY_Y + 4 * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        110, KEY_H, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                        keyLabel[28], KEY_TEXTSIZE);
  key[28].drawButton();
  key[29].initButton(&tft, 180,
                        KEY_Y + 4 * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                        110, KEY_H, TFT_WHITE, TFT_BLUE, TFT_WHITE,
                        keyLabel[29], KEY_TEXTSIZE);
  key[29].drawButton();
}

void drawKeypadNow(){
  tft.setTextDatum(TL_DATUM);        // Use top left corner as text coord datum
  tft.setFreeFont(&FreeSans18pt7b);  // Choose a nicefont that fits box
  tft.setTextColor(DISP_TCOLOR);     // Set the font colour

  // Draw the string, the value returned is the width in pixels
  int xwidth = tft.drawString(nameBuffer, DISP_X + 4, DISP_Y + 12);

  // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
  // but it will not work with italic or oblique fonts due to character overlap.
  tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);
}

void drawSetMenu()
{
  drawBox();

  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Usia (tahun) :", 120, 60);
  tft.drawString("Gender :", 120, 140);

  // Draw the key
  tft.setFreeFont(LABEL1_FONT);
  for (uint8_t row = 0; row < 2; row++) {
    setKey[row].initButton(&tft, 40 + row * (40 + 120), // x, y, w, h, outline, fill, text
                        110,
                        40, 35, TFT_WHITE, setKeyColor, 
                        TFT_WHITE, setKeyLabel[row], 1);
    setKey[row].drawButton(false, setKeyLabel[row]);
  }
  for (uint8_t row = 0; row < 2; row++) {
    setKey[row+2].initButton(&tft, 120, // x, y, w, h, outline, fill, text
                        240 + row * (35 + 10),
                        160, 35, TFT_WHITE, mainKeyColor, 
                        TFT_WHITE, setKeyLabel[row+2], 1);
    setKey[row+2].drawButton(false, setKeyLabel[row+2]);
  }
  for (uint8_t row = 0; row < 2; row++) {
    setKey[row+4].initButton(&tft, 65 + row * (100 + 10), // x, y, w, h, outline, fill, text
                        190,
                        100, 35, TFT_WHITE, setKeyColor, 
                        TFT_WHITE, setKeyLabel[row+4], 1);
    setKey[row+4].drawButton(false, setKeyLabel[row+4]);
  }

  drawSetMenuNow();
}

void drawSetMenuNow()
{
  char charBuffer[10];

  tft.fillRect(70, 100, 100, 30, TFT_DARKGREY);
  
  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextSize(1);

  tft.setTextDatum(TC_DATUM);
  sprintf(charBuffer, "%d", patientAge);
  tft.drawString(charBuffer, 120, 100);
}

void drawMeasMenu()
{
  drawBox();

  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  
  tft.drawString("Gula Darah", 20, 50);
  tft.drawString("Kolesterol", 20, 80);
/*  tft.drawString("Red DC", 20, 110);
  tft.drawString("IR AC", 20, 140);
  tft.drawString("IR DC", 20, 170);*/

   // Draw the key
  tft.setFreeFont(LABEL1_FONT);
  for (uint8_t row = 0; row < 2; row++) {
    measKey[row].initButton(&tft, 120, // x, y, w, h, outline, fill, text
                        240 + row * (35 + 10),
                        160, 35, TFT_WHITE, measKeyColor[row], 
                        TFT_WHITE, measKeyLabel[row], 1);
    measKey[row].drawButton(false, measKeyLabel[row]);
  }

  drawMeasStatNow();
}

void drawMeasStatNow()
{
  char charBuffer[10];

  tft.fillRect(120, 50, 110, 145, TFT_DARKGREY);
  
  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  
  sprintf(charBuffer, "%.2f", guladarah);
  tft.drawString(charBuffer, 120, 50);
  
  sprintf(charBuffer, "%.2f", kolesterol);
  tft.drawString(charBuffer, 120, 80);
}

void drawWaitScreen(){
  drawBox();

  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  
  tft.drawString("Mohon Tunggu", 20, 50);
  tft.drawString("Sedang Mengukur", 20, 80);
//  tft.drawString("Sedang Mengukur", 20, 110);
}

void drawDataMenu()
{
  drawBox();

  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  tft.drawString("Nama", 20, 120);
  tft.drawString("Gula Darah", 20, 150);
  tft.drawString("Kolesterol", 20, 180);
  tft.drawString("Usia", 20, 210);
  tft.drawString("Gender", 20, 240);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Data :", 120, 40);

    // Draw the key
  tft.setFreeFont(LABEL1_FONT);
  for (uint8_t row = 0; row < 2; row++) {
    dataKey[row].initButton(&tft, 40 + row * (40 + 120), // x, y, w, h, outline, fill, text
                        90,
                        40, 35, TFT_WHITE, dataKeyColor, 
                        TFT_WHITE, dataKeyLabel[row], 1);
    dataKey[row].drawButton(false, dataKeyLabel[row]);
  }
  dataKey[2].initButton(&tft, 170, // x, y, w, h, outline, fill, text
                      290,
                      100, 30, TFT_WHITE, dataKeyColor, 
                      TFT_WHITE, dataKeyLabel[2], 1);
  dataKey[2].drawButton(false, dataKeyLabel[2]);

  drawDataStatNow();
}

void drawDataStatNow()
{
  char charBuffer[20];
  nameBuffer[nameIndex] = 0; // Place null in buffer
  patientAge = 0;
  patientGenderSelected = 0;
  guladarah = 0;

  int dataFind = dataNumber - (dataShow - 1);
  if(dataFind <= 0){
    dataFind += 100;
  }

  getThisData(dataFind);

  tft.fillRect(120, 120, 110, 150, TFT_DARKGREY);
  tft.fillRect(70, 80, 100, 30, TFT_DARKGREY);
  
  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(4);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  tft.drawString(nameBuffer, 120, 120);
  
  sprintf(charBuffer, "%d,%d", guladarah, kolesterol);
  tft.drawString(charBuffer, 120, 150);

  sprintf(charBuffer, "%d", patientAge);
  tft.drawString(charBuffer, 120, 210);

  if(patientGenderSelected == 2){
    tft.drawString("Wanita", 120, 240);
  }else{
    tft.drawString("Pria", 120, 240);
  }

  tft.setTextDatum(TC_DATUM);
  sprintf(charBuffer, "%d", dataShow);
  tft.drawString(charBuffer, 120, 80);

  tft.setTextPadding(10);
  tft.setTextColor(TFT_NAVY, TFT_DARKGREY);
  tft.setTextFont(2);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  sprintf(charBuffer, "%02d/%02d/%d", dayDate, monthDate, yearDate);
  tft.drawString(charBuffer, 25, 280);
}

void sendSavedData()
{
  nameBuffer[nameIndex] = 0; // Place null in buffer
  patientAge = 0;
  patientGenderSelected = 0;
  guladarah = 0;
  kolesterol = 0;

  for(dataShow = 1;dataShow<=100;dataShow++){
    int dataFind = dataNumber - (dataShow - 1);
    getThisData(dataFind);
    
    sprintf(sendTreatBuf, "$%-9s,%d,%d,%d.%d,%02d/%02d/%d\n", nameBuffer, patientAge, patientGenderSelected, guladarah, kolesterol, dayDate, monthDate, yearDate);
//    SerialBT.print(sendTreatBuf);
//    Serial.write(sendTreatBuf);
    delay(10);
  }
    sprintf(sendTreatBuf, "@");
}
/*
void receiveTreatData()
{
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
//    parseData();
    
    newData = false;
  }
}
*/
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

//####################################################################################################
// Draw a JPEG on the TFT pulled from a program memory array
//####################################################################################################
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);
  renderJPEG(x, y);
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {

  // retrieve infomration about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = _min(mcu_w, max_x % mcu_w);
  uint32_t min_h = _min(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.read()) {
    
    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    tft.startWrite();

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
    {

      // Now set a MCU bounding window on the TFT to push pixels into (x, y, x + width - 1, y + height - 1)
      tft.setAddrWindow(mcu_x, mcu_y, win_w, win_h);

      // Write all MCU pixels to the TFT window
      while (mcu_pixels--) {
        // Push each pixel to the TFT MCU area
        tft.pushColor(*pImg++);
      }

    }
    else if ( (mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding

    tft.endWrite();
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;
}
