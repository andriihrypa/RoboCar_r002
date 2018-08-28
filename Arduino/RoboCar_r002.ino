#include <Wire.h>

#define DEBUG 0
#define SLAVE_ADDRESS 0x05
#define I2C_BUFFER_SIZE 6
#define LEFT_SIDE_DIRECTION_INDEX 2
#define LEFT_SIDE_SPEED_INDEX 3
#define RIGHT_SIDE_DIRECTION_INDEX 4
#define RIGHT_SIDE_SPEED_INDEX 5
#define CRITICAL_DISTANCE 20  // unit[cm]

// defines pins numbers
const int trigPin = 9;
const int echoPin = 10;

const int enRightSidePin = 6;
const int rightSideForwardDirectionPin = 7;
const int rightSideBackwardDirectionPin = 5;

const int enLeftSidePin = 3;
const int leftSideForwardDirectionPin = 4;
const int leftSideBackwardDirectionPin = 2;

// defines global variables
char rightSidePrevDirection = 0;
char leftSidePrevDirection = 0;
char rightSideDirection = 0;
char leftSideDirection = 0;
int rightSideSpeed = 0;
int leftSideSpeed = 0;

void setup()
{
    // configures pins Ultrasonic sensor
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input
    
    // configures pins for driving motors
    pinMode(enRightSidePin, OUTPUT);
    pinMode(enLeftSidePin, OUTPUT);
    pinMode(rightSideForwardDirectionPin, OUTPUT);
    pinMode(rightSideBackwardDirectionPin, OUTPUT);
    pinMode(leftSideForwardDirectionPin, OUTPUT);
    pinMode(leftSideBackwardDirectionPin, OUTPUT);

    digitalWrite(enRightSidePin, LOW);
    digitalWrite(enLeftSidePin, LOW);
    digitalWrite(rightSideForwardDirectionPin, LOW);
    digitalWrite(rightSideBackwardDirectionPin, LOW);
    digitalWrite(leftSideForwardDirectionPin, LOW);
    digitalWrite(leftSideBackwardDirectionPin, LOW);

    // Starts the serial communication
    Serial.begin(115200);

    // Starts I2C communication
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(i2cReceiveData);
}

void loop()
{
    if (DEBUG)
    {
        getDataViaUart();
    }
    int currentDistance = measureDistance();
    if (currentDistance > CRITICAL_DISTANCE || (leftSideDirection == 'b' || rightSideDirection == 'b'))
    {
        setDirections();
        setSpeeds();
    }
    else if (currentDistance <= CRITICAL_DISTANCE)
    {
        leftSideDirection = 0;
        leftSideSpeed = 0;
        rightSideDirection = 0;
        rightSideSpeed = 0;

        setDirections();
        setSpeeds();

        if (DEBUG)
        {
            Serial.println("Can't move");
        }
    }
}

void getDataViaUart()
{
    if (Serial.available() > 0)
    {
        String receivedString = Serial.readStringUntil('\n');
        String part01 = getValue(receivedString, ' ', 0);
        String part02 = getValue(receivedString, ' ', 1);
        String part03 = getValue(receivedString, ' ', 2);
        String part04 = getValue(receivedString, ' ', 3);

        leftSidePrevDirection = leftSideDirection;
        rightSidePrevDirection = rightSideDirection;

        leftSideDirection = part01.charAt(0);
        leftSideSpeed = part02.toInt();
        rightSideDirection = part03.charAt(0);
        rightSideSpeed = part04.toInt();

        if (DEBUG)
        {
            Serial.print("leftSideDirection: ");
            Serial.println(leftSideDirection);
            Serial.print("leftSideSpeed: ");
            Serial.println(leftSideSpeed);
            Serial.print("rightSideDirection: ");
            Serial.println(rightSideDirection);
            Serial.print("rightSideSpeed: ");
            Serial.println(rightSideSpeed);
        }
    }
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length()-1;

    for(int i = 0; i <= maxIndex && found <= index; i++){
    if(data.charAt(i) == separator || i == maxIndex){
        found++;
        strIndex[0] = strIndex[1] + 1;
        strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// return distance in cm
int measureDistance()
{
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH);

    // Calculating the distance
    int distance = duration*0.0343/2;

    if (DEBUG)
    {
        //Serial.print("Distance: ");
        //Serial.println(distance);
    }

    return distance;
}

void setDirections()
{
    bool leftSideDirectionChenged = (leftSidePrevDirection != leftSideDirection);
    
    if (leftSideDirectionChenged)
    {
        digitalWrite(leftSideForwardDirectionPin, LOW);
        digitalWrite(leftSideBackwardDirectionPin, LOW);
        delay(100);
        if (leftSideDirection == 'b'  || leftSideDirection == 98)
        {
            digitalWrite(leftSideBackwardDirectionPin, HIGH);
        }
        else if (leftSideDirection == 'f' || leftSideDirection == 102)
        {
            digitalWrite(leftSideForwardDirectionPin, HIGH);
        }
        else
        {
            if (DEBUG)
            {
                Serial.println("Invalid leftSideDirection");
            }
        }
    }

    bool rightSideDirectionChenged = (rightSidePrevDirection != rightSideDirection);
    if (rightSideDirectionChenged)
    {
        digitalWrite(rightSideForwardDirectionPin, LOW);
        digitalWrite(rightSideBackwardDirectionPin, LOW);
        delay(100);
        if (rightSideDirection == 'b' || rightSideDirection == 98)
        {
            digitalWrite(rightSideBackwardDirectionPin, HIGH);
        }
        else if (rightSideDirection == 'f' || rightSideDirection == 102)
        {
            digitalWrite(rightSideForwardDirectionPin, HIGH);
        }
        else
        {
            if (DEBUG)
            {
                Serial.println("Invalid rightSideDirection");
            }
        }
    }
}

void setSpeeds()
{
    analogWrite(enLeftSidePin, leftSideSpeed);
    analogWrite(enRightSidePin, rightSideSpeed);
}

//callback for received data via I2C
void i2cReceiveData(int byteCount)
{
    uint8_t receivedMessage[I2C_BUFFER_SIZE];
    int i = 0;
    while (Wire.available())
    {
        receivedMessage[i] = (uint8_t)Wire.read();
        if (DEBUG)
        {
            Serial.print("receivedMessage = ");
            Serial.println(receivedMessage[i]);
        }
        i++;

        if (i >= I2C_BUFFER_SIZE)
        {
            break;
        }
    }

    leftSidePrevDirection = leftSideDirection;
    rightSidePrevDirection = rightSideDirection;

    leftSideDirection = receivedMessage[LEFT_SIDE_DIRECTION_INDEX];
    leftSideSpeed = receivedMessage[LEFT_SIDE_SPEED_INDEX];
    rightSideDirection = receivedMessage[RIGHT_SIDE_DIRECTION_INDEX];
    rightSideSpeed = receivedMessage[RIGHT_SIDE_SPEED_INDEX];

    if (DEBUG)
    {
        Serial.print("leftSideDirection: ");
        Serial.println(leftSideDirection);
        Serial.print("leftSideSpeed: ");
        Serial.println(leftSideSpeed);
        Serial.print("rightSideDirection: ");
        Serial.println(rightSideDirection);
        Serial.print("rightSideSpeed: ");
        Serial.println(rightSideSpeed);
    }
}

// callback for sending data via I2C
void i2cSendData()
{
    Wire.write(42);  // just for fun :)
}
