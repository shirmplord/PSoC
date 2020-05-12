/* ========================================
 *
 * Copyright Quang Minh Vu Metropolia UAS
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CC - BY - SA 4.0
 *   
 * PSoC fundamentals Exercise 5
 * OneWire Communication
 * 
 * ========================================
*/
#include "onewirelib.h"

/* Subprocesses */
void SetSpeed()
{
    A = 6;
    B = 64;
    C = 60;
    D = 10;
    E = 9;
    F = 55;
    G = 0;
    H = 480;
    I = 70;
    J = 410;
}
/* Generate a 1-Wire reset, return 1 if no presence detect was found,
 * return 0 otherwise. */
int OWTouchReset(void)
{
    int result;
    CyDelay(G);
    OneWireD_Write(0); //Drives DQ Low
    CyDelayUs(H);
    OneWireD_Write(1); // Releases the bus
    CyDelayUs(I);
    result = OneWireD_Read();
    CyDelayUs(J); // Complete the reset sequence recovery
    return result; // Return sample presence pulse result
}
/* Write 1-Wire data byte. */
void OWWriteByte(int data)
{
    int loop;

    // Loop to write each bit in the byte, LS-bit first
    for (loop = 0; loop < 8; loop++)
    {
        if (data & 0x01)
        {
            OneWireD_Write(0); //Drives DQ Low
            CyDelayUs(A);
            OneWireD_Write(1); // Releases the bus
            CyDelayUs(B); // Complete the time slot and 10us recovery
        }   // Write '1' bit
        else
        {
            OneWireD_Write(0); //Drives DQ Low
            CyDelayUs(C);
            OneWireD_Write(1); // Releases the bus
            CyDelayUs(D);
        }   // Write '0' bit

        // shift the data byte for the next bit
        data >>= 1;
    }
}

/* Read 2 1-Wire data byte and return it. */
unsigned char OWReadByte(void)
{
    int loop;
    signed char result=0;

    for (loop = 0; loop < 8; loop++)
    {
        // shift the result to get it ready for the next bit
        result >>= 1;
        OneWireD_Write(0); //Drives DQ Low
        CyDelayUs(A);
        OneWireD_Write(1); // Releases the bus
        CyDelayUs(E);
        // if result is one, then set MS bit        
        result |= (OneWireD_Read()) ? 0x80 : 0x00;
        CyDelayUs(F); // Complete the time slot and 10us recovery

    }
    return result;
}
/* Calculate the checksum */
unsigned char OWCRC(unsigned char *pBuf, int len)
{
	unsigned char loop, i, shiftedBit;
	unsigned char crc = 0x00;
	
	for(loop=0; loop<len; loop++)
	{
		crc = (crc ^ pBuf[loop]);
		
		for(i=8; i>0; i--)
		{
			shiftedBit= (crc & 0x01);
			crc >>= 1;
			
			if(shiftedBit)
			{
				crc = (crc ^ 0x8c);
			}
		}
	}
	
	return crc;
}

/* [] END OF FILE */
