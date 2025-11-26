/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define STEP_PIN GPIO_PIN_0
#define DIR_PIN GPIO_PIN_1


#define STEPS_PER_REV 1600
#define VAL_PER_SEC_TIMER 1e9 //This sets what the units are for all the timer intevals and such. i.e. 1e9 means they're ns.
#define ERROR_FACTOR 16384 //This sets how much finer resolution the interval_error_term is compared to step_interval
#define ERROR_THRESHOLD 10 //This sets the cutoff for step_interval below which we start tracking the error factor
#define CLOCK_SPEED_HZ 32e6

#define ENCODER_POS_TO_REV 0.05
#define MAX_DAC_CURRENT 2579 //The current corresponding to maximum DAC output is 2579mA

#define LCD_WIDTH  132
#define LCD_HEIGHT 64

#define FONT_WIDTH  5
#define FONT_HEIGHT 8

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



//This font taken from Adafruit's GFX Library.
static const unsigned char font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
    0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
    0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
    0x18, 0x3C, 0x7E, 0x3C, 0x18,
    0x1C, 0x57, 0x7D, 0x57, 0x1C,
    0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
    0x00, 0x18, 0x3C, 0x18, 0x00,
    0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
    0x00, 0x18, 0x24, 0x18, 0x00,
    0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
    0x30, 0x48, 0x3A, 0x06, 0x0E,
    0x26, 0x29, 0x79, 0x29, 0x26,
    0x40, 0x7F, 0x05, 0x05, 0x07,
    0x40, 0x7F, 0x05, 0x25, 0x3F,
    0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
    0x7F, 0x3E, 0x1C, 0x1C, 0x08,
    0x08, 0x1C, 0x1C, 0x3E, 0x7F,
    0x14, 0x22, 0x7F, 0x22, 0x14,
    0x5F, 0x5F, 0x00, 0x5F, 0x5F,
    0x06, 0x09, 0x7F, 0x01, 0x7F,
    0x00, 0x66, 0x89, 0x95, 0x6A,
    0x60, 0x60, 0x60, 0x60, 0x60,
    0x94, 0xA2, 0xFF, 0xA2, 0x94,
    0x08, 0x04, 0x7E, 0x04, 0x08,
    0x10, 0x20, 0x7E, 0x20, 0x10,
    0x08, 0x08, 0x2A, 0x1C, 0x08,
    0x08, 0x1C, 0x2A, 0x08, 0x08,
    0x1E, 0x10, 0x10, 0x10, 0x10,
    0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
    0x30, 0x38, 0x3E, 0x38, 0x30,
    0x06, 0x0E, 0x3E, 0x0E, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x5F, 0x00, 0x00,
    0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x56, 0x20, 0x50,
    0x00, 0x08, 0x07, 0x03, 0x00,
    0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00,
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
    0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x80, 0x70, 0x30, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x00, 0x60, 0x60, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02,
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    0x00, 0x42, 0x7F, 0x40, 0x00,
    0x72, 0x49, 0x49, 0x49, 0x46,
    0x21, 0x41, 0x49, 0x4D, 0x33,
    0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39,
    0x3C, 0x4A, 0x49, 0x49, 0x31,
    0x41, 0x21, 0x11, 0x09, 0x07,
    0x36, 0x49, 0x49, 0x49, 0x36,
    0x46, 0x49, 0x49, 0x29, 0x1E,
    0x00, 0x00, 0x14, 0x00, 0x00,
    0x00, 0x40, 0x34, 0x00, 0x00,
    0x00, 0x08, 0x14, 0x22, 0x41,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08,
    0x02, 0x01, 0x59, 0x09, 0x06,
    0x3E, 0x41, 0x5D, 0x59, 0x4E,
    0x7C, 0x12, 0x11, 0x12, 0x7C,
    0x7F, 0x49, 0x49, 0x49, 0x36,
    0x3E, 0x41, 0x41, 0x41, 0x22,
    0x7F, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x49, 0x49, 0x49, 0x41,
    0x7F, 0x09, 0x09, 0x09, 0x01,
    0x3E, 0x41, 0x41, 0x51, 0x73,
    0x7F, 0x08, 0x08, 0x08, 0x7F,
    0x00, 0x41, 0x7F, 0x41, 0x00,
    0x20, 0x40, 0x41, 0x3F, 0x01,
    0x7F, 0x08, 0x14, 0x22, 0x41,
    0x7F, 0x40, 0x40, 0x40, 0x40,
    0x7F, 0x02, 0x1C, 0x02, 0x7F,
    0x7F, 0x04, 0x08, 0x10, 0x7F,
    0x3E, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x09, 0x09, 0x09, 0x06,
    0x3E, 0x41, 0x51, 0x21, 0x5E,
    0x7F, 0x09, 0x19, 0x29, 0x46,
    0x26, 0x49, 0x49, 0x49, 0x32,
    0x03, 0x01, 0x7F, 0x01, 0x03,
    0x3F, 0x40, 0x40, 0x40, 0x3F,
    0x1F, 0x20, 0x40, 0x20, 0x1F,
    0x3F, 0x40, 0x38, 0x40, 0x3F,
    0x63, 0x14, 0x08, 0x14, 0x63,
    0x03, 0x04, 0x78, 0x04, 0x03,
    0x61, 0x59, 0x49, 0x4D, 0x43,
    0x00, 0x7F, 0x41, 0x41, 0x41,
    0x02, 0x04, 0x08, 0x10, 0x20,
    0x00, 0x41, 0x41, 0x41, 0x7F,
    0x04, 0x02, 0x01, 0x02, 0x04,
    0x40, 0x40, 0x40, 0x40, 0x40,
    0x00, 0x03, 0x07, 0x08, 0x00,
    0x20, 0x54, 0x54, 0x78, 0x40,
    0x7F, 0x28, 0x44, 0x44, 0x38,
    0x38, 0x44, 0x44, 0x44, 0x28,
    0x38, 0x44, 0x44, 0x28, 0x7F,
    0x38, 0x54, 0x54, 0x54, 0x18,
    0x00, 0x08, 0x7E, 0x09, 0x02,
    0x18, 0xA4, 0xA4, 0x9C, 0x78,
    0x7F, 0x08, 0x04, 0x04, 0x78,
    0x00, 0x44, 0x7D, 0x40, 0x00,
    0x20, 0x40, 0x40, 0x3D, 0x00,
    0x7F, 0x10, 0x28, 0x44, 0x00,
    0x00, 0x41, 0x7F, 0x40, 0x00,
    0x7C, 0x04, 0x78, 0x04, 0x78,
    0x7C, 0x08, 0x04, 0x04, 0x78,
    0x38, 0x44, 0x44, 0x44, 0x38,
    0xFC, 0x18, 0x24, 0x24, 0x18,
    0x18, 0x24, 0x24, 0x18, 0xFC,
    0x7C, 0x08, 0x04, 0x04, 0x08,
    0x48, 0x54, 0x54, 0x54, 0x24,
    0x04, 0x04, 0x3F, 0x44, 0x24,
    0x3C, 0x40, 0x40, 0x20, 0x7C,
    0x1C, 0x20, 0x40, 0x20, 0x1C,
    0x3C, 0x40, 0x30, 0x40, 0x3C,
    0x44, 0x28, 0x10, 0x28, 0x44,
    0x4C, 0x90, 0x90, 0x90, 0x7C,
    0x44, 0x64, 0x54, 0x4C, 0x44
};



// -------------------------------- Variables --------------------------------

// Variables for the move planner (the numbers are 100x their "real" values)
int stepper_setSpeed = 100; //   rev/s  *100
int stepper_setAccel = 1000;//   rev/s2 *100
int stepper_setMulti = 100; //   unitless
int stepper_setDist = 100;//     rev
int stepper_targPos = 0;//        microsteps !
int stepper_dir = 1; //+1 for forward, -1 for reverse
int stepper_currentSpeed=0;//    rev/s *100
volatile int stepper_currentPos = 0;//    microsteps
int stepper_currentPos_Disp = 0;//revs * 100

volatile int accel_steps = 0;
volatile int stopping_steps =0;
volatile bool running = false;
volatile int steps_remaining=0;
volatile int step_count=0;
 bool step_state = false;
volatile uint32_t step_interval =0;
volatile uint32_t min_interval = 0;
 uint32_t interval_delta =0;
volatile uint32_t interval_error_delta=0;
 uint32_t interval_error_term=0;
volatile uint32_t total_ticks;
volatile uint32_t prescaler;
volatile uint32_t arr;


//Variables for the overall state machine
uint8_t stepper_mode = 0;  // 0 = momentary, 1 = continous, 2 = manJog, 3 = Reciprocate
int stepper_current = 1000;

//Manual Jog
uint8_t lastEncoderMode = 0; 

//Reciprocate
int cycleCount = 0;
bool stopNextCycle = 0;
int8_t reciprocate_dir = 0;
int8_t reciprocate_state = 0; // -1=inbound, 0= not running, 1=outbound

//Momentary
int8_t btnCommand = 0;  // -1 = reverse, 0 = stop, 1 = forward




//Encoder variables
//static bool ENC_B_lastState = 0;
int8_t EncStepDir = 0;
bool ENC_A = 0;
bool ENC_B = 0;
uint8_t numEncoderModes=2; //how many encoder modes are available for the current mode
uint8_t encoderMode = 0;  // Current mode
float incrementValue = 0;
uint32_t currPulseTime = 100;
uint32_t lastPulseTime = 0;


typedef struct EncoderParametersStruct {
  int* variable;       // Pointer to the variable we want to modify
  int minVal;          // Minimum value for the variable
  int maxVal;          // Maximum value for the variable
  int fineStepSize;    // Step size for each increment/decrement
  int midStepSize;     // Step size for each increment/decrement
  int coarseStepSize;  // Step size for each increment/decrement
} EncoderParametersStruct;

EncoderParametersStruct encoderParameter[] = {
  // variable, minVal, maxVal, fine, mid, coarse step size
  { &stepper_setSpeed, 1, 2500, 1, 10, 100 },
  { &stepper_setAccel, 1, 99999, 10, 100, 1000 },
  { &stepper_targPos, -32768, 32768, 100, 100, 100 },
  { &stepper_setMulti, 1, 99999, 1, 10, 100 },
  { &stepper_setDist, 1, 99999, 1, 10, 100 },
};

//Button Variables
bool lastStateFWD=false;
bool lastStateSTP=false;
bool lastStateREV=false;
bool lastStateMOD=false;
bool lastStateCUR=false;
bool lastStateENT=false;

//Display Variables
char strBuffer[15]; //a reusable string buffer
static uint8_t buffer[LCD_WIDTH * LCD_HEIGHT / 8];   // 1-bit framebuffer
int8_t cursorPos = 0;








///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////









bool isButtonPressedFWD() {
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)==GPIO_PIN_SET;
}

bool isButtonPressedSTP() {
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)==GPIO_PIN_SET;
}

bool isButtonPressedREV() {
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)==GPIO_PIN_SET;
}

bool isButtonPressedMOD() {
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)==GPIO_PIN_SET;
}

bool isButtonPressedCUR() {
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8)==GPIO_PIN_SET;
}

bool isButtonPressedENT() {
  return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)==GPIO_PIN_SET;
}

bool isRisingButtonFWD() {
  bool newState = isButtonPressedFWD();
  bool isRisingEdge = (lastStateFWD==false && newState==true);
  lastStateFWD=newState;
  return isRisingEdge;
}

/*
bool isRisingButtonSTP() {
  bool newState = isButtonPressedSTP();
  bool isRisingEdge = (lastStateSTP==false && newState==true);
  lastStateSTP=newState;
  return isRisingEdge;
}*/

bool isRisingButtonREV() {
  bool newState = isButtonPressedREV();
  bool isRisingEdge = (lastStateREV==false && newState==true);
  lastStateREV=newState;
  return isRisingEdge;
}

bool isRisingButtonMOD() {
  bool newState = isButtonPressedMOD();
  bool isRisingEdge = (lastStateMOD==false && newState==true);
  lastStateMOD=newState;
  return isRisingEdge;
}

bool isRisingButtonCUR() {
  bool newState = isButtonPressedCUR();
  bool isRisingEdge = (lastStateCUR==false && newState==true);
  lastStateCUR=newState;
  return isRisingEdge;
}

bool isRisingButtonENT() {
  bool newState = isButtonPressedENT();
  bool isRisingEdge = (lastStateENT==false && newState==true);
  lastStateENT=newState;
  return isRisingEdge;
}



void setCurrentPins() {
  //This task sets the 4 bits of the DAC to write a value for current limit
  //BIT 4 is the MSB

  float fraction = (float)stepper_current / MAX_DAC_CURRENT;
  uint8_t value = (uint8_t)(fraction * 15.0);  //Squash to a 0-15 range

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, (value & 0x1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, (value & 0x2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, (value & 0x4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, (value & 0x8) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

















///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////













void delay_us(uint32_t us) {
    uint32_t count = (SystemCoreClock / 1000000) * us / 5; 
    while(count--) {
        __NOP();  // assembly no-op
    }
}


static void ST7567_Select(void)   {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); delay_us(100);}
static void ST7567_Unselect(void)   {HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); delay_us(100);}


static void ST7567_Command(uint8_t cmd) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); //DC Bit
    ST7567_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    ST7567_Unselect();
}



void ST7567_Init(void) {
    // Reset pulse
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // pulsing the reset pin
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    // Typical ST7567 init sequence
    ST7567_Command(0xAE); 
    ST7567_Command(0xE2); 
    ST7567_Command(0xA1);
    ST7567_Command(0xC0);
    ST7567_Command(0xA2);
    ST7567_Command(0x2F);
    ST7567_Command(0x26);
    ST7567_Command(0x81);
    ST7567_Command(0x28);
    ST7567_Command(0xAF);
}


void ST7567_Clear(void) {
    memset(buffer, 0x00, sizeof(buffer));
}

void ST7567_DrawPixel(uint8_t x, uint8_t y) { //}, uint8_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    uint16_t index = x + (y / 8) * LCD_WIDTH;
    //if (color) 
    buffer[index] |=  (1 << (y & 7));
    //else       buffer[index] &= ~(1 << (y & 7));
}

void ST7567_DrawHLine(uint8_t x0, uint8_t x1, uint8_t y)
{
    if (x1 < x0) {
        uint8_t temp = x0;
        x0 = x1;
        x1 = temp;
    }

    for (uint8_t x = x0; x <= x1; x++) {
        ST7567_DrawPixel(x, y);
    }
}

void ST7567_DrawVLine(uint8_t x, uint8_t y0, uint8_t y1)
{
    if (y1 < y0) {
        uint8_t temp = y0;
        y0 = y1;
        y1 = temp;
    }

    for (uint8_t y = y0; y <= y1; y++) {
        ST7567_DrawPixel(x, y);
    }
}

void ST7567_DrawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    ST7567_DrawHLine(x0, x1, y0); // top
    ST7567_DrawHLine(x0, x1, y1); // bottom
    ST7567_DrawVLine(x0, y0, y1); // left
    ST7567_DrawVLine(x1, y0, y1); // right
}


void ST7567_Display(void) {
    for (uint8_t page = 0; page < (LCD_HEIGHT / 8); page++) {
        ST7567_Command(0xB0 | page);    // set page address
        ST7567_Command(0x10);           // set column high bits to 0
        ST7567_Command(0x00);           // set column low bits to 0

        ST7567_Select();
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET); //DC Bit
        HAL_SPI_Transmit(&hspi1, &buffer[LCD_WIDTH * page], LCD_WIDTH, HAL_MAX_DELAY);
        ST7567_Unselect();
    }
}

void ST7567_DrawChar(uint8_t x, uint8_t y, char c, uint8_t size)
{
    const uint8_t *glyph = &font[(c * 5)];

    for (uint8_t col = 0; col < FONT_WIDTH; col++) {
        uint8_t line = glyph[col];
        for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
            if (line & 0x01) {
                // Draw scaled pixel block
                for (uint8_t dx = 0; dx < size; dx++) {
                    for (uint8_t dy = 0; dy < size; dy++) {
                        ST7567_DrawPixel(x + col * size + dx, y + row * size + dy);
                    }
                }
            }
            line >>= 1;
        }
    }
}

void ST7567_DrawText(uint8_t x, uint8_t y, const char *text, uint8_t size)
{
    while (*text) {
        ST7567_DrawChar(x, y, *text, size);
        x += (FONT_WIDTH + 1) * size; // 1-pixel spacing scaled up
        text++;
    }
}

void DrawBitmap(int x, int y, const uint8_t *bitmap, int width, int height)
{
    int byteWidth = (width + 7) / 8; // each byte holds 8 horizontal pixels

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int byteIndex = j * byteWidth + i / 8;
            if (bitmap[byteIndex] & (0x80 >> (i & 7)))
            {
                ST7567_DrawPixel(x + i, y + j);
            }
        }
    }
}

uint8_t squareSymbolBitmap[] = {
  0b11100000,
  0b00100000,
  0b11100000,
  0b10000000,
  0b11100000,
};  

uint8_t upArrow[] = { 0x20, 0x50, 0x88 };    // bitmap image of up arrow
uint8_t downArrow[] = { 0x88, 0x50, 0x20 };  // bitmap image of down arrow


void displayDrawArrow() {
  int position = 0;
  if ((HAL_GetTick() % 500) > 125) position = 1;
  if ((HAL_GetTick() % 500) > 250) position = 2;
  if ((HAL_GetTick() % 500) > 375) position = 3;

  if (stepper_currentSpeed > 0) {
    DrawBitmap(81, 24 - position, upArrow, 5, 3); }

  if (stepper_currentSpeed < 0) {
    DrawBitmap(81, 21 + position, downArrow, 5, 3);}
}














///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////













void drawSplashScreen() {
  ST7567_Clear();
  memset(buffer, 0xFF, sizeof(buffer));
  ST7567_DrawText(5,23,"Zino Manufacturing",1);
  ST7567_DrawText(5,33,"Dominic Panzino 2025", 1);
  ST7567_Display();
}

//This function assumes the number is 100x its "real" value
void printToTwoDecimalPlaces(char formatString[], int number) {
  memset(strBuffer, 0x00, sizeof(strBuffer));
  snprintf(strBuffer, sizeof(strBuffer), formatString,number/100,abs((number/10)%10),abs((number%10)));
}




void drawScreenBasics() {
  ST7567_Clear();

  if (stepper_current == 0) {
    if((HAL_GetTick() % 1000) > 500) {
      ST7567_DrawText(91,20, "OFF", 1);
    }
  } else {
    memset(strBuffer, 0x00, sizeof(strBuffer));
    snprintf(strBuffer, sizeof(strBuffer), "%d mA",stepper_current);
    ST7567_DrawText(91,20, strBuffer, 1);
  }

  if(stepper_mode==0) {
    ST7567_DrawText(5,20, "Momentary", 1);
  } else if (stepper_mode==1) {
    ST7567_DrawText(5,20, "Continuous", 1);
  } else if (stepper_mode==2) {
    ST7567_DrawText(5,20, "Manual Jog", 1);
  } else if (stepper_mode==3) {
    ST7567_DrawText(5,20, "Reciprocate", 1);
  } else if (stepper_mode==4) {
    ST7567_DrawText(5,20, "Incremental", 1);
  }
  
  ST7567_DrawHLine(0,132,29);
  ST7567_DrawVLine(87,20,29);
  displayDrawArrow();
  if(cursorPos>=0) {ST7567_DrawRect(4, cursorPos * 10 + 42, 39, cursorPos * 10 + 32);}


    // Print Speed setting
  memset(strBuffer, 0x00, sizeof(strBuffer));
  printToTwoDecimalPlaces("Speed: %d.%d%d",stepper_setSpeed);
  ST7567_DrawText(6,34,strBuffer,1);
  ST7567_DrawText(93,34,"R/s",1);

  //Print Accel Setting
  if(stepper_setAccel < 0.05) {
    ST7567_DrawText(6,44,"Accel: inf",1);
  } else {
    memset(strBuffer, 0x00, sizeof(strBuffer));
    printToTwoDecimalPlaces("Accel: %d.%d%d",stepper_setAccel);
    ST7567_DrawText(6,44,strBuffer,1);
  }
  ST7567_DrawText(93,44,"R/s",1);
  DrawBitmap(111, 43, squareSymbolBitmap, 3, 5);
}




void screenMomentaryContinous() {

    drawScreenBasics();
    
    //Show Speed as main status
    memset(strBuffer, 0x00, sizeof(strBuffer));
    printToTwoDecimalPlaces("R/s %d.%d%d",stepper_currentSpeed);
    ST7567_DrawText(5,3,strBuffer,2);

    //Write to display
    ST7567_Display();
}


void screenManJog() {

    drawScreenBasics();

    //Show position as the main status value
    if (stepper_currentPos_Disp < -99999) { //0.225???
      ST7567_DrawText(5,3,"POS   --",2);
    } else if (stepper_currentPos_Disp > 999999) {
      ST7567_DrawText(5,3,"POS   ++",2);
    } else {
      memset(strBuffer, 0x00, sizeof(strBuffer));
      if(abs(stepper_currentPos_Disp) < 1000) {
        printToTwoDecimalPlaces("POS: %d.%d%d",stepper_currentPos_Disp);
      } else {
        printToTwoDecimalPlaces("POS: %d.%d%d",stepper_currentPos_Disp);
      }
      ST7567_DrawText(5,3,strBuffer,2);
    }

    //Print mult factor
    memset(strBuffer, 0x00, sizeof(strBuffer));
    printToTwoDecimalPlaces("Multi: %d.%d%d",stepper_setMulti);
    ST7567_DrawText(5,54,strBuffer,1);

    //Write to display
    ST7567_Display();

}


void screenReciprocate() {

    drawScreenBasics();

    //Print cycle count
    memset(strBuffer, 0x00, sizeof(strBuffer));
    snprintf(strBuffer, sizeof(strBuffer), "# %d",cycleCount);
    ST7567_DrawText(5,3,strBuffer,2);

    //Print Distance Setting
    memset(strBuffer, 0x00, sizeof(strBuffer));
    printToTwoDecimalPlaces("Dist: %d.%d%d",stepper_setDist);
    ST7567_DrawText(5,54,strBuffer,1);
    ST7567_DrawText(93,54,"Rev",1);

    //Write to display
    ST7567_Display();
}

void screenIncremental() {

    drawScreenBasics();

    //Show position as the main status value
    if (stepper_currentPos_Disp < -99999) { //0.225???
      ST7567_DrawText(5,3,"POS:   --",2);
    } else if (stepper_currentPos_Disp > 999999) {
      ST7567_DrawText(5,3,"POS:   ++",2);
    } else {
      memset(strBuffer, 0x00, sizeof(strBuffer));
      if(abs(stepper_currentPos_Disp) < 1000) {
        printToTwoDecimalPlaces("POS: %d.%d%d",stepper_currentPos_Disp);
      } else {
        printToTwoDecimalPlaces("POS: %d.%d%d",stepper_currentPos_Disp);
      }
      ST7567_DrawText(5,3,strBuffer,2);
    }

    //Print Distance Setting
    memset(strBuffer, 0x00, sizeof(strBuffer));
    printToTwoDecimalPlaces("Dist: %d.%d%d",stepper_setDist);
    ST7567_DrawText(5,54,strBuffer,1);
    ST7567_DrawText(93,54,"Rev",1);

    //Write to display
    ST7567_Display();
}



void screenDebug() {
  ST7567_Clear();
  if(isButtonPressedFWD()) {ST7567_DrawText(5,13,"fwd", 1);}
  if(isButtonPressedSTP()) {ST7567_DrawText(5,23,"stp", 1);}
  if(isButtonPressedREV()) {ST7567_DrawText(5,33,"rev", 1);}
  if(isButtonPressedMOD()) {ST7567_DrawText(45,13,"mod", 1);}
  if(isButtonPressedENT()) {ST7567_DrawText(45,23,"ent", 1);}
  if(isButtonPressedCUR()) {ST7567_DrawText(45,33,"cur", 1);}

  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)==GPIO_PIN_SET) {ST7567_DrawText(5,43,"a", 1);}//A
  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)==GPIO_PIN_SET) {ST7567_DrawText(45,43,"b", 1);}//B
  
  printToTwoDecimalPlaces("value: %d.%d%d",stepper_setSpeed);
  ST7567_DrawText(5,54,strBuffer,1);
  ST7567_Display();
}






void DisplayTask() {
  switch (stepper_mode) {
    case 0: screenMomentaryContinous(); break;
    case 1: screenMomentaryContinous(); break;
    case 2: screenManJog(); break;
    case 3: screenReciprocate(); break;
    case 4: screenIncremental(); break;
  }
}













///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void stepperInit() {
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET); //Put the stepper driver out of sleep mode.
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET); //Reset the stepper driver
  HAL_Delay(5);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET); //unreset the stepper driver
  setCurrentPins();
  HAL_TIM_Base_Start_IT(&htim3);
}


void stepperSetCurrentPosition(int pos) {
  stepper_currentPos=pos;
}

void stepperDeadstop() { //This function instantly stops the motor, ignoring all deceleration limits
  running=false;
  step_count=0;
  steps_remaining=0;
}

void stepperStop() {
  //stopping_steps=STEPS_PER_REV*(stepper_currentSpeed * stepper_currentSpeed) / (2.0 * stepper_setAccel) / 100;
  if(steps_remaining>=stoppingSteps) {steps_remaining=stoppingSteps;} //This will cause the motor to decelerate to a stop.
}

int stepperDistanceToGo() {
  return stepper_targPos - stepper_currentPos;
}

//Plans a move for that distance 
void stepperMove(int steps) {
  if (running) return; // just ignore if we're already moving
  if (steps==0) return; //why?

  //Direction setting
  stepper_dir = (steps>0 ? 1 : -1);
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,steps>0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
  steps=steps*stepper_dir;

  min_interval = (VAL_PER_SEC_TIMER / (stepper_setSpeed*STEPS_PER_REV/100) / 2);
  steps_remaining = steps;

  
  if(stepper_setAccel < 0.05) {
    accel_steps = 0;
    step_interval = min_interval;
    stopping_steps=0;
  } else {
    //accel_steps = STEPS_PER_REV*(stepper_setSpeed * stepper_setSpeed) / (2.0 * stepper_setAccel) / 100 * 1.2;
    accel_steps = (STEPS_PER_REV*stepper_setSpeed)/(2.0 * stepper_setAccel*100) * stepper_setSpeed*1.2 ;
    if (accel_steps * 2 > steps) { //Triangle Moves
      stopping_steps = steps / 2;
    } else { 
      stopping_steps = accel_steps;
    
    }
    step_interval = 0.5*VAL_PER_SEC_TIMER/(sqrt(stepper_setAccel*STEPS_PER_REV/100)); // need to figure out initial interval based on desired acceleration. The leading coefficient is just a fudge factor, TBD
  }

  running = true;
}

void stepperMoveTo(int position) {
  stepperMove(position - stepper_currentPos);
}

/*void stepperSetMaxSpeed(int speed) {
  stepper_setSpeed=speed;
  accel_steps = STEPS_PER_REV*(stepper_setSpeed * stepper_setSpeed) / (2.0 * stepper_setAccel);
}

void stepperSetAcceleration(int accel) {
  stepper_setAccel=accel;
  accel_steps = STEPS_PER_REV*(stepper_setSpeed * stepper_setSpeed) / (2.0 * stepper_setAccel);
}*/


void MetricsTask() {
  if(running) {
    stepper_currentSpeed = VAL_PER_SEC_TIMER/(2*step_interval)*100/STEPS_PER_REV*stepper_dir;
    min_interval = (VAL_PER_SEC_TIMER / (stepper_setSpeed*STEPS_PER_REV/100) / 2);
    stopping_steps=STEPS_PER_REV*(stepper_currentSpeed * stepper_currentSpeed) / (2.0 * stepper_setAccel) / 100;
  } else {
    stepper_currentSpeed = 0;
  }
  stepper_currentPos_Disp = stepper_currentPos*100/STEPS_PER_REV;
}



//This is the interrupt that's triggered when it's time to step
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if(!running) return; //nothing to do here
  if (step_state) {HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET); step_state=false; return;} //just reset the pin and move along

  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET); //Step !
  step_state=true;
  step_count++;
  steps_remaining--;
  stepper_currentPos+=stepper_dir;

  if(steps_remaining==0) {
    running=false;
    step_count=0;
    setTimer(1e6);//slow down the interupt timer so it isn't going crazy for no reason.
    return;
  }


  if (steps_remaining <= stopping_steps) { //time to start slowing down
    interval_delta = (2*step_interval)/(4*steps_remaining+1);
    step_interval += interval_delta;
    if(interval_delta<=ERROR_THRESHOLD) { //At this  point we need to keep track of fractional ns increments
      interval_error_term += ((2*ERROR_FACTOR*step_interval)/(4*steps_remaining+1))-interval_delta*ERROR_FACTOR;
      if(interval_error_term>ERROR_FACTOR) {
        interval_error_term-=ERROR_FACTOR;
        step_interval+=1; //increment the interval by 1ns every time the error term rolls over
      }
    }
  } else if (step_count < accel_steps) {
    interval_delta = (2*step_interval)/(4*step_count+1); //This is the special formula
    step_interval -= interval_delta;
    if(interval_delta<=ERROR_THRESHOLD) { //At this point we need to keep track of fractional ns increments
      interval_error_term += ((2*ERROR_FACTOR*step_interval)/(4*step_count+1))-interval_delta*ERROR_FACTOR;
      if(interval_error_term>ERROR_FACTOR) {
        interval_error_term-=ERROR_FACTOR;
        step_interval-=1; //decrement the interval by 1ns every time the error term rolls over
      }
    }            
    if (step_interval < min_interval) {
      step_interval = min_interval;
      interval_error_term=0;
    }
  }

  setTimer(step_interval/(32));//(VAL_PER_SEC_TIMER/CLOCK_SPEED_HZ);
}


void setTimer(uint32_t total_ticks) {
  prescaler = (uint32_t)(total_ticks / 32768);
  if(prescaler==0) {prescaler=1;}
  if (prescaler > 0xFFFF) prescaler = 0xFFFF;
  arr = (uint32_t)(total_ticks / (prescaler+1));


  if (arr > 0xFFFF) arr = 0xFFFF; else if (arr == 0) arr = 1;

  __HAL_TIM_SET_PRESCALER(htim, prescaler);
  __HAL_TIM_SET_AUTORELOAD(htim, arr - 1);
}









///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////












void ButtonTask() {
  //This purpose of this task is to monitor the current and mode buttons (mostly the latter)

  // Read Current Button
  if (isRisingButtonCUR()) {
    if (stepper_current + 200 <= 2000) {
      stepper_current += 200;
    } else {
      stepper_current = 0;
    }
    setCurrentPins();
    
    if(stepper_current == 0) {
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET); //stepper enable pin
    } else {
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
    }
  }

  if (isRisingButtonENT()) {
    cursorPos++;
    if(cursorPos>=numEncoderModes) {cursorPos=-1;}


    if(stepper_mode==0){ //Momentary
      switch(cursorPos) { 
        case -1: cursorPos=0; //-1 pos isn't allowed here. The lack of a break is deliberate
        case 0: encoderMode=0; break;//Speed
        case 1: encoderMode=1; break;//Accel
      }
    } else if(stepper_mode==1){//Continuous
      switch(cursorPos) {
        case -1: cursorPos=0; //-1 pos isn't allowed here. The lack of a break is deliberate
        case 0: encoderMode=0; break;//Speed
        case 1: encoderMode=1; break;//Accel
      }
    } else if(stepper_mode==2){//Manual Jog
      switch(cursorPos) {
        case -1: encoderMode=2; break;//Position
        case 0: encoderMode=0; break;//Speed
        case 1: encoderMode=1; break;//Accel
        case 2: encoderMode=3; break;//Multi
      }
    } else if(stepper_mode==3 || stepper_mode==4){//Reciprocate or Incremental
      switch(cursorPos) {
        case -1: cursorPos=0; //-1 pos isn't allowed here. The lack of a break is deliberate
        case 0: encoderMode=0; break;//Speed
        case 1: encoderMode=1; break;//Accel
        case 2: encoderMode=4; break;//Distance
      }
    }
  }


  if (isRisingButtonMOD()) {
    // 0 = momentary, 1 = continous, 2 = manJog, 3 = Reciprocate, 4 = Incremental
    stepper_mode+=1;
    if(stepper_mode>4) {stepper_mode = 0;}

    //Stop whatever was happening when the mode changes
    stepperDeadstop();
    stopNextCycle = 0;
    stepperSetCurrentPosition(0);
    reciprocate_state=0;
    reciprocate_dir=0;

    //Initialization commands for each mode
    switch (stepper_mode) {
      case 0: {
        cursorPos=0;
        encoderMode = 0;
        numEncoderModes=2;
        break;
      }
      case 1: {
        btnCommand = 0;
        cursorPos=0;
        encoderMode = 0;
        numEncoderModes=2;
        break;
      }
      case 2: {
        lastEncoderMode = 0;
        stepperSetCurrentPosition(0);
        stepper_targPos = 0;
        numEncoderModes=3;
        cursorPos = -1;
        encoderMode=2;
        break;
      }
      case 3: {
       cycleCount = 0;
       stepperSetCurrentPosition(0);
       stopNextCycle = 0;
       numEncoderModes=3;
       encoderMode=0;
       reciprocate_dir = 0;
       reciprocate_state=0;
       cursorPos=0;
       break;
      }
      case 4: {
       stepperSetCurrentPosition(0);
       numEncoderModes=3;
       encoderMode=0;
       cursorPos=0;
       break;
      }
    }
  }
}



//Encoder interrupt function
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  EncStepDir = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)==HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) ? 1 : -1);
  currPulseTime = HAL_GetTick();
}


void EncoderTask() {
  if (EncStepDir!=0) {

    if (currPulseTime - lastPulseTime > 80) {
      incrementValue = encoderParameter[encoderMode].fineStepSize;
    } else if (currPulseTime - lastPulseTime > 40) {
      incrementValue = encoderParameter[encoderMode].midStepSize;
    } else {
      incrementValue = encoderParameter[encoderMode].coarseStepSize;
    }
    lastPulseTime = currPulseTime;

    // Snap the value to the nearest multiple of incrementValue
    if (incrementValue > encoderParameter[encoderMode].fineStepSize) {
      *encoderParameter[encoderMode].variable = round(*encoderParameter[encoderMode].variable / incrementValue) * incrementValue;
    }

    // Adjust the variable based on encoder direction, ensuring it remains within bounds
    if (EncStepDir<0) {
      if (*encoderParameter[encoderMode].variable - incrementValue >= encoderParameter[encoderMode].minVal) {
        *encoderParameter[encoderMode].variable -= incrementValue;
      } else {
        *encoderParameter[encoderMode].variable = encoderParameter[encoderMode].minVal;
      }
    } else {
      if (*encoderParameter[encoderMode].variable + incrementValue <= encoderParameter[encoderMode].maxVal) {
        *encoderParameter[encoderMode].variable += incrementValue;
      } else {
        *encoderParameter[encoderMode].variable = encoderParameter[encoderMode].maxVal;
      }
    }
  }
  EncStepDir=0;
}













///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////













void stepperMomentary() {
  //stepperSetMaxSpeed(stepper_setSpeed * STEPS_PER_REV);
  //stepper_setAccel < 0.05 ? stepperSetAcceleration(999 * STEPS_PER_REV) : stepperSetAcceleration(stepper_setAccel * STEPS_PER_REV);

  if (isButtonPressedFWD()) {
    btnCommand=1;
    stepperMove(STEPS_PER_REV * 1000);  // Move a large distance forward
  } else if (isButtonPressedREV()) {
    btnCommand=-1;
    stepperMove(STEPS_PER_REV * -1000);  // Move a large distance backward
  } else {
    stepperStop();   // Stop the motor gracefully when you aren't hitting any button
  }

  if (isButtonPressedSTP()) {
    stepperDeadstop(); //Stop the motor instantly when you hit the stop button.
  }
}



void stepperContinous() {
  // Button latch
  if (isRisingButtonFWD()) {
    btnCommand = (btnCommand == 0 ? 1 : 0);
  }
  if (isRisingButtonREV()) {
    btnCommand = (btnCommand == 0 ? -1 : 0);
  }

  //stepper_setAccel < 0.05 ? stepperSetAcceleration(99999 * STEPS_PER_REV) : stepperSetAcceleration(stepper_setAccel * STEPS_PER_REV);

  if (btnCommand == 1) {
    stepperMove(STEPS_PER_REV * 1000);  // Move a large distance forward
  } else if (btnCommand == -1) {
    stepperMove(STEPS_PER_REV * -1000);  // Move a large distance backward
  } else {
    stepperMove(0);  // Stop the motor when there is no btn command
  }
  if (isButtonPressedSTP()) {
    btnCommand = 0;
    stepperStop();  // Stop the motor gracefully in this mode when you hit the stop button
  }
}



void stepperJog() {
    stepperMoveTo(stepper_targPos * stepper_setMulti*0.01);

    //This OR is to reset the home position when you change settings, since you might rescale everything.
    if (isButtonPressedSTP() || lastEncoderMode != encoderMode) { 
      stepperSetCurrentPosition(0);
      stepper_targPos = 0;
      stepperDeadstop();   // Stop the motor when you hit the stop button
    }
    lastEncoderMode = encoderMode;
}





void stepperReciprocate() {
    if (reciprocate_state==0) {
      if (isRisingButtonFWD()) {reciprocate_dir = 1;}
      if (isRisingButtonREV()) {reciprocate_dir = -1;}
      if (reciprocate_dir!=0) {
        reciprocate_state=1; stepperMoveTo(reciprocate_dir * stepper_setDist * STEPS_PER_REV * 0.01);
      }
    } else if(running) {
      if (isRisingButtonFWD() || isRisingButtonREV()) {stopNextCycle = 1;}
      if (isButtonPressedSTP()) {//Stops the motion immediately when hitting the stop button or changing modes
        stopNextCycle = 0;
        stepperSetCurrentPosition(0);
        stepperDeadstop(); 
        reciprocate_state=0;
        reciprocate_dir=0;
      } 
    } else if (reciprocate_state==1) {
      reciprocate_state=-1;
      stepperMoveTo(0);
    } else if (reciprocate_state==-1) {
        cycleCount++;
        if(stopNextCycle==1) {
          reciprocate_state=0;
          reciprocate_dir=0;
          stopNextCycle=0;
        } else {
          reciprocate_state=1;
          stepperMoveTo(reciprocate_dir * stepper_setDist * STEPS_PER_REV* 0.01);
        }
    }
}


void stepperIncremental() {
  if(running) {
    btnCommand=0;
    if (isButtonPressedSTP()) {//Stops the motion immediately when hitting the stop button or changing modes
      stepperSetCurrentPosition(0);
      stepperDeadstop(); 
    }
  } else {
    if (isRisingButtonFWD()) {btnCommand = 1;}
    if (isRisingButtonREV()) {btnCommand = -1;}
    if (btnCommand!=0) {
      stepperMoveTo(btnCommand * stepper_setDist * STEPS_PER_REV * 0.01 + stepper_currentPos);
    }
  }
}




void StepperTask() {
  switch (stepper_mode) {
    case 0: stepperMomentary(); break;
    case 1: stepperContinous(); break;
    case 2: stepperJog(); break;
    case 3: stepperReciprocate(); break;
    case 4: stepperIncremental(); break;
  }
}










///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
















/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */


  ST7567_Init();
  HAL_Delay(100);
  drawSplashScreen();
  HAL_Delay(1000);
  stepperInit();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    EncoderTask();  //This task reads the encoder rotary pulses and adjusts the specified parameter
    ButtonTask();   //This task reads the Mode, Current, and Encoder buttons. It also handles switching modes
    StepperTask();  //This task implements the motor commands
    MetricsTask();  //This task updates things like the currentSpeed and currentPosition in non-time-critical scope so those values can be displayed
    DisplayTask();  //This task draws the display every cycle
    //screenDebug();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CurrentBit1_Pin|CurrentBit2_Pin|CurrentBit3_Pin|CurrentBit4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, StepperStep_Pin|StepperDir_Pin|StepperEn_Pin|StepperSleep_Pin
                          |StepperRst_Pin|LCDcs_Pin|LCDrst_Pin|LCDa0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : CurrentBit1_Pin CurrentBit2_Pin CurrentBit3_Pin CurrentBit4_Pin */
  GPIO_InitStruct.Pin = CurrentBit1_Pin|CurrentBit2_Pin|CurrentBit3_Pin|CurrentBit4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : StepperStep_Pin StepperDir_Pin StepperEn_Pin StepperSleep_Pin
                           StepperRst_Pin LCDcs_Pin LCDrst_Pin LCDa0_Pin */
  GPIO_InitStruct.Pin = StepperStep_Pin|StepperDir_Pin|StepperEn_Pin|StepperSleep_Pin
                          |StepperRst_Pin|LCDcs_Pin|LCDrst_Pin|LCDa0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : RevBtn_Pin StopBtn_Pin FwdBtn_Pin EncBtn_Pin
                           EncB_Pin CurrBtn_Pin ModeBtn_Pin */
  GPIO_InitStruct.Pin = RevBtn_Pin|StopBtn_Pin|FwdBtn_Pin|EncBtn_Pin
                          |EncB_Pin|CurrBtn_Pin|ModeBtn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
