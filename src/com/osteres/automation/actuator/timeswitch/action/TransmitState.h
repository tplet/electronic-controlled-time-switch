//
// Created by Thibault PLET on 21/05/2016.
//

#ifndef COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_TRANSMITSTATE_H
#define COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_TRANSMITSTATE_H

#include <Arduino.h>
#include <StandardCplusplus.h>
#include <com/osteres/automation/action/Action.h>
#include <com/osteres/automation/transmission/Transmitter.h>
#include <com/osteres/automation/transmission/packet/Packet.h>
#include <com/osteres/automation/transmission/packet/Command.h>
#include <com/osteres/automation/arduino/memory/StoredProperty.h>
#include <com/osteres/automation/memory/Property.h>
#include <com/osteres/automation/actuator/timeswitch/PowerControl.h>

using com::osteres::automation::action::Action;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::memory::Property;
using com::osteres::automation::arduino::memory::StoredProperty;
using com::osteres::automation::actuator::timeswitch::PowerControl;

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
                        class TransmitState : public Action
                        {
                        public:
                            /**
                             * Constructor
                             */
                            TransmitState(
                                Property<unsigned char> *propertyType,
                                StoredProperty<unsigned char> *propertyIdentifier,
                                unsigned char to,
                                Transmitter *transmitter,
                                PowerControl * powerControl
                            )
                            {
                                this->propertyType = propertyType;
                                this->propertyIdentifier = propertyIdentifier;
                                this->to = to;
                                this->transmitter = transmitter;
                                this->powerControl = powerControl;
                            }

                            /**
                             * Execute action
                             */
                            bool execute()
                            {
                                // parent
                                Action::execute();

                                Packet *packet = new Packet(this->propertyType->get());

                                // Prepare data
                                packet->setSourceIdentifier(this->propertyIdentifier->get());
                                packet->setCommand(Command::DATA);
                                packet->setDataUChar1(this->powerControl->getOutputState() ? 1 : 0);
                                packet->setTarget(this->to);

                                // Transmit packet
                                this->transmitter->add(packet);

                                this->setSuccess();
                                return this->isSuccess();
                            }

                        protected:
                            /**
                             * Sensor type identifier property
                             */
                            Property<unsigned char> *propertyType = NULL;

                            /**
                             * Sensor identifier property
                             */
                            StoredProperty<unsigned char> *propertyIdentifier = NULL;

                            /**
                             * Target of transmission
                             */
                            unsigned char to;

                            /**
                             * Transmitter gateway
                             */
                            Transmitter *transmitter = NULL;

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

#endif //COM_OSTERES_AUTOMATION_ACTUATOR_TIMESWITCH_ACTION_TRANSMITSTATE_H
