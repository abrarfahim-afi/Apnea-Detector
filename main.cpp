/*
  _____________________RTES Final Project______________________
  *************************************************************
  Group Member Name           |||       NetID
  Abrar Fahim                  |        af4175
  Vincent Lee                  |        vl1240
  Hyeong Cho                   |        hjc423
  Raj Dharmesh Oza             |        ro2151
***************************************************************
*/

/*
___________________________________________Case for the project_________________________________________________
****************************************************************************************************************
The Sudden Infant Death Syndrome (SIDS) happens to babies under the age of 1 year. 
Were a baby experiences a sudden collapse of breathing cycle which in turns result
in an unexpected death.

For embedded challenge Fall 2022 we have developed a wearable sensor(Based on Long Flex sensor) based breathing
cycle detection system which can detect a sudden stop of breathing of a baby and send a warning by turning on an 
LED of the STM32F429 discovery board.
*/

/*
______________________________________________Materials Used_____________________________________________________
******************************************************************************************************************
For this project the STM32F429 Discovery Board was used as the brain of the whole project.
For sensing the breathing cycle the we have built a used a wearable sensor based on the Long Flex Sensor and an 
elastic velcro belt. Which changes its resistance based on how much the sensor is bended. The sensor has a flat 
resistance of 10K Ohms and bend resistance range of 60K Ohms to 110K Ohms. Now to get a resistance to voltage 
value directly we added an amplification circuit based on LM358. This circuit is very used interms of detecting 
very low degree bending. This circuit also removes some low margin noises from the signal. The value from the 
resistance to voltage converter circit is the fed to the GPIO port A pin 6 which has an internal connection
to the ADC. To show the warning of a stopped breathing cycle, the LCD screen and LED4 of the board was used.
*/

/*
_______________________________________________________________________Algorithm____________________________________________________________________________
************************************************************************************************************************************************************
Step0: The variable are declared and initialized.

Step1: At the beginning of the main function the the polling timer gets started and the program moves to an infinite while loop

Step2: The polling timer gets reseted to 0 ms. The sensor value is read from PA_6 and sensor voltage gets calculated from that.
Difference between the present sensor value and previous sensor value gets calculated.

Step3: The MCU takes takes one of the below steps based on the value difference of the current value of the sensor and the previous value of the sensor
     
     step3a: If the the value difference of the current value of the sensor and the previous value of the sensor becomes greater than the threshold 
     value (which is set by trial and error and depends only on the sensor itself rather than body time of the baby) the buzzer_timer_counter
     is again reseted to 0 msec and the LED is turned off. Which means the regular breathing cycle has resumed again. At this case the LCD  screen's background 
     becomes green and shows the current sensor value and the converted voltage value on the screen. Then the program moves to the next step.

     step3b: If the value difference of the current value of the sensor and the previous value of the sensor becomes less than the threshold value,
     the current value of the sensor gets stored in the previous value of the sensor. After that the time required to poll a sensor value gets 
     stored in the time_elapsed variable which is then added to the buzzer_timer_counter. 
     (For this case the LCD Screen's background remains green and shows the above mentioned value )
  
  Step4: The MCU takes takes one of the below steps based on the value of the buzzer_timer_counter variable value (in msec)
    
    step4a: Now if the buzzer_timer_counter value becomes more than or equal to 10000 msec then that indicates that the baby has stopped breathing for
    more than 10 seconds and turns on the LED4 of the MCU along with that the LCD screens background urns red and showhs how long the baby has stopped breathing (in msec).
    The the LCD screens red background and the LED will be on untill the baby resumes the breathing cycle or the reset button of the MCU is pressed

    step4b: Else the program will again go back to the top of the infinite while loop and follow the above mentioned steps
_______________________________________________________________________________________________________________________________________________________________
*/

// NOTE:
// Regular breathing cycle means Breath In and Breath Out
// Irregular breathing cycle means either the breathing stops at Breath In or Breath Out position. 


#include<mbed.h>
#include<stdio.h>

#include "drivers/LCD_DISCO_F429ZI.h"
#define BACKGROUND 1  // Background for the LCD Display
#define FOREGROUND 0  // Foreground for the LCD Display
#define GRAPH_PADDING 5

/* ____________________Variables and functions for Display________________________*/
LCD_DISCO_F429ZI lcd;
//buffer for holding displayed text strings
char display_buf[3][60];

//sets the background layer 
//to be visible, transparent, and
//resets its colors to all black
void setup_background_layer(){
  lcd.SelectLayer(BACKGROUND);
  lcd.Clear(LCD_COLOR_DARKRED);
  lcd.SetBackColor(LCD_COLOR_RED);
  lcd.SetTextColor(LCD_COLOR_GREEN);
  lcd.SetLayerVisible(BACKGROUND,ENABLE);
  lcd.SetTransparency(BACKGROUND,0x7Fu);
}

void setup_background_layer_green(){
  lcd.SelectLayer(BACKGROUND);
  lcd.Clear(LCD_COLOR_GREEN);
  lcd.SetBackColor(LCD_COLOR_GREEN);
  lcd.SetTextColor(LCD_COLOR_LIGHTMAGENTA);
  lcd.SetLayerVisible(BACKGROUND,ENABLE);
  lcd.SetTransparency(BACKGROUND,0x7Fu);
}

//resets the foreground layer to
//all RED
void setup_foreground_layer(){
    lcd.SelectLayer(FOREGROUND);
    lcd.Clear(LCD_COLOR_DARKRED);
    lcd.SetBackColor(LCD_COLOR_RED);
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
}

void setup_foreground_layer_green(){
    lcd.SelectLayer(FOREGROUND);
    lcd.Clear(LCD_COLOR_GREEN);
    lcd.SetBackColor(LCD_COLOR_GREEN);
    lcd.SetTextColor(LCD_COLOR_LIGHTMAGENTA);
}
/*_________________________________________________________________________*/


/*__________________Main Variables for the system design___________________*/
volatile float sensor_value;      
volatile float sensor_voltage;

volatile uint32_t time_elapsed;
uint32_t buzzer_timer_counter;  // Holds the sum of the milisecond time value when the baby stops the regular breathing cycle which. Whenever the regular breathing cycle resumes, the value of the variable goes to zero 

float value_difference; //The difference of a voltage reading from one time step to the other.
float previous_value = 0; //Initialization of the previous value of the sensor

// The threshold value has been set based on trail and error.
float value_threshold = 0.0048; //The sensitivity in which the sensor will determine if a person is breathing. Sensitivity increases with the decrease of the value.

AnalogIn sensor_input(PA_6); // GPIO Port-A, PIN-6 is used
DigitalOut led(LED4); //Inbuilt LED4 is used.
Timer polling_timer; // Allows us to track the 10 seconds duration (or more).


int main() {

  polling_timer.start(); // Starting the timer.
  while(1)
  {
    polling_timer.reset();
    sensor_value = sensor_input.read(); // Reads the sensor value at every time step.
    sensor_voltage = sensor_value *3.3f; //Converts the ADC value to the voltage value.
    printf("Read value = %f V Read voltage : %.3f\n ",sensor_value,sensor_voltage);
    value_difference = abs(sensor_value - previous_value); //Tracks the difference of current value of the sensor and the previous value one from the current timestep to the last (the value difference would rise if the patient breathes in/out)
    
    if(value_difference >= value_threshold)  //Value Difference is compared against the Value Threshold we set above. The following happens if we trip this amount:
    {
      buzzer_timer_counter = 0; //Timer resets to 0.
      led.write(0); //In the case that 10 seconds has elapsed and the LED is turned on, breathing in or out will turn on this line.
      
      // Turns the LCD Screen to green
      setup_background_layer_green();
      setup_foreground_layer_green();
      
      //creates c-strings in the display buffers, in preparation
      //for displaying them on the screen
      snprintf(display_buf[0],60,"Sensor Value %.3f",sensor_value);
      snprintf(display_buf[1],60,"Voltage %.3f V",sensor_voltage);
      lcd.SelectLayer(FOREGROUND);
      //display the buffered string on the screen
      lcd.DisplayStringAt(0, LINE(10), (uint8_t *)display_buf[0], CENTER_MODE);
      lcd.DisplayStringAt(0, LINE(14), (uint8_t *)display_buf[1], CENTER_MODE);
  
    }
    
    printf("Value Difference: %.4f ", value_difference);
    previous_value = sensor_value; //Set the previous value as the sensor value, since our approach tracks change, and doesn't care about the base size of someone. The product is designed for one size fitting all.
    time_elapsed = polling_timer.read_ms(); //Depending on computer run performance, this value is different. If we have quicker loops, this value is smaller.
    buzzer_timer_counter = buzzer_timer_counter + time_elapsed; //Pretty much real-time second tracker. 1 Second = 1000.
    printf("Buzzer Timer Elapsed: %lu ", buzzer_timer_counter);
    
    if(buzzer_timer_counter >= 10000 )  //If the system reaches above 10000 msec the LED will be turned on. The system will run beyond this value, until breathing cycle resumes which means that the program will be back to the first if condition.
          {
            led.write(1); //The LED is turned on.
            
            // Turns the LCD screen to RED
            setup_background_layer();
            setup_foreground_layer();
            
            //creates c-strings in the display buffers, in preparation
            //for displaying them on the screen
            snprintf(display_buf[0],60,"Baby Stopped");
            snprintf(display_buf[1],60,"Breathing ");
            snprintf(display_buf[2],60,"for %lu milisecond",buzzer_timer_counter);
            lcd.SelectLayer(FOREGROUND);
            //display the buffered string on the screen
            lcd.DisplayStringAt(0, LINE(10), (uint8_t *)display_buf[0], CENTER_MODE);
            lcd.DisplayStringAt(0, LINE(12), (uint8_t *)display_buf[1], CENTER_MODE);
            lcd.DisplayStringAt(0, LINE(14), (uint8_t *)display_buf[2], CENTER_MODE);
          }
    else
         continue; // The program will go back to the top of the while loop.




  }
}
