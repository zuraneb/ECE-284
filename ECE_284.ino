#include <TimerOne.h>
#include <SoftwareSerial.h>

const int led1 = 0;  // Pin 0
const int led2 = 1;  // Pin 1

volatile bool led1_blink = false;  // Control variable for LED 1 blinking
volatile bool led2_blink = false;  // Control variable for LED 2 blinking

// Bluetooth RX and TX pins
#define BT_RX 3
#define BT_TX 4
// HM-10 module power pin
#define HM10_POWER_PIN 2

// Declare the SoftwareSerial object
SoftwareSerial BTSerial(BT_RX, BT_TX);

const int bufferSize = 32;
char receivedBuffer[bufferSize];
int bufferIndex = 0;

void setup(void)
{  
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Timer1.initialize(50000);  // 50 ms interval initially
  Timer1.attachInterrupt(blinkLEDs);

  // Initialize Bluetooth communication
  pinMode(HM10_POWER_PIN, OUTPUT);
  digitalWrite(HM10_POWER_PIN, HIGH);
  BTSerial.begin(9600);

  memset(receivedBuffer, 0, bufferSize);  // Initialize the buffer

}

void blinkLEDs(void)
{
  static int led1State = LOW;
  static int led2State = LOW;
  
  if (led1_blink) {
    // Toggle LED 1
    led1State = !led1State;
    digitalWrite(led1, led1State);
    digitalWrite(led2, LOW);  // Ensure LED 2 is off
  } else if (led2_blink) {
    // Toggle LED 2
    led2State = !led2State;
    digitalWrite(led2, led2State);
    digitalWrite(led1, LOW);  // Ensure LED 1 is off
  } else {
    // Turn off both LEDs if neither is blinking
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
  }
}

void processReceivedBuffer()
{
  if (bufferIndex > 0) {
    receivedBuffer[bufferIndex] = '\0';  // Null-terminate the string
    
    if (strcmp(receivedBuffer, "i") == 0) {
      led1_blink = true;
      led2_blink = false;
    } else if (strcmp(receivedBuffer, "r") == 0) {
      led1_blink = false;
      led2_blink = true;
    } else {
      float frequency = atof(receivedBuffer);
      if (frequency > 0) {
        unsigned long interval = (unsigned long)(1000000.0 / frequency);
        Timer1.initialize(interval);  // Initialize Timer1 with the frequency in Hz
      } else {
        // Invalid input, turn off both LEDs
        led1_blink = false;
        led2_blink = false;
      }
    }
    
    bufferIndex = 0;  // Reset the buffer index
    memset(receivedBuffer, 0, bufferSize);  // Clear the buffer
  }
}

void loop(void)
{
  while (BTSerial.available()) {
    char receivedChar = BTSerial.read();
    if (receivedChar == ' ') {
      processReceivedBuffer();  // Process the received buffer if a space is received
    } else {
      if (bufferIndex < bufferSize - 1) {
        receivedBuffer[bufferIndex++] = receivedChar;  // Accumulate the received characters
      }
    }
  }
}
