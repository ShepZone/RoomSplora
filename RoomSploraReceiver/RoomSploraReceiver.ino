#include <SoftwareSerial.h>

#define PIN_ROOMBA_RX   3
#define PIN_ROOMBA_TX   4
#define PIN_XBEE_RX     5
#define PIN_XBEE_TX     6
#define PIN_LED         13

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

    String leftStr = "";

    for(int i = 1; i < input.length(); i++)
    {
        if(input.charAt(i) == '|')
        {
            leftStr = input.substring(0, i);
            break;
        }
    }

    input = input.substring(leftStr.length() + 1);

    String rightStr = input;

    for(int i = 1; i < input.length(); i++)
    {
        if(input.charAt(i) == '|')
        {
            rightStr = input.substring(0, i);
            break;
        }
    }

    input = input.substring(rightStr.length() + 1);
    input = input.substring(0, input.length() - 2);

    short left = (short)leftStr.toInt();
    short right = (short)rightStr.toInt();
    boolean UNUSED_1 = (input.charAt(0) == 'Y');
    boolean controlMode = (input.charAt(1) == 'Y');
    boolean UNUSED_2 = (input.charAt(2) == 'Y');
    boolean sideBrushMotor = (input.charAt(3) == 'Y');
    boolean vacuumMotors = (input.charAt(4) == 'Y');

    if(ToggleControlMode(controlMode))
    {
        HandleMotorChanged(sideBrushMotor, vacuumMotors);
        Steer(left, right);
    }
}

void Steer(short left, short right)
{
    Roomba.write(145); //Drive Direct
    Roomba.write((byte)highByte(right));
    Roomba.write((byte)lowByte(right));
    Roomba.write((byte)highByte(left));
    Roomba.write((byte)lowByte(left));
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
    Roomba.write(145); //Drive Direct
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
