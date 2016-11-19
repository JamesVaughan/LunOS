/*
 * Io.h
 *
 *  Created on: 25-Jun-2009
 *      Author: james
 */

#ifndef IO_H_
#define IO_H_

//DEVICE TYPES
namespace LunOS
{
namespace IO
{
namespace DeviceTypes
{
enum DeviceType{DEVICE_HDD, DEVICE_DISK, DEVICE_REMOVEABLE_DISK, DEVICE_GFX, DEVICE_KBD, DEVICE_MOUSE, DEVICE_NETWORK, DEVICE_AUDIO, DEVICE_SERIAL,
	DEVICE_PARALLEL, DEVICE_UNDEFINED, DEVICE_NULL, PCI_BUS};
}

typedef struct DeviceInfo
{
	DeviceTypes::DeviceType type;
	unsigned int deviceNumber;
} DeviceInfo;
}
}

#endif /* IO_H_ */
