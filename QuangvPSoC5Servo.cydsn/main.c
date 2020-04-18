/* ========================================
 *
 * Copyright Quang Minh Vu Metropolia UAS
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CC - BY - SA 4.0
 *   
 * PSoC fundamentals Exercise 2
 * Control Servo movement using PWM
 * 
 * ========================================
*/
#include "project.h"
#include "stdio.h"

CY_ISR_PROTO(ButtonISR_Handler);
CY_ISR_PROTO(PWMISR_Handler);

uint16 val = 1500;
static volatile CYBIT button_flag = 0;
static volatile CYBIT PWM_flag = 0;


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /* Initialization code */
    Clock_1MHz_Start();
    PWM_Servo_Start();
    
    PWM_Servo_WriteCompare(val);    // set servo to neutral position
    
    isr_button_StartEx(ButtonISR_Handler);
    isr_PWM_StartEx(PWMISR_Handler);
    
    for(;;)
    {
        if (button_flag) 
        {
            button_flag = 0;
            LED1_Write(!LED1_Read());
            CyDelay(100);
        }   // notify of button interupt, no interference
    }
}
// Subroutine
void nextpwm(void)
{
    uint16 arr[5] = {500,1000,1500,2000,2400};
    static uint8 count = 0;
    
    val = arr[count++];
    if (count>=5) count = 0;
    
    return;
} //steps for servo

// ISR Routine

CY_ISR(ButtonISR_Handler)
{
    if (PWM_flag)   // PWM flag set => reached the destination
    { 
        nextpwm();
        button_flag = 1;
    }
    Button_ClearInterrupt();
} //Button ISR

CY_ISR(PWMISR_Handler)
{
    static uint16 cval = 1500;
    static uint16 pwmstep = 20;
    
    PWM_flag = 0;
    // cval < val ? (cval += pwmstep) : (cval -= pwmstep);
    if (cval < val) 
    {
        cval += pwmstep;
        PWM_Servo_WriteCompare(cval);   //write to Compare register
    }   // going forward 
    else if (cval > val) 
    {
        cval -= pwmstep; 
        PWM_Servo_WriteCompare(cval);   //write to Compare register
    }   // going backward
    else if (cval == val)
        PWM_flag = 1;
    PWM_Servo_ReadStatusRegister(); //reset interupt
}   //PWM ISR
/* [] END OF FILE */
