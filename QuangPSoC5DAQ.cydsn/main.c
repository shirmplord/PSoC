/* ========================================
 *
 * Copyright Quang Minh Vu Metropolia UAS
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CC - BY - SA 4.0
 *   
 * PSoC fundamentals Exercise 3
 * Data Acquisition and ADC conversion
 * 
 * ========================================
*/

#include <project.h>
#include "stdio.h"

/* Project Defines */
#define FALSE  0
#define TRUE   1
#define TRANSMIT_BUFFER_SIZE 40
#define THRESHOLD 5000

/* ISR Handler */
CY_ISR_PROTO(ADC_ISR_Handler);

/* Flag for interrupt */
static volatile CYBIT ADC_flag = FALSE;

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  main() performs following functions:
*  1: Starts the ADC and UART components.
*  2: Checks for ADC end of conversion.  Stores latest result in output
*     if conversion complete.
*  3: Checks for UART input.
*     On 'C' or 'c' received: transmits the last sample via the UART.
*     On 'S' or 's' received: continuously transmits samples as they are completed.
*     On 'X' or 'x' received: stops continuously transmitting samples.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
int main()
{
    CyGlobalIntEnable;
    /* Variable to store ADC result */
    uint32 Output;
    /* Variable to store UART received character */
    uint8 Ch;
    /* Flags used to store transmit data commands */
    uint8 ContinuouslySendData;
    uint8 SendSingleByte;
    /* values for the down-sampling */
    uint32 sum = 0;
    uint32 cnt = 0;
    float tval = 0.0;
    /* Transmit Buffer */
    char TransmitBuffer[TRANSMIT_BUFFER_SIZE];
    
    /* Start the components */
    ADC_DelSig_1_Start();
    UART_1_Start();
    
    /* Initialize Variables */
    ContinuouslySendData = FALSE;
    SendSingleByte = FALSE;
    
    /* Start the ADC conversion */
    ADC_DelSig_1_StartConvert();
    
    /* Send message to verify COM port is connected properly */
    UART_1_PutString("COM Port Open");
    
    /* Start the ISR */
    ADC_DelSig_1_IRQ_StartEx(ADC_ISR_Handler);
    
    for(;;)
    {        
        /* Non-blocking call to get the latest data recieved  */
        Ch = UART_1_GetChar();
        
        /* Set flags based on UART command */
        switch(Ch)
        {
            case 0:
                /* No new data was recieved */
                break;
            case 'C':
            case 'c':
                SendSingleByte = TRUE;
                break;
            case 'S':
            case 's':
                ContinuouslySendData = TRUE;
                break;
            case 'X':
            case 'x':
                ContinuouslySendData = FALSE;
                break;
            default:
                /* Place error handling code here */
                break;    
        }
        
        /* Check to see if an ADC conversion has completed */
        if(ADC_DelSig_1_IsEndConversion(ADC_DelSig_1_RETURN_STATUS))
        {
            /* Use the GetResult16 API to get an 8 bit unsigned result in
             * single ended mode.  The API CountsTo_mVolts is then used
             * to convert the ADC counts into mV before transmitting via
             * the UART.  See the datasheet API description for more
             * details */
            Output = ADC_DelSig_1_CountsTo_mVolts(ADC_DelSig_1_GetResult16());
            
            /* Send data based on last UART command */
            if(SendSingleByte || ContinuouslySendData)
            {
                /* Flag set => reached 0.5s threshold */
                if (ADC_flag) 
                {
                    tval = sum/cnt/0xA;
                    /* Format ADC result for transmition */
                    /* The conversion of ADC value to temperature for this sensor is 10mV = 1 degree Celcius */
                    sprintf(TransmitBuffer, "{ ADC :%lu , Temperature :%.1f }\r\n", Output, tval);
                    /* Send out the data */
                    UART_1_PutString(TransmitBuffer);
                    /* Reset flags and values */
                    ADC_flag = FALSE;
                    sum = 0;
                    cnt = 0;
                } //output data 
                else 
                {
                    sum+=Output;
                    cnt++;
                } //gather data
                /* Reset the send once flag */
                SendSingleByte = FALSE;
            }
        }
    }
}
/* ISR routines */
CY_ISR(ADC_ISR_Handler)
{
    static uint32 clk = 0;
    /* Threshold = defined sample per seconds / sample rate 
     * The clock only runs when the flag is not set (calculation for output is finished 
     * This can result in data being captured but ignored completely 
     * However, the integrity of the data will be preserved */
    if (clk < THRESHOLD && !ADC_flag) clk++;
    else 
    {
        clk = 0;
        ADC_flag = TRUE;
    } //reset clock and set flags
    /* The interupt is automatically reset in this case */
}

/* [] END OF FILE */
