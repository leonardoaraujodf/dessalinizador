all: final.out
final.out: oled.o main.o
	sudo gcc -o final.out main.o oled.o -lwiringPi -lpthread
main.o: main.c oled.h
	sudo gcc -c main.c -lwiringPi -lpthread
oled.o: oled.c oled.h
	sudo gcc -c oled.c -lwiringPi -lpthread
clean:
	sudo rm -f *.o *.out
run:
	sudo ./final.out
