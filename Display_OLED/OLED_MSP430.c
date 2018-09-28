#include <msp430g2553.h>
#include <legacymsp430.h>

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;


#define SCL BIT6
#define SDA BIT7
#define LED BIT0

#define OLED_I2C_ADDRESS  0x3C

#define OLED_CONTROL_BYTE_CMD_SINGLE  0x80
#define OLED_CONTROL_BYTE_CMD_STREAM  0x00
#define OLED_CONTROL_BYTE_DATA_STREAM 0x40

//Fundamental Commands (pg. 28)
#define OLED_CMD_SET_CONTRAST           0x81 // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 + 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 + 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0x22
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

// NOP
#define OLED_CMD_NOP                    0xE3

uint16_t i;
uint8_t j,c,s,b;

unsigned char *PTXData;
unsigned char TXByteCtr;

const unsigned char init_cmd_array[] = {

    OLED_CONTROL_BYTE_CMD_STREAM,

    // Follow instructions on pg.64 of the dataSheet for software configuration of the SSD1306
    // Turn the Display OFF
    OLED_CMD_DISPLAY_OFF,
    // Set mux ration tp select max number of rows - 64
    OLED_CMD_SET_MUX_RATIO,
    0x3F,
    // Set the display offset to 0
    OLED_CMD_SET_DISPLAY_OFFSET,
    0x00,
    // Display start line to 0
    OLED_CMD_SET_DISPLAY_START_LINE,

    // Mirror the x-axis. In case you set it up such that the pins are north.
    // Wire.write(0xA0); - in case pins are south - default
    OLED_CMD_SET_SEGMENT_REMAP,

    // Mirror the y-axis. In case you set it up such that the pins are north.
    // Wire.write(0xC0); - in case pins are south - default
    OLED_CMD_SET_COM_SCAN_MODE,

    // Default - alternate COM pin map
    OLED_CMD_SET_COM_PIN_MAP,
    0x12,
    // set contrast
    OLED_CMD_SET_CONTRAST,
    0x7F,
    // Set display to enable rendering from GDDRAM (Graphic Display Data RAM)
    OLED_CMD_DISPLAY_RAM,
    // Normal mode!
    OLED_CMD_DISPLAY_NORMAL,
    // Default oscillator clock
    OLED_CMD_SET_DISPLAY_CLK_DIV,
    0x80,
    // Enable the charge pump
    OLED_CMD_SET_CHARGE_PUMP,
    0x14,
    // Set precharge cycles to high cap type
    OLED_CMD_SET_PRECHARGE,
    0x22,
    // Set the V_COMH deselect volatage to max
    OLED_CMD_SET_VCOMH_DESELCT,
    0x30,
    // Horizonatal addressing mode - same as the KS108 GLCD
    OLED_CMD_SET_MEMORY_ADDR_MODE,
    0x00,
    // Turn the Display ON
    OLED_CMD_DISPLAY_ON

};

const unsigned char WriteInit[] =
{
     OLED_CONTROL_BYTE_CMD_STREAM,
     OLED_CMD_SET_COLUMN_RANGE,
     0x00,
     0x7F,
     OLED_CMD_SET_PAGE_RANGE,
     0x00,
     0x07
};

uint8_t TX1[] = {
      0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
      0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
};

uint8_t TX2[] = {
      0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00,
      0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00
};


uint8_t TX3[]={
      0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
      0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81
};

const unsigned char SendData[] = {OLED_CONTROL_BYTE_DATA_STREAM};


void WriteOLED(const unsigned char* data, unsigned char size){
   PTXData = (unsigned char*)data;
   TXByteCtr = size;
   while(UCB0CTL1 & UCTXSTP);
   UCB0CTL1 |= UCTR + UCTXSTT;
}

void OLEDInit(){
    WriteOLED(init_cmd_array, sizeof(init_cmd_array));
    __delay_cycles(160000);
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;
	
	P1DIR |= LED;
	P1SEL |= SDA + SCL; //Assign I2C pins to USCI_B0
	P1SEL2 |= SDA + SCL; //Assign I2C pins to USCI_B0

	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
	UCB0CTL1 = UCSSEL_2 + UCSWRST;
	UCB0BR0 = 12;
	UCB0BR1 = 0;
	UCB0I2CSA = 0x3C;
	UCB0CTL1 &= ~UCSWRST;
	IE2 |= UCB0TXIE;

	__enable_interrupt();
  OLEDInit();  

	while(1){

	    WriteOLED(WriteInit,sizeof(WriteInit));
        for(i = 0; i<64; i++){
          WriteOLED(SendData,1);
          WriteOLED(TX2,sizeof(TX2));
      }
        
      __delay_cycles(160000);
	}

	return 0;
}

interrupt(USCIAB0TX_VECTOR) USCIAB0TX_ISR(void){

      if (TXByteCtr)                            // Check TX byte counter
      {
        UCB0TXBUF = *PTXData++;                 // Load TX buffer
        TXByteCtr--;                            // Decrement TX byte counter
      }
      else
      {
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
      }

    P1OUT ^= LED;
}
