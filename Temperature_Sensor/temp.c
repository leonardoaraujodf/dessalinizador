#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define PATH "/sys/bus/w1/devices/28-01131b9500b6/w1_slave"

int main(){
	int fp,length = 0,i_temp;
	float temp;
	char c,*text;
	fp = open(PATH, O_RDONLY);
	if(fp==-1){
		printf("Erro na abertura do arquivo.\n");
		exit(1);
	}
	while(read(fp, &c, 1) != 0){
		length++;
	}

	text = (char *)calloc(length,sizeof(char));
	lseek(fp, 0, SEEK_SET);
	read(fp,text,length);
	i_temp = atoi(&text[length-6]);
	temp = i_temp/1000.0;
	
	printf("Temperature: %.3f C\n",temp);

	close(fp);
}
