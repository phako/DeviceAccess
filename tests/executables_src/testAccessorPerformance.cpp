#include <iostream>
#include <fstream>

#include <boost/chrono.hpp>
using namespace boost::chrono;

#include "Device.h"
#include "Utilities.h"

using namespace mtca4u;

/**
 *
 * Usage: ( cd tests ; ../bin/testAccessorPerformance [<NumberOfIterations>] )
 *
 * <NumberOfIterations> is the number of iterations used for block access tests. Single word access tests will use
 * 100000 times the given number of iterations. If omitted, the number of iterations defaults to 10 (which is
 * acceptable also on slower machines in debug build mode).
 *
*/
int main(int argc, char **argv){
  steady_clock::time_point t0;
  duration<double> tdur;

  setDMapFilePath("dummies.dmap");

  Device device;
  device.open("PERFTEST");

  int niterBlock;
  if(argc <= 1) {
    niterBlock = 10;
  }
  else {
    niterBlock = atoi(argv[1]);
  }
  int niter = niterBlock*100000;

  int64_t sum = 0;

  std::ofstream fresult("performance_test.txt", std::ofstream::out);

  std::cout <<" ***************************************************************************" << std::endl;
  std::cout <<" Tests with the OneDRegisterAccessor:" << std::endl;

  auto acc1D = device.getOneDRegisterAccessor<int>("ADC/AREA_DMA_VIA_DMA");
  t0 = steady_clock::now();
  std::cout <<" reading block ";
  for(int i= 0; i < niterBlock; ++i){
      acc1D.read();
      sum += acc1D[i];
  }
  tdur = steady_clock::now()-t0;
  std::cout << "took " << tdur.count()*1000./niterBlock << " ms per block" << std::endl;
  fresult << "1D_COOKEDus=" << std::round(tdur.count()*1000000./niterBlock) << std::endl;

  auto acc1Draw = device.getOneDRegisterAccessor<int>("ADC/AREA_DMA_VIA_DMA",0,0, {AccessMode::raw});
  t0 = steady_clock::now();
  std::cout <<" raw-reading block ";
  for(int i= 0; i < niterBlock; ++i){
    acc1Draw.read();
      sum += acc1Draw[i];
  }
  tdur = steady_clock::now()-t0;
  std::cout << "took " << tdur.count()*1000./niterBlock << " ms per block" << std::endl;
  fresult << "1D_RAWus=" << std::round(tdur.count()*1000000./niterBlock) << std::endl;

  std::cout <<" ***************************************************************************" << std::endl;
  std::cout <<" Tests with the compatibility RegisterAccessor:" << std::endl;

  auto accessor = device.getRegisterAccessor("WORD_STATUS","BOARD");

  std::cout <<" reading ";
  sum += accessor->read<int>();
  t0 = steady_clock::now();
  for(int i= 0; i < niter; ++i){
    sum += accessor->read<int>();
  }
  tdur = steady_clock::now()-t0;
  std::cout << "took " << tdur.count()*1000000./niter << " us per word" << std::endl;
  fresult << "RACOMPAT_READns=" << std::round(tdur.count()*1000000000./niter) << std::endl;

  std::cout <<" writing ";
  t0 = steady_clock::now();
  for(int i= 0; i < niter; ++i){
    accessor->write<int>(i);
  }
  tdur = steady_clock::now()-t0;
  std::cout << "took " << tdur.count()*1000000./niter << " us per word" << std::endl;
  fresult << "RACOMPAT_WRITEns=" << std::round(tdur.count()*1000000000./niter) << std::endl;

  std::cout <<" reading raw ";
  t0 = steady_clock::now();
  for(int i= 0; i < niter; ++i){
    int buffer;
    accessor->readRaw(&buffer,sizeof(int));
    sum += buffer;
  }
  tdur = steady_clock::now()-t0;
  std::cout << "took " << tdur.count()*1000000./niter << " us per word" << std::endl;
  fresult << "RACOMPAT_READRAWns=" << std::round(tdur.count()*1000000000./niter) << std::endl;

  std::cout <<" writing raw ";
  t0 = steady_clock::now();
  for(int i= 0; i < niter; ++i){
    accessor->writeRaw(&i, sizeof(int));
  }
  tdur = steady_clock::now()-t0;
  std::cout << "took " << tdur.count()*1000000./niter << " us per word" << std::endl;
  fresult << "RACOMPAT_WRITERAWns=" << std::round(tdur.count()*1000000000./niter) << std::endl;

  auto accessor2 = device.getRegisterAccessor("AREA_DMAABLE","ADC");
  t0 = steady_clock::now();
  std::cout <<" reading moving word in block ";
  for(int i= 0; i < niterBlock; ++i){
      int buffer;
      accessor2->read<int>(&buffer, 1, i);
      sum += buffer;
  }
  tdur = steady_clock::now()-t0;
  std::cout << "took " << tdur.count()*1000./niterBlock << " ms per transfer" << std::endl;
  fresult << "RACOMPAT_READMOVINGWORDus=" << std::round(tdur.count()*1000000./niterBlock) << std::endl;


  std::cout <<" ***************************************************************************" << std::endl;
  t0 = steady_clock::now();
  std::cout << " Sum of all read data: " << sum << std::endl;
  tdur = steady_clock::now()-t0;
  std::cout << " Printing the sum took " << tdur.count()*1000. << " ms" << std::endl;
  std::cout <<" ***************************************************************************" << std::endl;

  fresult.close();

  return 0;
}
