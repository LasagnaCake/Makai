#ifndef CTL_ALGORITHM_CONVERT_BASE_H
#define CTL_ALGORITHM_CONVERT_BASE_H

#include "../../namespace.hpp"

CTL_NAMESPACE_BEGIN

namespace Convert {
	/// @brief Conversion base.
	enum class Base {
		CB_BASE2,
		CB_BASE4,
		CB_BASE8,
		CB_BASE16,
		CB_BASE32,
		CB_BASE64,
	};
}

CTL_NAMESPACE_END

#endif
