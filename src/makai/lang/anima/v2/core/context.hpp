#ifndef MAKAILIB_ANIMA_V2_CORE_CONTEXT_H
#define MAKAILIB_ANIMA_V2_CORE_CONTEXT_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "meta.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core {
	struct ILibrary;

	struct Context {
		enum class Error {
			AV2_CCE_MISSING_METHOD,
			AV2_CCE_MISSING_ART_TYPE,
			AV2_CCE_MISSING_ARGS,
			AV2_CCE_HOW_DID_YOU_GET_HERE,
		};

		struct ExternalMethod;

		using MethodResult = Result<Object::Storage, Error>;

		using ExternalInvocation = Function<MethodResult(Database<Definition>&, ExternalMethod&, List<Object::Storage> const&)>;

		struct ExternalMethodInfo {
			usize 		retTypeHash;
			List<usize>	argTypeHashes;
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
					Meta::arthashof<TReturn>(),
					List<usize>::from(Meta::arthashof<TArgs>()...)
				};
			}

			template <class TFunc>
			constexpr static Instance<ExternalInvocation> invoker(TFunc const& f) {
				return new ExternalInvocation(
					[f] (Database<Definition>& types, ExternalMethod& method, List<Object::Storage> const& args)
					-> MethodResult {
						if (types.byNameHash(Meta::arthashof<TReturn>()).empty())
							return Error::AV2_CCE_MISSING_ART_TYPE;
						if (args.size() < method.argc)
							return Error::AV2_CCE_MISSING_ARGS;
						auto tup = Meta::toArguments<TArgs...>(types, args.sliced(0, method.argc));
						if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
							invokeFromTuple<void>(f, tup);
							return Object::Storage();
						} else return Meta::ARTInfo<TReturn>::convert(
							types,
							invokeFromTuple<TReturn>(
								f,
								tup
							)
						);
						return Error::AV2_CCE_HOW_DID_YOU_GET_HERE;
					}
				);
			}
		};

		struct MethodAdder {
			template <class TFunc>
			bool add(String const& name, TFunc const& f) const {
				return context.addExternalMethod(name, f);
			}

			MethodAdder(Context& context): context(context) {}

		private:
			Context& context;
		};

		struct MethodRemover {
			void remove(String const& name) const {
				context.removeExternalMethod(name);
			}

			MethodRemover(Context& context): context(context) {}

		private:
			Context& context;
		};

		template <class TFunc>
		bool addExternalMethod(String const& name, TFunc const& f) {
			return addExternalMethod(ConstHasher::hash(name), f);
		}

		void removeExternalMethod(String const& name) {
			removeExternalMethod(ConstHasher::hash(name));
		}

		bool hasExternalMethod(String const& name) const {
			return hasExternalMethod(ConstHasher::hash(name));
		}

		Result<Object::Storage, Error> invokeExternalMethod(
			String const& name,
			List<Object::Storage> const& args
		) {
			return invokeExternalMethod(ConstHasher::hash(name), args);
		}

		template <class TFunc>
		bool addExternalMethod(usize const& hash, TFunc const& f) {
			if (hasExternalMethod(hash)) return false;
			using Resolver = ExternalMethodResolver<TFunc>;
			static auto const baseInfo = Resolver::info();
			ExternalMethod method;
			method.out		= true;
			method.argc		= Resolver::ARG_COUNT;
			method.hash		= hash;
			method.invoker	= Resolver::invoker(f);
			externalMethods[hash] = method;
			return true;
		}

		void removeExternalMethod(usize const& hash) {
			externalMethods.erase(hash);
		}

		bool hasExternalMethod(usize const& hash) const {
			return externalMethods.contains(hash);
		}

		Result<Object::Storage, Error> invokeExternalMethod(
			usize const& hash,
			List<Object::Storage> const& args
		) {
			if (!(
				hasExternalMethod(hash)
			&&	externalMethods[hash].invoker
			)) return Error::AV2_CCE_MISSING_METHOD;
			return externalMethods[hash].invoker->invoke(types, externalMethods[hash], args);
		}

		template <class T>
		constexpr Object::Storage newValue(T const& value) const {
			return Object::create(value, types.byNameHash(Meta::arthashof<T>()).front());
		}

		struct Library {
			Instance<ILibrary>	impl;
			CPP::Library		dll;

			~Library() {close();}

			static Nullable<Library> open(String const& path, Context& ctx);

			void close();
		};

		bool loadLibrary(String const& path) {
			auto const lib = Library::open(path, *this);
			if (!lib) return false;
			dynlibs.pushBack(*lib);
			return true;
		}

		void unloadLibraries() {
			dynlibs.clear();
		}

		Database<Definition>		types;
		Database<Method>			methods;
		Map<usize, ExternalMethod>	externalMethods;
		List<Library>				dynlibs;
	};
}

#endif
