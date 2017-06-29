#ifndef HANDSHAKING_BACKEND_REGISTER_ACCESSOR_H
#define HANDSHAKING_BACKEND_REGISTER_ACCESSOR_H

#include "NDRegisterAccessor.h"

namespace ChimeraTK{
  template<typename UserType>
  class HandshakingBackendRegisterAccessor : public NDRegisterAccessor<UserType> {
    public:

    HandshakingBackendRegisterAccessor(boost::shared_ptr<HandshakingBackend> backend, const RegisterPath &payloadRegisterPath, size_t numberOfWords, size_t wordOffsetInRegister, AccessModeFlags flags, const RegisterPath &handshakeRegisterPath)
      : NDRegisterAccessor<UserType>(payloadRegisterPath),
        _backend(backend),
        _payloadRegisterAccessor(backend->getRegisterAccessor<UserType>(payloadRegisterPath, numberOfWords, wordOffsetInRegister, flags)),
      _handshakeRegisterAccessor(backend->getRegisterAccessor<uint32_t>(handshakeRegisterPath, 1, 0,AccessModeFlags({})))
      {      
        // allocated the buffers
        NDRegisterAccessor<UserType>::buffer_2D.resize(1);
        NDRegisterAccessor<UserType>::buffer_2D[0].resize(numberOfWords);
      }

    bool isSameRegister(const boost::shared_ptr<TransferElement const> &other) const override {
      auto rhsCasted = boost::dynamic_pointer_cast< const HandshakingBackendRegisterAccessor<UserType> >(other);
      if(!rhsCasted) return false;
      if(_backend != rhsCasted->_backend) return false;
      if( ! _payloadRegisterAccessor->isSameRegister(rhsCasted->_payloadRegisterAccessor)) return false;
      if( ! _handshakeRegisterAccessor->isSameRegister(rhsCasted->_handshakeRegisterAccessor)) return false;
      return true;
    }

     bool isReadOnly() const override {
      return _payloadRegisterAccessor->isReadOnly();
    }

    bool isReadable() const override {
      return _payloadRegisterAccessor->isReadable();
    }

    bool isWriteable() const override {
      return _payloadRegisterAccessor->isWriteable();
    }

    void doReadTransfer() override {
      _payloadRegisterAccessor->doReadTransfer();
    }

    bool doReadTransferNonBlocking() override {
      return _payloadRegisterAccessor->doReadTransferNonBlocking();
    }

    bool doReadTransferLatest() override {
      return _payloadRegisterAccessor->doReadTransferLatest();
    }
   
    void postRead() override {
      _payloadRegisterAccessor->postRead();
      _payloadRegisterAccessor-> accessChannel(0).swap(NDRegisterAccessor<UserType>::buffer_2D[0]);
    };
    
    void preWrite() override {
      _payloadRegisterAccessor-> accessChannel(0).swap(NDRegisterAccessor<UserType>::buffer_2D[0]);
      _payloadRegisterAccessor->preWrite();
    }

    bool write() override {
      // FIXME: does it make sense to call preWrite and postRead here?
      // does it make sense to have them at all?
      preWrite();

      std::lock_guard< std::mutex > lock(_backend->_mutex);
      _payloadRegisterAccessor->write();
      // perform the handshake: wait for the busy register to turn off.
      for (int i = 0; i < 10; ++i){
        // start with sleeping, the hardware needs some time anyway
        // a sleep that can be overloaded for testing
        // FIXME: make it testable
        //testable_TC4_sleep::sleep_for( boost::chrono::microseconds(100) );
        boost::chrono::microseconds(100);

        _handshakeRegisterAccessor->read();
        if (_handshakeRegisterAccessor == 0){
          break;
        }
      }
      if (_handshakeRegisterAccessor != 0){
        throw DeviceException(std::string("Error waiting for handshake in ")+_payloadRegisterAccessor->getName(), DeviceException::I_O_ERROR);
      }
      
      postWrite();
      return false;
    }


    
    void postWrite() override {
      _payloadRegisterAccessor->postWrite();
      _payloadRegisterAccessor-> accessChannel(0).swap(NDRegisterAccessor<UserType>::buffer_2D[0]);
    }

    std::vector< boost::shared_ptr<TransferElement> > getHardwareAccessingElements() override {
      return { boost::enable_shared_from_this<TransferElement>::shared_from_this() };
    }

    

    void replaceTransferElement(boost::shared_ptr<TransferElement> /*newElement*/) override {} // LCOV_EXCL_LINE

    boost::shared_ptr<HandshakingBackend> _backend;
    boost::shared_ptr< NDRegisterAccessor<UserType> > _payloadRegisterAccessor;
    boost::shared_ptr< NDRegisterAccessor<uint32_t> > _handshakeRegisterAccessor;
  };

}

#endif // HANDSHAKING_BACKEND_REGISTER_ACCESSOR_H
