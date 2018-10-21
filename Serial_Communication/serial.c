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

int main(){
	
	unsigned char data[8];
	unsigned int ph_sensor;
	unsigned int turb_sensor;
	unsigned int tds_sensor;
	unsigned int bat_level;

	int i;
	unsigned char start_comm = 0x55;
	
	OpenTransmission(MSP430_ADDRESS);
	
	if(write(i2c_fd,&start_comm,1) < 0){
		printf("Error trying to write in i2c_fd.\n");
	}
			
	if(read(i2c_fd,data,8) < 0){
		printf("Error trying to read in i2c_fd.\n");
	}

	CloseTransmission();

	//for(i=0; i<2; i++){
	//	printf("Value: %d\n",data[i]);		
	//}

	ph_sensor = data[0];
	ph_sensor += data[1] << 8;
	
	turb_sensor = data[2];
	turb_sensor += data[3] << 8;

	tds_sensor = data[4];
	tds_sensor += data[5] << 8;

	bat_level = data[6];
	bat_level += data[7] << 8;

	printf("Ph sensor: %d\n",ph_sensor);
	printf("Turbidity sensor: %d\n",turb_sensor);
	printf("Tds sensor: %d\n",tds_sensor);
	printf("Battery level: %d\n",bat_level);
	
	return 0;
}
