#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include "oled.h"

int main(){
	
	signal(SIGINT, ctrl_c);
	int i, my_gpio[NUMBER_THREADS];
	
	GPIO_setup(ON_BTN);
	GPIO_setup(OFF_BTN);
	
	for(i=0; i < NUMBER_THREADS; i++){
		my_gpio[i] = i;
		pthread_attr_init(&attr[i]);
		pthread_create(&GPIO_tid[i],&attr[i],GPIO_handler,&my_gpio[i]);
	}
	
	for(i=0; i < NUMBER_THREADS; i++){
		pthread_join(GPIO_tid[i],NULL);
	}	
	
	return 0;
}
