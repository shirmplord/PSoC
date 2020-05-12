/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
/* Delays declaration */
int A,B,C,D,E,F,G,H,I,J;

/* Subprocesses definitions */
void SetSpeed();
int OWTouchReset(void);
void OWWriteByte(int data);
unsigned char OWReadByte(void);
unsigned char OWCRC(unsigned char *pBuf, int len);

/* [] END OF FILE */
