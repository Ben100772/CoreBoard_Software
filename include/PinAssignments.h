#ifndef PIN_ASSIGNMENTS_H
#define PIN_ASSIGNMENTS_H

//Drive Pins
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

#define FL_SERIAL       Serial7
#define ML_SERIAL       Serial6
#define BL_SERIAL       Serial8
#define FR_SERIAL       Serial5
#define MR_SERIAL       Serial4
#define BR_SERIAL       Serial3

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



//Accelerometer Pins
#define ACC_SDA         18
#define ACC_SCL         19

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