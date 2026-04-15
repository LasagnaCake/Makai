#ifndef MAKAILIB_ANIMA_V2_CORE_SHAREDLIBRARY_H
#define MAKAILIB_ANIMA_V2_CORE_SHAREDLIBRARY_H

#include "object.hpp"
#include "module.hpp"
#include "meta.hpp"

namespace Makai::Anima::V2::Core {
	struct SharedLibrary {
		enum class Error {
			AV2_SLE_MISSING_METHOD,
			AV2_SLE_MISSING_ART_TYPE,
			AV2_SLE_MISSING_ARGS,
		};

		struct LibraryMethod {
			using Invoker = Function<Result<Object::Storage, Error>(List<Object::Storage> const&)>;
			usize			argc;
			Invoker			invoker;
		};

		template <class T> struct LibraryMethodResolver;

		template <class TReturn, class... TArgs>
		struct LibraryMethodResolver<TReturn(TArgs...)> {
			static_assert((... && Makai::Type::EqualOrConst<TArgs, AsNonReference<TArgs>>), "Arument type(s) cannot be a reference!");

			constexpr static usize const ARG_COUNT = sizeof...(TArgs);

			template <class TFunc>
			constexpr static Instance<LibraryMethod> method(Function<TFunc> const& f) {
				return new LibraryMethod{
					sizeof...(TArgs),
					[f] (Database<Definition>& types, LibraryMethod& method, List<Object::Storage> const& args) {
						if (types.byName(Meta::artnameof<TReturn>()).empty())
							return Error::AV2_SLE_MISSING_ART_TYPE;
						if (args.size() < method.argc)
							return Error::AV2_SLE_MISSING_ARGS;
						if constexpr (Makai::Type::Void<TReturn>)
							::Makai::invokeFromTuple(f, toArguments<TArgs...>(args.sliced(0, method.argc)));
						else return Meta::ARTInfo<TReturn>::convert(types, invokeFromTuple(f, toArguments<TArgs...>(args.sliced(0, method.argc))));
					}
				};
			}
		};

		Instance<Module::Declaration>	type(uint64 const id);
		Instance<Module::Declaration>	type(UTF8String const& name);

		Instance<Module::Method>		method(uint64 const id);
		Instance<Module::Method>		method(UTF8String const& name);

		Object::Storage invoke(UTF8String const& name, List<Object::Storage> const& args);

		void addMethod(UTF8String const& name, LibraryMethod const& method);
	};
}

#endif
