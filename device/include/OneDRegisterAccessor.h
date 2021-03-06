/*
 * OneDRegisterAccessor.h
 *
 *  Created on: Mar 23, 2016
 *      Author: Martin Hierholzer <martin.hierholzer@desy.de>
 */

#ifndef CHIMERA_TK_ONE_D_REGISTER_ACCESSOR_H
#define CHIMERA_TK_ONE_D_REGISTER_ACCESSOR_H

#include "NDRegisterAccessorAbstractor.h"
#include "DeviceException.h"

namespace ChimeraTK {

  /*********************************************************************************************************************/
  /** Accessor class to read and write registers transparently by using the accessor object like a vector of the
   *  type UserType. Conversion to and from the UserType will be handled by the FixedPointConverter matching the
   *  register description in the map. Obtain the accessor using the Device::getBufferingRegisterAccessor() function.
   *
   *  Note: Transfers between the device and the internal buffer need to be triggered using the read() and write()
   *  functions before reading from resp. after writing to the buffer using the operators.
   */
  template<typename UserType>
  class OneDRegisterAccessor : public NDRegisterAccessorAbstractor<UserType> {
    public:

      /** Constructer. @attention Do not normally use directly.
       *  Users should call Device::getOneDRegisterAccessor() to obtain an instance instead. */
      OneDRegisterAccessor(boost::shared_ptr< NDRegisterAccessor<UserType> > impl)
      : NDRegisterAccessorAbstractor<UserType>(impl)
      {
        if(NDRegisterAccessorAbstractor<UserType>::_impl->getNumberOfChannels() != 1) {
          throw DeviceException("The OneDRegisterAccessor has a too low dimension to access this register.",
              DeviceException::WRONG_ACCESSOR);
        }
      }

      /** Placeholder constructer, to allow late initialisation of the accessor, e.g. in the open function.
       *  @attention Accessors created with this constructors will be dysfunctional, calling any member function
       *  will throw an exception (by the boost::shared_ptr)! */
      OneDRegisterAccessor() {}

      /** Get or set buffer content by [] operator.
       *  @attention No bounds checking is performed, use getNumberOfElements() to obtain the number of elements in
       *  the register.
       *  Note: Using the iterators is slightly more efficient than using this operator! */
      UserType& operator[](unsigned int element) {
        return NDRegisterAccessorAbstractor<UserType>::_impl->accessData(0,element);
      }

      /** Return number of elements/samples in the register */
      unsigned int getNElements() {
        return NDRegisterAccessorAbstractor<UserType>::_impl->getNumberOfSamples();
      }

      /** Access data with std::vector-like iterators */
      typedef typename std::vector<UserType>::iterator iterator;
      typedef typename std::vector<UserType>::const_iterator const_iterator;
      typedef typename std::vector<UserType>::reverse_iterator reverse_iterator;
      typedef typename std::vector<UserType>::const_reverse_iterator const_reverse_iterator;
      iterator begin() { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).begin(); }
      const_iterator begin() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).cbegin(); }
      const_iterator cbegin() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).cbegin(); }
      iterator end() { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).end(); }
      const_iterator end() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).cend(); }
      const_iterator cend() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).cend(); }
      reverse_iterator rbegin() { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).rbegin(); }
      const_reverse_iterator rbegin() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).crbegin(); }
      const_reverse_iterator crbegin() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).crbegin(); }
      reverse_iterator rend() { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).rend(); }
      const_reverse_iterator rend() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).crend(); }
      const_reverse_iterator crend() const { return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).crend(); }

      /* Swap content of (cooked) buffer with std::vector */
      void swap(std::vector<UserType> &x) {
        if(x.size() != NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).size()) {
          throw DeviceException("Swapping with a buffer of a different size is not allowed.",
              DeviceException::WRONG_PARAMETER);
        }
        NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).swap(x);
      }

      /* Copy content of (cooked) buffer from std::vector */
      OneDRegisterAccessor<UserType>& operator=(const std::vector<UserType> &x) {
        if(x.size() != NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).size()) {
          throw DeviceException("Copying in a buffer of a different size is not allowed.",
              DeviceException::WRONG_PARAMETER);
        }
        NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0) = x;
        return *this;
      }

      /* Convert content of (cooked) buffer into std::vector */
      operator const std::vector<UserType>&() {
        return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0);
      }


      /** Return a direct pointer to the memory buffer storng the elements.
       *  @attention Note that this pointer will be invalidated during read(), write() and swap(). If this accessor is
       *  part of a TransferGroup, any call to one of these functions on any element of the TransferGroup or the
       *  TransferGroup itself may invalidate the pointer! */
      UserType* data() {
        return NDRegisterAccessorAbstractor<UserType>::_impl->accessChannel(0).data();
      }

      /** Get the coocked values in case the accessor is a raw accessor (which does not do data conversion).
       *  This returns the converted data from the use buffer. It does not do any read or write transfer.
       */
      template <typename COOCKED_TYPE>
      COOCKED_TYPE getAsCoocked(unsigned int sample){
        return NDRegisterAccessorAbstractor<UserType>::_impl->template getAsCoocked<COOCKED_TYPE>(0,sample);
      }
     
      /** Set the coocked values in case the accessor is a raw accessor (which does not do data conversion).
       *  This converts to raw and writes the data to the user buffer. It does not do any read or write transfer.
       */
      template <typename COOCKED_TYPE>
      void setAsCoocked(unsigned int sample, COOCKED_TYPE value){
        return NDRegisterAccessorAbstractor<UserType>::_impl->template setAsCoocked<COOCKED_TYPE>(0,sample,value);
      }

      friend class TransferGroup;
  };

  // Do not declare the template for all user types as extern here.
  // This could avoid optimisation of the inline code.

}    // namespace ChimeraTK

#endif /* CHIMERA_TK_ONE_D_REGISTER_ACCESSOR_H */
