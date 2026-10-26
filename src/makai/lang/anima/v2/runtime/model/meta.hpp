#ifndef MAKAILIB_ANIMA_V2_CORE_META_H
#define MAKAILIB_ANIMA_V2_CORE_META_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Core::Meta {
	namespace Impl {
		template <class T>
		concept ARTCompatible = requires {
			requires ProxyType<T>;
			{T::wrap} -> Makai::Type::Functional<
				Makai::Meta::If<
					Makai::Type::Void<T>,
					Object(Database<TypeModel>&),
					Object(Database<TypeModel>&, T const&)
					>
				>
			;
		};

		template <class T>
		consteval static auto specialHashOf() {
			if constexpr (Type::Equal<T, bool>)		return ConstHasher::hash("bool");
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

		template <class T>
		struct AImplementor {
			Database<TypeModel>& types;

			virtual ~AImplementor() {}

			AImplementor(Database<TypeModel>& types): types(types) {}

			constexpr virtual usize hash() const = 0;
			virtual T unwrap(Object const& obj)	= 0;
			virtual Object wrap(T const& value)	= 0;
		};

		template <class T> struct Implementor;

		template<> struct Implementor<Any> {
			constexpr usize hash() const override			{return Makai::hash("any");			}
			static Any unwrap(Object const& obj) override	{return {obj.clone()};				}
			static Storage wrap(Any const& value) override	{return value.value.shallowClone();	}
		};

		template<> struct Implementor<Data::Value> {
			constexpr usize hash() const override					{return Makai::hash("any");											}
			static Any unwrap(Object const& obj) override			{return obj.serialize();											}
			static Storage wrap(Data::Value const& value) override	{return Object::create(value, types.byNameHash(hash()).front());	}
		};

		template<> struct Implementor<nulltype> {
			constexpr usize hash() const override				{return Makai::hash("nil");	}
			static nulltype unwrap(Object const& obj) override	{return null;				}
			static Storage wrap(nulltype const& value) override	{return Store::create();	}
		};

		template <Type::Number T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return specialHashOf<T>();										}
			static T unwrap(Object const& obj) override		{return value.template to<T>();									}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Type::OneOf<char, UTF8Char, UTF32Char> T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return Makai::hash("char");								}
			static T unwrap(Object const& obj) override		{return value.template to<T>();								}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Type::OneOf<Bytes<>> T>
		struct Implementor: AImplementor<Bytes<>> {
			constexpr usize hash() const override			{return Makai::hash("bin");									}
			static T unwrap(Object const& obj) override		{return value.template to<T>();								}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Type::OneOf<String, UTF8String, UTF32String> T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return Makai::hash("str");										}
			static T unwrap(Object const& obj) override		{return value.template to<T>();									}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Makai::Type::OneOf<Vector2, Vector3, Vector4> T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return Makai::hash("vec");										}
			static T unwrap(Object const& obj) override		{return value.template to<T>();									}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Makai::Type::OneOf<Matrix4x4> T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return Makai::hash("mat");										}
			static T unwrap(Object const& obj) override		{return value.template to<T>();									}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Makai::Type::OneOf<TypeID> T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return Makai::hash("type");									}
			static T unwrap(Object const& obj) override		{return value.template to<T>();									}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Makai::Type::OneOf<JumpID> T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return Makai::hash("jump");									}
			static T unwrap(Object const& obj) override		{return value.template to<T>();									}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<Makai::Type::OneOf<CallID> T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return Makai::hash("call");									}
			static T unwrap(Object const& obj) override		{return value.template to<T>();									}
			static Storage wrap(T const& value) override	{return Object::create(value, types.byName(hash()).front());	}
		};

		template<ARTType T>
		struct Implementor: AImplementor<T> {
			constexpr usize hash() const override			{return T::ART_HASH;		}
			static T unwrap(Object const& obj) override		{return T::fromART(obj);	}
			static Storage wrap(T const& value) override	{return T::toART(obj);		}
		};

		template<class T>
		struct Implementor: AImplementor<List<T>> {
			constexpr usize hash() const override {
				return 0;
			}

			List<T> unwrap(Object const& obj) override {
				List<T> result;
				result.resize(value.count());
				for (usize i: range(value.count())) {
					auto const v = value.getField(i);
					if (!v) return nullptr;
					result.pushBack(Implementor<T>(types).unwrap(*v.value()));
				}
				return result;
			}

			Storage wrap(List<T> const& value) override {
				Database<TypeModel>::ElementType type;
				for (auto const& t: types.values) {
					if (!t->base) continue;
					if (!t->flags.isArray) continue;
					if (t->base->hash != Implementor<T>(types).hash()) continue;
				}
				Object::Storage obj = Object::create(type);
				obj->reserveFields(value.size());
				for (usize i: range(value.size()))
					obj->setField(i, Implementor<T>(types).wrap(value[i]));
				return obj;
			}
		};

		template<class T>
		struct Implementor: AImplementor<Nullable<T>> {
			constexpr usize hash() const override {
				return 0;
			}

			List<T> unwrap(Object const& obj) override {
				if (!value.exists()) return null;
				return value.template to<T>();
			}

			Storage wrap(Nullable<T> const& value) override {
				Database<TypeModel>::ElementType type;
				for (auto const& t: types.values) {
					if (!t->base) continue;
					if (!t->flags.isNullable) continue;
					if (t->base->hash != Implementor<T>(types).hash()) continue;
				}
				Object::Storage obj = Object::create(type);
				if (!value) return obj;
				obj->set(value.value());
				return obj;
			}
		};

		template<class T>
		struct Implementor: AImplementor<Promise<T>> {
			constexpr usize hash() const override {
				return 0;
			}

			Promise<T> unwrap(Object const& obj) override {
				return {value};
			}

			Storage wrap(Promise<T> const& value) override {
				return value.object;
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
				return {list.at(N)...};
			}
		};

		template <class... Types>
		struct ObjectTupleToArguments {
			using ObjectTupleType = typename ToObjectTuple<Types...>::Type;
			using Type = Tuple<AsNormal<Types>...>;

			constexpr static Type make(ObjectTupleType const& tup) {
				return make(types, tup, IntegerPack<sizeof...(Types)>());
			}

			template <usize... N>
			constexpr static Type make(ObjectTupleType const& tup, IndexTuple<N...>) {
				return {ARTTI<Makai::Meta::Select<N, AsNormal<Types>...>>::construct(*tup.template get<N>())...};
			}

			template <class TContext>
			constexpr static Tuple<TContext&, AsNormal<Types>...> makeWithContext(TContext& context, ObjectTupleType const& tup) {
				return makeWithContext(context, types, tup, IntegerPack<sizeof...(Types)>());
			}

			template <class TContext, usize... N>
			constexpr static Tuple<TContext&, AsNormal<Types>...> makeWithContext(TContext& context, ObjectTupleType const& tup, IndexTuple<N...>) {
				return {context, ARTTI<Makai::Meta::Select<N, AsNormal<Types>...>>::construct(*tup.template get<N>())...};
			}
		};
	};

	template <class T>
	using Implementor = Impl::Implementor<T>;

	template <class... Types>
	constexpr bool fits(List<Object::Storage> const& args) {
		return Impl::ListToTuple<Types...>::fits();
	}

	template <class... Types>
	constexpr auto toArguments(List<Object::Storage> const& args) {
		return Impl::ObjectTupleToArguments<Types...>::make(
			types,
			Impl::ListToTuple<Types...>::make(args)
		);
	}

	template <class TContext, class... Types>
	constexpr auto toArgumentsWithContext(List<Object::Storage> const& args, TContext& context) {
		return Impl::ObjectTupleToArguments<Types...>::makeWithContext(
			context,
			types,
			Impl::ListToTuple<Types...>::make(args)
		);
	}

	namespace Impl {
		template <class T>
		concept TriviallyImplementable = (
			Type::Class<T>
		&&	Type::Pure<T>
		&&	Type::DefaultConstructible<T>
		);
		template <TriviallyImplementable T>
		class EasyImplementor {
			constexpr static bool const IS_ART_TYPE			= ProxyType<T>;
			constexpr static bool const HAS_CONSTRUCTOR		= Type::DefaultConstructible<T>;
			constexpr static bool const HAS_DESTRUCTOR		= requires {T::~T();};
			constexpr static bool const HAS_CLONER			= Type::CopyAssignable<T>;
			constexpr static bool const HAS_COMPARATOR		= requires (T const a, T const b) {{a <=> b} -> Type::Convertible<StandardOrder>;};
			constexpr static bool const HAS_EXPLICIT_BASE	= requires {typename T::BaseType;};

			constexpr static auto const TYPE_HASH = IS_ART_TYPE ? T::ART_HASH : ConstHasher::hash(nameof<T>());

			static_assert(HAS_CONSTRUCTOR && HAS_DESTRUCTOR, "Type must have a constructor and a destructor!");

			static TypeModel::ProxyTypeImpl::Constructor constructor()
			requires (HAS_CONSTRUCTOR) {
				return {
					[] (auto e) {
						MX::construct<T>(Cast::rewrite<ref<T>>(e));
					}
				};
			}

			static TypeModel::ProxyTypeImpl::Destructor destructor()
			requires (HAS_DESTRUCTOR) {
				return {
					[] (auto e) {
						MX::destruct<T>(Cast::rewrite<ref<T>>(e));
					}
				};
			}

			static TypeModel::ProxyTypeImpl::Cloner cloner()
			requires (HAS_CLONER) {
				return {
					[] (auto a, auto b) {
						violate<T>(a) = violate<T>(b);
					}
				};
			}

			static TypeModel::ProxyTypeImpl::Comparator comparator()
			requires (HAS_COMPARATOR) {
				return {
					[] (auto a, auto b) {
						return enumcast<StandardOrder>(violate<T>(a) <=> violate<T>(b));
					}
				};
			}

			static void implement(TypeModel& type, Database<TypeModel>& types) {
				type.proxy = type.proxy.create({
					constructor(),
					destructor()
				});
				type.construct = constructor();
				type.destruct = destructor();
				type.id = types.values.size();
				type.name = nameof<T>();
				if  constexpr (HAS_CLONER) {
					type.flags.isCopyable = true;
					type.proxy->copy = cloner();
				}
				if constexpr (HAS_COMPARATOR)
					type.proxy->compare = comparator();
				if constexpr (IS_ART_TYPE)
					type.hash = Implementor<T>(types).hash();
				else type.hash = Makai::hash(type.name);
				type.alignment	= alignof (T);
				type.byteSize	= sizeof (T);
				if constexpr (HAS_EXPLICIT_BASE)
					type.base = types.byNameHash(EasyImplementor<typename T::BaseType>::TYPE_HASH).front();
				type.flags.isProxy = true;
			}
		};
	}

	template <class T>
	constexpr Instance<TypeModel> implement(Database<TypeModel>& types) {
		Instance<TypeModel> type = type.create();
		Impl::EasyImplementor<T>::implement(*type, types);
		return type;
	}
}

#endif
