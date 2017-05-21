//
// Created by Thibault PLET on 31/05/2016.
//

#ifndef COM_OSTERES_AUTOMATION_SENSOR_MOVEMENT_ACTION_TRANSMITMOVEMENT_H
#define COM_OSTERES_AUTOMATION_SENSOR_MOVEMENT_ACTION_TRANSMITMOVEMENT_H

#include <Arduino.h>
#include <StandardCplusplus.h>
#include <com/osteres/automation/action/Action.h>
#include <com/osteres/automation/transmission/Transmitter.h>
#include <com/osteres/automation/transmission/packet/Packet.h>
#include <com/osteres/automation/transmission/packet/Command.h>
#include <com/osteres/automation/arduino/memory/StoredProperty.h>
#include <com/osteres/automation/memory/Property.h>

using com::osteres::automation::action::Action;
using com::osteres::automation::transmission::Transmitter;
using com::osteres::automation::transmission::packet::Packet;
using com::osteres::automation::transmission::packet::Command;
using com::osteres::automation::memory::Property;
using com::osteres::automation::arduino::memory::StoredProperty;

namespace com
{
    namespace osteres
    {
        namespace automation
        {
            namespace sensor
            {
                namespace movement
                {
                    namespace action
                    {
                        class TransmitMovement : public Action
                        {
                        public:
                            /**
                             * Constructor
                             */
                            TransmitMovement(
                                Property<unsigned char> *propertyType,
                                StoredProperty<unsigned char> *propertyIdentifier,
                                unsigned char to,
                                Transmitter *transmitter
                            )
                            {
                                this->propertyType = propertyType;
                                this->propertyIdentifier = propertyIdentifier;
                                this->to = to;
                                this->transmitter = transmitter;
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
                                packet->setDataUChar1(1);
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
                        };
                    }
                }
            }
        }
    }
}

#endif //COM_OSTERES_AUTOMATION_SENSOR_MOVEMENT_ACTION_TRANSMITMOVEMENT_H
