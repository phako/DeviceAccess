#ifndef MTCA4U_SHARED_DUMMY_BACKEND_H
#define MTCA4U_SHARED_DUMMY_BACKEND_H

#include <vector>
#include <map>
#include <list>
#include <set>
#include <mutex>
#include <utility>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/function.hpp>

#include "Exception.h"
#include "RegisterInfoMap.h"
#include "NumericAddressedBackend.h"


// Define shared-memory compatible vector type and corresponding allocator
typedef boost::interprocess::allocator<int32_t, boost::interprocess::managed_shared_memory::segment_manager>  ShmemAllocator;
typedef boost::interprocess::vector<int32_t, ShmemAllocator> SharedMemoryVector;


namespace ChimeraTK {

  /** TODO DOCUMENTATION
   */
  class SharedDummyBackend : public NumericAddressedBackend
  {
    public:
      SharedDummyBackend(std::string instanceId, std::string mapFileName);
      virtual ~SharedDummyBackend();

      virtual void open();
      virtual void close();
      virtual void read(uint8_t bar, uint32_t address, int32_t* data,  size_t sizeInBytes);
      virtual void write(uint8_t bar, uint32_t address, int32_t const* data,  size_t sizeInBytes);
      virtual std::string readDeviceInfo();
      
      int32_t& getRegisterContent(uint8_t bar, uint32_t address);

      static boost::shared_ptr<DeviceBackend> createInstance(std::string host, std::string instance,
                                                            std::list<std::string> parameters, std::string mapFileName);

    private:

      /** name of the map file */
      std::string _mapFile;

      RegisterInfoMapPointer _registerMapping;
      
      // Bar contents with shared-memory compatible vector type. Plain pointers are used here since this is what we
      // get from the shared memory allocation.
      std::map<uint8_t, SharedMemoryVector*> _barContents;
      
      // Bar sizes
      std::map<uint8_t, size_t> _barSizesInBytes;
      

      // Helper class to manage the shared memory: automatically construct if necessary,
      // automatically destroy if last using process closes.
      /*\
       * TODO: * Include User and URI in hashes
       *         -> Additional overhead?
       *
       */
      class SharedMemoryManager {
        
      public:
        SharedMemoryManager(SharedDummyBackend &sharedDummyBackend_,
                            const std::string instanceId,
                            const std::string mapFileName) :
          sharedDummyBackend(sharedDummyBackend_),
          mapFileHash(std::to_string(std::hash<std::string>{}(mapFileName))),
          instanceIdHash(std::to_string(std::hash<std::string>{}(instanceId))),
          name("ChimeraTK_SharedDummy_"+mapFileHash+"_"+instanceIdHash),
          segment(boost::interprocess::open_or_create, name.c_str(), getRequiredMemoryWithOverhead()),
          alloc_inst(segment.get_segment_manager()),
          globalMutex(boost::interprocess::open_or_create, name.c_str())
        {
          // lock guard with the interprocess mutex
          std::lock_guard<boost::interprocess::named_mutex> lock(globalMutex);

          // find the use counter
          auto res = segment.find<size_t>("UseCounter");
          if(res.second != 1) {  // if not found: create it
            useCount = segment.construct<size_t>("UseCounter")(0);
          }
          else {
            useCount = res.first;
          }

          // increment use counter
          (*useCount)++;

#ifdef _DEBUG
          std::cout << "Created shared memory with a size of " << segment.get_size() << " bytes." << std::endl;
          std::cout << "    Free space : " << segment.get_free_memory() << std::endl;
          std::cout << "    useCount is: " << *useCount << std::endl << std::flush;
#endif
        }

        ~SharedMemoryManager() {

          // lock guard with the interprocess mutex
          std::lock_guard<boost::interprocess::named_mutex> lock(globalMutex);
          
          // decrement use counter
          (*useCount)--;
          
          // if use count at 0, destroy shared memory and the interprocess mutex
          if(*useCount == 0) {
            boost::interprocess::shared_memory_object::remove(name.c_str());
            boost::interprocess::named_mutex::remove(name.c_str());
          }
        }
        
        /**
         * Finds or constructs a vector object in the shared memory.
         */
        SharedMemoryVector* findOrConstructVector(const std::string& objName, const size_t size);

        /**
         * Get information on the shared memory segment
         * @retval std::pair<size_t, size_t> first: Size of the memory segment, second: free memory in segment
         */
        std::pair<size_t, size_t> getInfoOnMemory();

      private:

        // Constants to take overhead of managed shared memory into respect
        // (approx. linear function, evaluated using Boost 1.58)
        // TODO Adjust for additional overhead (PIDs, ...)
        static const size_t SHARED_MEMORY_CONST_OVERHEAD = 1000;
        static const size_t SHARED_MEMORY_OVERHEAD_PER_VECTOR = 80;

        SharedDummyBackend& sharedDummyBackend;

        // Hashes to assure match of shared memory accessing processes
        std::string mapFileHash;
        std::string instanceIdHash;

        // the name of the segment
        std::string name;

        // the shared memory segment
        boost::interprocess::managed_shared_memory segment;

        // the allocator instance
        const ShmemAllocator alloc_inst;
        
        // global (interprocess) mutex
        boost::interprocess::named_mutex globalMutex;

        // pointer to the use count on shared memory;
        size_t *useCount{nullptr};

        size_t getRequiredMemoryWithOverhead();

      };  /* class SharedMemoryManager */
      
      // Managed shared memory object
      SharedMemoryManager sharedMemoryManager;
      
      void setupBarContents();
      std::map< uint8_t, size_t > getBarSizesInBytesFromRegisterMapping() const;

      // Helper routines called in init list
      // TODO Remove std::map<uint8_t, size_t> getBarSizesInWords() const;
      size_t getTotalRegisterSizeInBytes() const;

      static void checkSizeIsMultipleOfWordSize(size_t sizeInBytes);

      static std::string convertPathRelativeToDmapToAbs(std::string const & mapfileName);

      /** map of instance names and pointers to allow re-connecting to the same instance with multiple Devices */
      /* FIXME Namespace change ok? */
      static std::map< std::string, boost::shared_ptr<DeviceBackend> >& getInstanceMap() {
        static std::map< std::string, boost::shared_ptr<DeviceBackend> > instanceMap;
        return instanceMap;
      }

      /**
       * @brief Method looks up and returns an existing instance of class 'T'
       * corresponding to instanceId, if instanceId is a valid  key in the
       * internal map. For an instanceId not in the internal map, a new instance
       * of class T is created, cached and returned. Future calls to
       * returnInstance with this instanceId, returns this cached instance. If
       * the instanceId is "" a new instance of class T is created and
       * returned. This instance will not be cached in the internal memory.
       *
       * @param instanceId Used as key for the object instance look up. "" as
       *                   instanceId will return a new T instance that is not
       *                   cached.
       * @param arguments  This is a template argument list. The constructor of
       *                   the created class T, gets called with the contents of
       *                   the argument list as parameters.
       */
      template <typename T, typename... Args>
      static boost::shared_ptr<DeviceBackend> returnInstance(const std::string& instanceId, Args&&... arguments) {
        if (instanceId == "") {
          // std::forward because template accepts forwarding references
          // (Args&&) and this can have both lvalue and rvalue references passed
          // as arguments.
          return boost::shared_ptr<DeviceBackend>(new T(std::forward<Args>(arguments)...));
        }
        // search instance map and create new instanceId, if not found under the
        // name
        if (getInstanceMap().find(instanceId) == getInstanceMap().end()) {
          boost::shared_ptr<DeviceBackend> ptr(new T(std::forward<Args>(arguments)...));
          getInstanceMap().insert(std::make_pair(instanceId, ptr));
          return ptr;
        }
        // return existing instanceId from the map
        return boost::shared_ptr<DeviceBackend>(getInstanceMap()[instanceId]);
      }

  };

} // namespace ChimeraTK

#endif // MTCA4U_SHARED_DUMMY_BACKEND_H
