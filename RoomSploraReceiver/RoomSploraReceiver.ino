#include <SoftwareSerial.h>

#define PIN_ROOMBA_RX   3
#define PIN_ROOMBA_TX   4
#define PIN_XBEE_RX     5
#define PIN_XBEE_TX     6
#define PIN_LED         13
#define TOLERANCE_X     50
#define TOLERANCE_Y     50

const boolean IGNORE_SAFETY = true;
SoftwareSerial Roomba(PIN_ROOMBA_RX, PIN_ROOMBA_TX);
SoftwareSerial XBee(PIN_XBEE_RX, PIN_XBEE_TX);
boolean wasControlMode = false;
boolean wasSideBrushMotor = false;
boolean wasVacuumMotors = false;
String currInstruction = "";

void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(115200);
    Roomba.begin(115200);
    XBee.begin(9600);
    Serial.println("BOOTED!");
}

void loop()
{
    if(XBee.available())
    {
        char input = (char)XBee.read();

        if(input == '\n')
        {
            ReadInput(currInstruction);
            currInstruction = "";
        }
        else
        {
            currInstruction += input;
        }
    }
}

void ReadInput(String input)
{
    while(input.startsWith("|"))
    {
        input = input.substring(1);
    }

    String xStr = "";

    for(int i = 1; i < input.length(); i++)
    {
        if(input.charAt(i) == '|')
        {
            xStr = input.substring(0, i);
            break;
        }
    }

    input = input.substring(xStr.length() + 1);

    String yStr = input;

    for(int i = 1; i < input.length(); i++)
    {
        if(input.charAt(i) == '|')
        {
            yStr = input.substring(0, i);
            break;
        }
    }

    input = input.substring(yStr.length() + 1);
    input = input.substring(0, input.length() - 2);

    short x = (short)xStr.toInt();
    short y = (short)yStr.toInt();
    boolean UNUSED_1 = (input.charAt(0) == 'Y');
    boolean controlMode = (input.charAt(1) == 'Y');
    boolean UNUSED_2 = (input.charAt(2) == 'Y');
    boolean sideBrushMotor = (input.charAt(3) == 'Y');
    boolean vacuumMotors = (input.charAt(4) == 'Y');

    if(ToggleControlMode(controlMode))
    {
        HandleMotorChanged(sideBrushMotor, vacuumMotors);
        Steer(x, y);
    }
}

void Steer(short x, short y)
{
    //Allow some calibration tolerance
    if(abs(y) < (short)TOLERANCE_Y)
    {
        y = 0;
    }

    if(abs(x) < (short)TOLERANCE_X)
    {
        x = 0;
    }

    Serial.print(y);
    Serial.print("\t");
    Serial.print(x);

    if(y > 0)
    {
        y += abs(x);
    }
    else if(y < 0)
    {
        y -= abs(x);
    }
    else
    {
        y = x;
    }

    Serial.print("\t");
    Serial.print(y);
    Serial.print("\t");
    Serial.print(x);

    //Convert joystick angle to radius
    if(x < 0)
    {
        x = -2000 - x;
    }
    else if(x > 0)
    {
        x = 2000 - x; 
    }
    else if(y != 0) //If joystick not centered
    {
        x = 32767; //Straight
    }

    if(x < -2000)
    {
        x = -2000;
    }
    else if(x > 2000 && x < 32767) //Ignore "Straight"
    {
        x = 2000;
    }

    if(y < -500)
    {
        y = -500;
    }
    else if(y > 500)
    {
        y = 500;
    }

    Roomba.write(137); //Drive
    Roomba.write((byte)highByte(y));
    Roomba.write((byte)lowByte(y));
    Roomba.write((byte)highByte(x));
    Roomba.write((byte)lowByte(x));

    Serial.print("\t");
    Serial.print(y);
    Serial.print("\t");
    Serial.println(x);
}

boolean ToggleControlMode(boolean controlMode)
{
    if(!controlMode)
    {
        if(wasControlMode)
        {
            wasControlMode = false;
            PowerDown();
        }

        return false;
    }
    else if (!wasControlMode)
    {
        PowerUp();
        wasControlMode = true;
    }

    return true;
}

void HandleMotorChanged(boolean sideBrushMotor, boolean vacuumMotors)
{
    if(sideBrushMotor != wasSideBrushMotor || vacuumMotors != wasVacuumMotors)
    {
        wasSideBrushMotor = sideBrushMotor;
        wasVacuumMotors = vacuumMotors;

        Roomba.write(138); //Motors
        if(sideBrushMotor && vacuumMotors)
        {
            Roomba.write((byte)0x07); //Side, vacuum, main
        }
        else if(sideBrushMotor)
        {
            Roomba.write((byte)0x01); //Side ONLY
        }
        else if(vacuumMotors)
        {
            Roomba.write((byte)0x06); //Vacuum and main ONLY
        }
        else
        {
            Roomba.write((byte)0x00); //Disable all vacuum motors
        }
    }
}

void PowerDown()
{
    digitalWrite(PIN_LED, LOW); //POWER DOWN
    Roomba.write(137); //Drive
    Roomba.write((byte)0x00);
    Roomba.write((byte)0x00);
    Roomba.write((byte)0x00);
    Roomba.write((byte)0x00);
    Roomba.write(138); //Motors
    Roomba.write((byte)0x00);
    Roomba.write(133); //Power Down
    Serial.println("Power off");
}

void PowerUp()
{
    Roomba.write(128); //Start ROI
    Serial.println("Power on");
    delay(150);
    Roomba.write(IGNORE_SAFETY ? 132 : 131); //Set ROI Mode
    digitalWrite(PIN_LED, HIGH); //Ready for action!
}
