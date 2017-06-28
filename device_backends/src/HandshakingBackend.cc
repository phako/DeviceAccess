#include "HandshakingBackend.h"
#include "HandshakingBackendRegisterAccessor.h"
#include "BackendFactory.h"
#include "DeviceAccessVersion.h"

namespace ChimeraTK{

  HandshakingBackend::HandshakingBackend(std::string parentBackend, std::string payloadRegister, std::string handshakeRegister):
    _parentBackendAlias(parentBackend), _payloadRegisterName(payloadRegister), _handshakeRegisterName(handshakeRegister){
    FILL_VIRTUAL_FUNCTION_TEMPLATE_VTABLE(getRegisterAccessor_impl);    
  }

  void HandshakingBackend::open(){
    _parentBackend =  BackendFactory::getInstance().createBackend(_parentBackendAlias);

    auto parentCatalogue = _parentBackend->getRegisterCatalogue();
    _catalogue = RegisterCatalogue(); // empty the catalogue by replacing with a new one
    _catalogue.addRegister( parentCatalogue.getRegister(_payloadRegisterName) );
    
    _opened=true;
  }

  void HandshakingBackend::close(){
    _parentBackend = nullptr;
    _opened=false;
  }
  
  boost::shared_ptr<DeviceBackend> HandshakingBackend::createInstance(std::string /*host*/,
    std::string /*instance*/, std::list<std::string> parameters, std::string /*mapFileName*/){
    if (parameters.size() != 3){
      throw(DeviceBackendException("Wrong number of parameters in HandshakingBackend URI", DeviceBackendException::EX_WRONG_PARAMETER));
    }

    // fill list into vector for better handling
    std::vector<std::string> parameterVec;
    for (auto & s : parameters){
      parameterVec.push_back(s);
    }

    return boost::shared_ptr<DeviceBackend>(new HandshakingBackend(parameterVec.at(0), parameterVec.at(1),parameterVec.at(2)));
  }

  template<typename UserType>
  boost::shared_ptr< NDRegisterAccessor<UserType> > HandshakingBackend::getRegisterAccessor_impl(
      const RegisterPath &registerPathName, size_t numberOfWords, size_t wordOffsetInRegister, AccessModeFlags flags) {
    if (registerPathName != _payloadRegisterName){
      throw(DeviceBackendException("Unknown register path " + registerPathName, DeviceBackendException::EX_WRONG_PARAMETER));
    }

    return boost::shared_ptr< NDRegisterAccessor<UserType> >(
      new HandshakingBackendRegisterAccessor<UserType>(boost::dynamic_pointer_cast<HandshakingBackend>(shared_from_this()), _payloadRegisterName, numberOfWords, wordOffsetInRegister, flags, _handshakeRegisterName));
  }

  struct HandshakingBackendRegisterer{
    HandshakingBackendRegisterer(){
      BackendFactory::getInstance().registerBackendType("handshaking","",&HandshakingBackend::createInstance, CHIMERATK_DEVICEACCESS_VERSION);
    }
  };

  HandshakingBackendRegisterer theRegisterer;
}//namespace ChimeraTK
