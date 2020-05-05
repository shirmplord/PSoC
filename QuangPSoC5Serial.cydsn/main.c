/* ========================================
 *
 * Copyright Quang Minh Vu Metropolia UAS
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CC - BY - SA 4.0
 *   
 * PSoC fundamentals Exercise 4
 * SPI and I2C
 * 
 * ========================================
*/

#include <project.h>
#include "stdio.h"
#include "stdlib.h"

/* Project Defines */
#define FALSE  0
#define TRUE   1
#define TRANSMIT_BUFFER_SIZE 40
#define THRESHOLD 5000
#define SLAVE_ADDR 0x4Du

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
    uint32 ADCOutput;
    /* Variable to store the SPI data */
    uint8 SPIOutput;
    /* value to store the buffer data from the slave */
    uint8 I2COutput;    
    /* Variable to store UART received character */
    uint8 Ch;
    /* Flags used to store transmit data commands */
    uint8 ContinuouslySendData;
    uint8 SendSingleByte;
    /* values for the down-sampling */
    uint32 sum = 0;
    uint32 cnt = 0;
    /* values to send to the Tx buffer of SPI */
    uint8 SPIdummy = 0;
    /* Dummy value to test */
    uint8 I2Cdummy = 0;
    /* Transmit Buffer */
    char TransmitBuffer[TRANSMIT_BUFFER_SIZE];
    
    /* Start the components */
    ADC_DelSig_1_Start();
    UART_1_Start();
    SPIM_1_Start();
    I2C_1_Start();
    
    /* Set drive mode */
    SDA_SetDriveMode(SDA_DM_RES_UP);
    SCL_SetDriveMode(SCL_DM_RES_UP);
    
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
        /*---------------I2C---------------*/
        /* Send start to the slave */
        /* Check master's status
         * No error bits set */
        uint8 status = I2C_1_MasterSendStart(SLAVE_ADDR, I2C_1_WRITE_XFER_MODE);
        if (status == I2C_1_MSTR_NO_ERROR)
        {
            /* Write command 00 to read the temperature */
            I2C_1_MasterWriteByte(0);
            /* Resend the slave address */
            status = I2C_1_MasterSendRestart(SLAVE_ADDR, I2C_1_READ_XFER_MODE);
            if (status == I2C_1_MSTR_NO_ERROR) 
                /* Read the TEMP data and generate NACK */
                I2COutput = I2C_1_MasterReadByte(I2C_1_NAK_DATA);
            /* End transaction */
            I2C_1_MasterSendStop();
            for (int i = 0; i<8; i++) {}
            /* Check 7th bit to determine positive or negative */
            if ((I2COutput & 0x80) != 0)
            {
                int temp = (int)(I2COutput & 0x7F);
                temp-=128;
                I2COutput = temp;
            }
        }

        /*---------------SPI---------------*/
        if (SPIM_1_ReadTxStatus() & 0x02u)
        {
             /* Driving SS high with software */
            SPISS_1_Write(TRUE);
            /* Write to Tx buffer */
            SPIM_1_WriteTxData(SPIdummy);
            /* Wait for the SPI transfer to finish
             * Tx status register bit 0 is set 
             * Another solution is to set interupt on SPI done and set flag */
            while(SPIM_1_ReadTxStatus() & 0x01u);
            /* Check for something to be sent to the Rx buffer
             * Rx status register bit 5 is set */
            if (SPIM_1_ReadRxStatus() & 0x20u) 
            {
                /* Driving SS low with software */
                SPISS_1_Write(FALSE);
                /* Read data from Rx buffer */
                SPIOutput = SPIM_1_ReadRxData();
            }   
        }
        /* Check to see if an ADC conversion has completed */
        /* Check to see if Tx buffer is empty 
         * by comparing with a mask
         * The compare value is to check whether bit 1 was set */
        if(ADC_DelSig_1_IsEndConversion(ADC_DelSig_1_RETURN_STATUS))
        {
            /* Use the GetResult16 API to get an 8 bit unsigned result in
             * single ended mode.  The API CountsTo_mVolts is then used
             * to convert the ADC counts into mV before transmitting via
             * the UART.  See the datasheet API description for more
             * details */
            ADCOutput = ADC_DelSig_1_CountsTo_mVolts(ADC_DelSig_1_GetResult16());

            /* Send data based on last UART command */
            if(SendSingleByte || ContinuouslySendData)
            {
                /* Flag set => reached 0.5s threshold */
                if (ADC_flag) 
                {
                    /* Format ADC result for transmition */
                    /* The conversion of ADC value to temperature for this sensor is 10mV = 1 degree Celcius */
                    sprintf(TransmitBuffer, "{ ADC :%lu , Temperature :%.1f , SPI : %.1f , I2C : %d }\r\n", ADCOutput,(float) sum/cnt/10, (float) SPIOutput/10, I2COutput);
                    /* Send out the data */
                    UART_1_PutString(TransmitBuffer);
                    /* Reset flags and values */
                    ADC_flag = FALSE;
                    sum = 0;
                    cnt = 0;
                } //output data 
                else 
                {
                    sum+=ADCOutput;
                    cnt++;
                } //gather data
                /* Reset the send once flag */
                SendSingleByte = FALSE;
            }
        }

    }
}
/* Subprocesses */

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
