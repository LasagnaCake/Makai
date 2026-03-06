#ifndef MAKAILIB_ANIMA_V2_CORE_CONTEXT_H
#define MAKAILIB_ANIMA_V2_CORE_CONTEXT_H

#include "type.hpp"
#include "method.hpp"
#include "value.hpp"
#include "meta.hpp"

namespace Makai::Anima::V2::Core {
	struct Context {
		enum class Error {
			AV2_CCE_MISSING_ART_TYPE,
			AV2_CCE_MISSING_ARGS,
		};

		struct ExternalMethod;

		using ExternalInvocation = Function<Result<Object, Error>(Definition::Database&, ExternalMethod&, List<Object> const&)>;

		struct ExternalMethodInfo {
			String 		retTypeName;
			StringList	argTypeNames;
		};

		struct ExternalMethod: Method {
			ExternalInvocation	invoker;
			usize				argc;
		};

		template <class T> struct ExternalMethodResolver;

		template <class TReturn, class... TArgs>
		struct ExternalMethodResolver<TReturn(TArgs...)> {
			static_assert((... && Type::EqualOrConst<TArgs, AsNonReference<TArgs>>), "Arument type(s) cannot be a reference!");

			constexpr static usize const ARG_COUNT = sizeof...(TArgs);

			constexpr static ExternalMethodInfo info() {
				return {
					artnameof<TReturn>(),
					StringList::from(artnameof<TArgs>()...)
				};
			}

			template <class TFunc>
			constexpr static ExternalInvocation invoker(Function<TFunc> const& f) {
				return [f] (Definition::Database& types, ExternalMethod& method, List<Object> const& args) {
					if (!types.byAlias(artnameof<TReturn>()))
						return Error::AV2_CCE_MISSING_ART_TYPE;
					if (args.size() < method.argc)
						return Error::AV2_CCE_MISSING_ARGS;
					if constexpr (Type::Void<TReturn>)
						invoke(f, toArguments<TArgs...>(args));
					else return converter<TReturn>()(types, invokeFromTuple(f, toArguments<TArgs...>(args)));
				};
			}
		};

		template <class TFunc>
		void addExternalMethod(String const& name, TFunc const& f) {
			using Resolver = ExternalMethodResolver<TFunc>;
			static auto const baseInfo = typename Resolver::info();
			ExternalMethod method;
			method.out		= true;
			method.argc		= Resolver::ARG_COUNT;
			method.name		= name;
			method.invoker	= typename Resolver::invoker(f);
		}
	};
}

#endif
