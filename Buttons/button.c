#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>


#define ON_BTN 26
#define OFF_BTN 19
#define ON_GPIO_fd 0
#define OFF_GPIO_fd 1
#define NUMBER_GPIO_OUTPUTS 2
#define NUMBER_THREADS NUMBER_GPIO_OUTPUTS


int GPIO_fd[NUMBER_GPIO_OUTPUTS];
pthread_t GPIO_tid[NUMBER_THREADS];
pthread_attr_t attr[NUMBER_THREADS];


void GPIO_setup(int GPIO_num);
void GPIO_free(int GPIO_num);
void GPIO_open(int GPIO_fd_num,int GPIO_num);
void GPIO_close(int GPIO_fd_num);
void ctrl_c(int sig);
int GPIO_get(int GPIO_fd_num);
void *GPIO_handler(void *param);


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

void ctrl_c(int sig){
	
	int i;
	for(i=0; i < NUMBER_THREADS; i++){
		pthread_cancel(GPIO_tid[i]);
	}
	
	GPIO_close(ON_GPIO_fd);
	GPIO_close(OFF_GPIO_fd);
	GPIO_free(ON_BTN);
	GPIO_free(OFF_BTN);
	//exit(-1);
}

void *GPIO_handler(void *param){

	int *GPIO_fd_num = (int *) param;
	char my_gpio[10];

	if((*GPIO_fd_num) == ON_GPIO_fd){
		sprintf(my_gpio,"On button");
		//puts("I'm on button");
	}
	else if((*GPIO_fd_num) == OFF_GPIO_fd){
		sprintf(my_gpio,"Off button");
		//puts("I'm off button");
	}


	while(1){
		if(GPIO_get(*GPIO_fd_num) == 0){
			printf("[LOG]: %s pressed.\n",my_gpio);
		}
		usleep(300000);
	}

	printf("[LOG] Closing... \n");

}

void GPIO_setup(int GPIO_num){
//Create a GPIO

	char str[50];
	printf("[LOG]: Seeting GPIO %d as a digital input pin.\n", GPIO_num);
	sprintf(str,"echo %d > /sys/class/gpio/export", GPIO_num);
	system(str);
	sprintf(str,"echo in > /sys/class/gpio/gpio%d/direction", GPIO_num);
	system(str);
	printf("[LOG]: Configured.\n");
	
	if(GPIO_num == ON_BTN){
		GPIO_open(ON_GPIO_fd,GPIO_num);
	}
	else if(GPIO_num == OFF_BTN){
		GPIO_open(OFF_GPIO_fd,GPIO_num);
	}
}

void GPIO_free(int GPIO_num){
	int fd;
	char str[2];
	sprintf(str,"%d",GPIO_num);
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if(fd == -1){
		printf("[ERROR]: Could not release the GPIO %d\n", GPIO_num);
	}
	else{
		write(fd, str, 2);
		close(fd);
		printf("[LOG]: GPIO %d released.\n", GPIO_num);
	}
}

void GPIO_open(int GPIO_fd_num,int GPIO_num){
	char str[50];
	sprintf(str,"/sys/class/gpio/gpio%d/value", GPIO_num);
	GPIO_fd[GPIO_fd_num] = open(str, O_RDONLY);	
}

void GPIO_close(int GPIO_fd_num){
	
	close(GPIO_fd[GPIO_fd_num]);
}

int GPIO_get(int GPIO_fd_num){
	int iValue;
	char cValue[1];
	read(GPIO_fd[GPIO_fd_num], cValue, 1);
	lseek(GPIO_fd[GPIO_fd_num],0,SEEK_SET);
	iValue = atoi(cValue);
	return iValue;
}

