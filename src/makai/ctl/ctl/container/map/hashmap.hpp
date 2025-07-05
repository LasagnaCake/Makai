#ifndef CTL_CONTAINER_MAP_HASHMAP_H
#define CTL_CONTAINER_MAP_HASHMAP_H

#include "treemap.hpp"
#include "../../algorithm/hash.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Associative container comprised of key-value pairs, with hashed keys.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TSize Size type.
/// @param TCompare<class> Comparator type.
/// @param TTree<class, class, class<class>> Tree type.
template<
	class TKey,
	class TValue,
	Type::Integer													THashKey	= usize,
	template <class> class											TCompare	= SimpleComparator,
	class															THasher		= Hasher,
	template <class, class, class, template <class> class> class	TMap		= TreeMap
>
struct HashMap:
Derived<TMap<THashKey, TValue, THashKey, TCompare>>,
SelfIdentified<HashMap<TKey, TValue, THashKey, TCompare, THasher, TMap>>,
Indexed<THashKey>,
private TMap<THashKey, TValue, THashKey, TCompare> {
	using Derived			= ::CTL::Derived<TMap<usize, TValue, THashKey, TCompare>>;
	using SelfIdentified	= ::CTL::SelfIdentified<HashMap<TKey, TValue, THashKey, TCompare, THasher, TMap>>;
	using Indexed			= ::CTL::Indexed<THashKey>;

	using HasherType = THasher;

	using typename Derived::BaseType;

	using typename BaseType::ValueType;

	using KeyType	= TKey;
	using PairType	= KeyValuePair<KeyType, ValueType>;

	using DataType		= KeyValuePair<KeyType const&, ValueType&>;
	using ConstantType	= KeyValuePair<KeyType const&, ValueType const&>;

	using typename SelfIdentified::SelfType;

	using typename Indexed::SizeType;

	using BaseType::begin;
	using BaseType::end;
	using BaseType::rbegin;
	using BaseType::rend;
	using BaseType::front;
	using BaseType::back;
	using BaseType::clear;
	using BaseType::empty;
	using BaseType::allocator;
	using BaseType::size;

	using BaseType::BaseType;

	static_assert(Type::Algorithm::Hashable<TKey, THasher>, "Key type must be a hashable type!");

	constexpr static SizeType hash(KeyType const& key) {
		return HasherType::hash(key);
	}

	/// @brief Empty constructor (defaulted).
	constexpr HashMap()									= default;
	/// @brief Copy constructor (defaulted).
	constexpr HashMap(HashMap const& other)				= default;
	/// @brief Move constructor (defaulted).
	constexpr HashMap(HashMap&& other)					= default;
	/// @brief Copy assignment operator (defaulted).
	constexpr HashMap& operator=(HashMap const& other)	= default;
	/// @brief Move assignment operator (defaulted).
	constexpr HashMap& operator=(HashMap&& other)		= default;

	/// @brief Constructs the container from a list of pairs.
	/// @param values Values to add.
	constexpr HashMap(List<PairType> const& values) {insert(values);}

	/// @brief Constructs the container from a range of key-value pairs.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	template<bool R>
	constexpr HashMap(Iterator<PairType, R> const& begin, Iterator<PairType, R> const& end)	{insert(begin, end);}

	/// @brief Adds a set of key-value pairs to the container.
	/// @param values Pairs to add.
	/// @return Reference to self.
	template <SizeType S>
	constexpr HashMap(As<PairType[S]> const& values) {insert(values);}

	/// @brief Gets the value of the element that matches the given key.
	/// @param key Key to look for.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when key does not exist.
	constexpr ValueType const& at(KeyType const& key) const {
		return BaseType::at(hash(key));
	}

	/// @brief Gets the value of the element that matches the given key.
	/// @param key Key to look for.
	/// @return Value of the element.
	constexpr ValueType& at(KeyType const& key) {
		return BaseType::at(hash(key));
	}

	/// @brief Gets the value of the element that matches the given key.
	/// @param key Key to look for.
	/// @return Value of the element.
	constexpr ValueType& operator[](KeyType const& key)				{return at(key);}

	/// @brief Gets the value of the element that matches the given key.
	/// @param key Key to look for.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when key does not exist.
	constexpr ValueType const& operator[](KeyType const& key) const	{return at(key);}

	/// @brief Inserts a key-value pair into the container, if key does not exist.
	/// @param pair Key-value pair to insert.
	/// @return Reference to self.
	constexpr SelfType& insert(PairType const& pair) {
		SizeType const id = hash(pair.front());
		BaseType::insert({id, pair.back()});
		names.insert({id, pair.front()});
		return *this;
	}

	/// @brief Adds another container's items to this one.
	/// @param other Container to copy from.
	/// @return Reference to self.
	/// @note Existing values are updated.
	constexpr SelfType& append(SelfType const& other) {
		BaseType::append(other);
		names.append(other.names);
		return *this;
	}
	
	/// @brief Adds a set of key-value pairs to the container.
	/// @param values Pairs to add.
	/// @return Reference to self.
	/// @note Only the most recent value for a key is kept.
	constexpr SelfType& insert(List<PairType> const& values) {
		for (auto const& v: values)
			insert(v);
		return *this;
	}

	/// @brief Adds a range of key-value pairs to the container.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @return Reference to self.
	template<bool R>
	constexpr SelfType& insert(Iterator<PairType, R> const& begin, Iterator<PairType, R> const& end) {
		for (auto v : Range::iterate(begin, end))
			insert(v);
		return *this;
	}

	/// @brief Adds a set of key-value pairs to the container.
	/// @param values Pairs to add.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& insert(As<PairType[S]> const& values) {
		for (SizeType i = 0; i < S; ++i)
			insert(values[i]);
		return *this;
	}

	/// @brief Erases an element that matches the given key.
	/// @param key Key to search for.
	/// @return Reference to self.
	constexpr SelfType& erase(KeyType const& key) {
		SizeType const id = hash(key);
		BaseType::erase(id);
		names.erase(id);
		return *this;
	}

	/// @brief Returns whether the container contains a given key.
	/// @param key Key to search for.
	/// @return Whether container has key.
	constexpr bool contains(KeyType const& key) const {return BaseType::find(key);}

	/// @brief Returns all keys in the container.
	/// @return `List` of keys.
	constexpr List<KeyType> keys() const {
		return names.values();
	}

	/// @brief Returns all values in the container.
	/// @return `List` of values.
	constexpr List<ValueType> values() const {
		return BaseType::values();
	}

	/// @brief Returns all key-value pairs in the container.
	/// @return `List` of key-value pairs.
	constexpr List<PairType> items() const {
		List<PairType> result;
		result.reserve(size());
		for (auto i : *this)
			result.pushBack({names[i.front()], i.back()});
		return result;
	}

	/// @brief Erases a given set of keys.
	/// @param keys Keys to remove.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& eraseKeys(As<KeyType[S]> const& keys) {
		for (SizeType i = 0; i < S; ++i)
			erase(keys[i]);
		return *this;
	}

	/// @brief Erases a given list of keys.
	/// @param keys Keys to remove.
	/// @return Reference to self.
	constexpr SelfType& eraseKeys(List<KeyType> const& keys) {
		return eraseKeys(keys.begin(), keys.end());
	}

	/// @brief Erases a given range of keys.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @return Reference to self;
	template<bool R>
	constexpr SelfType& eraseKeys(Iterator<KeyType, R> const& begin, Iterator<KeyType, R> const& end) {
		for (auto v: Range::iterate(begin, end))
			erase(v);
		return *this;
	}
	
	/// @brief Returns all keys that match a given predicate.
	/// @param predicate Predicate to use as check.
	/// @return Matching keys.
	template <Type::Functional<bool(ConstantType const&)> TPredicate>
	constexpr List<KeyType> matchIf(TPredicate const& predicate) {
		List<KeyType> match;
		for (auto v : *this)
			if (predicate({names[v.key], v.value}))
				match.pushBack(v.key);
		return match;
	}
	
	/// @brief Returns all keys that do not match a given predicate.
	/// @param predicate Predicate to use as check.
	/// @return Non-matching keys.
	template <Type::Functional<bool(ConstantType const&)> TPredicate>
	constexpr List<KeyType> matchIfNot(TPredicate const& predicate) {
		List<KeyType> match;
		for (auto v : *this)
			if (!predicate({names[v.key], v.value}))
				match.pushBack(v.key);
		return match;
	}

	/// @brief Erases elements that do match a given predicate.
	/// @param predicate Predicate to use as check.
	/// @return Reference to self.
	template <Type::Functional<bool(ConstantType const&)> TPredicate>
	constexpr SelfType& eraseIf(TPredicate const& predicate) {
		return eraseKeys(matchIf(predicate));
	}

	/// @brief Erases elements that do not match a given predicate.
	/// @param predicate Predicate to use as check.
	/// @return Reference to self.
	template <Type::Functional<bool(ConstantType const&)> TPredicate>
	constexpr SelfType& eraseIfNot(TPredicate const& predicate) {
		return eraseKeys(matchIfNot(predicate));
	}

private:
	TMap<SizeType, KeyType, SizeType, TCompare> names;
};

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Implementation of type constraints.
	namespace Impl {
		template<class T>
		struct IsHashMap;

		template<
			template <
				class,
				class,
				class,
				template <class> class,
				class,
				template <class, class, class, template <class> class> class
			> class T0,
			class T1,
			class T2,
			class T3,
			template <class> class T4,
			class T5,
			template <class, class, class, template <class> class> class T6
		>
		struct IsHashMap<T0<T1, T2, T3, T4, T5, T6>>:
			BooleanConstant<
				Type::Equal<T0<T1, T2, T3, T4, T5, T6>,
				::CTL::HashMap<T1, T2, T3, T4, T5, T6>
			>> {};
	}

	/// @brief Type must be `HashMap`.
	template<class T>
	concept HashMap = Impl::IsHashMap<T>::value;
}

CTL_NAMESPACE_END

#endif