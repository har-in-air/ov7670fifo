#ifndef OV7670_H_
#define OV7670_H_



#define OV7670_I2C_ADDR  0x21


#define REG_GAIN        0x00    /* Gain lower 8 bits (rest in vref) */
#define REG_BLUE        0x01    /* blue gain */
#define REG_RED         0x02    /* red gain */
#define REG_VREF        0x03    /* Pieces of GAIN, VSTART, VSTOP */
#define REG_COM1        0x04    /* Control 1 */
#define COM1_CCIR656    0x40    /* CCIR656 enable */
#define REG_BAVE        0x05    /* U/B Average level */
#define REG_GbAVE       0x06    /* Y/Gb Average level */
#define REG_AECHH       0x07    /* AEC MS 5 bits */
#define REG_RAVE        0x08    /* V/R Average level */
#define REG_COM2        0x09    /* Control 2 */
	#define COM2_SSLEEP     0x10    /* Soft sleep mode */
#define REG_PID         0x0a    /* Product ID MSB */
#define REG_VER         0x0b    /* Product ID LSB */
#define REG_COM3        0x0c    /* Control 3 */
	#define COM3_SWAP       0x40    /* Byte swap */
	#define COM3_SCALEEN    0x08    /* Enable scaling */
	#define COM3_DCWEN      0x04    /* Enable downsamp/crop/window */
#define REG_COM4        0x0d    /* Control 4 */
#define REG_COM5        0x0e    /* All "reserved" */
#define REG_COM6        0x0f    /* Control 6 */
#define REG_AECH        0x10    /* More bits of AEC value */
#define REG_CLKRC       0x11    /* Clocl control */
#define CLK_EXT         0x40    /* Use external clock directly */
#define CLK_SCALE       0x3f    /* Mask for internal clock scale */
#define REG_COM7        0x12    /* Control 7 */
	#define COM7_RESET      0x80    /* Register reset */
	#define COM7_FMT_MASK   0x38
	#define COM7_FMT_VGA    0x00
	#define COM7_FMT_CIF    0x20    /* CIF format */
	#define COM7_FMT_QVGA   0x10    /* QVGA format */
	#define COM7_FMT_QCIF   0x08    /* QCIF format */
	#define COM7_RGB        0x04    /* bits 0 and 2 - RGB format */
	#define COM7_YUV        0x00    /* YUV */
	#define COM7_BAYER      0x01    /* Bayer format */
	#define COM7_PBAYER     0x05    /* "Processed bayer" */
#define REG_COM8        0x13    /* Control 8 */
	#define COM8_FASTAEC    0x80    /* Enable fast AGC/AEC */
	#define COM8_AECSTEP    0x40    /* Unlimited AEC step size */
	#define COM8_BFILT      0x20    /* Band filter enable */
	#define COM8_AGC        0x04    /* Auto gain enable */
	#define COM8_AWB        0x02    /* White balance enable */
	#define COM8_AEC        0x01    /* Auto exposure enable */
#define REG_COM9        0x14    /* Control 9  - gain ceiling */
#define REG_COM10       0x15    /* Control 10 */
	#define COM10_HSYNC     0x40    /* HSYNC instead of HREF */
	#define COM10_PCLK_HB   0x20    /* Suppress PCLK on horiz blank */
	#define COM10_HREF_REV  0x08    /* Reverse HREF */
	#define COM10_VS_LEAD   0x04    /* VSYNC on clock leading edge */
	#define COM10_VS_NEG    0x02    /* VSYNC negative */
	#define COM10_HS_NEG    0x01    /* HSYNC negative */
#define REG_HSTART      0x17    /* Horiz start high bits */
#define REG_HSTOP       0x18    /* Horiz stop high bits */
#define REG_VSTART      0x19    /* Vert start high bits */
#define REG_VSTOP       0x1a    /* Vert stop high bits */
#define REG_PSHFT       0x1b    /* Pixel delay after HREF */
#define REG_MIDH        0x1c    /* Manuf. ID high */
#define REG_MIDL        0x1d    /* Manuf. ID low */
#define REG_MVFP        0x1e    /* Mirror / vflip */
#define MVFP_MIRROR     0x20    /* Mirror image */
#define MVFP_FLIP       0x10    /* Vertical flip */
#define REG_AEW         0x24    /* AGC upper limit */
#define REG_AEB         0x25    /* AGC lower limit */
#define REG_VPT         0x26    /* AGC/AEC fast mode op region */
#define REG_HSYST       0x30    /* HSYNC rising edge delay */
#define REG_HSYEN       0x31    /* HSYNC falling edge delay */
#define REG_HREF        0x32    /* HREF pieces */
#define REG_TSLB        0x3a    /* lots of stuff */
	#define TSLB_YLAST      0x04    /* UYVY or VYUY - see com13 */
#define REG_COM11       0x3b    /* Control 11 */
	#define COM11_NIGHT     0x80    /* NIght mode enable */
	#define COM11_NMFR      0x60    /* Two bit NM frame rate */
	#define COM11_HZAUTO    0x10    /* Auto detect 50/60 Hz */
	#define COM11_50HZ      0x08    /* Manual 50Hz select */
	#define COM11_EXP       0x02
#define REG_COM12       0x3c    /* Control 12 */
	#define COM12_HREF      0x80    /* HREF always */
#define REG_COM13       0x3d    /* Control 13 */
	#define COM13_GAMMA     0x80    /* Gamma enable */
	#define COM13_UVSAT     0x40    /* UV saturation auto adjustment */
	#define COM13_UVSWAP    0x01    /* V before U - w/TSLB */
#define REG_COM14       0x3e    /* Control 14 */
	#define COM14_DCWEN     0x10    /* DCW/PCLK-scale enable */
#define REG_EDGE        0x3f    /* Edge enhancement factor */
#define REG_COM15       0x40    /* Control 15 */
	#define COM15_R10F0     0x00    /* Data range 10 to F0 */
	#define COM15_R01FE     0x80    /*            01 to FE */
	#define COM15_R00FF     0xc0    /*            00 to FF */
	#define COM15_RGB565    0x10    /* RGB565 output */
	#define COM15_RGB555    0x30    /* RGB555 output */
#define REG_COM16       0x41    /* Control 16 */
	#define COM16_AWBGAIN   0x08    /* AWB gain enable */
#define REG_COM17       0x42    /* Control 17 */
	#define COM17_AECWIN    0xc0    /* AEC window - must match COM4 */
	#define COM17_CBAR      0x08    /* DSP Color bar */
#define REG_CMATRIX_BASE 0x4f
#define CMATRIX_LEN 6
#define REG_CMATRIX_SIGN 0x58
#define REG_BRIGHT      0x55    /* Brightness */
#define REG_CONTRAS     0x56    /* Contrast control */
#define REG_GFIX        0x69    /* Fix gain control */
#define REG_REG76       0x76    /* OV's name */
#define R76_BLKPCOR     0x80    /* Black pixel correction enable */
#define R76_WHTPCOR     0x40    /* White pixel correction enable */
#define REG_RGB444      0x8c    /* RGB 444 control */
#define R444_ENABLE     0x02    /* Turn on RGB444, overrides 5x5 */
#define R444_RGBX       0x01    /* Empty nibble at end */
#define REG_HAECC1      0x9f    /* Hist AEC/AGC control 1 */
#define REG_HAECC2      0xa0    /* Hist AEC/AGC control 2 */
#define REG_BD50MAX     0xa5    /* 50hz banding step limit */
#define REG_HAECC3      0xa6    /* Hist AEC/AGC control 3 */
#define REG_HAECC4      0xa7    /* Hist AEC/AGC control 4 */
#define REG_HAECC5      0xa8    /* Hist AEC/AGC control 5 */
#define REG_HAECC6      0xa9    /* Hist AEC/AGC control 6 */
#define REG_HAECC7      0xaa    /* Hist AEC/AGC control 7 */
#define REG_BD60MAX     0xab    /* 60hz banding step limit */

////////// OV7670+AL422 module interface to ESP32 pins  ///////

#define pinSCL		19  // ov7670 i2c clock
#define pinSDA		18  // ov7670 i2c data
#define pinVSYNC  	22 	// ov7670 vsync
#define pinRST		5 	// ov7670 reset

#define pinRCK    	12 	// fifo read clock
#define pinWR     	15 	// fifo write enable
#define pinRRST   	13 	// fifo read pointer reset to beginning of frame

#define pinD0		14  // fifo read data pins D0 ... D7
#define pinD1		2
#define pinD2		27
#define pinD3		4
#define pinD4		25
#define pinD5		16
#define pinD6		26
#define pinD7		17

#define CAM_RST_1   	{GPIO.out_w1ts = (1 << pinRST);}
#define CAM_RST_0   	{GPIO.out_w1tc = (1 << pinRST);}
#define CAM_VSYNCSET	(GPIO.in & (1 << pinVSYNC))

#define FIFO_RRST_1   	{GPIO.out_w1ts = (1 << pinRRST);}
#define FIFO_RRST_0   	{GPIO.out_w1tc = (1 << pinRRST);}

#define FIFO_RCK_1   	{GPIO.out_w1ts = (1 << pinRCK);}
#define FIFO_RCK_0   	{GPIO.out_w1tc = (1 << pinRCK);}

#define FIFO_WR_1   	{GPIO.out_w1ts = (1 << pinWR);}
#define FIFO_WR_0   	{GPIO.out_w1tc = (1 << pinWR);}

#define FIFO_D0SET     	(GPIO.in & (1 << pinD0))
#define FIFO_D1SET     	(GPIO.in & (1 << pinD1))
#define FIFO_D2SET     	(GPIO.in & (1 << pinD2))
#define FIFO_D3SET     	(GPIO.in & (1 << pinD3))
#define FIFO_D4SET     	(GPIO.in & (1 << pinD4))
#define FIFO_D5SET     	(GPIO.in & (1 << pinD5))
#define FIFO_D6SET     	(GPIO.in & (1 << pinD6))
#define FIFO_D7SET     	(GPIO.in & (1 << pinD7))


#define NOP() { asm volatile ("nop"); }

typedef struct regval_list_ {
  uint8_t addr;
  uint8_t value;
} regval_list;

//#define VGA_YUV  
//#define VGA_RGB444  
//#define VGA_RGB565  
//#define VGA_MONO

#define QVGA_YUV  
//#define QVGA_RGB565  
//#define QVGA_MONO

#if defined(QVGA_RGB565) || defined(QVGA_YUV)
	#define LINE_BYTES   	(320*2)
	#define NUM_LINES		240
#elif defined(VGA_RGB444) || defined(VGA_YUV) || defined(VGA_RGB565)
	#define LINE_BYTES   	(640*2)
	#define NUM_LINES		480
#elif defined(VGA_MONO)
	#define LINE_BYTES  	640
	#define NUM_LINES		480
#elif defined(QVGA_MONO)
	#define LINE_BYTES  	320
	#define NUM_LINES		240
#endif

extern	uint8_t imageLine[LINE_BYTES];	

void OV7670_init();
void OV7670_rrst(void);
void OV7670_captureFrame(void);
void OV7670_readLine(void);
void OV7670_readLineMono(void);


#endif
