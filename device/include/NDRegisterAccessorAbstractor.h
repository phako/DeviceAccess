/*
 * NDRegisterAccessorAbstractor.h
 *
 *  Created on: Mar 23, 2016
 *      Author: Martin Hierholzer
 */

#ifndef CHIMERA_TK_N_D_REGISTER_ACCESSOR_ABSTRACTOR_H
#define CHIMERA_TK_N_D_REGISTER_ACCESSOR_ABSTRACTOR_H

#include "ForwardDeclarations.h"
#include "TransferElementAbstractor.h"
#include "NDRegisterAccessor.h"
#include "CopyRegisterDecorator.h"
#include "ReadAny.h"    // just for convenience...

namespace ChimeraTK {

  /** Base class for the reigster accessor abstractors (ScalarRegisterAccessor, OneDRegisterAccessor and
   *  TwoDRegisterAccessor). Provides a private implementation of the TransferElement interface to allow the bridges
   *  to be added to a TransferGroup. Also stores the shared pointer to the NDRegisterAccessor implementation. */
  template<typename UserType>
  class NDRegisterAccessorAbstractor : public TransferElementAbstractor {

    public:

      /** Create an uninitialised abstractor - just for late initialisation */
      NDRegisterAccessorAbstractor() {}

      /** Assign a new accessor to this NDRegisterAccessorAbstractor. Since another NDRegisterAccessorAbstractor is passed as
       *  argument, both NDRegisterAccessorAbstractors will then point to the same accessor and thus are sharing the
       *  same buffer. To obtain a new copy of the accessor with a distinct buffer, the corresponding
       *  getXXRegisterAccessor() function of Device must be called. */
      void replace(const NDRegisterAccessorAbstractor<UserType> &newAccessor) {
        _impl = newAccessor._impl;
        TransferElementAbstractor::_implUntyped = boost::static_pointer_cast<TransferElement>(_impl);
      }

      /** Alternative signature of relace() with the same functionality, used when a pointer to the implementation
       *  has been obtained directly (instead of a NDRegisterAccessorAbstractor). */
      void replace(boost::shared_ptr<NDRegisterAccessor<UserType>> newImpl) {
        _impl = newImpl;
        TransferElementAbstractor::_implUntyped = boost::static_pointer_cast<TransferElement>(_impl);
      }

      void replaceTransferElement(boost::shared_ptr<TransferElement> newElement) override {
        if(newElement->mayReplaceOther(_impl)) {
          if(newElement != _impl) {
            auto casted = boost::dynamic_pointer_cast<NDRegisterAccessor<UserType>>(newElement);
            _impl = boost::make_shared<ChimeraTK::CopyRegisterDecorator<UserType>>(casted);
            TransferElementAbstractor::_implUntyped = boost::static_pointer_cast<TransferElement>(_impl);
          }
        }
        else {
          _impl->replaceTransferElement(newElement);
        }
      }

      /** prevent copying by operator=, since it will be confusing (operator= may also be overloaded to access the
       *  content of the buffer!) */
      const NDRegisterAccessorAbstractor& operator=(const NDRegisterAccessorAbstractor& rightHandSide) const = delete;

    protected:

      NDRegisterAccessorAbstractor(boost::shared_ptr< NDRegisterAccessor<UserType> > impl)
      : TransferElementAbstractor(impl), _impl(impl)
      {}

      /** pointer to the implementation */
      boost::shared_ptr< NDRegisterAccessor<UserType> > _impl;

  };

  // Do not declare the template for all user types as extern here.
  // This could avoid optimisation of the inline code.

}

#endif /* CHIMERA_TK_N_D_REGISTER_ACCESSOR_BRIDGE_H */
