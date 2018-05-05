// TimerOne & TimerThree Libraries
//  https://www.pjrc.com/teensy/td_libs_TimerOne.html
#include <TimerOne.h>
// Arduino Joystick Library
//  https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Joystick.h>

// Arduino Pro Micro (32U4) - This part can not change.
// PORTB D11 D10 D9 D8 MISO MOSI SCLK TX_LED
#define S0 8
#define S1 9
// PIND 32U4 D6 D12 RX D4 D1 D0 D2 D3
#define Y0 2
#define Y1 3
#define Y2 4
#define Y3 6

// 512 Hz - 128 Hz per controller
#define SCAN_FREQ 976

#define JOYSTICK_BUTTON_COUNT 9
#define JOYSTICK_CONNECTION 1

Joystick_ Joystick(
    JOYSTICK_DEFAULT_REPORT_ID,
    JOYSTICK_TYPE_GAMEPAD,
    JOYSTICK_BUTTON_COUNT,
    JOYSTICK_CONNECTION,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false
);

uint8_t page[4] = { B00000000, B00010000, B00100000, B00110000 };
uint8_t lastButtonState[3][4] = {
    { 1, 1, 0, 0 },     // L  -  -  - 
    { 1, 1, 1, 1 },     // R  X  Y  Z
    { 1, 1, 1, 1 }      // S  A  C  B
};
uint8_t buttonMap[3][4] = {
    { 4, 9, 9, 9 },     // L  -  -  - 
    { 6, 0, 3, 5 },     // R  X  Y  Z
    { 8, 1, 7, 2 }      // S  A  C  B
};
uint8_t lastHatState = B00001111;
int16_t roundTable[16];

void scanKeys(void)
{
    // read current state
    uint8_t currentButtonState[3][4];
    uint8_t currentHatState;
    for(int i = 0; i < 4; i++) {
        PORTB = page[i];
        uint8_t currentButton = PIND;
        if(i < 3) {
            // buttons
            currentButtonState[i][0] = (currentButton >> 7) & B00000001; // D3
            currentButtonState[i][1] = (currentButton >> 4) & B00000001; // D2
            currentButtonState[i][2] = (currentButton >> 0) & B00000001; // D1
            currentButtonState[i][3] = (currentButton >> 1) & B00000001; // D0
        } else {
            // hat switch
            currentHatState = 
                ((currentButton & B00000010) << 2) | // D0 Up
                ((currentButton & B10000000) >> 5) | // D3 Rt
                ((currentButton & B00000001) << 1) | // D1 Dn
                ((currentButton & B00010000) >> 4);  // D2 Lt
        }
    }

    // convert joystick button
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 4; j++) {
            if(lastButtonState[i][j] != currentButtonState[i][j]) {
                Joystick.setButton(buttonMap[i][j], currentButtonState[i][j] ^ B00000001);
                lastButtonState[i][j] = currentButtonState[i][j];
            }
        }
    }

    // convert joystick hat switch
    if(lastHatState != currentHatState) {
        Joystick.setHatSwitch(0, roundTable[currentHatState]);
        lastHatState = currentHatState;
    }
}

void setup()
{
    // hat keytable init
    roundTable[B00001111] = -1;
    roundTable[B00000111] = 0;
    roundTable[B00000011] = 45;
    roundTable[B00001011] = 90;
    roundTable[B00001001] = 135;
    roundTable[B00001101] = 180;
    roundTable[B00001100] = 225;
    roundTable[B00001110] = 270;
    roundTable[B00000110] = 315;

    // init page pins
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    digitalWrite(S0, LOW);
    digitalWrite(S1, LOW);

    // init data pins
    pinMode(Y0, INPUT_PULLUP);
    pinMode(Y1, INPUT_PULLUP);
    pinMode(Y2, INPUT_PULLUP);
    pinMode(Y3, INPUT_PULLUP);

    Joystick.begin();

    Timer1.initialize(SCAN_FREQ);
    Timer1.attachInterrupt(scanKeys);
}

void loop()
{
    delay(100);
}
