#ifndef INC_MODULATOR_H_
#define INC_MODULATOR_H_

#include <stdint.h>


enum Modulator
{
	QAM16,
	QAM4,
	QPSK = QAM4,
	BPSK,
	DEFAULT,
};

void ModulatorInit(void);
void ModulatorPut(uint8_t value);
void ModulatorSetMode(enum Modulator m);
uint32_t ModulatorSetBaudrate(uint32_t baud);

void ModulatorStartTimer(uint8_t (*symbolCallback)(void));
void ModulatorStop(void);
void ModulatorStartRandom(void);

#endif /* INC_MODULATOR_H_ */
