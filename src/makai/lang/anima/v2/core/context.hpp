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

	struct Context;

	template <class T>
	using AsConsumable = Makai::Meta::If<Makai::Type::OneOf<T, Context&>, T, AsNormal<T>>;

	struct Context {
		using Arguments = List<Object::Storage>;

		enum class Error {
			AV2_CCE_MISSING_METHOD,
			AV2_CCE_MISSING_INVOKER,
			AV2_CCE_MISSING_ART_TYPE,
			AV2_CCE_MISSING_ARGS,
			AV2_CCE_HOW_DID_YOU_GET_HERE,
		};

		struct NativeCall;

		using MethodResult = Result<Object::Storage, Error>;

		using ICallable = IConstInvokable<MethodResult(Context&, NativeCall&, Arguments const&)>;

		using ExternalInvocation = owner<ICallable>;

		struct NativeCallInfo {
			usize 		retTypeHash;
			List<usize>	argTypeHashes;
		};

		struct NativeCall: Method {
			ExternalInvocation	invoker;
			usize				argc;

			Nullable<Error> validate(Context& context, Arguments const& args);
		};

		template <class T> struct NativeCallResolver;

		static void debugArgs(Arguments const& args);

		static void debugExternalFunction(bool const isExiting);

		template <class TReturn, class... TArgs>
		struct NativeCallResolver<TReturn(TArgs...)> {
			constexpr static bool const CONTEXTUAL = Type::OneOf<AsNonVolatile<Makai::Meta::First<TArgs...>>, Context&, Context const&>;

			constexpr static usize const ARG_COUNT = sizeof...(TArgs) + CONTEXTUAL;

			constexpr static bool const HAS_ARGS = ARG_COUNT > 0;

			template <class TFirst, class... TRest>
			constexpr static NativeCallInfo makeInfo() requires (!CONTEXTUAL && HAS_ARGS) {
				return {
					Meta::arthashof<TReturn>(),
					List<usize>::from(Meta::arthashof<TFirst>(), Meta::arthashof<TRest>()...)
				};
			}

			template <class TFirst, class... TRest>
			constexpr static NativeCallInfo makeInfo() requires (CONTEXTUAL && HAS_ARGS) {
				return {
					Meta::arthashof<TReturn>(),
					List<usize>::from(Meta::arthashof<TRest>()...)
				};
			}

			constexpr static NativeCallInfo info() requires (HAS_ARGS) {
				return makeInfo<TArgs...>();
			}

			static auto makeArgumentTuple(Context& context, NativeCall& method, Arguments const& args)
			requires (!CONTEXTUAL && HAS_ARGS) {
				return Meta::toArguments<TArgs...>(context.types, args.sliced(0, method.argc));
			}

			static auto makeArgumentTuple(Context& context, NativeCall& method, Arguments const& args)
			requires (CONTEXTUAL && HAS_ARGS) {
				return Meta::toArgumentsWithContext<TArgs...>(context.types, args.sliced(0, method.argc), context);
			}

			template <Type::Functional<TReturn(TArgs...)> TFunc, class... T2>
			static TReturn bridgeCall(TFunc& f, Tuple<T2...>& tup) {
				return bridgeCall(
					f,
					tup,
					IntegerPack<sizeof...(TArgs)>()
				);
			}

			template <Type::Functional<TReturn(TArgs...)> TFunc, usize... N, class... T2>
			static TReturn bridgeCall(TFunc& f, Tuple<T2...>& tup, IndexTuple<N...>) {
				auto bridge = Tuple<TArgs...>{tup.template get<N>()...};
				return invokeFromTuple<TReturn, TArgs...>(
					f,
					bridge
				);
			}

			template <Type::Functional<TReturn(TArgs...)> TFunc>
			[[gnu::noinline]]
			static MethodResult handleInvocation(Context& context, NativeCall& method, Arguments const& args, TFunc& f) {
				CTL_DO_NOT_INLINE;
				MAKAILIB_DEBUGLN_FULL("Invoking function...");
				if constexpr (HAS_ARGS) {
					MAKAILIB_DEBUGLN_FULL("Function has arguments");
					auto tup = makeArgumentTuple(context, method, args);
					if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
						MAKAILIB_DEBUGLN_FULL("Void function");
						bridgeCall(f, tup);
					} else {
						MAKAILIB_DEBUGLN_FULL("Function returns value");
						return Meta::ARTInfo<TReturn>::convert(
							context.types,
							bridgeCall(
								f,
								tup
							)
						);
					}
				} else if constexpr (Type::OneOf<AsNormal<TReturn>, Void, void>) {
					MAKAILIB_DEBUGLN_FULL("Pure void function");
					static_assert(false);
					f();
				} else {
					MAKAILIB_DEBUGLN_FULL("Getter-like function");
					static_assert(false);
					return Meta::ARTInfo<TReturn>::convert(
						context.types,
						f()
					);
				}
				return Object::Storage();
			}

			template <Type::Functional<TReturn(TArgs...)> TFunc>
			struct Invoker: ICallable {
				TFunc& f;

				virtual ~Invoker() {debugExternalFunction(true);}

				template <Type::Functional<TReturn(TArgs...)> T>
				Invoker(T& f): f(f) {debugExternalFunction(false);}

				MethodResult invoke(Context& context, NativeCall& method, Arguments const& args) const override {
					return handleInvocation(context, method, args, f);
				}
			};

			template <Type::Functional<TReturn(TArgs...)> TFunc>
			[[gnu::noinline]]
			static ExternalInvocation invoker(TFunc& f) {
				return new Invoker<TFunc>(f);
			}
		};

		struct MethodAdder {
			template <class TFunc>
			bool add(String const& name, TFunc& f) const {
				auto const hash = ConstHasher::hash(name);
				using FuncResolver = NativeCallResolver<TFunc>;
				if (context.hasNativeCall(hash)) return false;
				return add(hash, FuncResolver::ARG_COUNT, FuncResolver::invoker(f));
			}

			constexpr MethodAdder(Context& context): context(context) {}

			virtual ~MethodAdder();

		private:
			virtual bool add(usize const hash, usize const argc, ExternalInvocation const& invoker) const;

			Context& context;
		};

		struct MethodRemover {
			void remove(String const& name) const {
				context.removeNativeCall(name);
			}

			virtual ~MethodRemover();

			constexpr MethodRemover(Context& context): context(context) {}

		private:
			Context& context;
		};

		struct TypeAdder {
			template <class T>
			bool add() const {
				return context.addNativeType<T>();
			}

			virtual ~TypeAdder();

			constexpr TypeAdder(Context& context): context(context) {}

		private:
			Context& context;
		};

		struct TypeRemover {
			template <class T>
			void remove() const {
				context.removeNativeType<T>();
			}

			virtual ~TypeRemover();

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
		bool addNativeCall(String const& name, TFunc const& f) {
			return addNativeCall(ConstHasher::hash(name), f);
		}

		void removeNativeCall(String const& name) {
			removeNativeCall(ConstHasher::hash(name));
		}

		bool hasNativeCall(String const& name) const {
			return hasNativeCall(ConstHasher::hash(name));
		}

		Result<Object::Storage, Error> invokeNativeCall(
			String const& name,
			List<Object::Storage> const& args
		) {
			return invokeNativeCall(ConstHasher::hash(name), args);
		}

		template <class TFunc>
		bool addNativeCall(usize const& hash, TFunc& f) {
			using FuncResolver = NativeCallResolver<TFunc>;
			if (hasNativeCall(hash)) return false;
			return addNativeCall(hash, FuncResolver::ARG_COUNT, FuncResolver::invoker(f));
		}

		bool addNativeCall(usize const hash, usize const argc, ExternalInvocation const& invoker);

		void removeNativeCall(usize const& hash) {
			if (!hasNativeCall(hash)) return;
			auto em = externalMethods[hash];
			externalMethods.erase(hash);
			loadedMethods.eraseLike(em);
		}

		bool hasNativeCall(usize const& hash) const {
			return
				externalMethods.contains(hash)
			&&	externalMethods[hash]
			&&	externalMethods[hash]->invoker
			;
		}

		usize argumentCountOf(usize const& hash) const {
			return hasNativeCall(hash) ? externalMethods[hash]->argc : 0;
		}

		MethodResult callNative(usize const hash, List<Object::Storage> const& args);

		template <class T>
		Object::Storage newValue(T const& value) const {
			auto const query = types.byNameHash(Meta::arthashof<T>());
			if (query.empty() or !query.front())
				throw Makai::Error::NotFound(
					"Could not find ART analog for the given type!",
					CTL_CPP_PRETTY_SOURCE
				);
			MAKAILIB_DEBUGLN_FULL("Selected Type: ", query.front()->hash);
			return Object::create(value, query.front());
		}

		template <class T>
		Object::Storage newEmpty() const {
			auto const query = types.byNameHash(Meta::arthashof<T>());
			if (query.empty() or !query.front())
				throw Makai::Error::NotFound(
					"Could not find ART analog for the given type!",
					CTL_CPP_PRETTY_SOURCE
				);
			MAKAILIB_DEBUGLN_FULL("Selected Type: ", query.front()->hash);
			return Object::create(query.front());
		}

		struct Library {
			struct Impl;

			~Library();

			Library();

			owner<Impl> impl;
		};

		bool openLibrary(String const& path);

		template <class T>
		bool addNativeType() {
			if (hasNativeType<T>()) return false;
			types.addElement(Meta::implement<T>(types));
			return true;
		}

		template <class T>
		bool hasNativeType() const {
			return types.contains(Meta::arthashof<T>());
		}

		template <class T>
		void removeNativeType() {
			if (!hasNativeType<T>()) return;
			auto const type = types.queryByNameHash(Meta::arthashof<T>()).front();
			if (type->flags.isProxy)
				types.values[type->id] = nullptr;
		}

		void loadLibraries();
		void unloadLibraries();

		~Context();

		Database<Definition>				types;
		Database<Method>					methods;
		Map<usize, Instance<NativeCall>>	externalMethods;
		Dictionary<Instance<Library>>		dynlibs;

		static Instance<OutputStringWriter> writer;

	private:
		List<Instance<NativeCall>>	loadedMethods;
		List<Instance<Library>>			loadedLibraries;
		List<Reference<ALibrary>>		toBeLoaded;
	};
}

#endif
