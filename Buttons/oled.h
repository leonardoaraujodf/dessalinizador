#ifndef OLED_H
#define OLED_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

//For the water bomb
#define TURN_BOMB_ON 0x56
#define TURN_BOMB_OFF 0x57

//UART file for the GPS module
#define TTY "/dev/ttyS0"

//File for the temperature sensor - One wire protocol
#define TEMP_PATH "/sys/bus/w1/devices/28-01131b9500b6/w1_slave"

//I2C file descriptor
int i2c_fd;

//MSP430 i2c address
#define MSP430_ADDRESS 0x0F

//OLED display i2c address
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

// GPS Formatting
#define MAX_NUMBER_OF_FIELDS 13
#define MAX_STRING_LENGTH 50
#define LATITUDE_INDEX 2
#define LONGITUDE_INDEX 4
#define VERTICAL_DIRECTION_INDEX 3
#define HORIZONTAL_DIRECTION_INDEX 5
#define X 0
#define Y 1

// Sample file path
#define NEW_SAMPLE_PATH "sample.txt"

//GPIO Definitions

#define ON_BTN 26
#define OFF_BTN 19
#define ON_GPIO_fd 0
#define OFF_GPIO_fd 1
#define NUMBER_GPIO_OUTPUTS 2
#define NUMBER_THREADS NUMBER_GPIO_OUTPUTS

int GPIO_fd[NUMBER_GPIO_OUTPUTS];
pthread_t GPIO_tid[NUMBER_THREADS];
pthread_attr_t attr[NUMBER_THREADS];


struct analog
{
	unsigned char data[8];
	unsigned int ph_sensor[30];
	unsigned int turb_sensor[30];
	unsigned int tds_sensor[30];
	unsigned int bat_level[30];
};

void WriteCmdOLED(const unsigned char* data, unsigned char size);
void WriteDataOLED(const unsigned char* data, unsigned int size);
void OLEDInit(void);
void OpenTransmission(int address);
void CloseTransmission(void);
void get_Battlevel(int *level);
float get_Temp(void);
float get_TDS(int *values);
float get_Turb(int *values);
float get_PH(int *values);
int compare (const void * a, const void * b);
float get_median(int *values);
int get_localization(char *gps_data);
void turn_Bomb(unsigned char value);
void split_by(char *token, char *string, char array[MAX_NUMBER_OF_FIELDS][MAX_STRING_LENGTH]);
double convert_dms_cordinate_to_decimal(double dms_coordinate);
int compute_direction(char *raw_direction);
void format_coordinate(char coordinate, char gps_info[MAX_NUMBER_OF_FIELDS][MAX_STRING_LENGTH], char coordinates[2][MAX_STRING_LENGTH]);
void format_coordinates(char gps_info[MAX_NUMBER_OF_FIELDS][MAX_STRING_LENGTH], char coordinates[2][MAX_STRING_LENGTH]);
void get_collection_date(char *formatted_time);
void create_new_sample_file(float turbidity, float ph, float temperature, float tds, char *x_coordinate, char *y_coordinate);
void ctrl_c(int sig);
void *GPIO_handler(void *param);
void GPIO_setup(int GPIO_num);
void GPIO_free(int GPIO_num);
void GPIO_open(int GPIO_fd_num,int GPIO_num);
void GPIO_close(int GPIO_fd_num);
int GPIO_get(int GPIO_fd_num);
void get_samples(void);

#endif
