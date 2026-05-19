#ifndef MAKAILIB_ANIMA_V2_CORE_META_H
#define MAKAILIB_ANIMA_V2_CORE_META_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core::Meta {
	namespace Impl {
		template <class T>
		concept ValidType = requires {
			requires ARTType<T>;
			{T::convert} -> Makai::Type::Functional<
				Makai::Meta::If<
					Makai::Type::Void<T>,
					Object(Database<Definition>&),
					Object(Database<Definition>&, T const&)
					>
				>
			;
		};

		template <class T>
		consteval static auto specialHashOf() {
			if constexpr (Type::Equal<T, int8>)		return ConstHasher::hash("i8");
			if constexpr (Type::Equal<T, int16>)	return ConstHasher::hash("i16");
			if constexpr (Type::Equal<T, int32>)	return ConstHasher::hash("i32");
			if constexpr (Type::Equal<T, int64>)	return ConstHasher::hash("i64");
			if constexpr (Type::Equal<T, uint8>)	return ConstHasher::hash("u8");
			if constexpr (Type::Equal<T, uint16>)	return ConstHasher::hash("u16");
			if constexpr (Type::Equal<T, uint32>)	return ConstHasher::hash("u32");
			if constexpr (Type::Equal<T, uint64>)	return ConstHasher::hash("u64");
			if constexpr (Type::Equal<T, float32>)	return ConstHasher::hash("f32");
			if constexpr (Type::Equal<T, float64>)	return ConstHasher::hash("f64");
			if constexpr (Type::Equal<T, float128>)	return ConstHasher::hash("f128");
		}

		template <class T> struct ARTTI;

		template<> struct ARTTI<Void> {
			constexpr static auto const ART_HASH = ConstHasher::hash("void");

			static void construct(Object const&) {}

			static Object::Storage convert(Database<Definition>& db) {
				return Object::create(db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<void> {
			constexpr static auto const ART_HASH = ConstHasher::hash("void");

			static void construct(Object const&) {}

			static Object::Storage convert(Database<Definition>& db) {
				return Object::create(db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Any> {
			constexpr static auto const ART_HASH = ConstHasher::hash("any");

			static Any construct(Object const& obj) {
				return {obj.clone()};
			}

			static Object::Storage convert(Database<Definition>& db, Any const& value) {
				return Object::create(value.value, db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<nulltype> {
			constexpr static auto const ART_HASH = ConstHasher::hash("nil");

			static nulltype construct(Object const&) {
				return {};
			}

			static Object::Storage convert(Database<Definition>& db, nulltype const&) {
				return Object::create(db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<bool> {
			constexpr static auto const ART_HASH = ConstHasher::hash("bool");

			static bool construct(Object const& value) {
				return value.toValue<bool>();
			}

			static Object::Storage convert(Database<Definition>& db, bool const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Type::OneOf<char, UTF8Char, UTF32Char> T>
		struct ARTTI<T> {
			constexpr static auto const ART_HASH = ConstHasher::hash("char");

			static bool construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::SignedInteger T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = specialHashOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::UnsignedInteger T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = specialHashOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::Real T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = specialHashOf<T>();

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Binary<>> {
			constexpr static auto const ART_HASH = ConstHasher::hash("bin");

			static Binary<> construct(Object const& value) {
				return value.toValue<Binary<>>();
			}

			static Object::Storage convert(Database<Definition>& db, Binary<> const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::OneOf<String, UTF8String, UTF32String> T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = ConstHasher::hash("str");

			static T construct(Object const& value) {
				return value.toValue<T>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<Makai::Type::OneOf<Vector2, Vector3, Vector4> T> struct ARTTI<T> {
			constexpr static auto const ART_HASH = ConstHasher::hash("vec");

			static T construct(Object const& value) {
				return value.toValue<Vector4>();
			}

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(Vector4(value), db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Matrix4x4> {
			constexpr static auto const ART_HASH = ConstHasher::hash("mat");

			static Matrix4x4 construct(Object const& value) {
				return value.toValue<Matrix4x4>();
			}

			static Object::Storage convert(Database<Definition>& db, Matrix4x4 const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<TypeID> {
			constexpr static auto const ART_HASH = ConstHasher::hash("type");

			static TypeID construct(Object const& value) {
				return value.toValue<TypeID>();
			}

			static Object::Storage convert(Database<Definition>& db, TypeID const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<> struct ARTTI<Data::Value> {
			constexpr static auto const ART_HASH = ConstHasher::hash("any");

			static Data::Value construct(Object const& value) {
				return value.toValue<Data::Value>();
			}

			static Object::Storage convert(Database<Definition>& db, TypeID const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<ARTType T>
		struct ARTTI<T>: T {
			constexpr static auto const ART_HASH = ConstHasher::hash(T::ART_NAME);

			static Object::Storage convert(Database<Definition>& db, T const& value) {
				return Object::create(value, db.byNameHash(ART_HASH).front());
			}
		};

		template<class T>
		struct ARTTI<List<T>>: List<T> {
			inline static auto const ART_HASH = ConstHasher::hash(toString(nameof<T>(), "_array"));

			static List<T> construct(Object const& value) {
				List<T> result;
				result.resize(value.count());
				for (usize i: range(value.count()))
					result.pushBack(ARTTI<T>::construct(value.getAtIndex(i)));
				return result;
			}

			static Object::Storage convert(Database<Definition>& db, List<T> const& value) {
				Object::Storage obj = Object::create(db.byNameHash(ART_HASH).front());
				for (usize i: range(value.size()))
					obj->setAtIndex(i, Object::create(value[i], db.byNameHash(ARTTI<T>::ART_HASH).front()));
				return obj;
			}
		};

		template <class... Types>
		struct ToObjectTuple {
			using Type = SingleTypeTuple<Object::Storage, Types...>;
		};

		template <class... Types>
		struct ListToTuple {
			using Type = typename ToObjectTuple<Types...>::Type;
			using ListType = List<Object::Storage>;

			constexpr static bool fits(ListType const& list) {
				return list.size() >= sizeof...(Types);
			}

			constexpr static Type make(ListType const& list) {
				return make(list, IntegerPack<sizeof...(Types)>());
			}

			template <usize... N>
			constexpr static Type make(ListType const& list, IndexTuple<N...>) {
				return {list[N]...};
			}
		};

		template <class... Types>
		struct ObjectTupleToArguments {
			using ObjectTupleType = typename ToObjectTuple<Types...>::Type;
			using Type = Tuple<Types...>;

			constexpr static Type make(Database<Definition>& db, ObjectTupleType const& tup) {
				return make(db, tup, IntegerPack<sizeof...(Types)>());
			}

			template <usize... N>
			constexpr static Type make(Database<Definition>& db, ObjectTupleType const& tup, IndexTuple<N...>) {
				return {ARTTI<Makai::Meta::Select<N, Types...>>::construct(*tup.template get<N>())...};
			}

			template <class TContext>
			constexpr static Tuple<TContext&, Types...> makeWithContext(TContext& context, Database<Definition>& db, ObjectTupleType const& tup) {
				return makeWithContext(context, db, tup, IntegerPack<sizeof...(Types)>());
			}

			template <class TContext, usize... N>
			constexpr static Tuple<TContext&, Types...> makeWithContext(TContext& context, Database<Definition>& db, ObjectTupleType const& tup, IndexTuple<N...>) {
				return {context, ARTTI<Makai::Meta::Select<N, Types...>>::construct(*tup.template get<N>())...};
			}
		};
	};

	template <class T>
	using ARTInfo = Impl::ARTTI<T>;

	template <class T>
	constexpr usize arthashof() {
		return ARTInfo<T>::ART_HASH;
	}

	template <class... Types>
	constexpr bool fits(List<Object::Storage> const& args) {
		return Impl::ListToTuple<Types...>::fits();
	}

	template <class... Types>
	constexpr Tuple<Types...> toArguments(Database<Definition>& db, List<Object::Storage> const& args) {
		return Impl::ObjectTupleToArguments<Types...>::make(
			db,
			Impl::ListToTuple<Types...>::make(args)
		);
	}

	template <class TContext, class... Types>
	constexpr auto toArgumentsWithContext(Database<Definition>& db, List<Object::Storage> const& args, TContext& context) {
		return Impl::ObjectTupleToArguments<Types...>::makeWithContext(
			context,
			db,
			Impl::ListToTuple<Types...>::make(args)
		);
	}

	namespace Impl {
		template <Type::Class T>
		class EasyImplementor {
			constexpr static bool const IS_ART_TYPE			= ARTType<T>;
			constexpr static bool const HAS_CONSTRUCTOR		= Type::Constructible<T>;
			constexpr static bool const HAS_DESTRUCTOR		= requires {T::~T();};
			constexpr static bool const HAS_CLONER			= Type::CopyAssignable<T>;
			constexpr static bool const HAS_COMPARATOR		= requires (T const a, T const b) {{a <=> b} -> Type::Convertible<StandardOrder>;};
			constexpr static bool const HAS_EXPLICIT_BASE	= requires {typename T::BaseType;};

			constexpr static auto const TYPE_HASH = IS_ART_TYPE ? T::ART_HASH : ConstHasher::hash(nameof<T>());

			static_assert(HAS_CONSTRUCTOR && HAS_DESTRUCTOR, "Type must have a constructor and a destructor!");

			static Definition::Constructor constructor()
			requires (HAS_CONSTRUCTOR) {
				return {
					[] (auto e) {
						MX::construct<T>(Cast::rewrite<ref<T>>(e));
					}
				};
			}

			static Definition::Destructor destructor()
			requires (HAS_DESTRUCTOR) {
				return {
					[] (auto e) {
						MX::destruct<T>(Cast::rewrite<ref<T>>(e));
					}
				};
			}

			static Definition::Cloner cloner()
			requires (HAS_CLONER) {
				return {
					[] (auto a, auto b) {
						violate<T>(a) = violate<T>(b);
					}
				};
			}

			static Definition::Comparator comparator()
			requires (HAS_COMPARATOR) {
				return {
					[] (auto a, auto b) {
						return enumcast<StandardOrder>(violate<T>(a) <=> violate<T>(b));
					}
				};
			}

			static void implement(Definition& type, Database<Definition>& db) {
				type.construct = constructor();
				type.destruct = destructor();
				type.id = db.values.size();
				type.name = nameof<T>();
				if  constexpr (HAS_CLONER) {
					type.flags |= Definition::Flags::AV2_DF_CLONABLE;
					type.copy = cloner();
				}
				if constexpr (HAS_COMPARATOR)
					type.compare = comparator();
				if constexpr (IS_ART_TYPE)
					type.hash = arthashof<T>();
				else type.hash = ConstHasher::hash(type.name);
				type.alignment	= alignof (T);
				type.byteSize	= sizeof (T);
				if constexpr (HAS_EXPLICIT_BASE)
					type.base = db.byNameHash(EasyImplementor<typename T::BaseType>::TYPE_HASH).front();
				type.flags |= Definition::Flags::AV2_DF_ART_EQUIVALENT;
			}
		};
	}

	template <class T>
	constexpr Instance<Definition> implement(Database<Definition>& db) {
		Instance<Definition> type = type.create();
		Impl::EasyImplementor<T>::implement(*type, db);
		return type;
	}
}

#endif
