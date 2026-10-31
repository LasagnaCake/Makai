#ifndef MAKAILIB_ANIMA_V2_CORE_META_H
#define MAKAILIB_ANIMA_V2_CORE_META_H

#include "type.hpp"
#include "method.hpp"
#include "object.hpp"
#include "database.hpp"

namespace Makai::Anima::V2::Runtime::Meta {
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

		struct BaseImplementor {
			Database<TypeModel>& types;

			virtual ~BaseImplementor() {}

			BaseImplementor(Database<TypeModel>& types): types(types) {}
		};

		template <class T> struct Implementor;

		template <class T>
		concept IsImplementor = (
			requires (Object& obj, T& impl) {
				requires Type::Subclass<T, BaseImplementor>;
				requires Type::Constructible<T, BaseImplementor, Database<TypeModel>&>;
				typename T::DataType;
				{T::hash()}								-> Type::Equal<usize>;
				{T::unwrap(obj)}						-> Type::Equal<Object>;
				{impl.wrap(As<typename T::DataType>())}	-> Type::Equal<typename T::DataType>;
			}
		);

		template<> struct Implementor<Any>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("any");				}
			static Any unwrap(Object const& obj)	{return {obj.clone()};					}
			Storage wrap(Any const& value) const	{return value.value->shallowClone();	}
		};

		template<> struct Implementor<Data::Value>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 					{return Makai::hash("any");											}
			static Data::Value unwrap(Object const& obj)	{return obj.serialize();											}
			Storage wrap(Data::Value const& value) const	{return Object::create(value, types.byNameHash(hash()).front());	}
		};

		template<> struct Implementor<nulltype>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 				{return Makai::hash("nil");	}
			static nulltype unwrap(Object const& obj)	{return null;				}
			Storage wrap(nulltype const& value) const	{return Storage::create();	}
		};

		template <Type::Number T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return specialHashOf<T>();										}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Type::OneOf<char, UTF8Char, UTF32Char> T>
		struct Implementor<T>: BaseImplementor{
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("char");									}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Type::OneOf<Bytes<>> T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("bin");										}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Type::OneOf<String, UTF8String, UTF32String> T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("str");										}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Makai::Type::OneOf<Vector2, Vector3, Vector4> T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("vec");										}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Makai::Type::OneOf<Matrix4x4> T>
		struct Implementor<T>: BaseImplementor {
			constexpr static usize hash() 			{return Makai::hash("mat");										}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Makai::Type::OneOf<TypeID> T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("type");									}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Makai::Type::OneOf<JumpID> T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("jump");									}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<Makai::Type::OneOf<CallID> T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return Makai::hash("call");									}
			static T unwrap(Object const& obj)		{return obj.template to<T>();									}
			Storage wrap(T const& obj) const		{return Object::create(obj, types.byNameHash(hash()).front());	}
		};

		template<ProxyType T>
		struct Implementor<T>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() 			{return T::ART_HASH;			}
			static T unwrap(Object const& obj)		{return T::fromART(obj);		}
			Storage wrap(T const& obj) const		{return T::toART(types, obj);	}
		};

		template<class T>
		struct Implementor<List<T>>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() {
				return 0;
			}

			static List<T> unwrap(Object const& obj) {
				List<T> result;
				result.resize(obj.count());
				for (usize i: range(obj.count())) {
					auto const v = obj.getField(i);
					if (!v) return nullptr;
					result.pushBack(Implementor<T>::unwrap(*v.value()));
				}
				return result;
			}

			Storage wrap(List<T> const& value) {
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
		struct Implementor<Nullable<T>>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() {
				return 0;
			}

			static Nullable<T> unwrap(Object const& obj) {
				if (!obj.exists()) return null;
				return obj.template to<T>();
			}

			Storage wrap(Nullable<T> const& value) {
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
		struct Implementor<Promise<T>>: BaseImplementor {
			constexpr static usize hash() {
				return 0;
			}

			static Promise<T> unwrap(Object const& obj) {
				return {obj};
			}

			Storage wrap(Promise<T> const& value) {
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

			constexpr static Type make(Database<TypeModel>& types, ObjectTupleType const& tup) {
				return make(types, tup, IntegerPack<sizeof...(Types)>());
			}

			template <usize... N>
			constexpr static Type make(Database<TypeModel>& types, ObjectTupleType const& tup, IndexTuple<N...>) {
				return {Implementor<Makai::Meta::Select<N, AsNormal<Types>...>>::unwrap(*tup.template get<N>())...};
			}

			template <class TContext>
			constexpr static Tuple<TContext&, AsNormal<Types>...> makeWithContext(Database<TypeModel>& types, TContext& context, ObjectTupleType const& tup) {
				return makeWithContext(context, types, tup, IntegerPack<sizeof...(Types)>());
			}

			template <class TContext, usize... N>
			constexpr static Tuple<TContext&, AsNormal<Types>...> makeWithContext(Database<TypeModel>& types, TContext& context, ObjectTupleType const& tup, IndexTuple<N...>) {
				return {context, Implementor<Makai::Meta::Select<N, AsNormal<Types>...>>::unwrap(*tup.template get<N>())...};
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
	constexpr auto toArguments(Database<TypeModel>& types, List<Object::Storage> const& args) {
		return Impl::ObjectTupleToArguments<Types...>::make(
			types,
			Impl::ListToTuple<Types...>::make(args)
		);
	}

	template <class TContext, class... Types>
	constexpr auto toArgumentsWithContext(Database<TypeModel>& types, List<Object::Storage> const& args, TContext& context) {
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
		&&	Type::DefaultConstructible<T>
		&&	Type::CopyConstructible<T>
		&&	Type::CopyAssignable<T>
		);

		template <TriviallyImplementable T>
		class EasyImplementor {
			constexpr static bool const IS_ART_TYPE			= ProxyType<T>;
			constexpr static bool const HAS_CONSTRUCTOR		= Type::DefaultConstructible<T>;
			constexpr static bool const HAS_DESTRUCTOR		= requires {T::~T();};
			constexpr static bool const HAS_COMPARATOR		= requires (T const a, T const b) {{a <=> b} -> Type::Convertible<StandardOrder>;};
			constexpr static bool const HAS_EXPLICIT_BASE	= requires {typename T::BaseType;};

			constexpr static auto const TYPE_HASH = IS_ART_TYPE ? T::ART_HASH : ConstHasher::hash(nameof<T>());

			static_assert(HAS_CONSTRUCTOR && HAS_DESTRUCTOR, "Type must have a constructor and a destructor!");

			static TypeModel::Proxy::Constructor constructor()
			requires (HAS_CONSTRUCTOR) {
				return {
					[] (auto e) {
						MX::construct<T>(Cast::rewrite<ref<T>>(e));
					}
				};
			}

			static TypeModel::Proxy::Destructor destructor()
			requires (HAS_DESTRUCTOR) {
				return {
					[] (auto e) {
						MX::destruct<T>(Cast::rewrite<ref<T>>(e));
					}
				};
			}

			static TypeModel::Proxy::Cloner cloner() {
				return {
					[] (auto a, auto b) {
						violate<T>(a) = violate<T>(b);
					}
				};
			}

			static TypeModel::Proxy::Comparator comparator()
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
				type.proxy->construct = constructor();
				type.proxy->destruct = destructor();
				type.id = types.values.size();
				type.name = nameof<T>();
				type.flags.isCopyable = true;
				type.proxy->copy = cloner();
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

	template <Impl::TriviallyImplementable T>
	requires (!ProxyType<T>)
	struct Proxy: Self<T> {
		constexpr static usize ART_HASH = hash(nameof<Proxy<T>>());

		using T::T;
		using T::operator=;

		static T fromART(Object const& object) {
			return *(ref<T const>)object.data();
		}

		static Storage toART(Proxy<T> const& object, Database<TypeModel>& types) {
			return Object::create(object, types.byNameHash(ART_HASH).front());
		}
	};

	namespace Impl {
		template <class T>
		struct Implementor<Proxy<T>>: BaseImplementor {
			using BaseImplementor::BaseImplementor;

			constexpr static usize hash() {
				return Proxy<T>::ART_HASH;
			}

			static T unwrap(Object const& obj) {
				return Proxy<T>::fromART(obj);
			}

			Storage wrap(T const& value) {
				return Proxy<T>::toART(value, types);
			}
		};
	}

	template <class T>
	constexpr Instance<TypeModel> implement(Database<TypeModel>& types) {
		Instance<TypeModel> type = type.create();
		Impl::EasyImplementor<T>::implement(*type, types);
		return type;
	}

	template <class T> constexpr usize arthashof() {return Meta::Implementor<T>::hash();}
}

#endif
