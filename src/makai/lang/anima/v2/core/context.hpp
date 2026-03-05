#ifndef MAKAILIB_ANIMA_V2_CORE_CONTEXT_H
#define MAKAILIB_ANIMA_V2_CORE_CONTEXT_H

#include "type.hpp"
#include "method.hpp"
#include "value.hpp"

namespace Makai::Anima::V2::Core {
	struct Context {
		struct ExternalMethodInfo {
			String 		retTypeName;
			StringList	argTypeNames;
		};

		template <class T> struct ExternalMethod;

		template <class TReturn, class... TArgs>
		struct ExternalMethod<TReturn(TArgs...)> {
			static_assert((... && Type::EqualOrConst<TArgs, AsNonReference<TArgs>>), "Arument type(s) cannot be a reference!");
			constexpr static ExternalMethodInfo info() {
				return {
					artnameof<TReturn>(),
					StringList::from(artnameof<TArgs>()...)
				};
			}
		};

		template <class TFunc>
		void addExternalMethod(String const& name, TFunc const& f) {
			static auto const baseInfo = ExternalMethod<TFunc>::info();
			Method method;
			method.name = name;
		}
	};
}

#endif
