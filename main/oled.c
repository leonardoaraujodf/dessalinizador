#include "oled.h"

void WriteCmdOLED(const unsigned char* data, unsigned char size){
    int data_sent;
    OpenTransmission(OLED_I2C_ADDRESS);
   	data_sent = write(i2c_fd,data,size);
    if(data_sent < size){
        printf("Error trying to send a command.\n");
    }
    CloseTransmission();
}

void WriteDataOLED(const unsigned char* data, unsigned int size){
    int data_sent;
	WriteCmdOLED(WriteInit,sizeof(WriteInit));
    
    /*command for sending data*/
        OpenTransmission(OLED_I2C_ADDRESS);
            
        data_sent = write(i2c_fd,data,size);
        if(data_sent < size){
            printf("Error trying to send data.\n");
        }

        CloseTransmission();
    /*command for sending data*/    
      
}

void OLEDInit(){
    WriteCmdOLED(init_cmd_array, sizeof(init_cmd_array));
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

void get_Battlevel(int *values){

	qsort(values,30,sizeof(int),compare);
	float level = get_median(values);
	
	if(level > 848 && level < 865){
		printf("Battery level: 20 %\n");
		WriteDataOLED(bat_20,sizeof(bat_20));
	}
	else if(level > 865 && level < 884){
		printf("Battery level: 40 %\n");
		WriteDataOLED(bat_40,sizeof(bat_40));
	}
	else if(level > 884 && level < 902){
		printf("Battery level: 60 %\n");
		WriteDataOLED(bat_60,sizeof(bat_60));
	}
	else if(level > 902 && level < 920){
		printf("Battery level: 80 %\n");
		WriteDataOLED(bat_80,sizeof(bat_80));
	}
	else if(level > 920){
		printf("Battery level: 100 %\n");
		WriteDataOLED(bat_100,sizeof(bat_100));
	}
}

float get_TDS(int *values){
	qsort(values,30,sizeof(int),compare);
	float AD_value = get_median(values);
	float voltage = (3.3/1024)*AD_value;
	float TDS_value = (133.42*voltage*voltage*voltage - 255.86*voltage*voltage + 857.39*voltage)*0.5
	return TDS_value;

}

float get_Turb(int *values){
	qsort(values,30,sizeof(int),compare);
	float AD_value = get_median(values);
	float voltage = (3.3/1024)*AD_value;
	float Turbidity_value = -1120.4*voltage*voltage+5742.3*voltage-4352.9;
	return Turbidity_value;
}

int compare (const void * a, const void * b){
  return ( *(int*)a - *(int*)b );
}

float get_median(int *values){
	return (values[14] + values[15])/2 ;
}