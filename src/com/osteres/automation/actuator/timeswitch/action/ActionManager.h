//
// Created by Thibault PLET on 14/05/2017.
//

#ifndef COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_ACTIONMANAGER_H
#define COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_ACTIONMANAGER_H

#include <Arduino.h>
#include <StandardCplusplus.h>
#include <string>
#include <com/osteres/automation/transmission/packet/Command.h>
#include <com/osteres/automation/transmission/packet/Packet.h>
#include <com/osteres/automation/actuator/timeswitch/PowerControl.h>

using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::actuator::timeswitch::PowerControl;
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
                            ActionManager(PowerControl * powerControl) : ArduinoActionManager()
                            {
                                this->powerControl = powerControl;
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
                                    if (this->getPowerControl()->isAutoMode()) {
                                        // If power on command and output currently power off
                                        if (enable && !this->getPowerControl()->getOutputState()) {
                                            // Power on
                                            this->getPowerControl()->powerOn();
                                        }
                                        // Else if power off command and output currently power on
                                        else if (!enable && this->getPowerControl()->getOutputState()) {
                                            // Power off
                                            this->getPowerControl()->securePowerOff();
                                        }
                                    }
                                }
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
                             * Power control component
                             */
                            PowerControl * powerControl = NULL;

                        };
                    }
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_ACTIONMANAGER_H
