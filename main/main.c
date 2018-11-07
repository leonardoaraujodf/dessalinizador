#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include "oled.h"


void ctrl_c(int sig){
	close(i2c_fd);
	exit(-1);
}

int main(){
	
	struct analog sensors;

	int i;
	unsigned char start_comm = 0x55;
	OLEDInit();
	OpenTransmission(MSP430_ADDRESS);


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
	
	/*
	for(i=0;i<30;i++){
		printf("Ph sensor: %d\n",sensors.ph_sensor[i]);
		printf("Turbidity sensor: %d\n",sensors.turb_sensor[i]);
		printf("Tds sensor: %d\n",sensors.tds_sensor[i]);
		printf("Battery level: %d\n",sensors.bat_level[i]);
	}
	*/
	char gps_data[100] = {0};
	float PH_value, TDS_value, Turbidity_value, Temp;

	Temp = get_Temp();
	PH_value = get_PH(sensors.ph_sensor);
	TDS_value = get_TDS(sensors.tds_sensor);
	Turbidity_value = get_Turb(sensors.turb_sensor);
	get_Battlevel(sensors.bat_level);
	get_localization(gps_data);

	char gps_info[MAX_NUMBER_OF_FIELDS][MAX_STRING_LENGTH];
	char coordinates[2][MAX_STRING_LENGTH] = {0};
	char gps_data_copy[100] = {0};

	strcpy(gps_data_copy, gps_data);
	split_by(",", gps_data_copy, gps_info);

	format_coordinates(gps_info, coordinates);

	// printf("**************\n");
	// printf("X raw: %s\n", gps_info[LATITUDE_INDEX]);
	// printf("Y raw: %s\n", gps_info[LONGITUDE_INDEX]);
	// printf("Horizontal direction: %s\n", gps_info[HORIZONTAL_DIRECTION_INDEX]);
	// printf("Vertical direction: %s\n", gps_info[VERTICAL_DIRECTION_INDEX]);
	// printf("X: %s\n", coordinates[X]);
	// printf("Y: %s\n", coordinates[Y]);
	// printf("**************\n");

	printf("turbidity: %.2f \n", Turbidity_value);
	printf("ph: %.2f\n", PH_value);
	printf("temperature: %.2f\n", Temp);
	printf("tds: %.2f\n", TDS_value);
	printf("collection_date: \n");
	printf("%s\n",gps_data);	

	return 0;
}
