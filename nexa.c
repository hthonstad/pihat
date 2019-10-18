/******************************************************************************/
/*              PiHAT routines for Nexa remote power switches                 */
/*                                                                            */
/*                          Jon Petter Skagmo, 2012                           */
/******************************************************************************/

#include "nexa.h"

#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>
#include "radio.h"

/* Nexa has three different symbols, 0, 1 and 2, where 2 is used only for the special dim-packet */
void nexaTxSymbol(uint8_t symbol,void (*askLo)(),void (*askHi)()){
	askHi();
	usleep(NEXA_HIGH);

	askLo();
	if (symbol == 1) usleep(NEXA_LOWB);
	else usleep(NEXA_LOWA);

	askHi();
	usleep(NEXA_HIGH);

	askLo();
	if (symbol == 2) usleep(NEXA_LOWA);	// Send special dim-symbol
	else if (symbol == 1) usleep(NEXA_LOWA);
	else usleep(NEXA_LOWB);
}

void nexaTxPacket(uint64_t *d, uint8_t dim_packet_flag, uint8_t rep,void (*askLo)(),void (*askHi)()){
	uint64_t mask;
	uint8_t symbol_nr;
	uint8_t symbol_nr_max;
	uint8_t p_nr;

	if (dim_packet_flag) symbol_nr_max = 36;
	else symbol_nr_max = 32;

	for (p_nr = 0; p_nr < rep; p_nr++){
		/* Preamble */
		askHi();
		usleep(NEXA_HIGH);
		askLo();
		usleep(NEXA_LOWC);

		mask = 0x800000000;

		/* Transmit the packet content */
		for (symbol_nr = 0; symbol_nr<symbol_nr_max; symbol_nr++){
			if (dim_packet_flag && (symbol_nr == 27)) nexaTxSymbol(2,askLo,askHi);	// Insert the dim-symbol
			else if ((*d) & mask) nexaTxSymbol(1,askLo,askHi);
			else nexaTxSymbol(0,askLo,askHi);
			mask >>= 1;
		}

		/* "Postamble" */
		askHi();
		usleep(NEXA_HIGH);
		askLo();
		usleep(NEXA_BURSTDELAY);
	}
}
