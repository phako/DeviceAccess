#include <mtca4u/Device.h>
#include <mtca4u/Utilities.h>
#include <iostream>

int main() {
  mtca4u::setDMapFilePath("example.dmap");
  mtca4u::Device myDevice;
  myDevice.open("MY_DEVICE");

  /* We populate the memory region with multiple multiplexed sequences
   * so that we can use this for demonstrating the demultiplexing of the
   * TwoDRegisterAccessor (for some implementations depeding on the backend).
   * 
   * In this example we only have 4 sequences with 4 samples each.
   * We write numbers 0 to 15 as multiplexed data and expect the following result:
   * sequence 0:  0   4   8   12
   * sequence 1:  1   5   9   13
   * sequence 2:  2   6   10  14
   * sequence 3:  3   7   11  15
   *
   * We use a register named AREA_DATA_RAW which provides plain access to the data region.
   */
  auto dataRegion 
    = myDevice.getBufferingRegisterAccessor<double>("ADC/AREA_DATA_RAW");
  int counter = 0;
  for (auto & dataWord : dataRegion){
    dataWord=counter++;
  }
  dataRegion.write();
  
  /* Now check how it looks using the TwoDRegisterAccessor. We just copy it from
   * the accessor2D.cpp example.
   */
  mtca4u::TwoDRegisterAccessor<double> twoDAccessor =
    myDevice.getTwoDRegisterAccessor<double>("ADC/DATA");
  twoDAccessor.read();

  for (size_t i = 0; i < twoDAccessor.getNumberOfDataSequences(); ++i){
    std::cout << "Channel " << i << ":";
    std::vector<double> & channel = twoDAccessor[i];
    for (double sample : channel){
      std::cout << " " << sample;
    }
    std::cout << std::endl;
  }

  myDevice.close();

  return 0;
}
