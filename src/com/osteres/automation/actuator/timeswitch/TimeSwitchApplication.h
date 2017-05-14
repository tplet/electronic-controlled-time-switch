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

using com::osteres::automation::arduino::ArduinoApplication;
using com::osteres::automation::sensor::Identity;
using com::osteres::automation::actuator::timeswitch::action::ActionManager;
using com::osteres::automation::actuator::timeswitch::PowerControl;
using com::osteres::automation::arduino::memory::PinProperty;

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

                                // Listen and send
                                this->transmitter->rsr();

                                // Forced power on
                                if (this->getPowerControl()->isLockPowerOn()) {
                                    // If power off, so power on
                                    if (!this->getPowerControl()->getOutputState()) {
                                        this->getPowerControl()->powerOn();
                                    }
                                    // Else, nothing to do
                                }
                                // Otherwise, power control done by ActionManager

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

                            // Create action manager (process when receive transmission)
                            ActionManager *actionManager = new ActionManager(this->powerControl);
                            this->setActionManager(actionManager);
                        }

                        /**
                         * Power control component
                         */
                        PowerControl * powerControl = NULL;
                    };
                }
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_TIMESWITCHAPPLICATION_H
