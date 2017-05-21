//
// Created by Thibault PLET on 14/05/2017.
//

#ifndef COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_TIMESWITCHAPPLICATION_H
#define COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_TIMESWITCHAPPLICATION_H

#include <Arduino.h>
#include <com/osteres/automation/arduino/ArduinoApplication.h>
#include <com/osteres/automation/sensor/Identity.h>
#include <com/osteres/automation/actuator/timeswitch/action/ActionManager.h>
#include <com/osteres/automation/actuator/timeswitch/PowerControl.h>
#include <com/osteres/automation/arduino/memory/PinProperty.h>
#include <com/osteres/automation/arduino/memory/StoredProperty.h>
#include <com/osteres/automation/arduino/component/DataBuffer.h>

using com::osteres::automation::arduino::ArduinoApplication;
using com::osteres::automation::sensor::Identity;
using com::osteres::automation::actuator::timeswitch::action::ActionManager;
using com::osteres::automation::actuator::timeswitch::PowerControl;
using com::osteres::automation::arduino::memory::PinProperty;
using com::osteres::automation::arduino::memory::StoredProperty;
using com::osteres::automation::arduino::component::DataBuffer;

namespace com
{
    namespace osteres
    {
        namespace automation
        {
            namespace actuator
            {
                namespace timeswitch
                {
                    class TimeSwitchApplication : public ArduinoApplication
                    {
                    public:
                        /**
                         * Sensor identifier
                         */
                        static byte const SENSOR = Identity::SWITCH;

                        /**
                         * Constructor
                         */
                        TimeSwitchApplication(
                            Transmitter *transmitter,
                            unsigned int powerOffCommandPin,
                            unsigned int shutdownCommandPin,
                            unsigned int currentSensorPin,
                            unsigned int switchLockPowerOnPin,
                            unsigned int switchAutoModePin
                        ) : ArduinoApplication(TimeSwitchApplication::SENSOR, transmitter)
                        {
                            this->construct(
                                powerOffCommandPin,
                                shutdownCommandPin,
                                currentSensorPin,
                                switchLockPowerOnPin,
                                switchAutoModePin
                            );
                        }

                        /**
                         * Destructor
                         */
                        virtual ~TimeSwitchApplication()
                        {
                            // Remove power control component
                            if (this->powerControl != NULL) {
                                delete this->powerControl;
                                this->powerControl = NULL;
                            }
                            // Remove shutdown buffer
                            if (this->shutdownBuffer != NULL) {
                                delete this->shutdownBuffer;
                                this->shutdownBuffer = NULL;
                            }
                            // Shutdown delay property
                            if (this->shutdownDelayProperty != NULL) {
                                delete this->shutdownDelayProperty;
                                this->shutdownDelayProperty = NULL;
                            }
                        }

                        /**
                         * Setup application
                         */
                        virtual void setup()
                        {
                            // Parent
                            ArduinoApplication::setup();

                            // Ensure that power command is off
                            this->getPowerControl()->hardPowerOff();

                            // Transmission
                            this->transmitter->setActionManager(this->getActionManager());


                            // TEMP
                            this->getShutdownBuffer()->setBufferDelay(10000); //10s
                        }

                        /**
                         * Process application
                         */
                        virtual void process()
                        {
                            // Request an identifier if needed. Note: Not mandatory anymore
                            if (this->isNeedIdentifier()) {
                                this->requestForAnIdentifier();

                                // Send and listen
                                this->transmitter->srs(3000); // 3s

                            } // Process
                            else {

                                PowerControl * powerControl = this->getPowerControl();

                                //
                                // Update real state of device
                                //
                                // If shutdown has been requested, keep in touch to terminate process
                                if (powerControl->isShutdownRequested()) {
                                    powerControl->securePowerOff();
                                }
                                // Else, check by using real state measured
                                else {
                                    bool reallyPowerOn = powerControl->isReallyPowerOn();
                                    // Except if power on is locked, if mark as powered on but in reality device is powered off
                                    if (!powerControl->isLockPowerOn() && powerControl->getOutputState() && !reallyPowerOn) {
                                        // Hard power off
                                        powerControl->hardPowerOff();
                                    }
                                }

                                // Listen and send
                                this->transmitter->rsr();

                                // Forced power on
                                if (powerControl->isLockPowerOn()) {
                                    // If power off, so power on
                                    if (!powerControl->getOutputState() || powerControl->isShutdownRequested()) {
                                        powerControl->powerOn();

                                        // Reset shutdown buffer
                                        this->getShutdownBuffer()->reset();
                                    }
                                    // Else, nothing to do
                                }
                                // Auto mode, timeout
                                else if (powerControl->isAutoMode()) {
                                    // If power on and if timeout, so power off and reset buffer
                                    if (
                                        powerControl->getOutputState() &&
                                        this->getShutdownBuffer()->isOutdated()
                                    ) {
                                        // Request for power off
                                        powerControl->securePowerOff();
                                    }
                                }

                                // Otherwise, power control done by ActionManager
                                Serial.println(this->getPowerControl()->getOutputState() ? "on" : "off");
                            }

                            // Wait 100ms
                            delay(100);
                        }

                        /**
                         * Get power control component
                         */
                        PowerControl * getPowerControl()
                        {
                            return this->powerControl;
                        }

                        /**
                         * Get shutdown delay propery
                         */
                        StoredProperty<unsigned int> * getShutdownDelayProperty()
                        {
                            return this->shutdownDelayProperty;
                        }

                        /**
                         * Get shutdown buffer: time before send shutdown command
                         */
                        DataBuffer * getShutdownBuffer()
                        {
                            return this->shutdownBuffer;
                        }

                    protected:

                        /**
                         * Common part constructor
                         */
                        void construct(
                            unsigned int powerOffCommandPin,
                            unsigned int shutdownCommandPin,
                            unsigned int currentSensorPin,
                            unsigned int switchLockPowerOnPin,
                            unsigned int switchAutoModePin
                        ) {
                            // Create power control
                            this->powerControl = new PowerControl(
                                powerOffCommandPin,
                                shutdownCommandPin,
                                currentSensorPin,
                                switchLockPowerOnPin,
                                switchAutoModePin
                            );

                            // Time feature
                            this->shutdownDelayProperty = new StoredProperty<unsigned int>();
                            StoredPropertyManager::configure(this->shutdownDelayProperty);
                            if (this->shutdownDelayProperty->get() == 0) {
                                this->shutdownDelayProperty->set(30000); // 30s
                            }
                            this->shutdownBuffer = new DataBuffer(this->shutdownDelayProperty->get());

                            // Create action manager (process when receive transmission)
                            ActionManager *actionManager = new ActionManager(this->powerControl, this->shutdownBuffer);
                            this->setActionManager(actionManager);
                        }

                        /**
                         * Power control component
                         */
                        PowerControl * powerControl = NULL;

                        /**
                         * Shutdown buffer: time before send shutdown command
                         */
                        DataBuffer * shutdownBuffer = NULL;

                        /**
                         * Shutdown delay property
                         */
                        StoredProperty<unsigned int> * shutdownDelayProperty = NULL;
                    };
                }
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_TIMESWITCHAPPLICATION_H
