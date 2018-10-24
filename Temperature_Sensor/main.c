#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH "/sys/bus/w1/devices/28-000007602ffa/w1_slave"


int main(){

	int fp,i = 0,i_temp;
	char c,temp_buffer[100],*p_temp; //temperature buffer
	float temperature;
	fp = open(PATH,O_RDONLY);
	if(fp < 0){
		printf("Erro na abertura do arquivo");
	}
	while(read(fp,&c,1) != 0){
		temp_buffer[i] = c;
		i++;
	}
	i--;
	p_temp = &temp_buffer[i-5]; //points to the temperature read
	i_temp = atoi(p_temp);
	temperature = i_temp/1000.0;
	printf("Temperature read: %d\n",temperature);

	close(fp);

	return 0;
}