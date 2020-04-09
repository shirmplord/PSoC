/* ========================================
 *
 * Copyright Quang Minh Vu Metropolia UAS
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CC - BY - SA 4.0
 *   
 * PSoC fundamentals Exercise 1
 * Blink LED approx. 1.s when button is pressed
 * 
 * ========================================
*/
#include "project.h"
#include "stdio.h"

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    uint8 count = 0;
    char str[20] = {"Quang Vu\r\n"};
    
    UART_Start();
    /* Transmit name terminated with CR/LF (string) on the beginning of the program */
    UART_PutString(str);
    for(;;)
    {
        if (!Button_Read()) {
            LED1_Write(count++ % 2);
            CyDelay(500);
        }
        LED1_Write(0); /* Turn off LED when the button is released */
        if (UART_GetChar()) { /* On UART read */
            char buffer[5];
            sprintf(buffer, "%d ", count/2); /* Save int to string buffer, divide by 2 for number of blinks */
            UART_PutString(buffer); /* Print to screen */
        }
    }
}

/* [] END OF FILE */
