#ifndef _MTCA4U_DMAP_FILE_DEFAULTS_H__
#define _MTCA4U_DMAP_FILE_DEFAULTS_H__

// This header should only be included in implementations because it contains
// the DMAP_FILE_PREFIX constant which is defined as compile paramter by CMAKE. You don't
// want to expose this detail to user code.

namespace mtca4u {
  static const std::string DMAP_FILE_ENVIROMENT_VARIABLE("DMAP_FILE"); ///< The envorinment variable which contains the dmap file
  static const std::string DMAP_FILE_DEFAULT(std::string(DMAP_FILE_PREFIX)
					     +"/etc/mtca4u/devices.dmap"); ///< The default dmap file
}// namespace mtca4u

#endif //_MTCA4U_DMAP_FILE_DEFAULTS_H__