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
 **/

#include <msp430g2553.h>
#include <legacymsp430.h>

#define AD_IN 0x0F
// 0x07 = 0000 1111
#define SLAVE_ADDR 0x0F
#define SCL BIT6
#define SDA BIT7


#define START_SAMPLING 0x55

#define PH_SENSOR_DATA 0x01
#define TURB_SENSOR_DATA 0x02
#define TDS_SENSOR_DATA 0x03
#define BAT_LEVEL_DATA 0x04
#define LEVEL_SENSOR BIT4
#define FLOW_SENSOR BIT5

//Definitions for the water pump that goes with the desalinator
#define PUMP_PIN BIT0 //for P2IO
#define TURN_PUMP_ON 0x56
#define TURN_PUMP_OFF 0x57

//Definitions for the motor that goes with the top valve in the samples reservoir
#define SAMPLES_MOTOR BIT1
#define SAMPLES_MOTOR_ON 0x58
#define SAMPLES_MOTOR_OFF 0x59

//Definitions for the top valve in the samples reservoir
#define TOP_VALVE BIT2
#define TOP_VALVE_ON 0x5A
#define TOP_VALVE_OFF 0x5B

//Definitions for the low valve in the samples reservoir. The RPI will also
//send this byte to say that the
//samples were taken and the water can be thrown off.

#define LOW_VALVE BIT3
#define LOW_VALVE_ON 0x5C
#define LOW_VALVE_OFF 0x5D

//Communication to say if the level sensor detects the water level
//that can make the sensors work correctly
#define DETECT_LEVEL_SENSOR 0x5E
#define LEVEL_SENSOR_OFF  0x5F
#define LEVEL_SENSOR_ON 0x60


unsigned int sensors[4];
unsigned int ph_sensor;
unsigned int turb_sensor;
unsigned int tds_sensor;
unsigned int bat_level;

//This variable is used to describe if the level sensor detected the water level
//If this variable is zero, it says that the water level is not good enough
//Otherwise, it will say that the water level is OK to take samples.

volatile unsigned int level_sensor = 0;

void init_I2C(void);
void Transmit(unsigned int rdata,unsigned int length);
void init_AD(void);
void init_Pump(void);
void turn_Pump(unsigned char value);
void init_SamplesMotor(void);
void turn_SamplesMotor(unsigned char value);
void init_TopValve(void);
void turn_TopValve(unsigned char value);
void init_LowValve(void);
void turn_LowValve(unsigned char value);
void setupLevelSensor(void);
void treat_DataReceived(void);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	init_Pump();
	init_SamplesMotor();
	init_TopValve();
	init_LowValve();
	init_AD();
	init_I2C();
	_BIS_SR(LPM0_bits + GIE);

	return 0;
}

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

void init_Pump(void){
	P2OUT &= ~PUMP_PIN;
	P2DIR |= PUMP_PIN;
}

void turn_Pump(unsigned char value){
	if(value == TURN_PUMP_ON){
		P2OUT |= PUMP_PIN;
	}
	else{
		P2OUT &= ~PUMP_PIN;
	}
}

void init_SamplesMotor(void){
  P2OUT &= ~SAMPLES_MOTOR;
  P2DIR |= SAMPLES_MOTOR;
}

void turn_SamplesMotor(unsigned char value){
  if(value == SAMPLES_MOTOR_ON){
    P2OUT |= SAMPLES_MOTOR;
  }
  else{
    P2OUT &= ~SAMPLES_MOTOR;
  }
}

void init_TopValve(void){
  P2OUT |= TOP_VALVE; //when the valve is 1, it won't let water pass
  P2DIR |= TOP_VALVE;
}

void turn_TopValve(unsigned char value){
  if(value == TOP_VALVE_ON){
		P2OUT &= ~TOP_VALVE; //top valve is on
  }
  else{
    P2OUT |= TOP_VALVE; //top valve is off
  }
}

void init_LowValve(){
  P2OUT |= LOW_VALVE; //low valve is initialized in off
  P2DIR |= LOW_VALVE;
}

void turn_LowValve(unsigned char value){
  if(value == LOW_VALVE_ON){
		P2OUT &= ~LOW_VALVE; //low valve is on
  }
  else{
    P2OUT |= LOW_VALVE; //low valve is off
  }
}

void setupLevelSensor(void){
  P1DIR &= ~LEVEL_SENSOR; //level sensor as input
  P1IES |= LEVEL_SENSOR; //falling edge from HIGH TO LOW
  P1IE |= LEVEL_SENSOR; //Interrupt on Input Pin LEVEL_SENSOR
  P1IFG &= ~LEVEL_SENSOR; //Interrupt flag cleared
}

void treat_DataReceived(void){
	if(UCB0RXBUF == START_SAMPLING){
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
	else if(UCB0RXBUF == TURN_PUMP_ON){
		turn_TopValve(TOP_VALVE_ON);
		turn_SamplesMotor(SAMPLES_MOTOR_ON);

		turn_Pump(TURN_PUMP_ON);
		while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
		Transmit(TURN_PUMP_ON,1);
		UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
	}
	else if(UCB0RXBUF == TURN_PUMP_OFF){
		turn_Pump(TURN_PUMP_OFF);
		while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
		Transmit(TURN_PUMP_OFF,1);
		UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
	}
	else if(UCB0RXBUF == DETECT_LEVEL_SENSOR){
		if(level_sensor == 0){
			while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
			Transmit(LEVEL_SENSOR_OFF,1);
			UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
		}
		else{
			while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
			Transmit(LEVEL_SENSOR_ON,1);
			level_sensor = 0; //RPI was advised about the level sensor, now turn the
			//variable off for another sample in the future.
			UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
		}
	}
	else if(UCB0RXBUF == LOW_VALVE_ON){
		turn_LowValve(LOW_VALVE_ON);
		while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
		Transmit(LOW_VALVE_ON,1);
		UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
	}
	else if(UCB0RXBUF == LOW_VALVE_OFF){
		turn_LowValve(LOW_VALVE_OFF);
		while( (UCB0STAT & UCSTTIFG)==0); // wait master for the start condition
		Transmit(LOW_VALVE_OFF,1);
		UCB0STAT &= ~(UCSTPIFG | UCSTTIFG);
	}
}

interrupt(USCIAB0TX_VECTOR) USCIAB0TX_ISR(void){
  if(IFG2 & UCB0RXIFG){
    treat_DataReceived();
	}
  IFG2 &= ~(UCB0TXIFG|UCB0RXIFG);
}

interrupt(PORT1_VECTOR) Port_1(void){

  while((P1IN & LEVEL_SENSOR) == 0){
    turn_TopValve(TOP_VALVE_OFF);
    turn_SamplesMotor(SAMPLES_MOTOR_OFF);
		level_sensor = 1; //level sensor variable is 1, it means that the samples
//could be taken, and RPI should be advised.
	}
  P1IFG &= ~LEVEL_SENSOR;
}
