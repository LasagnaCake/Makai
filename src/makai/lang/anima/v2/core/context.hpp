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
		using Arguments = List<Object::Storage>;

		enum class Error {
			AV2_CCE_MISSING_METHOD,
			AV2_CCE_MISSING_INVOKER,
			AV2_CCE_MISSING_ART_TYPE,
			AV2_CCE_MISSING_ARGS,
			AV2_CCE_HOW_DID_YOU_GET_HERE,
		};

		struct ExternalMethod;

		using MethodResult = Result<Object::Storage, Error>;

		using ExternalInvocation = Function<MethodResult(Context& context, ExternalMethod& method, Arguments const& args)>;

		struct ExternalMethodInfo {
			usize 		retTypeHash;
			List<usize>	argTypeHashes;
		};

		struct ExternalMethod: Method {
			ExternalInvocation	invoker;
			usize				argc;

			Nullable<Error> validate(Context& context, Arguments const& args);
		};

		template <class T> struct ExternalMethodResolver;

		static void debugArgs(Arguments const& args);

		template <class TReturn, class... TArgs>
		struct ExternalMethodResolver<TReturn(TArgs...)> {
			constexpr static bool const CONTEXTUAL = Type::OneOf<AsNonVolatile<Makai::Meta::First<TArgs...>>, Context&, Context const&>;

			constexpr static usize const ARG_COUNT = sizeof...(TArgs) + CONTEXTUAL;

			constexpr static bool const HAS_ARGS = ARG_COUNT > 0;

			template <class TFirst, class... TRest>
			constexpr static ExternalMethodInfo makeInfo() requires (!CONTEXTUAL && HAS_ARGS) {
				return {
					Meta::arthashof<TReturn>(),
					List<usize>::from(Meta::arthashof<TFirst>(), Meta::arthashof<TRest>()...)
				};
			}

			template <class TFirst, class... TRest>
			constexpr static ExternalMethodInfo makeInfo() requires (CONTEXTUAL && HAS_ARGS) {
				return {
					Meta::arthashof<TReturn>(),
					List<usize>::from(Meta::arthashof<TRest>()...)
				};
			}

			constexpr static ExternalMethodInfo info() requires (HAS_ARGS) {
				return makeInfo<TArgs...>();
			}

			static auto makeArgumentTuple(Context& context, ExternalMethod& method, Arguments const& args)
			requires (!CONTEXTUAL && HAS_ARGS) {
				return Meta::toArguments<TArgs...>(context.types, args.sliced(0, method.argc));
			}

			static auto makeArgumentTuple(Context& context, ExternalMethod& method, Arguments const& args)
			requires (CONTEXTUAL && HAS_ARGS) {
				return Meta::toArgumentsWithContext<TArgs...>(context.types, args.sliced(0, method.argc), context);
			}

			template <Type::Functional<TReturn(TArgs...)> TFunc>
			[[gnu::noinline]]
			static MethodResult handleInvocation(Context& context, ExternalMethod& method, Arguments const& args, TFunc const& f) {
				CTL_DO_NOT_INLINE;
				DEBUGLN("Invoking function...");
				panic();
				debugArgs(args);
				/*
				if constexpr (HAS_ARGS) {
					DEBUGLN("Function has arguments");
					auto tup = makeArgumentTuple(context, method, args);
					if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
						DEBUGLN("Void function");
						invokeFromTuple<void, TArgs...>(f, tup);
					} else {
						DEBUGLN("Function returns value");
						return Meta::ARTInfo<TReturn>::convert(
							context.types,
							invokeFromTuple<TReturn, TArgs...>(
								f,
								tup
							)
						);
					}
				} else if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
					DEBUGLN("Pure void function");
					static_assert(false);
					f();
				} else {
					DEBUGLN("Getter-like function");
					static_assert(false);
					return Meta::ARTInfo<TReturn>::convert(
						context.types,
						f()
					);
				}
				*/
				return Object::Storage();
			}

			template <Type::Functional<TReturn(TArgs...)> TFunc>
			[[gnu::noinline]]
			static ExternalInvocation invoker(TFunc const& f) {
				return ExternalInvocation {
					[=] (Context& context, ExternalMethod& method, Arguments const& args) -> MethodResult {
						return handleInvocation(context, method, args, f);
					}
				};
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
			using FuncResolver = ExternalMethodResolver<TFunc>;
			if (hasExternalMethod(hash)) return false;
			static auto const baseInfo = FuncResolver::info();
			return addExternalMethod(hash, FuncResolver::ARG_COUNT, FuncResolver::invoker(f));
		}

		bool addExternalMethod(usize const hash, usize const argc, ExternalInvocation const& invoker);

		void removeExternalMethod(usize const& hash) {
			if (!hasExternalMethod(hash)) return;
			externalMethods.erase(hash);
		}

		bool hasExternalMethod(usize const& hash) const {
			return externalMethods.contains(hash) && externalMethods[hash].invoker;
		}

		MethodResult invokeExternalMethod(usize const hash, List<Object::Storage> const& args);

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
			if (type->flags & Definition::Flags::AV2_DF_PROXY)
				types.values[type->id] = nullptr;
		}

		void loadLibraries();
		void unloadLibraries();

		~Context();

		Database<Definition>			types;
		Database<Method>				methods;
		Map<usize, ExternalMethod>		externalMethods;
		Dictionary<Instance<Library>>	dynlibs;

		static Instance<OutputStringWriter> writer;

	private:
		List<Reference<ALibrary>>	toBeLoaded;
	};
}

#endif
