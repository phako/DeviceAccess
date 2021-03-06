/*
 * ScalarRegisterAccessor.h
 *
 *  Created on: Mar 23, 2016
 *      Author: Martin Hierholzer <martin.hierholzer@desy.de>
 */

#ifndef CHIMERA_TK_SCALAR_REGISTER_ACCESSOR_H
#define CHIMERA_TK_SCALAR_REGISTER_ACCESSOR_H

#include "NDRegisterAccessorAbstractor.h"
#include "DeviceException.h"

namespace ChimeraTK {

  /*********************************************************************************************************************/
  /** Accessor class to read and write scalar registers transparently by using the accessor object like a variable of
   *  the type UserType. Conversion to and from the UserType will be handled by the FixedPointConverter matching the
   *  register description in the map, if required. Obtain the accessor using the Device::getScalarRegisterAccessor()
   *  function.
   *
   *  Note: Transfers between the device and the internal buffer need to be triggered using the read() and write()
   *  functions before reading from resp. after writing to the buffer using the operators.
   */
  template<typename UserType>
  class ScalarRegisterAccessor : public NDRegisterAccessorAbstractor<UserType> {
    public:

      /** Constructor. @attention Do not normally use directly.
       *  Users should call Device::getScalarRegisterAccessor() to obtain an instance instead. */
      ScalarRegisterAccessor(boost::shared_ptr< NDRegisterAccessor<UserType> > impl)
      : NDRegisterAccessorAbstractor<UserType>(impl)
      {}

      /** Placeholder constructor, to allow late initialisation of the accessor, e.g. in the open function.
       *  @attention Accessors created with this constructors will be dysfunctional, calling any member function
       *  will throw an exception (by the boost::shared_ptr)! */
      ScalarRegisterAccessor() {}

      /** Implicit type conversion to user type T to access the first element (often the only element).
       *  This covers already a lot of operations like arithmetics and comparison */
      operator UserType&() {
        return NDRegisterAccessorAbstractor<UserType>::_impl->accessData(0,0);
      }

      /** Assignment operator, assigns the first element. */
      ScalarRegisterAccessor<UserType>& operator=(UserType rightHandSide)
      {
        NDRegisterAccessorAbstractor<UserType>::_impl->accessData(0,0) = rightHandSide;
        return *this;
      }

      /** Pre-increment operator for the first element. */
      ScalarRegisterAccessor<UserType>& operator++() {
        return operator=( NDRegisterAccessorAbstractor<UserType>::_impl->accessData(0,0) + 1 );
      }

      /** Pre-decrement operator for the first element. */
      ScalarRegisterAccessor<UserType>& operator--() {
        return operator=( NDRegisterAccessorAbstractor<UserType>::_impl->accessData(0,0) - 1 );
      }

      /** Post-increment operator for the first element. */
      UserType operator++(int) {
        UserType v = NDRegisterAccessorAbstractor<UserType>::_impl->accessData(0,0);
        operator=( v + 1 );
        return v;
      }

      /** Post-decrement operator for the first element. */
      UserType operator--(int) {
        UserType v = NDRegisterAccessorAbstractor<UserType>::_impl->accessData(0,0);
        operator=( v - 1 );
        return v;
      }

      /** Get the coocked values in case the accessor is a raw accessor (which does not do data conversion).
       *  This returns the converted data from the user buffer. It does not do any read or write transfer.
       */
      template <typename COOCKED_TYPE>
      COOCKED_TYPE getAsCoocked(){
        return NDRegisterAccessorAbstractor<UserType>::_impl->template getAsCoocked<COOCKED_TYPE>(0,0);
      }

      /** Set the coocked values in case the accessor is a raw accessor (which does not do data conversion).
       *  This converts to raw and writes the data to the user buffer. It does not do any read or write transfer.
       */
      template <typename COOCKED_TYPE>
      void setAsCoocked(COOCKED_TYPE value){
        return NDRegisterAccessorAbstractor<UserType>::_impl->template setAsCoocked<COOCKED_TYPE>(0,0,value);
      }

      friend class TransferGroup;

  };

  // Template specialisation for string. It does not have the ++ and -- operators
  template<>
    class ScalarRegisterAccessor<std::string> : public NDRegisterAccessorAbstractor<std::string> {
    public:
    inline ScalarRegisterAccessor(boost::shared_ptr< NDRegisterAccessor<std::string> > impl)
      : NDRegisterAccessorAbstractor<std::string>(impl)
      {}

    inline ScalarRegisterAccessor() {}

    inline operator std::string&() {
      return NDRegisterAccessorAbstractor<std::string>::_impl->accessData(0,0);
    }

    inline ScalarRegisterAccessor<std::string>& operator=(std::string rightHandSide){
      NDRegisterAccessorAbstractor<std::string>::_impl->accessData(0,0) = rightHandSide;
      return *this;
    }

    friend class TransferGroup;
  };

  // Do not declare the template for all user types as extern here.
  // This could avoid optimisation of the inline code.

}    // namespace ChimeraTK

#endif /* CHIMERA_TK_SCALAR_REGISTER_ACCESSOR_H */
