// #define timeSeconds 10
// #include "Arduino.h"
// // Set GPIOs for LED and PIR Motion Sensor
// const int led = 2;
// const int motionSensor = 34;

// // Timer: Auxiliary variables
// unsigned long now = millis();
// unsigned long lastTrigger = 0;
// boolean startTimer = false;

// // Checks if motion was detected, sets LED HIGH and starts a timer
// void IRAM_ATTR detectsMovement() {
//   Serial.println("MOTION DETECTED!!!");
//   digitalWrite(led, HIGH);
//   startTimer = true;
//   lastTrigger = millis();
// }

// void initInfrared(void *param)
// {
//   pinMode(motionSensor, INPUT);
//   int stateMotion=0;
//   while(1){
//     stateMotion = digitalRead(motionSensor);
//     //Serial.println(stateMotion);
//     if(stateMotion){
//       Serial.println("MOTION DETECTED!!!");
//       digitalWrite(led, HIGH);
//       delay(5000);
//       digitalWrite(led, LOW);
//       Serial.println("Stopped motion");
//     }
    
//     vTaskDelay(100);
//   }
//   /* pinMode(motionSensor, INPUT_PULLUP);
//   // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
//   attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
//   // Set LED to LOW
//   //pinMode(led, OUTPUT);
//   digitalWrite(led, LOW); */

// }

// void runInfrared() {
//   // Current time
//    now = millis();
//   // Turn off the LED after the number of seconds defined in the timeSeconds variable
//   if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
//     Serial.println("Motion stopped...");
//     digitalWrite(led, LOW);
//     startTimer = false;
//   }
   
// }