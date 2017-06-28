#define BOOST_TEST_MODULE HandshakingBackendTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "Device.h"
#include "Utilities.h"
using namespace mtca4u;

BOOST_AUTO_TEST_CASE( testCreation ){
  setDMapFilePath("dummies.dmap");

  Device d;
  d.open("sdm://./handshaking=DUMMYD1,APP0/MODULE0,APP0/WORD_STATUS");

  auto registerCatalogue = d.getRegisterCatalogue();
  BOOST_CHECK( registerCatalogue.getNumberOfRegisters() == 1 );
  BOOST_CHECK( registerCatalogue.hasRegister("APP0/MODULE0"));
}
