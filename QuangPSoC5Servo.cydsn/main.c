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

    
    UART_Start();
    for(;;)
    {

    }
}

/* [] END OF FILE */
