#include <Arduino.h>

#include "CoreBoardSoftware.h"

void setup() {
    //Initialize debug serial port
    Serial.begin(9600);
    Serial.println("CoreBoard Setup");

    //Attach Servos to Pins

    leftPanServo.attach(LeftPan, 500, 2500);
    leftTiltServo.attach(LeftTilt, 500, 2500);


    rightPanServo.attach(RightPan, 500, 2500);
    rightTiltServo.attach(RightTilt, 500, 2500);

    backPanServo.attach(BackPan, 500, 2500);
    backTiltServo.attach(BackTilt, 500, 2500);
    
    spare1.attach(Spare1, 500, 2500);
    spare2.attach(Spare2, 500, 2500);

#if USE_RPM_CONTROL
    RoveVESC *motors[6] = { &FL_Motor, &ML_Motor, &BL_Motor, &FR_Motor, &MR_Motor, &BR_Motor };
    for (RoveVESC *motor : motors) {
        motor->configMotorPoles(14);
        motor->configGearRatio(36); // 36 : 1 input/output
        motor->configMaxRPM(840); // About 30 mph
    }
#endif

    //Initialize VESC serial ports
    FL_SERIAL.begin(115200);
    ML_SERIAL.begin(115200);
    BL_SERIAL.begin(115200);
    FR_SERIAL.begin(115200);
    MR_SERIAL.begin(115200);
    BR_SERIAL.begin(115200);
    while(!(FL_SERIAL) || !(ML_SERIAL) || !(BL_SERIAL) || !(FR_SERIAL) || !(MR_SERIAL) || !(BR_SERIAL));

    //Initialize Drive Mode
    driveMode(true);

    //Initialize Buttons 
    pinMode(Back, INPUT);
    pinMode(Forward, INPUT);
    pinMode(Right, INPUT);
    pinMode(Left, INPUT);

    //Initialize Switches
    pinMode(FL_EN, INPUT);
    pinMode(ML_EN, INPUT);
    pinMode(BL_EN, INPUT);
    pinMode(FR_EN, INPUT);
    pinMode(MR_EN, INPUT);
    pinMode(BR_EN, INPUT);

    //Initialize Rotary
    pinMode(RTRY1, INPUT);
    pinMode(RTRY2, INPUT);
    pinMode(RTRY4, INPUT);

    //Initialize NeoPixel
    neoPixel.begin();
    neoPixel.setBrightness(MAX_BRIGHTNESS / 2);

    //Start RoveComm
    Serial.println("RoveComm Initializing...");
    RoveComm.begin(RC_COREBOARD_IPADDRESS);
    Serial.println("Complete.");

    servoStartups();
    feedWatchdog();

    accelerometer.begin();
}

void loop() {
    RoveComm.read(packet);
    
    //Multimedia Packets
    switch(packet.dataId) {
        
        //[R, G, B] -> [(0 - 255), (0 - 255), (0 - 255)]
        case RC_COREBOARD_LEDRGB_DATA_ID:
        {
            uint8_t* data = (uint8_t*)packet.data;
            customDisplayColor = neoPixel.Color(data[0], data[1], data[2]);
            if (customDisplayColor == 0x000000) { // Black
                setDisplayState(DisplayState::OFF);
            } else {
                setDisplayState(DisplayState::CUSTOM);
            }
            break;
        }

        //Flash Pattern selected by data
        case RC_COREBOARD_LEDPATTERNS_DATA_ID:
        {
            uint8_t* data = (uint8_t*)packet.data;
            switch(data[0])
            {
                default:
                    break;
            }

        }

        //[Teleop, Autonomy, Reached Goal] -> Color
        case RC_COREBOARD_STATEDISPLAY_DATA_ID:
        {
            uint8_t* data = (uint8_t*)packet.data;
            switch (data[0])
            {
                case TELEOP:
                    setDisplayState(DisplayState::TELEOP);
                    driveMode(true);
                    break;
                
                case AUTONOMY:
                    setDisplayState(DisplayState::AUTONOMY);
                    driveMode(false);
                    break;

                case REACHED_GOAL:
                    setDisplayState(DisplayState::REACHED_GOAL);
                    break;
            }
            break;
        }

        //Set Brightness (0, 255)
        case RC_COREBOARD_BRIGHTNESS_DATA_ID:
        {
            uint8_t* data = (uint8_t*)packet.data;
            if(data[0] >= MAX_BRIGHTNESS) data[0] = MAX_BRIGHTNESS;
            neoPixel.setBrightness(data[0]);
            neoPixel.show();
            break;
        }

    }

    //Gimbal Packets
    switch (packet.dataId) {

        // Increment left pan and tilt gimbals by [-180, 180]
        case RC_COREBOARD_LEFTMAINGIMBALINCREMENT_DATA_ID:
        {
            int16_t* data = (int16_t*) packet.data;
            leftPanServo.target += data[0];
            leftTiltServo.target += data[1];
            break;
        }

        // Increment right pan and tilt gimbals by [-180, 180]
        case RC_COREBOARD_RIGHTMAINGIMBALINCREMENT_DATA_ID:
        {
            int16_t* data = (int16_t*) packet.data;
            
            rightPanServo.target += data[0];
            rightTiltServo.target += data[1];
            break;
        }
        // Increment back pan and tilt gimbals by [-180, 180]
        case RC_COREBOARD_BACKMAINGIMBALINCREMENT_DATA_ID:
        {
            int16_t* data = (int16_t*) packet.data;

            backPanServo.target += data[0];
            backTiltServo.target += data[1];

        }

    }

    //Drive Packets
    switch(packet.dataId) {
        
        //Set All Left and All Right Motors to a DutyCycle [-1, 1]
        case RC_COREBOARD_DRIVELEFTRIGHT_DATA_ID:
        {
            float* data;
            data = (float*)packet.data;

            float leftSpeed = data[0];
            float rightSpeed = data[1];

            for(int i = 0; i < 6; i++) {
                motorTargets[i] = (i < 3) ? leftSpeed : rightSpeed;
            }

            feedWatchdog();
            break;
        }

        //Set All individual Motors to a DutyCycle [-1, 1]
        case RC_COREBOARD_DRIVEINDIVIDUAL_DATA_ID:
        {
            float* data;
            data = (float*)packet.data;

            for(int i = 0; i < 6; i++) 
                motorTargets[i] = data[i];

            feedWatchdog();
            break;
        }

        case RC_COREBOARD_SETWATCHDOGMODE_DATA_ID:
        {
            uint8_t* data = (uint8_t*) packet.data;

            watchdogMode = data[0];
            break;
        }

    }

    uint32_t now = millis();
    
    if (now - lastDriveUpdate >= DRIVE_UPDATE_PERIOD) {
        manualButtons();

#if USE_RPM_CONTROL
        // because drive() also does speed ramping, we can't do caching like for the servos
        // convert to decipercent so RoveVESC can convert BACK to a float
        FL_Motor.driveRPM(motorTargets[0]);
        ML_Motor.driveRPM(motorTargets[1]);
        BL_Motor.driveRPM(motorTargets[2]);
        FR_Motor.driveRPM(motorTargets[3]);
        MR_Motor.driveRPM(motorTargets[4]);
        BR_Motor.driveRPM(motorTargets[5]);
#else
        FL_Motor.drive((int16_t)(motorTargets[0] * 1000));
        ML_Motor.drive((int16_t)(motorTargets[1] * 1000));
        BL_Motor.drive((int16_t)(motorTargets[2] * 1000));
        FR_Motor.drive((int16_t)(motorTargets[3] * 1000));
        MR_Motor.drive((int16_t)(motorTargets[4] * 1000));
        BR_Motor.drive((int16_t)(motorTargets[5] * 1000));
#endif

        
        leftPanServo.write();
        leftTiltServo.write();
        rightPanServo.write();
        rightTiltServo.write();
        backPanServo.write();
        backTiltServo.write();
        spare1.write();
        spare2.write();

        lastDriveUpdate = now;
    }

    if (now - lastTelemetry >= TELEMETRY_PERIOD) {
        telemetry();
        lastTelemetry = now;
    }

    if (now - lastLightingPanelUpdate >= LIGHTING_PANEL_UPDATE_PERIOD) {
        updateLightingPanel();
        lastLightingPanelUpdate = now;
    }
}
//Manual buttons rework in progress, need to know which of either forward or back is pressed
//Rotary encoder replacing old outputs in schematic(B_ENC_X)
void manualButtons() {
    bool forward = digitalRead(Forward); //Forward is pressed
    bool backwards = digitalRead(Back);  //Back is pressed
    uint8_t rotaryState = (digitalRead(RTRY4)<<2) | (digitalRead(RTRY2)<<1) | (digitalRead(RTRY1)<<0);

    // Servos
    switch(rotaryState)
    {
        case 6:  //Wheels(110)

            if (digitalRead(FL_EN))
            {
                if(forward)
                motorTargets[0] = 0.5;
                if(backwards)
                motorTargets[0] = -0.5;
            } 
            else motorTargets[0] = 0;
    
            if (digitalRead(ML_SWITCH)) motorTargets[1] = (reverse? -0.5 : 0.5);
            else motorTargets[1] = 0;

            if (digitalRead(BL_SWITCH)) motorTargets[2] = (reverse? -0.5 : 0.5);
            else motorTargets[2] = 0;

            if (digitalRead(FR_SWITCH)) motorTargets[3] = (reverse? -0.5 : 0.5);
            else motorTargets[3] = 0;

            if (digitalRead(MR_SWITCH)) motorTargets[4] = (reverse? -0.5 : 0.5);
            else motorTargets[4] = 0;

            if (digitalRead(BR_SWITCH)) motorTargets[5] = (reverse? -0.5 : 0.5);
            else motorTargets[5] = 0;
            break;

        case 1:             //LeftCam(001)
            leftPanServo.target += (reverse? -1 : 1);
            break;

        case 5:             //RightCam(101)
            rightPanServo.target += (reverse? -1 : 1);
            break;

        case 3:             //BackCam(011)
            backDriveServo.target += (reverse? -1 : 1);
            break;

        case 2:             //Spare1(010)
            servo2.target += (reverse? -1 : 1);
            break;
        case 4:             //Spare2(100)
            break;
    }
    //Cases above need to be finished
}

void telemetry() {
    accelerometer.read();

    // hack
    RoveVESC *motors[6] = {&FL_Motor, &ML_Motor, &BL_Motor, &FR_Motor, &MR_Motor, &BR_Motor};
    for (int i = 0; i < 6; i++) {
        VescValues values = motors[i]->getVescTelemetry();
        motorSpeeds[i] = values.rpm;
        motorCurrents[i] = values.avgMotorCurrent;
        vescCurrents[i] = values.avgInputCurrent;
        if (values.error) {
            RoveComm.write(RC_COREBOARD_VESCFAULT_DATA_ID, (uint8_t)values.error);
        }
    }

    RoveComm.write(RC_COREBOARD_ACCELEROMETERDATA_DATA_ID, RC_COREBOARD_ACCELEROMETERDATA_DATA_COUNT, accelerometer.acceleration);
    // RoveComm.write(RC_COREBOARD_MOTORSPEEDS_DATA_ID, RC_COREBOARD_MOTORSPEEDS_DATA_COUNT, motorSpeeds);
    RoveComm.write(RC_COREBOARD_MOTORCURRENTS_DATA_ID, RC_COREBOARD_MOTORCURRENTS_DATA_COUNT, motorCurrents);
    RoveComm.write(RC_COREBOARD_VESCCURRENTS_DATA_ID, RC_COREBOARD_VESCCURRENTS_DATA_COUNT, vescCurrents);
}

void servoStartups() {
    leftDriveServo.write(LEFT_DRIVE_MIN);
    leftPanServo.write(LEFT_PAN_MIN);
    leftTiltServo.write(LEFT_TILT_MIN);
    rightDriveServo.write(RIGHT_DRIVE_MAX);
    rightPanServo.write(RIGHT_PAN_MAX);
    rightTiltServo.write(RIGHT_TILT_MAX);
    backDriveServo.write(BACK_DRIVE_MIN);

    delay(2000);

    leftDriveServo.write(LEFT_DRIVE_MAX);
    leftPanServo.write(LEFT_PAN_MAX);
    leftTiltServo.write(LEFT_TILT_MAX);
    rightDriveServo.write(RIGHT_DRIVE_MIN);
    rightPanServo.write(RIGHT_PAN_MIN);
    rightTiltServo.write(RIGHT_TILT_MIN);
    backDriveServo.write(BACK_DRIVE_MAX);

    delay(2000);
    
    // the below is necessary even tho we send these during every loop and i have no idea why
    leftDriveServo.write(20);
    leftPanServo.write(90);
    leftTiltServo.write(40);
    rightDriveServo.write(160);
    rightPanServo.write(90);
    rightTiltServo.write(140);
    backDriveServo.write(20);

    delay(50);
}

void driveMode(bool isTeleop) {
    if (isTeleop) {
        FL_Motor.configRampRate(TELEOP_MAX_RAMP_RATE);
        FL_Motor.configMaxOutputs(-TELEOP_MAX_SPEED, TELEOP_MAX_SPEED);
        ML_Motor.configRampRate(TELEOP_MAX_RAMP_RATE);
        ML_Motor.configMaxOutputs(-TELEOP_MAX_SPEED, TELEOP_MAX_SPEED);
        BL_Motor.configRampRate(TELEOP_MAX_RAMP_RATE);
        BL_Motor.configMaxOutputs(-TELEOP_MAX_SPEED, TELEOP_MAX_SPEED);
        FR_Motor.configRampRate(TELEOP_MAX_RAMP_RATE);
        FR_Motor.configMaxOutputs(-TELEOP_MAX_SPEED, TELEOP_MAX_SPEED);
        MR_Motor.configRampRate(TELEOP_MAX_RAMP_RATE);
        MR_Motor.configMaxOutputs(-TELEOP_MAX_SPEED, TELEOP_MAX_SPEED);
        BR_Motor.configRampRate(TELEOP_MAX_RAMP_RATE);
        BR_Motor.configMaxOutputs(-TELEOP_MAX_SPEED, TELEOP_MAX_SPEED);
    } else {
        FL_Motor.configRampRate(AUTONOMY_MAX_RAMP_RATE);
        FL_Motor.configMaxOutputs(-AUTONOMY_MAX_SPEED, AUTONOMY_MAX_SPEED);
        ML_Motor.configRampRate(AUTONOMY_MAX_RAMP_RATE);
        ML_Motor.configMaxOutputs(-AUTONOMY_MAX_SPEED, AUTONOMY_MAX_SPEED);
        BL_Motor.configRampRate(AUTONOMY_MAX_RAMP_RATE);
        BL_Motor.configMaxOutputs(-AUTONOMY_MAX_SPEED, AUTONOMY_MAX_SPEED);
        FR_Motor.configRampRate(AUTONOMY_MAX_RAMP_RATE);
        FR_Motor.configMaxOutputs(-AUTONOMY_MAX_SPEED, AUTONOMY_MAX_SPEED);
        MR_Motor.configRampRate(AUTONOMY_MAX_RAMP_RATE);
        MR_Motor.configMaxOutputs(-AUTONOMY_MAX_SPEED, AUTONOMY_MAX_SPEED);
        BR_Motor.configRampRate(AUTONOMY_MAX_RAMP_RATE);
        BR_Motor.configMaxOutputs(-AUTONOMY_MAX_SPEED, AUTONOMY_MAX_SPEED);
    }
}

void setDisplayState(DisplayState newState) {
    displayState = newState;
    lightingPanelChanged = true;
    displayStateProgress = 0;
}

void updateLightingPanel() {
    switch (displayState) {
        case DisplayState::OFF:
            neoPixel.clear();
            break;
        case DisplayState::TELEOP:
            neoPixel.fill(0x0000FF); // Blue
            break;
        case DisplayState::AUTONOMY:
            neoPixel.fill(0xFF0000); // Red
            break;
        case DisplayState::REACHED_GOAL:
        {
            uint32_t lastColor = neoPixel.getPixelColor(0);
            uint32_t nextColor = (displayStateProgress / 1000) % 2 == 0 ? 0x00FF00 : 0x000000; // Blink green each second
            if (lastColor != nextColor) {
                lightingPanelChanged = true;
            }
            neoPixel.fill(nextColor);
            break;
        }
        case DisplayState::CUSTOM:
            neoPixel.fill(customDisplayColor);
            break;
    }
    if (lightingPanelChanged) {
        neoPixel.show(); // this takes like 7ms so we want to call it as little as possible.
        lightingPanelChanged = false;
    }

    displayStateProgress += LIGHTING_PANEL_UPDATE_PERIOD;
}

void estop() {
    if(!watchdogOverride) {
        for(int i = 0; i < 6; i++) {
            motorTargets[i] = 0;
            motorSpeeds[i] = 0;
            motorCurrents[i] = 0;
            vescCurrents[i] = 0;
        }
    }
}

void feedWatchdog() {
    watchdog.begin(estop, (watchdogMode? WATCHDOG_TIMEOUT_AUTONOMY : WATCHDOG_TIMEOUT_TELEOP));
}
