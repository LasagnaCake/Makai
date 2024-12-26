#ifndef CTL_EX_NAMESPACE_H
#define CTL_EX_NAMESPACE_H

#include "namespace.hpp"

/// @brief CTL extensions namespace.
#define CTL_EX_NAMESPACE_BEGIN	CTL_NAMESPACE_BEGIN namespace Ex {
/// @brief CTL extensions namespace.
#define CTL_EX_NAMESPACE_END	} CTL_NAMESPACE_END

CTL_NAMESPACE_BEGIN
/// @brief Type constraints.
namespace Type {}
/// CTL extensions.
namespace Ex {}
CTL_NAMESPACE_END

CTL_EX_NAMESPACE_BEGIN

/// @brief CTL extensions.
namespace Ex = ::CTL::Ex;

namespace {
	using namespace ::CTL;
}

/// @brief Type constraints.
namespace Type {
	using namespace ::CTL::Type;

	/// @brief CTL extensions type constraints.
	namespace Ex {}

	/// @brief Math extensions type constraints.
	namespace Math{}
}

/// @brief Math extensions.
namespace Math{}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_NAMESPACE_H
