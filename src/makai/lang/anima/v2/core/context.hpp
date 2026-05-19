#ifndef MAKAILIB_ANIMA_V2_CORE_CONTEXT_H
#define MAKAILIB_ANIMA_V2_CORE_CONTEXT_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "meta.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core {
	template <class... Args>
	concept NonMutableReferenceArgs = (... && Makai::Type::OneOf<AsNonVolatile<Args>, AsNormal<Args>, AsReference<AsConstant<AsNormal<Args>>>>);

	struct Context {
		enum class Error {
			AV2_CCE_MISSING_METHOD,
			AV2_CCE_MISSING_ART_TYPE,
			AV2_CCE_MISSING_ARGS,
			AV2_CCE_HOW_DID_YOU_GET_HERE,
		};

		struct ExternalMethod;

		using MethodResult = Result<Object::Storage, Error>;

		using ExternalInvocation = Function<MethodResult(Context&, ExternalMethod&, List<Object::Storage> const&)>;

		struct ExternalMethodInfo {
			usize 		retTypeHash;
			List<usize>	argTypeHashes;
		};

		struct ExternalMethod: Method {
			Instance<ExternalInvocation>	invoker;
			usize							argc;
		};

		template <class T> struct ExternalMethodResolver;

		template <class TReturn>
		struct ExternalMethodResolver<TReturn()> {
			constexpr static bool const CONTEXTUAL = false;

			constexpr static usize const ARG_COUNT = 0;

			constexpr static ExternalMethodInfo info() {
				return {Meta::arthashof<TReturn>(), {}};
			}

			template <class TFunc>
			constexpr static Instance<ExternalInvocation> invoker(TFunc const& f) {
				return new ExternalInvocation(
					[f] (Context& context, ExternalMethod& method, List<Object::Storage> const& args)
					-> MethodResult {
						if (context.types.byNameHash(Meta::arthashof<TReturn>()).empty())
							return Error::AV2_CCE_MISSING_ART_TYPE;
						if (args.size() < method.argc)
							return Error::AV2_CCE_MISSING_ARGS;
						if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
							f();
							return Object::Storage();
						} else return Meta::ARTInfo<TReturn>::convert(
							context.types,
							f()
						);
						return Error::AV2_CCE_HOW_DID_YOU_GET_HERE;
					}
				);
			}
		};

		template <class TReturn, class TFirst, NonMutableReferenceArgs... TArgs>
		struct ExternalMethodResolver<TReturn(TFirst, TArgs...)> {
			constexpr static bool const CONTEXTUAL = Type::OneOf<AsNonVolatile<TFirst>, Context&, Context const&>;

			constexpr static usize const ARG_COUNT = sizeof...(TArgs) + CONTEXTUAL;

			constexpr static ExternalMethodInfo info() requires (!CONTEXTUAL) {
				return {
					Meta::arthashof<TReturn>(),
					List<usize>::from(Meta::arthashof<TFirst>(), Meta::arthashof<TArgs>()...)
				};
			}

			constexpr static ExternalMethodInfo info()  requires (CONTEXTUAL) {
				return {
					Meta::arthashof<TReturn>(),
					List<usize>::from(Meta::arthashof<TArgs>()...)
				};
			}

			template <class TFunc>
			constexpr static Instance<ExternalInvocation> invoker(TFunc const& f)
			requires (!CONTEXTUAL) {
				static_assert(NonMutableReferenceArgs<TFirst>, "Arument type(s) cannot be a reference!");
				return new ExternalInvocation(
					[f] (Context& context, ExternalMethod& method, List<Object::Storage> const& args)
					-> MethodResult {
						if (context.types.byNameHash(Meta::arthashof<TReturn>()).empty())
							return Error::AV2_CCE_MISSING_ART_TYPE;
						if (args.size() < method.argc)
							return Error::AV2_CCE_MISSING_ARGS;
						auto tup = Meta::toArguments<TFirst, TArgs...>(context.types, args.sliced(0, method.argc));
						if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
							invokeFromTuple<void>(f, tup);
							return Object::Storage();
						} else return Meta::ARTInfo<TReturn>::convert(
							context.types,
							invokeFromTuple<TReturn>(
								f,
								tup
							)
						);
						return Error::AV2_CCE_HOW_DID_YOU_GET_HERE;
					}
				);
			}

			template <class TFunc>
			constexpr static Instance<ExternalInvocation> invoker(TFunc const& f)
			requires (CONTEXTUAL) {
				return new ExternalInvocation(
					[f] (Context& context, ExternalMethod& method, List<Object::Storage> const& args)
					-> MethodResult {
						if (context.types.byNameHash(Meta::arthashof<TReturn>()).empty())
							return Error::AV2_CCE_MISSING_ART_TYPE;
						if (args.size() < method.argc)
							return Error::AV2_CCE_MISSING_ARGS;
						auto tup = Meta::toArgumentsWithContext<Context, TArgs...>(context.types, args.sliced(0, method.argc), context);
						if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
							invokeFromTuple<void>(f, tup);
							return Object::Storage();
						} else return Meta::ARTInfo<TReturn>::convert(
							context.types,
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

			constexpr MethodAdder(Context& context): context(context) {}

		private:
			Context& context;
		};

		struct MethodRemover {
			void remove(String const& name) const {
				context.removeExternalMethod(name);
			}

			constexpr MethodRemover(Context& context): context(context) {}

		private:
			Context& context;
		};

		struct TypeAdder {
			template <class T>
			bool add() const {
				return context.addExternalType<T>();
			}

			constexpr TypeAdder(Context& context): context(context) {}

		private:
			Context& context;
		};

		struct TypeRemover {
			template <class T>
			void remove() const {
				context.removeExternalType<T>();
			}

			constexpr TypeRemover(Context& context): context(context) {}

		private:
			Context& context;
		};

		template <class TMethodHandler, class TTypeHandler>
		struct ContextHandler {
			TMethodHandler	const methods;
			TTypeHandler	const types;

			constexpr ContextHandler(Context& context): methods(context), types(context) {}
		};

		using Adder		= ContextHandler<MethodAdder, TypeAdder>;
		using Remover	= ContextHandler<MethodRemover, TypeRemover>;

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
			DEBUGLN("Looking for method ", hash, "...");
			for (auto& m: externalMethods)
				DEBUGLN("  > ", m.key);
			if (!(
				hasExternalMethod(hash)
			&&	externalMethods[hash].invoker
			)) return Error::AV2_CCE_MISSING_METHOD;
			return externalMethods[hash].invoker->invoke(*this, externalMethods[hash], args);
		}

		template <class T>
		constexpr Object::Storage newValue(T const& value) const {
			auto const query = types.byNameHash(Meta::arthashof<T>());
			if (query.empty())
				throw Makai::Error::NotFound(
					"Could not find ART analog for the given type!",
					CTL_CPP_PRETTY_SOURCE
				);
			return Object::create(value, query.front());
		}

		struct Library {
			struct Impl;

			~Library();

			Library();

			owner<Impl> impl;
		};

		bool openLibrary(String const& path);

		template <class T>
		bool addExternalType() {
			if (hasExternalType<T>()) return false;
			types.addElement(Meta::implement<T>(types));
			return true;
		}

		template <class T>
		bool hasExternalType() const {
			return externalMethods.contains(Meta::arthashof<T>());
		}

		template <class T>
		void removeExternalType() {
			if (!hasExternalType<T>()) return;
			auto const type = types.queryByNameHash(Meta::arthashof<T>()).front();
			if (type->flags & Definition::Flags::AV2_DF_ART_EQUIVALENT)
				types.values[type->id] = nullptr;
		}

		void loadLibraries();
		void unloadLibraries();

		~Context();

		Database<Definition>			types;
		Database<Method>				methods;
		Map<usize, ExternalMethod>		externalMethods;
		Dictionary<Instance<Library>>	dynlibs;

	private:
		List<Reference<ALibrary>>		toBeLoaded;
	};
}

#endif
