#ifndef CHIMERATK_HANDSHAKING_BACKEND_H
#define CHIMERATK_HANDSHAKING_BACKEND_H

#include "DeviceBackendImpl.h"
#include <mutex>

namespace ChimeraTK{

  /** Oftern registers or areas need some time to process a write request, with a separate busy
   *  register indicating that a new write command cannot be processed yet.
   *  This backend provides accessors which perform the handshake and have a lock,
   *  so partial accessors can be used in different threads.
   *  The only register it provides is the payload register.
   *
   *  URI syntax:
   *  sdm://./handshaking=ALIAS,PAYLOAD_REGISTER,HANDSHAKE_REGISTER
   */
  class HandshakingBackend : public DeviceBackendImpl {

    public:

      HandshakingBackend(std::string parentBackend, std::string payloadRegister, std::string handshakeRegister);

      virtual ~HandshakingBackend(){}

      virtual void open();

      virtual void close();

      virtual std::string readDeviceInfo() {
        return std::string("Handshaking backend on ") + _parentBackendAlias + " register " + _payloadRegisterName;
      }

      static boost::shared_ptr<DeviceBackend> createInstance(std::string host, std::string instance,
          std::list<std::string> parameters, std::string mapFileName);

      virtual void read(const std::string &, const std::string &,
          int32_t *, size_t  = 0, uint32_t  = 0) {
        throw DeviceException("Not implemented", DeviceException::NOT_IMPLEMENTED);
      }

      virtual void write(const std::string &,
          const std::string &, int32_t const *,
          size_t  = 0, uint32_t  = 0)  {
        throw DeviceException("Not implemented", DeviceException::NOT_IMPLEMENTED);
      }

    protected:

      template<typename UserType>
      boost::shared_ptr< NDRegisterAccessor<UserType> > getRegisterAccessor_impl(
          const RegisterPath &registerPathName, size_t numberOfWords, size_t wordOffsetInRegister, AccessModeFlags flags);
      DEFINE_VIRTUAL_FUNCTION_TEMPLATE_VTABLE_FILLER( HandshakingBackend, getRegisterAccessor_impl, 4);

      /// name of the parent backend
      std::string _parentBackendAlias;
      std::string _payloadRegisterName;
      std::string _handshakeRegisterName;

      /// the parent backend itself
      boost::shared_ptr<DeviceBackend> _parentBackend;
      
      /// The lock for this backend.
      std::mutex _mutex;
      
      template<typename T>
      friend class HandshakingAccessor;
  };

}

#endif /* CHIMERATK_LOGICAL_NAME_MAPPING_BACKEND_H */
