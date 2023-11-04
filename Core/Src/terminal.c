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
//	else if(!strncmp(cmd, "quiet", 5))
//	{
//		int64_t t = StrToInt(&cmd[6], len - 6);
//		if((t > 2550) || (t < 100))
//		{
//			UartSendString(src, "Incorrect quiet time!\r\n", 0);
//			return;
//		}
//		else
//		{
//			Ax25Config.quietTime = (uint16_t)t;
//		}
//	}
//	else if(!strncmp(cmd, "uart", 4))
//	{
//		Uart *u = NULL;
//		if((cmd[5] - '0') == 1)
//			u = &Uart1;
//		else if((cmd[5] - '0') == 2)
//			u = &Uart2;
//		else if((cmd[5] - '0') == 0)
//			u = &UartUsb;
//		else
//		{
//			UartSendString(src, "Incorrect UART number!\r\n", 0);
//			return;
//		}
//		if(!strncmp(&cmd[7], "baud", 4))
//		{
//			int64_t t = StrToInt(&cmd[12], len - 12);
//			if((t > 115200) || (t < 1200))
//			{
//				UartSendString(src, "Incorrect baud rate!\r\n", 0);
//				return;
//			}
//			u->baudrate = (uint32_t)t;
//		}
//		else if(!strncmp(&cmd[7], "mode", 4))
//		{
//			if(!strncmp(&cmd[12], "kiss", 4))
//			{
//				u->defaultMode = MODE_KISS;
//			}
//			else if(!strncmp(&cmd[12], "monitor", 7))
//			{
//				u->defaultMode = MODE_MONITOR;
//			}
//			else if(!strncmp(&cmd[12], "config", 6))
//			{
//				u->defaultMode = MODE_TERM;
//			}
//			else
//			{
//				UartSendString(src, "Incorrect UART mode!\r\n", 0);
//				return;
//			}
//		}
//		else
//		{
//			UartSendString(src, "Incorrect option!\r\n", 0);
//			return;
//		}
//	}
//	else if(!strncmp(cmd, "beacon", 6))
//	{
//		uint8_t bcno = 0;
//		bcno = cmd[7] - 48;
//		if(bcno > 7)
//		{
//			UartSendString(src, "Incorrect beacon number\r\n", 0);
//			return;
//		}
//		if(!strncmp(&cmd[9], "on", 2))
//			beacon[bcno].enable = 1;
//		else if(!strncmp(&cmd[9], "off", 3))
//			beacon[bcno].enable = 0;
//		else if(!strncmp(&cmd[9], "iv", 2) || !strncmp(&cmd[9], "dl", 2)) //interval or delay
//		{
//			int64_t t = StrToInt(&cmd[12], len - 12);
//			if(t > 720)
//			{
//				UartSendString(src, "Interval/delay must lesser or equal to 720 minutes\r\n", 0);
//				return;
//			}
//			if(!strncmp(&cmd[9], "iv", 2))
//				beacon[bcno].interval = t * 6000;
//			else
//				beacon[bcno].delay = t * 60;
//
//		}
//		else if(!strncmp(&cmd[9], "data", 4))
//		{
//			if((len - 14) > BEACON_MAX_PAYLOAD_SIZE)
//			{
//				UartSendString(src, "Data is too long\r\n", 0);
//				return;
//			}
//			uint16_t i = 0;
//			for(; i < (len - 14); i++)
//				beacon[bcno].data[i] = cmd[14 + i];
//			beacon[bcno].data[i] = 0;
//		}
//		else if(!strncmp(&cmd[9], "path", 4))
//		{
//
//			if((len - 14) < 0)
//			{
//				UartSendString(src, "Path cannot be empty. Use \"none\" for empty path. \r\n", 0);
//				return;
//			}
//			if(((len - 14) == 4) && !strncmp(&cmd[14], "none", 4)) //"none" path
//			{
//				memset(beacon[bcno].path, 0, sizeof(beacon[bcno].path));
//
//				UartSendString(src, "OK\r\n", 0);
//				return;
//			}
//
//			uint8_t tmp[14];
//			uint8_t tmpIdx = 0;
//			uint16_t elementStart = 14;
//			for(uint8_t i = 0; i < (len - 14); i++)
//			{
//				if((cmd[14 + i] == ',') || ((14 + i + 1) == len))
//				{
//					if((14 + i + 1) == len) //end of data
//					{
//						i++;
//						tmp[7] = 0;
//					}
//
//					if((14 + i - elementStart) > 0)
//					{
//						if(!ParseCallsignWithSsid(&cmd[elementStart], 14 + i - elementStart, &tmp[tmpIdx], &tmp[tmpIdx + 6]))
//						{
//							err = true;
//							break;
//						}
//						tmpIdx += 7;
//						if(tmpIdx == 14)
//							break;
//					}
//					elementStart = 14 + i + 1;
//				}
//			}
//			if(err)
//			{
//				UartSendString(src, "Incorrect path!\r\n", 0);
//				return;
//			}
//
//			memcpy(beacon[bcno].path, tmp, 14);
//		}
//		else
//		{
//			err = true;
//		}
//	}
//	else if(!strncmp(cmd, "digi", 4))
//	{
//		if(!strncmp(&cmd[5], "on", 2))
//			DigiConfig.enable = 1;
//		else if(!strncmp(&cmd[5], "off", 3))
//			DigiConfig.enable = 0;
//		else if(IS_NUMBER(cmd[5]))
//		{
//			uint8_t alno = 0;
//			alno = cmd[5] - 48;
//			if(alno > 7)
//			{
//				UartSendString(src, "Incorrect alias number\r\n", 0);
//				return;
//			}
//			if(!strncmp(&cmd[7], "on", 2))
//				DigiConfig.enableAlias |= (1 << alno);
//			else if(!strncmp(&cmd[7], "off", 3))
//				DigiConfig.enableAlias &= ~(1 << alno);
//			else if(!strncmp(&cmd[7], "alias ", 6))
//			{
//				if(alno < 4) //New-N aliases
//				{
//					if((len - 13) <= 5)
//					{
//						if(false == (err = !ParseCallsign(&cmd[13], len - 13, DigiConfig.alias[alno])))
//							DigiConfig.alias[alno][len - 13] = 0;
//					}
//					else
//						err = true;
//				}
//				else  //simple aliases
//				{
//					if(!ParseCallsignWithSsid(&cmd[13], len - 13, DigiConfig.alias[alno], &DigiConfig.ssid[alno - 4]))
//					{
//						err = true;
//					}
//				}
//				if(err)
//				{
//					UartSendString(src, "Incorrect alias!\r\n", 0);
//					return;
//				}
//			}
//			else if((!strncmp(&cmd[7], "max ", 4) || !strncmp(&cmd[7], "rep ", 4)) && (alno < 4))
//			{
//				int64_t t = StrToInt(&cmd[11], len - 11);
//				if(!strncmp(&cmd[7], "max ", 4) && (t >= 1 || t <= 7))
//				{
//					DigiConfig.max[alno] = t;
//				}
//				else if(!strncmp(&cmd[7], "rep ", 4) && (t <= 7))
//				{
//					DigiConfig.rep[alno] = t;
//				}
//				else
//				{
//					UartSendString(src, "Incorrect value!\r\n", 0);
//					return;
//				}
//			}
//			else if(!strncmp(&cmd[7], "trac ", 5))
//			{
//				if(!strncmp(&cmd[12], "on", 2))
//					DigiConfig.traced |= 1 << alno;
//				else if(!strncmp(&cmd[12], "off", 3))
//					DigiConfig.traced &= ~(1 << alno);
//				else
//				{
//					err = true;
//				}
//			}
//			else if(!strncmp(&cmd[7], "filter ", 7))
//			{
//				if(!strncmp(&cmd[14], "on", 2))
//					DigiConfig.callFilterEnable |= 1 << alno;
//				else if(!strncmp(&cmd[14], "off", 3))
//					DigiConfig.callFilterEnable &= ~(1 << alno);
//				else
//				{
//					err = true;
//				}
//			}
//			else if(!strncmp(&cmd[7], "viscous ", 8))
//			{
//				if(!strncmp(&cmd[15], "on", 2))
//				{
//					DigiConfig.viscous |= (1 << alno);
//					DigiConfig.directOnly &= ~(1 << alno); //disable directonly mode
//				}
//				else if(!strncmp(&cmd[15], "off", 3))
//					DigiConfig.viscous &= ~(1 << alno);
//				else
//				{
//					err = true;
//				}
//			}
//			else if(!strncmp(&cmd[7], "direct ", 7))
//			{
//				if(!strncmp(&cmd[14], "on", 2))
//				{
//					DigiConfig.directOnly |= (1 << alno);
//					DigiConfig.viscous &= ~(1 << alno); //disable viscous delay mode
//				}
//				else if(!strncmp(&cmd[14], "off", 3))
//					DigiConfig.directOnly &= ~(1 << alno);
//				else
//				{
//					err = true;
//				}
//			}
//			else
//				err = true;
//
//
//		}
//		else if(!strncmp(&cmd[5], "filter ", 7))
//		{
//			if(!strncmp(&cmd[12], "white", 5))
//				DigiConfig.filterPolarity = 1;
//			else if(!strncmp(&cmd[12], "black", 5))
//				DigiConfig.filterPolarity = 0;
//			else
//				err = true;
//		}
//		else if(!strncmp(&cmd[5], "dupe ", 5))
//		{
//			int64_t t = StrToInt(&cmd[10], len - 10);
//			if((t > 255) || (t < 5))
//			{
//				UartSendString(src, "Incorrect anti-dupe time!\r\n", 0);
//				return;
//			}
//			else
//				DigiConfig.dupeTime = (uint8_t)t;
//		}
//		else if(!strncmp(&cmd[5], "list ", 5))
//		{
//			uint16_t shift = 10;
//
//			while(shift < len)
//			{
//				if(cmd[shift] == ' ')
//					break;
//
//				shift++;
//			}
//			int64_t number = StrToInt(&cmd[10], shift - 10);
//			if((number < 0) || (number >= (sizeof(DigiConfig.callFilter) / sizeof(*(DigiConfig.callFilter)))))
//			{
//				 UartSendString(src, "Incorrect filter slot!\r\n", 0);
//				 return;
//			}
//			shift++;
//			if((len - shift) < 4)
//			{
//				 UartSendString(src, "Incorrect format!\r\n", 0);
//				 return;
//			}
//
//			if(!strncmp(&cmd[shift], "set ", 4))
//			{
//				shift += 4;
//
//				uint8_t tmp[7] = {0};
//				uint16_t tmpIdx = 0;
//
//				for(uint16_t i = 0; i < (len - shift); i++)
//                {
//                      if(cmd[shift + i] == '-') //SSID separator
//                      {
//                          if((cmd[shift + i + 1] == '*') || (cmd[shift + i + 1] == '?')) //and there is any wildcard
//                              tmp[6] = 0xFF;
//                          else if(!ParseSsid(&cmd[shift + i + 1], len - (shift + i + 1), &tmp[6])) //otherwise it is a normal SSID
//                        	  err = true;
//                          break;
//                      }
//                      else if(cmd[shift + i] == '?')
//                      {
//                          tmp[tmpIdx++] = 0xFF;
//                      }
//                      else if(cmd[shift + i] == '*')
//                      {
//                          while(tmpIdx < 6)
//                          {
//                              tmp[tmpIdx++] = 0xFF;
//                          }
//                          continue;
//                      }
//                      else if(IS_UPPERCASE_ALPHANUMERIC(cmd[shift + i]))
//                    	  tmp[tmpIdx++] = cmd[shift + i];
//                      else
//                      {
//                    	  err = true;
//                    	  break;
//                      }
//                 }
//
//                 if(!err)
//                 {
//                	 while(tmpIdx < 6) //fill with spaces
//                	 {
//                		 tmp[tmpIdx++] = ' ';
//                 	 }
//
//                	 memcpy(DigiConfig.callFilter[number], tmp, 7);
//                 }
//                 else
//                 {
//                	 UartSendString(src, "Incorrect format!\r\n", 0);
//                	 return;
//                 }
//			}
//			else if(!strncmp(&cmd[shift], "remove", 6))
//			{
//				memset(DigiConfig.callFilter[number], 0, 7);
//			}
//			else
//				err = true;
//		}
//		else
//			err = true;
//	}
//	else if(!strncmp(cmd, "pwm ", 4))
//	{
//		if(!strncmp(&cmd[4], "on", 2))
//			ModemConfig.usePWM = 1;
//		else if(!strncmp(&cmd[4], "off", 3))
//			ModemConfig.usePWM = 0;
//		else
//			err = true;
//	}
//	else if(!strncmp(cmd, "flat ", 5))
//	{
//		if(!strncmp(&cmd[5], "on", 2))
//			ModemConfig.flatAudioIn = 1;
//		else if(!strncmp(&cmd[5], "off", 3))
//			ModemConfig.flatAudioIn = 0;
//		else
//			err = true;
//	}
//	else if(!strncmp(cmd, "monkiss ", 8))
//	{
//		if(!strncmp(&cmd[8], "on", 2))
//			GeneralConfig.kissMonitor = 1;
//		else if(!strncmp(&cmd[8], "off", 3))
//			GeneralConfig.kissMonitor = 0;
//		else
//			err = true;
//	}
//	else if(!strncmp(cmd, "nonaprs ", 8))
//	{
//		if(!strncmp(&cmd[8], "on", 2))
//			Ax25Config.allowNonAprs = 1;
//		else if(!strncmp(&cmd[8], "off", 2))
//			Ax25Config.allowNonAprs = 0;
//		else
//			err = true;
//	}
//	else if(!strncmp(cmd, "fx25 ", 5))
//	{
//		if(!strncmp(&cmd[5], "on", 2))
//			Ax25Config.fx25 = 1;
//		else if(!strncmp(&cmd[5], "off", 2))
//			Ax25Config.fx25 = 0;
//		else
//			err = true;
//	}
//	else if(!strncmp(cmd, "fx25tx ", 7))
//	{
//		if(!strncmp(&cmd[7], "on", 2))
//			Ax25Config.fx25Tx = 1;
//		else if(!strncmp(&cmd[7], "off", 2))
//			Ax25Config.fx25Tx = 0;
//		else
//			err = true;
//	}
	else
	{
		UartSendString(src, "Unknown command\r\n", 0);
		return;
	}


	UartSendString(src, "OK\r\n", 0);
}
