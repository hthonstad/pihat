all: app.c nexa.c radio.c bcm2835.c
	gcc -lm -std=c99 -o pihat app.c nexa.c radio.c minIni.c -l bcm2835

