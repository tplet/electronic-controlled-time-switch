//
// Created by Thibault PLET on 14/05/2017.
//

#ifndef COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_TIMESWITCHAPPLICATION_H
#define COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_TIMESWITCHAPPLICATION_H

/* Defined values */
#define IGNORE_PACKET_SUCCESS_RESPONSE true


#include <Arduino.h>
#include <com/osteres/automation/arduino/ArduinoApplication.h>
#include <com/osteres/automation/sensor/Identity.h>
#include <com/osteres/automation/actuator/timeswitch/action/ActionManager.h>
#include <com/osteres/automation/arduino/memory/PinProperty.h>

using com::osteres::automation::arduino::ArduinoApplication;
using com::osteres::automation::sensor::Identity;
using com::osteres::automation::actuator::timeswitch::action::ActionManager;
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
                            unsigned int powerCommandPin,
                            unsigned int shutdownCommandPin,
                            unsigned int currentSensorPin,
                            unsigned int switchLockPowerOnPin,
                            unsigned int switchAutoModePin
                        ) : ArduinoApplication(MovementApplication::SENSOR, transmitter)
                        {
                            this->construct(
                                powerCommandPin,
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
                            // Remove power command property
                            if (this->powerCommandProperty != NULL) {
                                delete this->powerCommandProperty;
                                this->powerCommandProperty = NULL;
                            }
                            // Remove shutdown command property
                            if (this->shutdownCommandProperty != NULL) {
                                delete this->shutdownCommandProperty;
                                this->shutdownCommandProperty = NULL;
                            }
                            // Remove lock power on property
                            if (this->switchLockPowerOnProperty != NULL) {
                                delete this->switchLockPowerOnProperty;
                                this->switchLockPowerOnProperty = NULL;
                            }
                            // Remove current sensor property
                            if (this->currentSensorProperty != NULL) {
                                delete this->currentSensorProperty;
                                this->currentSensorProperty = NULL;
                            }
                            // Remove auto mode property
                            if (this->switchAutoModeProperty != NULL) {
                                delete this->switchAutoModeProperty;
                                this->switchAutoModeProperty = NULL;
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
                            this->getPowerCommandProperty()->set(0);
                            this->getShutdownCommandProperty()->set(0);

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
                                if (this->isLockPowerOn()) {
                                    // If power off, so power on
                                    if (!this->getOutputState()) {
                                        this->powerOn();
                                    }
                                    // Else, nothing to do
                                }
                                // Otherwise, power control done by ActionManager

                            }

                            // Wait 100ms
                            delay(100);
                        }

                        /**
                         * Process to power off output with security
                         * 1. Enable shutdown command
                         * 2. Wait current consumption falls
                         * 3. Power off output
                         */
                        void securePowerOff()
                        {
                            // Enable shutdown command
                            this->getShutdownCommandProperty()->set(1);

                            // Wait current consumption falls
                            if (this->getCurrentSensorProperty()->read() < 10) {
                                // If no current consumption, power off
                                this->getPowerCommandProperty()->set(0);

                                // Reinit shutdown command
                                this->getShutdownCommandProperty()->set(0);

                                this->setOutputState(false);
                            }

                        }

                        /**
                         * Power on output
                         */
                        void powerOn()
                        {
                            this->getPowerCommandProperty()->set(1);
                            this->getShutdownCommandProperty()->set(0);

                            this->setOutputState(true);
                        }

                        /**
                         * Hard power off output without security
                         */
                        void hardPowerOff()
                        {
                            this->getPowerCommandProperty()->set(0);
                            this->getShutdownCommandProperty()->set(0);

                            this->setOutputState(false);
                        }

                        /**
                         * Flag to indicate if auto mode is enable
                         */
                        bool isAutoMode()
                        {
                            return this->getAutoModeProperty()->read() == 1;
                        }

                        /**
                         * Flag to indicate if power on is locked
                         */
                        bool isLockPowerOn()
                        {
                            return this->getLockPowerOnProperty()->read() == 1;
                        }

                        /**
                         * Get power command property
                         */
                        PinProperty<unsigned int> * getPowerCommandProperty()
                        {
                            return this->powerCommandProperty;
                        }

                        /**
                         * Get shutdown command property
                         */
                        PinProperty<unsigned int> * getShutdownCommandProperty()
                        {
                            return this->shutdownCommandProperty;
                        }

                        /**
                         * Get current sensor property
                         */
                        PinProperty<unsigned int> * getCurrentSensorProperty()
                        {
                            return this->currentSensorProperty;
                        }

                        /**
                         * Get lock power on property
                         */
                        PinProperty<unsigned int> * getLockPowerOnProperty()
                        {
                            return this->switchLockPowerOnProperty;
                        }

                        /**
                         * Get auto mode property
                         */
                        PinProperty<unsigned int> * getAutoModeProperty()
                        {
                            return this->switchAutoModeProperty;
                        }

                        /**
                         * Flag to indicate output state
                         */
                        bool getOutputState()
                        {
                            return this->outputState;
                        }

                        /**
                         * Set flag to indicate output state
                         */
                        void setOutputState(bool state)
                        {
                            this->outputState = state;
                        }

                    protected:

                        /**
                         * Common part constructor
                         */
                        void construct(
                            unsigned int powerCommandPin,
                            unsigned int shutdownCommandPin,
                            unsigned int currentSensorPin,
                            unsigned int switchLockPowerOnPin,
                            unsigned int switchAutoModePin
                        ) {
                            // Create action manager (process when receive transmission)
                            ActionManager *actionManager = new ActionManager();
                            this->setActionManager(actionManager);

                            // Power command property
                            this->powerCommandProperty = new PinProperty<unsigned int>(powerCommandPin, true, false);

                            // Shutdown command property
                            this->shutdownCommandProperty = new PinProperty<unsigned int>(shutdownCommandPin, true, false);

                            // Current sensor property
                            this->currentSensorProperty = new PinProperty<unsigned int>(currentSensorPin, false, true);

                            // Lock power on property
                            this->switchLockPowerOnProperty = new PinProperty<unsigned int>(switchLockPowerOnPin, true, true);

                            // Auto mode property
                            this->switchAutoModeProperty = new PinProperty<unsigned int>(switchAutoModePin, true, true);
                        }

                        /**
                         * Power command property (digital output)
                         * Set 1 to power on Raspberry, 0 to power off
                         */
                        PinProperty<unsigned int> * powerCommandProperty = NULL;

                        /**
                         * Shutdown command property (digital output)
                         * Set 1 to command raspberry shutdown
                         * Need to listen this signal from raspberry
                         */
                        PinProperty<unsigned int> * shutdownCommandProperty = NULL;

                        /**
                         * Current sensor property (analog input)
                         * Read current consumption from output.
                         * Permit to ensure that device not running on when power off output
                         */
                        PinProperty<unsigned int> * currentSensorProperty = NULL;

                        /**
                         * Lock power on property (digital input)
                         * When 1, raspberry locked to power on. When 0, auto mode is used
                         * Note: If both lockPowerOn and autoMode is equal to 0, state of Raspberry is maintained
                         */
                        PinProperty<unsigned int> * switchLockPowerOnProperty = NULL;

                        /**
                         * Auto mode property (digital input)
                         * When 1, raspberry power on can remote by transmission, when 0, lock power on is used
                         * Note: If both lockPowerOn and autoMode is equal to 0, state of Raspberry is maintained
                         */
                        PinProperty<unsigned int> * switchAutoModeProperty = NULL;

                        /**
                         * Output state
                         * Flag to indicate output state
                         * true: Output is considering as power on
                         * false: Output is considering as power off
                         */
                        bool outputState = false;
                    };
                }
            }
        }
    }
}


#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_TIMESWITCHAPPLICATION_H
