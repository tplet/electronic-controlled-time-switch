//
// Created by Thibault PLET on 14/05/2017.
//

#ifndef COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_ACTIONMANAGER_H
#define COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_ACTIONMANAGER_H

#include <Arduino.h>
#include <StandardCplusplus.h>
#include <string>
#include <com/osteres/automation/transmission/packet/Command.h>
#include <com/osteres/automation/transmission/packet/CommandString.h>
#include <com/osteres/automation/transmission/packet/Packet.h>
#include <com/osteres/automation/arduino/action/ArduinoActionManager.h>
#include <com/osteres/automation/arduino/action/SensorIdentifierAction.h>
#include <com/osteres/automation/actuator/timeswitch/TimeSwitchApplication.h>

using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::transmission::packet::CommandString;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::arduino::action::ArduinoActionManager;
using com::osteres::automation::arduino::action::SensorIdentifierAction;
using com::osteres::automation::actuator::timeswitch::TimeSwitchApplication;
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
                    namespace action
                    {
                        class ActionManager : public ArduinoActionManager
                        {
                        public:
                            /**
                             * Constructor
                             */
                            ActionManager(TimeSwitchApplication *application) : ArduinoActionManager()
                            {
                                this->application = application;
                            }

                            /**
                             * Process packet
                             */
                            virtual void processPacket(Packet *packet)
                            {
                                // Parent
                                ArduinoActionManager::processPacket(packet);

                                // ENABLE command or DATA command (same process)
                                if (packet->getCommand() == Command::ENABLE || packet->getCommand() == Command::DATA) {
                                    // Read enable value
                                    bool enable = packet->getDataUChar1() == 1;

                                    // If auto-mode enable only
                                    if (application->isAutoMode()) {
                                        // If power on command and output currently power off
                                        if (enable && !application->getOutputState()) {
                                            // Power on
                                            application->powerOn();
                                        }
                                        // Else if power off command and output currently power on
                                        else if (!enable && application->getOutputState()) {
                                            // Power off
                                            application->securePowerOff();
                                        }
                                    }
                                }
                            }

                        protected:

                            /**
                             * Application
                             */
                            TimeSwitchApplication * application = NULL;

                        };
                    }
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_ACTIONMANAGER_H
