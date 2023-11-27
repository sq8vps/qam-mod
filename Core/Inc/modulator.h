/*
Copyright 2023 Piotr Wilkon
This file is part of QAM Modulator Controller.

QAM Modulator Controller is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

QAM Modulator Controller is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QAM Modulator Controller.  If not, see <http://www.gnu.org/licenses/>.
*/


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
