//
// Created by Thibault PLET on 14/05/2017.
//

#ifndef COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_POWERCONTROL_H
#define COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_POWERCONTROL_H

#define ACS712_RAPPORT 0.185 // V per A

#include <Arduino.h>
#include <StandardCplusplus.h>
#include <string>
#include <com/osteres/automation/arduino/memory/PinProperty.h>
#include <com/osteres/arduino/util/VccReader.h>

using com::osteres::automation::arduino::memory::PinProperty;
using com::osteres::arduino::util::VccReader;
using std::string;

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
                    class PowerControl
                    {
                    public:
                        /**
                         * Constructor
                         */
                        PowerControl(
                            unsigned int powerOffCommandPin,
                            unsigned int shutdownCommandPin,
                            unsigned int currentSensorPin,
                            unsigned int switchLockPowerOnPin,
                            unsigned int switchAutoModePin
                        ) {
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
                        virtual ~PowerControl()
                        {
                            // Remove power command property
                            if (this->powerOffCommandProperty != NULL) {
                                delete this->powerOffCommandProperty;
                                this->powerOffCommandProperty = NULL;
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
                         * Process to power off output with security
                         * 1. Enable shutdown command
                         * 2. Wait current consumption falls
                         * 3. Power off output
                         */
                        void securePowerOff()
                        {
                            Serial.println("Secure power off or check to terminate process");
                            // Enable shutdown command
                            this->getShutdownCommandProperty()->set(1);

                            // Flag to indicate that shutdown has been requested
                            this->setShutdownRequested(true);

                            // Wait current consumption falls
                            if (!this->isReallyPowerOn()) {
                                // If no current consumption, power off
                                this->getPowerOffCommandProperty()->set(1);

                                // Reinit shutdown command
                                this->getShutdownCommandProperty()->set(0);

                                this->setShutdownRequested(false);
                                this->setOutputState(false);
                            }
                        }

                        /**
                         * Power on output
                         */
                        void powerOn()
                        {
                            Serial.println("Power on");

                            this->getPowerOffCommandProperty()->set(0);
                            this->getShutdownCommandProperty()->set(0);

                            this->setShutdownRequested(false);
                            this->setOutputState(true);
                        }

                        /**
                         * Hard power off output without security
                         */
                        void hardPowerOff()
                        {
                            Serial.println("Hard power off");
                            this->getPowerOffCommandProperty()->set(1);
                            this->getShutdownCommandProperty()->set(0);

                            this->setShutdownRequested(false);
                            this->setOutputState(false);
                        }

                        /**
                         * Check if device really power on by checking current consumption
                         */
                        bool isReallyPowerOn()
                        {
                            // Calculate current consumption (in mA)
                            unsigned int vRead = this->getCurrentSensorProperty()->read(10);
                            float vcc = VccReader::readV();
                            float halfVcc = vcc/2.0;
                            float current = abs(round(100 * 1000 * ( halfVcc - ( vcc * vRead / 1023.0 ) ) / ACS712_RAPPORT)) / 100.0;

                            // Wait current consumption falls
                            return current >= 100;
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
                        PinProperty<unsigned int> * getPowerOffCommandProperty()
                        {
                            return this->powerOffCommandProperty;
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

                        /**
                         * Flag to indicate if shutdown has been requested
                         */
                        bool isShutdownRequested()
                        {
                            return this->shutdownRequested;
                        }

                        /**
                         * Set flag to indicate if shutdown has been requested
                         */
                        void setShutdownRequested(bool flag)
                        {
                            this->shutdownRequested = flag;
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
                            // Power command property
                            this->powerOffCommandProperty = new PinProperty<unsigned int>(powerOffCommandPin, true, false);

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
                         * Set 0 to power on Raspberry, 1 to power off
                         */
                        PinProperty<unsigned int> * powerOffCommandProperty = NULL;

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

                        /**
                         * Flag to indicate if shutdown has been requested
                         */
                        bool shutdownRequested = false;
                    };
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_POWERCONTROL_H
