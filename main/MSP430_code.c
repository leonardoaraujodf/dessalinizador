/*
 * MSP430 AS A I2C SLAVE.
 *
 * The MSP430 will work as a i2c slave using the following protocol:
 *
 * I. It will receive the byte 0x55, what means that the Raspberry Pi
 * needs data from the sensors.
 * II. Sensors order
 		Sensors[3] - PH sensor
		Sensors[2] - Turbidity sensor
		Sensors[1] - TDS sensor
		Sensors[0] - Battery Level
 * */

#include <msp430g2553.h>
#include <legacymsp430.h>

#define AD_IN 0x0F 
// 0x07 = 0000 1111
#define SLAVE_ADDR 0x0F
#define SCL BIT6
#define SDA BIT7
#define PH_SENSOR_DATA 0x01
#define TURB_SENSOR_DATA 0x02
#define TDS_SENSOR_DATA 0x03
#define BAT_LEVEL_DATA 0x04
#define BOMB_PIN BIT4
#define TURN_BOMB_ON 0x56
#define TURN_BOMB_OFF 0x57



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

void Transmit(unsigned int rdata,unsigned int length){
	volatile unsigned int i;
	unsigned char data[2];
	
	if(length > 1){	
		data[0] = rdata & 0xFF;
		data[1] = rdata >> 8;
	}
	else{
		data[0] = rdata & 0xFF;
	}

	for(i = 0; i < length; i++){
		while((IFG2 & UCB0TXIFG) == 0);
		UCB0TXBUF = data[i];
	}
}

void init_AD(void){
	ADC10AE0 |= AD_IN;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + MSC;
	ADC10CTL1 = INCH_3 + SHS_0 + ADC10DIV_0 + ADC10SSEL_3 + CONSEQ_1;	
	ADC10DTC1 = 0x04; //how many data will be transfered to the buffer
}

void init_Bomb(void){
	P1OUT &= ~BOMB_PIN;
	P1DIR |= BOMB_PIN;
}

void turn_Bomb(unsigned char value){
	if(value == TURN_BOMB_ON){
		P1OUT |= BOMB_PIN;
	}
	else{
		P1OUT &= ~BOMB_PIN;
	}
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	init_Bomb();
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
    			while((ADC10CTL1 & BUSY));
			ADC10CTL0 &= ~ENC;
    			ADC10SA = (unsigned int) sensors;
		
			/*
        		sensors[0] = 1010;
			sensors[1] = 1001;
			sensors[2] = 1002;
			sensors[3] = 256;
			*/

        		while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
		
			Transmit(sensors[3],2);
			Transmit(sensors[2],2);
			Transmit(sensors[1],2);
			Transmit(sensors[0],2);

        		UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
    		}
		else if(UCB0RXBUF == TURN_BOMB_ON){
			
			turn_Bomb(TURN_BOMB_ON);

        		while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition

			Transmit(TURN_BOMB_ON,1);
        		UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
			

		}
		else if(UCB0RXBUF == TURN_BOMB_OFF){

			turn_Bomb(TURN_BOMB_OFF);

        		while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition

			Transmit(TURN_BOMB_OFF,1);
        		UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
		}
	}
    IFG2 &= ~(UCB0TXIFG|UCB0RXIFG);
}
