#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <signal.h>
#include <string.h>

//compile using
//gcc gps.c -lwiringPi -o gps


#define TTY "/dev/ttyS0"
//#define TTY "/dev/ttyAMA0"

int uart0_fd;

void ctrl_c(int sig){
	puts("Fechando " TTY "...");
	serialClose(uart0_fd);
	exit(-1);
}


int main(){
	char character,sentence[6],gps_data[100] = {0};
	int i;
	signal(SIGINT, ctrl_c);
	uart0_fd = serialOpen(TTY, 9600);
	if(uart0_fd == -1){
		puts("Error trying to open UART.");
		return -1;
	}
	if(wiringPiSetup() == -1){
		puts("Error in wiringPiSetup().");
		return -1;
	}
	serialFlush(uart0_fd);
	system("stty -F " TTY " 9600");
	system("stty -F " TTY " -echo");
	//system("stty -F " TTY);
	while(1){
		character = serialGetchar(uart0_fd);
		//printf("%c",character);
		if(character == '$'){
			for(i=0; i<5; i++){
				sentence[i] = serialGetchar(uart0_fd);
			}
			if(strcmp(sentence,"GPGGA") == 0){
				sprintf(gps_data,"%c%s",character,sentence);
				i = 0;
				do{
					character = serialGetchar(uart0_fd);
					gps_data[i + 6] = character;
					i++;

				}while(character != '\n');

				printf("%s",gps_data);
				printf("\n");
				break;
				}
				
			}
	}

	puts("Fechando " TTY "...");
	serialClose(uart0_fd);
	return(0);
}

