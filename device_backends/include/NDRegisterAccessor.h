/*
 * NDRegisterAccessor.h - N-dimensional register accessor
 *
 *  Created on: Mar 21, 2016
 *      Author: Martin Hierholzer
 */

#ifndef CHIMERA_TK_N_D_REGISTER_ACCESSOR_H
#define CHIMERA_TK_N_D_REGISTER_ACCESSOR_H

#include <boost/make_shared.hpp>

#include "ForwardDeclarations.h"
#include "TransferElement.h"
#include "FixedPointConverter.h"
#include "DeviceException.h"
#include "ExperimentalFeatures.h"
#include "VirtualFunctionTemplate.h"

namespace ChimeraTK {

  /** N-dimensional register accessor. Base class for all register accessor implementations. The user frontend classes
   *  BufferingRegisterAccessor and TwoDRegisterAccessor are using implementations based on this class to perform
   *  the actual IO. */
  template<typename UserType>
  class NDRegisterAccessor : public TransferElement {

    public:
      /** Creates an NDRegisterAccessor with the specified name (passed on to the
       *  transfer element). */
      NDRegisterAccessor(std::string const &name,
                         std::string const &unit = std::string(TransferElement::unitNotSet),
                         std::string const &description = std::string())
      : TransferElement(name, unit, description)
      {
        TransferElement::makeUniqueId();
        FILL_VIRTUAL_FUNCTION_TEMPLATE_VTABLE(getAsCoocked_impl);
        FILL_VIRTUAL_FUNCTION_TEMPLATE_VTABLE(setAsCoocked_impl);
      }

      /** Get or set register accessor's buffer content (1D version).
       *  @attention No bounds checking is performed, use getNumberOfSamples() to obtain the number of elements in
       *  the register. */
      UserType& accessData(unsigned int sample) {
        return buffer_2D[0][sample];
      }

      /** Get or set register accessor's buffer content (2D version).
       *  @attention No bounds checking is performed, use getNumberOfChannels() and getNumberOfSamples() to obtain the
       *   number of channels and samples in the register. */
      UserType& accessData(unsigned int channel, unsigned int sample) {
        return buffer_2D[channel][sample];
      }

      /** Get or set register accessor's channel vector.
       *  @attention No bounds checking is performed, use getNumberOfChannels() to obtain the number of elements in
       *  the register. */
      std::vector<UserType>& accessChannel(unsigned int channel) {
        return buffer_2D[channel];
      }

      /** Return number of elements per channel */
      unsigned int getNumberOfSamples() const {
        return buffer_2D[0].size();
      }

      /** Return number of channels */
      unsigned int getNumberOfChannels() const {
        return buffer_2D.size();
      }

      const std::type_info& getValueType() const override {
        return typeid(UserType);
      }

      template<typename COOCKED_TYPE>
      COOCKED_TYPE getAsCoocked(unsigned int channel, unsigned int sample);
      DEFINE_VIRTUAL_FUNCTION_TEMPLATE_VTABLE( getAsCoocked_impl, T (unsigned int, unsigned int) );
      template<typename COOCKED_TYPE>
      COOCKED_TYPE getAsCoocked_impl(unsigned int channel, unsigned int sample);

      template<typename COOCKED_TYPE>
      void setAsCoocked(unsigned int channel, unsigned int sample, COOCKED_TYPE value);
      DEFINE_VIRTUAL_FUNCTION_TEMPLATE_VTABLE( setAsCoocked_impl, void (unsigned int, unsigned int, T) );
      template<typename COOCKED_TYPE>
      void setAsCoocked_impl(unsigned int channel, unsigned int sample, COOCKED_TYPE value);

      /** DEPRECATED DO NOT USE! Instead make a call to readNonBlocking() and check the return value.
       *  \deprecated This function is deprecated, remove it at some point!
       *
       *  This function was deprecated since it cannot be implemented for lockfree implementations (like the
       *  ControlSystemAdapter's ProcessVariable).
       *
       *  Return number of waiting data elements in the queue (or buffer). Use when the accessor was obtained with
       *  AccessMode::wait_for_new_data to obtain the amount of data waiting for retrieval in this accessor. If the
       *  returned value is 0, the call to read() will block until new data has arrived. If the returned value is > 0,
       *  it is guaranteed that the next call to read() will not block. If the accessor was obtained without the
       *  AccessMode::wait_for_new_data flag, this function will always return 1. */
      virtual unsigned int getNInputQueueElements() const {
        return 1;
      }

      /** DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY.
       *
       *  \deprecated This function is for backwards compatibility with the deprecated RegisterAccessor only.
       *  Return the fixed point converter used to convert the raw data from the device to the type T. If no conversion
       *  by the fixed point converter is required, this function will throw an exception. */
      virtual FixedPointConverter getFixedPointConverter() const  {
        throw DeviceException("Not implemented", DeviceException::NOT_IMPLEMENTED);
      }

    protected:

      /** Buffer of converted data elements. The buffer is always two dimensional. If a register with a single
       *  dimension should be accessed, the outer vector has only a single element. For a scalar register, only a
       *  single element is present in total (buffer_2D[0][0]). This has a negligible performance impact when
       *  optimisations are enabled, but allows a coherent interface for all accessors independent of their dimension.
       *
       *  Implementation note: The buffer must be created with the right number of elements in the constructor! */
      std::vector< std::vector<UserType> > buffer_2D;

      /// the compatibility layers need access to the buffer_2D
      friend class MultiplexedDataAccessor<UserType>;
      friend class RegisterAccessor;

      DEFINE_VIRTUAL_FUNCTION_TEMPLATE_VTABLE_FILLER( NDRegisterAccessor<UserType>, getAsCoocked_impl, 2 );
      DEFINE_VIRTUAL_FUNCTION_TEMPLATE_VTABLE_FILLER( NDRegisterAccessor<UserType>, setAsCoocked_impl, 3 );

  };

  template<typename UserType> template<typename COOCKED_TYPE>
  COOCKED_TYPE NDRegisterAccessor<UserType>::getAsCoocked(unsigned int channel, unsigned int sample){
    return CALL_VIRTUAL_FUNCTION_TEMPLATE(getAsCoocked_impl, COOCKED_TYPE, channel, sample);
  }

  template<typename UserType> template<typename COOCKED_TYPE>
  COOCKED_TYPE NDRegisterAccessor<UserType>::getAsCoocked_impl(unsigned int /*channel*/, unsigned int /*sample*/){
    throw DeviceException("Reading as coocked is not available for this accessor", DeviceException::NOT_AVAILABLE);
  }
  
  template<typename UserType> template<typename COOCKED_TYPE>
  void NDRegisterAccessor<UserType>::setAsCoocked(unsigned int channel, unsigned int sample, COOCKED_TYPE value){
    CALL_VIRTUAL_FUNCTION_TEMPLATE(setAsCoocked_impl, COOCKED_TYPE, channel, sample, value);
  }

  template<typename UserType> template<typename COOCKED_TYPE>
  void NDRegisterAccessor<UserType>::setAsCoocked_impl(unsigned int /*channel*/, unsigned int /*sample*/, COOCKED_TYPE /*value*/){
    throw DeviceException("Setting as coocked is not available for this accessor", DeviceException::NOT_AVAILABLE);
  }

  DECLARE_TEMPLATE_FOR_CHIMERATK_USER_TYPES(NDRegisterAccessor);
  
}

#endif /* CHIMERA_TK_N_D_REGISTER_ACCESSOR_H */
