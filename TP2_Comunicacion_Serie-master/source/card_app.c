#include "card_app.h"
#include "CardReaderDrv.h"

uint64_t convertToID2(uint32_t* buff) {
	uint64_t ids = 0;
	uint32_t j = 5;
	uint8_t simbol = 0;
	while (simbol != FEILD_SEPARATOR) {
		ids *= 10;
		ids += simbol;
		simbol = 0;
		for (int i=0; i<4; i++) {
			simbol += (!buff[j+i])<<i;
		}
		j+=5;
	}
	return ids;
}






