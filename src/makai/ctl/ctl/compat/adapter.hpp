#ifndef CTL_COMPAT_ADAPTER_H
#define CTL_COMPAT_ADAPTER_H

#include "namespace.hpp"

CTL_NAMESPACE_BEGIN

template<class TParent>
struct CTL2STDAdapter: TParent {
};

template<class TParent>
struct STDT2CTLAdapter: TParent {
};

CTL_NAMESPACE_END

#endif // CTL_COMPAT_ADAPTER_H
