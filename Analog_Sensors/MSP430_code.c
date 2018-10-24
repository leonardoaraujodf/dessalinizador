/*
 * MSP430 AS A I2C SLAVE.
 *
 * The MSP430 will work as a i2c slave using the following protocol:
 *
 * I. It will receive the byte 0x55, what means that the Raspberry Pi
 * needs data from the sensors.
 * II. Sensors order
 		Sensors[0] - PH sensor
		Sensors[1] - Turbidity sensor
		Sensors[2] - TDS sensor
		Sensors[3] - Battery Level
 * */

#include <msp430g2553.h>
#include <legacymsp430.h>

#define AD_IN BIT0|BIT1|BIT2|BIT3
#define AD_INCH INCH_3

#define SLAVE_ADDR 0x0F
#define SCL BIT6
#define SDA BIT7
#define PH_SENSOR_DATA 0x01
#define TURB_SENSOR_DATA 0x02
#define TDS_SENSOR_DATA 0x03
#define BAT_LEVEL_DATA 0x04

unsigned int sensors[4];
unsigned int ph_sensor;
unsigned int turb_sensor;
unsigned int tds_sensor;
unsigned int bat_level;

void init_I2C(void){
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL1 &= ~UCTR;
    UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Master, synchronous mode
    UCB0I2COA = SLAVE_ADDR;                   //|UCGCEN;
    P1SEL |= SCL | SDA;                       // Assign I2C pins to USCI_B0
    P1SEL2|= SCL | SDA;                       // Assign I2C pins to USCI_B0
    UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    IE2 |= UCB0RXIE;
}

void Transmit(unsigned int rdata){
	volatile unsigned int i;
	unsigned char data[2];
		
	data[0] = rdata & 0xFF;
	data[1] = rdata >> 8;

	for(i = 0; i < 2; i++){
		while((IFG2 & UCB0TXIFG) == 0);
		UCB0TXBUF = data[i];
		}
}

void init_AD(void){
	ADC10AE0 = AD_IN;
	ADC10CTL1 = AD_INCH + SHS_0 + ADC10DIV_0 + ADC10SSEL_3 + CONSEQ_1;
	ADC10CTL0 = SREF_0 + ADC10SHT_0 + ADC10IE + ADC10ON;
	ADC10DTC1 = 0x04; //how many data will be transfered to the buffer
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	init_AD();
	init_I2C();
	_BIS_SR(LPM0_bits + GIE);
	
	return 0;
}

interrupt(USCIAB0TX_VECTOR) USCIAB0TX_ISR(void)
{
	if(IFG2 & UCB0RXIFG){
    	if(UCB0RXBUF == 0x55){

    		ADC10CTL0 |= ENC + ADC10SC;
    		while((ADC10CTL0 & ADC10IFG) == 0);

    		ADC10SA = (unsigned int) sensors;

        	//ph_sensor = 1010;
			//turb_sensor = 1001;
			//tds_sensor = 1002;
			//bat_level = 256;
        	while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
        	Transmit(sensors[0]);
			Transmit(sensors[1]);
			Transmit(sensors[2]);
			Transmit(sensors[3]);
        	UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
    	}
	}
    IFG2 &= ~(UCB0TXIFG|UCB0RXIFG);
}
