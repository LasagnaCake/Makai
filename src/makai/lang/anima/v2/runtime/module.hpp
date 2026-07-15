#ifndef MAKAILIB_ANIMA_V2_RUNTIME_MODULE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_MODULE_H

#include "context.hpp"

namespace Makai::Anima::V2::Runtime::ARTModule {
	constexpr auto const BUILDER_FN_NAME = "AV2_ART_builder";

	struct IBuilder {
		struct Error {
			String	message;
			String	file;
			usize	line;
			usize	column = 0;
		}
		virtual ~IBuilder() {}

		virtual Result<Bytes<>, Error> build(String const& source) = 0;
	};
}

#define MODULE_BUILDER(BUILDER) CTL_CDECL CTL_DYNEXPORT ref<Makai::Anima::V2::Runtime::ARTModule::IBuilder> AV2_ART_builder() {\
	static BUILDER builder; return &builder;\
}\

#endif
