all: final.out
final.out: oled.o main.o
	sudo gcc -o final.out main.o oled.o -lwiringPi
main.o: main.c oled.h
	sudo gcc -c main.c -lwiringPi
oled.o: oled.c oled.h
	sudo gcc -c oled.c -lwiringPi
clean:
	sudo rm -f *.o *.out
