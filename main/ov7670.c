#include <stdio.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "wiring.h"
#include "sccb.h"
#include "twi.h"
#include "ov7670.h"
#include "ov7670reg.h"


static void OV7670_setupPins();
static void OV7670_reset();
static void OV7670_writeRegList(regval_list *list);
static void delayNOPs(int n);

uint8_t imageLine[LINE_BYTES];	
static uint8_t i2cSlaveAddr;	

static const char* TAG = "OV7670";

static void delayNOPs(int n) {
    while (n--) NOP();
    }
	
	
static void OV7670_setupPins() {
	pinMode(pinRST, OUTPUT);
	CAM_RST_1;
	pinMode(pinRCK, OUTPUT);
	FIFO_RRST_1;
	pinMode(pinWR, OUTPUT);
	FIFO_WR_0;
	pinMode(pinRRST, OUTPUT);
	FIFO_RCK_1;

	pinMode(pinVSYNC, INPUT);			
	pinMode(pinD0, INPUT_PULLUP);
	pinMode(pinD1, INPUT_PULLUP);
	pinMode(pinD2, INPUT_PULLUP);
	pinMode(pinD3, INPUT_PULLUP);
	pinMode(pinD4, INPUT_PULLUP);
	pinMode(pinD5, INPUT_PULLUP);
	pinMode(pinD6, INPUT_PULLUP);
	pinMode(pinD7, INPUT_PULLUP);
  
    ESP_LOGD(TAG, "Initializing SSCB");
    SCCB_Init(pinSDA, pinSCL);
	}

static void OV7670_reset(void) {	
    ESP_LOGD(TAG, "Reset camera");
	CAM_RST_0;
	delay(5);
	CAM_RST_1;
    delay(10);
    ESP_LOGD(TAG, "Searching for camera address");
    i2cSlaveAddr = SCCB_Probe();
    ESP_LOGD(TAG, "Detected camera at address=0x%02x", i2cSlaveAddr);
    uint8_t PID = SCCB_Read(i2cSlaveAddr, REG_PID);
    uint8_t VER = SCCB_Read(i2cSlaveAddr, REG_VER);
    uint8_t MIDL = SCCB_Read(i2cSlaveAddr, REG_MIDL);
    uint8_t MIDH = SCCB_Read(i2cSlaveAddr, REG_MIDH);
    delay(10);
    ESP_LOGD(TAG, "Camera PID=0x%02x VER=0x%02x MIDL=0x%02x MIDH=0x%02x", PID, VER, MIDH, MIDL);
	}


// write a list of register settings. Address 0xff stops the process.
static void OV7670_writeRegList(regval_list *vals){
	while (vals->addr != 0xff) {
		SCCB_Write(i2cSlaveAddr, vals->addr, vals->value);
		delay(5);
		vals++;
		}
	}
  


#if defined(VGA_RGB444)
#include "vgargb444.h"
  
int OV7670_init_VGA_RGB444(void){
  FIFO_RRST_1;
  FIFO_RCK_1;

  if (SCCB_Read(i2cSlaveAddr, REG_PID) != 0x76) {
    return 0;
    }
  SCCB_Write(i2cSlaveAddr, REG_COM7, 0x80); // reset to defaults
  delay(100);
  OV7670_writeRegList(vgargb444_regs);
}

#elif defined(VGA_YUV)
//#include "vgayuv.h"
#include "defaultregs.h"

int OV7670_init_VGA_YUV(void){
  FIFO_RRST_1;
  FIFO_RCK_1;

  if (SCCB_Read(i2cSlaveAddr, REG_PID) != 0x76) {
    return 0;
    }
  SCCB_Write(i2cSlaveAddr, REG_COM7, 0x80); // reset to defaults
  delay(100);
  //OV7670_writeRegList(vgayuv_regs);
  //OV7670_writeRegList(vga_regs);
  OV7670_writeRegList(ov7670_default_regs);
  //OV7670_writeRegList(yuv422_ov7670);
  //OV7670_writeRegList(vga_ov7670);


  //OV7670_writeRegList(ov7670_fmt_yuv422);
  //OV7670_writeRegList(colorsettings_regs);
  return 1;
  }
  
#elif defined(QVGA_YUV)
#include "qvgayuv.h"
  
int OV7670_init_QVGA_YUV(void){
  FIFO_RRST_1;
  FIFO_RCK_1;

  if (SCCB_Read(i2cSlaveAddr, REG_PID) != 0x76) {
    return 0;
    }

  SCCB_Write(i2cSlaveAddr, REG_COM7, 0x80); // reset to defaults
  delay(100);
  OV7670_writeRegList(qvgayuv_regs);
  //SCCB_Write(OV7670_I2C_ADDR, 0x70, 0xf0);// test bars
  //delay(1);
  //SCCB_Write(OV7670_I2C_ADDR, 0x71, 0xf0);// test bars  
  //delay(1);
  return 1;
  }

#elif defined(QVGA_RGB565)
#include "qvgargb565.h"
  
int OV7670_init_QVGA_RGB565(void){
  FIFO_RRST_1;
  FIFO_RCK_1;

  if (SCCB_Read(i2cSlaveAddr, REG_PID) != 0x76) {
    return 0;
    }
  SCCB_Write(i2cSlaveAddr, REG_COM7, 0x80); // reset to defaults
  delay(100);
  OV7670_writeRegList(qvgargb565_regs);
  return 1;
  }
#endif


void OV7670_init() {  
	OV7670_setupPins();
    OV7670_reset();
#if defined(QVGA_RGB565)    
    if (OV7670_init_QVGA_RGB565()) {
      ESP_LOGI(TAG,"OV7670 QVGA RGB565 init OK");
      }
#elif defined(QVGA_YUV)      
    if (OV7670_init_QVGA_YUV()) {
      ESP_LOGI(TAG,"OV7670 QVGA YUV init OK");
      }
#endif	
	}

// read one line of pixels from the fifo image
void OV7670_readLine(void) {
  uint8_t* p = imageLine;
  uint8_t val;
  for (int n = 0; n < LINE_BYTES; n++) {
    FIFO_RCK_0;
	val = 0;
	if (FIFO_D0SET) val |= 0x01;
	if (FIFO_D1SET) val |= 0x02;
	if (FIFO_D2SET) val |= 0x04;
	if (FIFO_D3SET) val |= 0x08;
	if (FIFO_D4SET) val |= 0x10;
	if (FIFO_D5SET) val |= 0x20;
	if (FIFO_D6SET) val |= 0x40;
	if (FIFO_D7SET) val |= 0x80;	
    *p++ = val;
    FIFO_RCK_1;
    }
  }
  
// read one line of graylevel pixels from the fifo image
// assumes YUV image, i.e. YUYV pixel order
void OV7670_readLineMono(void) {
  uint8_t* p = imageLine;
  uint8_t val;
  for (int n = 0; n < LINE_BYTES; n++) {
    FIFO_RCK_0;
	val = 0;
	if (FIFO_D0SET) val |= 0x01;
	if (FIFO_D1SET) val |= 0x02;
	if (FIFO_D2SET) val |= 0x04;
	if (FIFO_D3SET) val |= 0x08;
	if (FIFO_D4SET) val |= 0x10;
	if (FIFO_D5SET) val |= 0x20;
	if (FIFO_D6SET) val |= 0x40;
	if (FIFO_D7SET) val |= 0x80;	
    if (!(n & 1)) *p++ = val;
    FIFO_RCK_1;
    }
  }  


// Capture frame from OV7670 sensor into AL422 fifo. Works with negative vsync
void OV7670_captureFrame(void){
  while (CAM_VSYNCSET);     // wait for an old frame to end
  while (!CAM_VSYNCSET);    // wait for a new frame to start
  FIFO_WR_1;               // enable writing to fifo
  while (CAM_VSYNCSET);     // wait for the current frame to end
  FIFO_WR_0;              // disable writing to fifo
  delayNOPs(150);
  FIFO_RRST_1;
  }


 
// Reset the AL422 fifo read pointer to beginning of image
void OV7670_rrst(void){
  FIFO_RCK_1;
  NOP();
  FIFO_RRST_0;
  NOP();
  FIFO_RCK_0;
  NOP();
  FIFO_RCK_1;
  NOP();
  FIFO_RRST_1;
  NOP();
  FIFO_RCK_0;
  NOP();
  FIFO_RCK_1;
  NOP();
  }

