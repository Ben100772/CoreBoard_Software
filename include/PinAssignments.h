#ifndef PIN_ASSIGNMENTS_H
#define PIN_ASSIGNMENTS_H

//Drive Pins
<<<<<<< HEAD
#define FL_TX           1
#define FL_RX           0
#define ML_TX           29
#define ML_RX           28
#define BL_TX           35
#define BL_RX           34

#define FR_TX           14
#define FR_RX           15
#define MR_TX           17
#define MR_RX           16
#define BR_TX           20
#define BR_RX           21
=======
#define FL_TX           35
#define FL_RX           34
#define ML_TX           14
#define ML_RX           15
#define BL_TX           20
#define BL_RX           21

#define FR_TX           29
#define FR_RX           28
#define MR_TX           24
#define MR_RX           25
#define BR_TX           1
#define BR_RX           0
>>>>>>> b274a414a15913f7953c934dea64611fc9ab7cb1

#define FL_SERIAL       Serial8
#define ML_SERIAL       Serial3
#define BL_SERIAL       Serial5
#define FR_SERIAL       Serial7
#define MR_SERIAL       Serial6
#define BR_SERIAL       Serial1

<<<<<<< HEAD
//Rotary Bits
#define RTRY1        26
#define RTRY2        30
#define RTRY4        31

//Selection LEDs
#define FR_EN        39
#define FL_EN        36
#define MR_EN        40
#define ML_EN        37
#define BR_EN        41
#define BL_EN        38

//Fan and Temperature Pins
#define FTACH1      3
#define FTACH2      4
#define FPWM1       33
#define FPWM2       9
#define TEMP_SDA         18
#define TEMP_SCL         19


//Lighting Pins
#define RGB_Strip        25
#define LightingPanel    10


=======
//Servo Pins
#define LEFT_TILT_SERVO       8
#define LEFT_PAN_SERVO        7
#define RIGHT_TILT_SERVO      6
#define RIGHT_PAN_SERVO       2
#define BACK_TILT_SERVO       11
#define BACK_PAN_SERVO        12
#define SPARE_1_SERVO         22
#define SPARE_2_SERVO         5

//NeoPixels Pins
#define BACK_STRIP_PIN        17
#define INNER_STRIP_PIN       16

//Button Pins
#define RTRY_1          26
#define RTRY_2          30
#define RTRY_4          31

#define DIR_FORWARD     32
#define DIR_BACK        13
#define DIR_RIGHT       23
#define DIR_LEFT        27

//Motor Switches
#define FL_SWITCH       36
#define ML_SWITCH       37
#define BL_SWITCH       38
#define FR_SWITCH       39
#define MR_SWITCH       40
#define BR_SWITCH       41
>>>>>>> b274a414a15913f7953c934dea64611fc9ab7cb1

//Sensor I2C Pins
#define SENS_SDA         18
#define SENS_SCL         19

<<<<<<< HEAD
// Button Assignments 
#define Back            13
#define Forward         32
#define Left            27
#define Right           23

// Servos
#define LeftPan         7      
#define LeftTilt        8
#define RightPan        2
#define RightTilt       6
#define BackPan         12
#define BackTilt        11
#define Spare1          22
#define Spare2          5

#endif
=======
//Fan Pins
#define FAN_PWM_1        10
#define FAN_PWM_2        9
#define FAN_TACH_1       3
#define FAN_TACH_2       4

#endif
>>>>>>> b274a414a15913f7953c934dea64611fc9ab7cb1
