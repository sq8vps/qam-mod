/*
Copyright 2020-2023 Piotr Wilkon
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


#include "terminal.h"
#include <stdbool.h>
#include "modulator.h"
#include "common.h"

void TermSendToAll(enum UartMode mode, uint8_t *data, uint16_t size)
{
	if(UartUsb.mode == MODE_TERM)
		UartSendString(&UartUsb, data, size);
	if(Uart1.mode == MODE_TERM)
		UartSendString(&Uart1, data, size);
	if(Uart2.mode == MODE_TERM)
		UartSendString(&Uart2, data, size);
}

void TermSendNumberToAll(enum UartMode mode, int32_t n)
{
	if(MODE_TERM == mode)
	{
		if(UartUsb.mode == MODE_TERM)
			UartSendNumber(&UartUsb, n);
		if(Uart1.mode == MODE_TERM)
			UartSendNumber(&Uart1, n);
		if(Uart2.mode == MODE_TERM)
			UartSendNumber(&Uart2, n);
	}
}

void TermHandleSpecial(Uart *u)
{
	if(u->lastRxBufferHead >= u->rxBufferHead) //UART RX buffer index was probably cleared
		u->lastRxBufferHead = 0;

	if(u->rxBuffer[u->rxBufferHead - 1] == '\b') //user entered backspace
	{
		if(u->rxBufferHead > 1) //there was some data in buffer
		{
			u->rxBufferHead -= 2; //remove backspace and preceding character
			UartSendString(u, "\b \b", 3); //backspace (one character left), remove backspaced character (send space) and backspace again
			if(u->lastRxBufferHead > 0)
				u->lastRxBufferHead--; //1 character was removed
		}
		else //no preceding character
			u->rxBufferHead = 0;
	}
	uint16_t t = u->rxBufferHead; //store last index
	if(u->lastRxBufferHead < t) //local echo handling
	{
		UartSendString(u, (uint8_t*)&u->rxBuffer[u->lastRxBufferHead], t - u->lastRxBufferHead); //echo characters entered by user
		if((u->rxBuffer[t - 1] == '\r') || (u->rxBuffer[t - 1] == '\n'))
			UartSendString(u, "\r\n", 2);
		u->lastRxBufferHead = t;
	}

}

const char helpPage[] = "out <n> - set output symbol to n (Gray coded)\r\n"
		"baud <baudrate> - set baud rate\r\n"
		"mod [16qam|4qam|qpsk|bpsk] - set modulator mode\r\n"
		"random - start random sequence transmission\r\n"
		"stop - stop transmission\r\n\r\n";

void TermParse(Uart *src)
{
	const char *cmd = (char*)src->rxBuffer;
	uint16_t len = src->rxBufferHead;
	for(uint16_t i = 0; i < len; i++)
	{
		if((cmd[i] == '\r') || (cmd[i] == '\n'))
		{
			len = i;
			break;
		}
	}

	/*
	 * Settings insertion handling
	 */
	if(!strncmp(cmd, "mod", 3))
	{
		if(!strncmp(&cmd[4], "16qam", 5))
			ModulatorSetMode(QAM16);
		else if(!strncmp(&cmd[4], "4qam", 4))
			ModulatorSetMode(QAM4);
		else if(!strncmp(&cmd[4], "qpsk", 4))
			ModulatorSetMode(QPSK);
		else if(!strncmp(&cmd[4], "bpsk", 4))
			ModulatorSetMode(BPSK);
		else
		{
			UartSendString(src, "Bad modulator mode!\r\n", 0);
			return;
		}
	}
	else if(!strncmp(cmd, "baud", 4))
	{
		int64_t t = StrToInt(&cmd[5], len - 5);
		if(0 == t)
		{
			UartSendString(src, "Bad baud rate!\r\n", 0);
			return;
		}
		else
		{
			UartSendString(src, "Real baud rate: ", 0);
			UartSendNumber(src, ModulatorSetBaudrate(t));
			UartSendString(src, "\r\n", 0);
		}
	}
	else if(!strncmp(cmd, "out", 3))
	{
		int64_t t = StrToInt(&cmd[4], len - 4);
		ModulatorPut(t);
	}
	else if(!strncmp(cmd, "stop", 4))
	{
		ModulatorStop();
	}
	else if(!strncmp(cmd, "random", 6))
	{
		ModulatorStartRandom();
	}
	else if(!strncmp(cmd, "help", 4))
	{
		UartSendString(src, (char*)helpPage, 0);
	}
	else
	{
		UartSendString(src, "Unknown command. Use \"help\" to show available commands.\r\n", 0);
		return;
	}


	UartSendString(src, "OK\r\n", 0);
}
