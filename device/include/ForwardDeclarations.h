/*
 * ForwardDeclarations.h
 *
 *  Created on: Mar 16, 2016
 *      Author: Martin Hierholzer
 */

#ifndef MTCA4U_FORWARD_DECLARATIONS_H
#define MTCA4U_FORWARD_DECLARATIONS_H

#include <vector>

namespace mtca4u {

  class Device;
  class DeviceBackend;
  class TransferGroup;
  class RegisterAccessor;

  template< typename UserType >
  class BufferingRegisterAccessor;

  template< typename UserType >
  class BufferingRegisterAccessorImpl;

  template<typename UserType>
  class TwoDRegisterAccessorImpl;

  template<typename T>
  class ExternalBufferAllocator;

}

#endif /* MTCA4U_FORWARD_DECLARATIONS_H */