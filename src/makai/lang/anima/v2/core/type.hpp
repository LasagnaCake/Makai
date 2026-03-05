#ifndef MAKAILIB_ANIMA_V2_CORE_TYPE_H
#define MAKAILIB_ANIMA_V2_CORE_TYPE_H

#include "../../../../compat/ctl.hpp"

namespace Makai::Anima::V2::Core {
	enum class BasicType {
		AV2_BT_VOID,
		AV2_BT_ANY,
		AV2_BT_NULL,
		AV2_BT_BOOL,
		AV2_BT_INT,
		AV2_BT_UINT,
		AV2_BT_REAL,
		AV2_BT_STRING,
		AV2_BT_BYTES,
		AV2_BT_VECTOR,
	};

	struct Definition {
		struct Flags {
			constexpr static uint64 const AV2_DF_BASIC		= 1 << 0;
			constexpr static uint64 const AV2_DF_NULLABLE	= 1 << 1;
			constexpr static uint64 const AV2_DF_EMPTY		= 1 << 2;
			constexpr static uint64 const AV2_DF_ARRAY		= 1 << 3;
			constexpr static uint64 const AV2_DF_VALUE		= 1 << 4;
			constexpr static uint64 const AV2_DF_STRUCTURE	= 1 << 5;
			constexpr static uint64 const AV2_DF_DYNAMIC	= 1 << 6;
		};
		StringList					names;
		uint64						flags		= 0;
		Nullable<Core::BasicType>	basic		= Core::BasicType::AV2_BT_VOID;
		Instance<Definition>		base		= nullptr;
		uint64						byteSize	= 0;
		uint64						alignment	= 1;
	};
}

#endif
