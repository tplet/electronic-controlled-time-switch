
#include <Arduino.h>
#include <StandardCplusplus.h>
#include <SPI.h>
#include "RF24/nRF24L01.h"
#include <RF24/RF24.h>
#include <com/osteres/automation/actuator/timeswitch/TimeSwitchApplication.h>
#include <com/osteres/automation/transmission/Transmitter.h>
#include <com/osteres/automation/arduino/transmission/ArduinoRequester.h>


using com::osteres::automation::actuator::timeswitch::TimeSwitchApplication;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::arduino::transmission::ArduinoRequester;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;

/*
 * Pin
 */
// Pins CE, CSN for ARDUINO
#define RF_CE    9
#define RF_CSN   10
// Input analog pin for current sensor
#define PIN_CURRENT_SENSOR_ANALOG 0
// Output digital pin for power control (1 -> power on, 0 -> power off)
#define PIN_POWER_COMMAND 4
// Output digital pin for shutdown command (1 -> request for shutdown, 0 -> nothing)
#define PIN_SHUTDOWN_COMMAND 5
// Input digital pin for lock power on configuration (1 -> lock power on, 0 -> nothing)
#define PIN_SWITCH_LOCK_POWER_ON 2
// Input digital pin for auto mode configuration (1 -> auto mode enable, 0 -> nothing)
#define PIN_SWITCH_AUTO_MODE 3

/**
 * Configuration
 */
// Nothing

/*
 * Prepare electronic component
 */
// Radio transmitter
RF24 radio(RF_CE, RF_CSN);

/*
 * Prepare object manager
 */
// Transmission (master mode)
Transmitter transmitter(&radio, false);
// Application
TimeSwitchApplication application(
    &transmitter,
    PIN_POWER_COMMAND,
    PIN_SHUTDOWN_COMMAND,
    PIN_CURRENT_SENSOR_ANALOG,
    PIN_SWITCH_LOCK_POWER_ON,
    PIN_SWITCH_AUTO_MODE
);

/**
 * Initialize
 */
void setup() {
    Serial.begin(9600);

    // Set requester manually
    transmitter.setRequester(new ArduinoRequester(transmitter.getRadio(), transmitter.getWritingChannel()));

    // Setup transmitter
    transmitter.setup();

    // Setup (configuration)
    application.setup();
}

/**
 * Loop
 */
void loop()
{
    // Process
    application.process();
}
