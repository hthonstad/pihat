/******************************************************************************/
/* PiHAT - A software based home automation transmitter for the Raspberry PI  */
/*                                                                            */
/* Based on the work of the PiFM-guys, PiHAT enables you to control wireless  */
/* remote power switches with a Raspberry Pi with no extra hardware!          */
/* For proof of concept, simply connect a wire to GPIO 4,                     */
/* but for long term use a 433.92 MHz band pass filter _must_ be used!        */
/*                                                                            */
/* Use of this application is solely at your own risk!                        */
/*                                                                            */
/* Original PiFm-project: http://www.icrobotics.co.uk/wiki/index.php/         */
/*                        Turning_the_Raspberry_Pi_Into_an_FM_Transmitter     */
/*                                                                            */
/* PiHAT project: http://skagmo.com -> Projects -> PiHAT                      */
/*                                                                            */
/*                          Jon Petter Skagmo, 2012                           */
/******************************************************************************/

#include <inttypes.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "minIni.h"
#include <time.h>

#define BRAND_NEXA 			0x01
#define BRAND_EVERFLOURISH 	0x02
#define BRAND_WAVEMAN	 	0x03
#define BRAND_KANGHTAI	 	0x04
#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

/***** Argp configuration start *****/

const char *argp_program_version = "PiHAT 0.2";
const char *argp_program_bug_address = "<web@skagmo.com>";
static char doc[] = "PiHAT -- Raspberry Pi Home Automation Transmitter.";
static char args_doc[] = "";
//const char inifile[] = "/home/pi/pihat/HomeAutomation.ini";
const char inifile[] = "HomeAutomation.ini";

struct arguments{
	uint8_t brand;
	uint32_t id;
	uint8_t state;
	uint8_t channel;
	uint8_t group;
	uint8_t dim_level;
	uint8_t repeats;
	uint8_t pinMode;
};

void parseIni(struct arguments *argument)
{

  argument->repeats = ini_getl("nexa", "repeats", -1, inifile);
  argument->id = ini_getl("nexa", "id", -1, inifile);
  argument->channel = ini_getl("nexa", "channel", -1, inifile);
  argument->state = ini_getl("status", "power", -1, inifile);


  
}

void getNowDate(char *buffer)
{
    time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    
}

void printIni() 
{ 
    FILE *fptr;
	char c; 
  
    // Open file 
    fptr = fopen(inifile, "r"); 
    if (fptr == NULL) 
    { 
        printf("Cannot open file \n"); 
        exit(0); 
    } 
    printf("printing open file \n"); 
    // Read contents from file 
    c = fgetc(fptr); 
    while (c != EOF) 
    { 
        printf ("%c", c); 
        c = fgetc(fptr); 
    } 
  
    fclose(fptr); 

}






/***** ARGP configuration stop *****/

#include "radio.h"
#include "nexa.h"

int main (int argc, char **argv){
	struct arguments arguments;
	char timeBuffer[26];
	void (*ask[2])(void); 

	/* Set argument defaults */
	arguments.brand = BRAND_NEXA;
	arguments.id = 0;
	arguments.state = 0;
	arguments.channel = 0;
	arguments.group = 0;
	arguments.dim_level = 0x10;	// Invalid entry - to determine if dim_level has been set at a later time
	arguments.repeats = 3;
	arguments.pinMode=0;

	/* Parse arguments */

	if (argc>1) {
	    getNowDate(timeBuffer);
		printf(timeBuffer);
		ini_puts("status", "lastChange", timeBuffer, inifile );
		//printIni();
		if (!strcmp(argv[1], "1")){
		    ini_puts("status", "power", "1", inifile );
			

			}
			
	    else
			ini_puts("status", "power", "0", inifile );
	}
    parseIni(&arguments);
	printf("power value =%ld\n",arguments.state);

	/* Data variables for RF-packets */
	uint64_t data;			// General purpose data variable

	/* Setup RF-configuration */
	setup_io();
	setup_fm();
	ACCESS(CM_GP0DIV) = (0x5a << 24) + 0x374F; // Tune to 144.64 MHz to get the third harmonic at 433.92 MHz
    switch (arguments.pinMode){
		case 0: // 0 is for modulating pin at 144 MHz
			/* Setup RF-configuration */
			setup_io();
			setup_fm();
			ACCESS(CM_GP0DIV) = (0x5a << 24) + 0x374F; // Tune to 144.64 MHz to get the third harmonic at 433.92 MHz
			ask[0]=askLow;
			ask[1]=askHigh;
			break;
		case 1:
			ask[0]=askHigh_bcm2835;
			ask[1]=askHigh_bcm2835;
			break;
		
	}

	switch(arguments.brand){
		case BRAND_NEXA:
			/* Fill the data-variable and call the TX-function */
			data = 0;
			data |= ((uint64_t)arguments.id) << 10;
			data |= ((uint64_t)arguments.group) << 9;
			data |= ((uint64_t)arguments.state) << 8;
			data |= arguments.channel << 4;
			if (arguments.dim_level != 0x10) data |= arguments.dim_level;
			nexaTxPacket(&data, (arguments.dim_level != 0x10), arguments.repeats,ask[0],ask[1]);
			break;
	}

	return 0;
}

