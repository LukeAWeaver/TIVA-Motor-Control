#include    <stddef.h>
#include    <stdbool.h>
#include    <stdint.h>
#include    <stdarg.h>

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_timer.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/qei.c"
#include "FreeRTOS.h"
#include    "task.h"
#include    "Tasks/Task_ReportData.h"


char userInput[3];
int32_t currentMultiplier;
uint32_t servoPW;
uint32_t servoPeriod;
int32_t servoPosition;
int32_t qeipos;
char mode[1];

extern void Task_PWM( void *pvParameters ) {

    ReportData_Item        QeiInfo;


    servoPeriod = 120000000/3200;
    servoPW = 2813;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG); //port G
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); //port L

    GPIOPinConfigure(GPIO_PG1_M0PWM5); //pg p1
    GPIOPinConfigure(GPIO_PL1_PHA0); //pg L1
    GPIOPinConfigure(GPIO_PL2_PHB0); //pg L2

    GPIOPinTypeQEI(GPIO_PORTL_BASE, GPIO_PIN_1);
    GPIOPinTypeQEI(GPIO_PORTL_BASE, GPIO_PIN_2);
    //pwm
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);
    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_64);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, (PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC));
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, servoPeriod);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, (servoPW));
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
    PWMOutputState(PWM0_BASE, (PWM_OUT_5_BIT), true);

    //qei
      QEIConfigure(QEI0_BASE,(QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_RESET_IDX | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP),1120);
      QEIEnable (QEI0_BASE);
      QEIVelocityEnable(QEI0_BASE);
      QEIVelocityConfigure(QEI0_BASE,QEI_VELDIV_1,1120000);
      QEIPositionSet(QEI0_BASE,560); //set baseline
    /*
     *     servoPeriod = 40704; //16 least signifcant bits 1001111100000000. 20ms
        servoPW = 43392; //54464 cw, 43392 ccw
        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

        GPIOPinConfigure(GPIO_PA7_T3CCP1); //Sets up pin to use alternative function. The General Purpose timer. Timer B Base 3 has 3 different modes: Capture, Compare, and PWM
        GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_7); //Sets as a timer pin.

        TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_B_PWM); //Specifies mode as PWM.
        TimerMatchSet(TIMER3_BASE, TIMER_B, servoPW);                    //Initialized Pulse Width value
        TimerPrescaleMatchSet(TIMER3_BASE,TIMER_B,3); //counter clock wise, 2 clockwise
        TimerPrescaleSet(TIMER3_BASE,TIMER_B,36); //8 most significant bits 100100, extends of 16-bit timer total val: 2400000
        TimerLoadSet(TIMER3_BASE, TIMER_B, servoPeriod);                        //Set period
        TimerEnable(TIMER3_BASE, TIMER_B);                                    //enables timer
     */
        UARTprintf("Press 1 to set velocity OR press 2 to set position.");
        UARTgets(mode,2);
        while (1) {
            if(mode[0] == '1')
            {
            UARTprintf("Set speed (-100 - 100) : ");
            //alternate between high and low pulse widths
            userInput[0] = NULL;
            userInput[1] = NULL;
            userInput[2] = NULL;
            userInput[3] = NULL;

            //set pulse width
            //TimerMatchSet(TIMER3_BASE, TIMER_B, servoPW);
            vTaskDelay( ( 5000 * configTICK_RATE_HZ ) / 10000 );
            UARTgets(userInput,5);
            currentMultiplier = atoi(userInput);
            if(currentMultiplier>-101 && currentMultiplier<101)
            {
         //1875 - 3750
            		servoPW = 2813 + (937*currentMultiplier)/100;
            }
            else
            {
                UARTprintf("%d is not a valid input. Please choose a value between -100 and 100 inclusive. \n",currentMultiplier);
            }
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, (servoPW));

            UARTprintf("user input: %d \n",currentMultiplier);
            UARTprintf("current Pulse Width Value: %d \n",servoPW);

            vTaskDelay( ( 5000 * configTICK_RATE_HZ ) / 10000 );

            QeiInfo.TimeStamp = xPortSysTickCount;
            QeiInfo.ReportName = 1;
            QeiInfo.ReportValue_0 = QEIVelocityGet(QEI0_BASE);
            QeiInfo.ReportValue_1 = QEIDirectionGet(QEI0_BASE);
            QeiInfo.ReportValue_2 = QEIPositionGet(QEI0_BASE);
            QeiInfo.ReportValue_3 = 0;
            xQueueSend(  ReportData_Queue, &QeiInfo, 0 );


            //UARTprintf("Position: %d \n",QEIPositionGet(QEI0_BASE));
            //UARTprintf("Velocity: %d \n",QEIVelocityGet(QEI0_BASE));
            //UARTprintf("Direction: %d \n",QEIDirectionGet(QEI0_BASE));
            currentMultiplier = 0;
            }


            else if(mode[0] == '2')
            {
                //get user input
            	qeipos = QEIPositionGet(QEI0_BASE);
            UARTprintf("Set position in degrees (-180 - 180): ");
            userInput[0] = NULL;
            userInput[1] = NULL;
            userInput[2] = NULL;
            userInput[3] = NULL;
            vTaskDelay( ( 5000 * configTICK_RATE_HZ ) / 10000 );
            UARTgets(userInput,5);
            currentMultiplier = atoi(userInput);
            if(currentMultiplier>-181 && currentMultiplier<181){

                servoPosition = 560+ (currentMultiplier*560)/180; //destination
            }
            else{
                UARTprintf("%d is not a valid input. Please choose a value between -180 and 180 inclusive. \n",servoPosition);
                servoPosition = 560;
            }
            UARTprintf("Current Position: %d \n",QEIPositionGet(QEI0_BASE));
            UARTprintf("Desired Position: %d \n",servoPosition);
            vTaskDelay( ( 5000 * configTICK_RATE_HZ ) / 10000 );

            //Determine which direction to go

            if(qeipos - servoPosition > 0) //ccw
            {
                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, (2700));
            }
            else if(qeipos - servoPosition == 0)
            {
                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, (2813)); //zero
            }
            else
            {
                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, (2950)); //cw
            }

            while(abs((qeipos - servoPosition)) > 2)
            {

                qeipos = QEIPositionGet(QEI0_BASE);
                vTaskDelay( configTICK_RATE_HZ / 1000 );
            }
            UARTprintf("New Current Position: %d \n",QEIPositionGet(QEI0_BASE));
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, (2813));

            //report data
            QeiInfo.TimeStamp = xPortSysTickCount;
            QeiInfo.ReportName = 2; //1 is velocity, 2 is position
            QeiInfo.ReportValue_0 = QEIPositionGet(QEI0_BASE);
            QeiInfo.ReportValue_1 = 0;
            QeiInfo.ReportValue_2 = 0;
            QeiInfo.ReportValue_3 = 0;
            xQueueSend(  ReportData_Queue, &QeiInfo, 0 );
            currentMultiplier = 0;
            servoPosition = 560;
            }


            else
            {
                UARTprintf("Enter a valid option 1 or 2.");
                UARTgets(mode,2);
            }
        }
}

