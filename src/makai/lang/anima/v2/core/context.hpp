#ifndef MAKAILIB_ANIMA_V2_CORE_CONTEXT_H
#define MAKAILIB_ANIMA_V2_CORE_CONTEXT_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "meta.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core {
	struct Context {
		enum class Error {
			AV2_CCE_MISSING_METHOD,
			AV2_CCE_MISSING_ART_TYPE,
			AV2_CCE_MISSING_ARGS,
		};

		struct ExternalMethod;

		using ExternalInvocation = Function<Result<Object::Storage, Error>(Database<Definition>&, ExternalMethod&, List<Object::Storage> const&)>;

		struct ExternalMethodInfo {
			String 		retTypeName;
			StringList	argTypeNames;
		};

		struct ExternalMethod: Method {
			Instance<ExternalInvocation>	invoker;
			usize							argc;
		};

		template <class T> struct ExternalMethodResolver;

		template <class TReturn, class... TArgs>
		struct ExternalMethodResolver<TReturn(TArgs...)> {
			static_assert((... && Makai::Type::EqualOrConst<TArgs, AsNonReference<TArgs>>), "Arument type(s) cannot be a reference!");

			constexpr static usize const ARG_COUNT = sizeof...(TArgs);

			constexpr static ExternalMethodInfo info() {
				return {
					Meta::artnameof<TReturn>(),
					StringList::from(Meta::artnameof<TArgs>()...)
				};
			}

			template <class TFunc>
			constexpr static Instance<ExternalInvocation> invoker(Function<TFunc> const& f) {
				return new ExternalInvocation(
					[f] (Database<Definition>& types, ExternalMethod& method, List<Object::Storage> const& args) {
						if (types.byName(Meta::artnameof<TReturn>()).empty())
							return Error::AV2_CCE_MISSING_ART_TYPE;
						if (args.size() < method.argc)
							return Error::AV2_CCE_MISSING_ARGS;
						if constexpr (Makai::Type::Void<TReturn>)
							invoke(f, toArguments<TArgs...>(args));
						else return Meta::ARTInfo<TReturn>::convert(types, invokeFromTuple(f, toArguments<TArgs...>(args.sliced(0, method.argc))));
					}
				);
			}
		};

		template <class TFunc>
		bool addExternalMethod(String const& name, TFunc const& f) {
			if (hasExternalMethod(name)) return false;
			using Resolver = ExternalMethodResolver<TFunc>;
			static auto const baseInfo = typename Resolver::info();
			ExternalMethod method;
			method.out		= true;
			method.argc		= Resolver::ARG_COUNT;
			method.name		= name;
			method.invoker	= typename Resolver::invoker(f);
			externalMethods[name] = method;
			return true;
		}

		void removeExternalMethod(String const& name) {
			externalMethods.erase(name);
		}

		bool hasExternalMethod(String const& name) const {
			return externalMethods.contains(name);
		}

		Result<Object::Storage, Error> invokeExternalMethod(
			String const& name,
			List<Object::Storage> const& args
		) {
			if (!(
				hasExternalMethod(name)
			&&	externalMethods[name].invoker
			)) return Error::AV2_CCE_MISSING_METHOD;
			return externalMethods[name].invoker->invoke(types, externalMethods[name], args);
		}

		template <class T>
		constexpr Object::Storage newValue(T const& value) const {
			return Object::create(value, types.byName(Meta::artnameof<T>()).front());
		}

		Database<Definition>		types;
		Database<Method>			methods;
		Dictionary<ExternalMethod>	externalMethods;
	};
}

#endif
