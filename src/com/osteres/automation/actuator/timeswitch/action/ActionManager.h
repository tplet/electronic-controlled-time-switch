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
#include <com/osteres/automation/arduino/component/DataBuffer.h>

using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::actuator::timeswitch::PowerControl;
using com::osteres::automation::arduino::component::DataBuffer;
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
                            ActionManager(PowerControl * powerControl, DataBuffer * shutdownBuffer) : ArduinoActionManager()
                            {
                                this->powerControl = powerControl;
                                this->shutdownBuffer = shutdownBuffer;
                            }

                            /**
                             * Process packet
                             */
                            virtual void processPacket(Packet *packet)
                            {
                                // Parent
                                ArduinoActionManager::processPacket(packet);

                                // PowerControl alias
                                PowerControl * powerControl = this->getPowerControl();

                                // ENABLE command or DATA command (same process)
                                if (packet->getCommand() == Command::ENABLE) {
                                    // Read enable value
                                    bool enable = packet->getDataUChar1() == 1;

                                    // If power on command and output currently power off
                                    if (enable && !powerControl->getOutputState()) {
                                        // Power on
                                        powerControl->powerOn();
                                        // Reset buffer
                                        this->getShutdownBuffer()->reset();
                                    }
                                    // Else if power off command and output currently power on
                                    else if (!enable && powerControl->getOutputState()) {
                                        // Power off
                                        powerControl->securePowerOff();
                                    }
                                }
                                // PING command to keep alive output
                                else if (packet->getCommand() == Command::PING) {
                                    // If auto-mode enable only
                                    if (powerControl->isAutoMode()) {
                                        // Reset buffer
                                        this->getShutdownBuffer()->reset();
                                        // Power on if necessary
                                        if (!powerControl->getOutputState() || powerControl->isShutdownRequested()) {
                                            powerControl->powerOn();
                                        }
                                    }
                                }
                                // CONFIG command
                                else if (packet->getCommand() == Command::CONFIG) {
                                    // TODO
                                }
                            }

                            /**
                             * Get power control component
                             */
                            PowerControl * getPowerControl()
                            {
                                return this->powerControl;
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
                             * Power control component
                             */
                            PowerControl * powerControl = NULL;

                            /**
                             * Shutdown buffer: time before send shutdown command
                             */
                            DataBuffer * shutdownBuffer = NULL;

                        };
                    }
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_ACTIONMANAGER_H
