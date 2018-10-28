#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>


#define MSP430_ADDRESS 0x0F

int i2c_fd;

void ctrl_c(int sig){
	close(i2c_fd);
	exit(-1);
}

void OpenTransmission(int address){
	i2c_fd = open("/dev/i2c-1", O_RDWR);
	if(i2c_fd < 0){
		printf("Error opening i2c\n");
	}
	if(ioctl(i2c_fd, I2C_SLAVE,address) < 0){
		printf("Error at ioctl\n");
	}
}

void CloseTransmission(){
	close(i2c_fd);
}

struct analog
{
	unsigned char data[8];
	unsigned int ph_sensor[30];
	unsigned int turb_sensor[30];
	unsigned int tds_sensor[30];
	unsigned int bat_level[30];
};

int main(){
	
	struct analog sensors;
	int i;
	unsigned char start_comm = 0x55;


	OpenTransmission(MSP430_ADDRESS);
	
	//Order 30 conversions 

	for(i=0; i<30; i++){

		if(write(i2c_fd,&start_comm,1) < 0){
			printf("Error trying to write in i2c_fd.\n");
		}

		if(read(i2c_fd,sensors.data,8) < 0){
			printf("Error trying to read in i2c_fd.\n");
		}

		sensors.ph_sensor[i] = sensors.data[0];
		sensors.ph_sensor[i] += sensors.data[1] << 8;

		sensors.turb_sensor[i] = sensors.data[2];
		sensors.turb_sensor[i] += sensors.data[3] << 8;

		sensors.tds_sensor[i] = sensors.data[4];
		sensors.tds_sensor[i] += sensors.data[5] << 8;

		sensors.bat_level[i] = sensors.data[6];
		sensors.bat_level[i] += sensors.data[7] << 8;

	}

	CloseTransmission();

	for(i=0;i<30;i++){
		printf("Ph sensor: %d\n",sensors.ph_sensor[i]);
		printf("Turbidity sensor: %d\n",sensors.turb_sensor[i]);
		printf("Tds sensor: %d\n",sensors.tds_sensor[i]);
		printf("Battery level: %d\n",sensors.bat_level[i]);
	}




	return 0;
}
