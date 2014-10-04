#include <Esplora.h>
#include <SoftwareSerial.h>

#define PIN_XBEE_RX          3
#define PIN_XBEE_TX          11
#define SEND_DELAY           100
#define BUTTON_CHECK_DELAY   50
#define JOYSTICK_RANGE_IN    510
#define JOYSTICK_RANGE_OUT_X 2000
#define JOYSTICK_RANGE_OUT_Y 500

SoftwareSerial XBee = SoftwareSerial(PIN_XBEE_RX, PIN_XBEE_TX);
unsigned long dlyJoy = 0;
unsigned long dlyDown = 0;
unsigned long dlyLeft = 0;
unsigned long dlyUp = 0;
unsigned long dlyRight = 0;
unsigned long dlySend = 0;
boolean actJoy = false;
boolean actDown = false;
boolean actLeft = false;
boolean actUp = false;
boolean actRight = false;
boolean tglJoy = false;
boolean tglDown = false;
boolean tglLeft = false;
boolean tglUp = false;
boolean tglRight = false;

void setup()
{
    Serial.begin(57600);
    XBee.begin(9600);
    Serial.println("Booted!");
}

void loop()
{
    if(millis() > dlyJoy)
    {
        dlyJoy = millis() + ((unsigned long)BUTTON_CHECK_DELAY);

        if(!actJoy)
        {
            actJoy = true;

            if (Esplora.readJoystickSwitch() <= 0)
            {
                tglJoy = !tglJoy;
            } 
        }
    }
    if (Esplora.readJoystickSwitch() > 0)
    {
        actJoy = false;
    }
    if(millis() > dlyDown)
    {
        dlyDown = millis() + ((unsigned long)BUTTON_CHECK_DELAY);

        if(!actDown)
        {
            actDown = true;

            if (!Esplora.readButton(SWITCH_1))
            {
                tglDown = !tglDown;
            }
        }
    }
    if(Esplora.readButton(SWITCH_1))
    {
        actDown = false;
    }
    if(millis() > dlyLeft)
    {
        dlyLeft = millis() + ((unsigned long)BUTTON_CHECK_DELAY);
        if(!actLeft)
        {
            actLeft = true;

            if (!Esplora.readButton(SWITCH_2))
            {
                tglLeft = !tglLeft;
            }
        }
    }
    if(Esplora.readButton(SWITCH_2))
    {
        actLeft = false;
    }
    if(millis() > dlyUp)
    {
        dlyUp = millis() + ((unsigned long)BUTTON_CHECK_DELAY);

        if(!actUp)
        {
            actUp = true;

            if (!Esplora.readButton(SWITCH_3))
            {
                tglUp = !tglUp;
            }
        }
    }
    if(Esplora.readButton(SWITCH_3))
    {
        actUp = false;
    }
    if(millis() > dlyRight)
    {
        dlyRight = millis() + ((unsigned long)BUTTON_CHECK_DELAY);

        if(!actRight)
        {
            actRight = true;

            if (!Esplora.readButton(SWITCH_4))
            {
                tglRight = !tglRight;
            }
        }
    }
    if(Esplora.readButton(SWITCH_4))
    {
        actRight = false;
    }

    if(millis() > dlySend)
    {
        dlySend = millis() + ((unsigned long)SEND_DELAY);
        SendMappings();
    }
}

void SendMappings()
{
    String str = "|";
    str += mapJoyX();
    str += "|";
    str += mapJoyY();
    str += "|";
    str += DBtn(tglJoy);
    str += DBtn(tglUp);
    str += DBtn(tglLeft);
    str += DBtn(tglRight);
    str += DBtn(tglDown);
    str += "|";

    XBee.println(str);
}

char* DBtn(boolean btn)
{
    if(btn)
    {
        return "Y";
    }
    else
    {
        return "_";
    }
}

short mapJoyX()
{
    return map(Esplora.readJoystickX(), -512, 511, 2000, -2000);
}

short mapJoyY()
{
    return map(Esplora.readJoystickY(), -512, 511, 500, -500);
}
