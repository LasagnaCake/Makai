#ifndef CTL_CONTAINER_MAP_LISTMAP_H
#define CTL_CONTAINER_MAP_LISTMAP_H

#include "../arguments.hpp"
#include "../lists/list.hpp"
#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../typetraits/traits.hpp"
#include "../../algorithm/hash.hpp"
#include "../pair.hpp"

CTL_NAMESPACE_BEGIN

#define NOT_IN_MAP [this](PairType const& p) -> bool {return notInMap(p);}

/// @brief Tags the deriving class as a collection of key-value pairs, stored in a list structure.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TPair<class, class> Pair type.
/// @tparam TList<class, class> List type. By default, it is `List`.
template<
	class TKey,
	class TValue,
	template <class TPairKey, class TPairValue> class TPair = Pair,
	template <class TListElement, class TListIndex> class TList = List
>
struct ListCollected: Paired<TKey, TValue, TPair> {
	static_assert(Type::Container::PairLike<TPair<TKey, TValue>>, "Type is not a valid pair type!");
};

/// @brief Associative container comprised of key-value pairs.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TIndex Index type for underlying storage.
/// @tparam SORT Whether the container is sorted by default.
/// @tparam TList<class, class> List type. By default, it is `List`.
template<
	class TKey,
	class TValue,
	Type::Integer TIndex = usize,
	bool SORT = true,
	template <class TListElement, class TListIndex> class TList = List
>
struct BaseListMap:
	ListCollected<TKey, TValue, KeyValuePair>,
	Derived<TList<KeyValuePair<TKey, TValue>, TIndex>>,
	SelfIdentified<BaseListMap<TKey, TValue, TIndex, SORT>>,
	private TList<KeyValuePair<TKey, TValue>, TIndex> {
public:
	/// @brief Whether the container is sorted by default.
	constexpr static bool SORTED = SORT;

	using Derived			= ::CTL::Derived<TList<KeyValuePair<TKey, TValue>, TIndex>>;
	using ListCollected		= ::CTL::ListCollected<TKey, TValue, KeyValuePair>;
	using SelfIdentified	= ::CTL::SelfIdentified<BaseListMap<TKey, TValue, TIndex, SORTED>>;

	using typename Derived::BaseType;

	using
		typename ListCollected::KeyType,
		typename ListCollected::ValueType,
		typename ListCollected::PairType
	;

	using typename SelfIdentified::SelfType;

	using
		typename BaseType::ArgumentListType,
		typename BaseType::SizeType,
		typename BaseType::IndexType,
		typename BaseType::IteratorType,
		typename BaseType::ReverseIteratorType,
		typename BaseType::ConstIteratorType,
		typename BaseType::ConstReverseIteratorType
	;

	static_assert(
		!SORTED || Type::Algorithm::Sortable<KeyType>, 
		"Cannot form a sortable map whithout an sortable key!"
	);
	static_assert(
		!SORTED || Type::Algorithm::Sortable<PairType>, 
		"Cannot form a sortable map whithout an sortable pair type!"
	);

	using
		typename BaseType::OrderType,
		typename BaseType::Order
	;

	using
		typename BaseType::PredicateType,
		typename BaseType::CompareType
	;

	using
		BaseType::tighten,
		BaseType::clear,
		BaseType::dispose,
		BaseType::operator==,
		BaseType::operator<=>,
		BaseType::equals,
		BaseType::compare,
		BaseType::disparity,
		BaseType::data,
		BaseType::front,
		BaseType::back,
		BaseType::begin,
		BaseType::end,
		BaseType::cbegin,
		BaseType::cend,
		BaseType::rbegin,
		BaseType::rend,
		BaseType::sort,
		BaseType::size,
		BaseType::filter,
		BaseType::filtered,
		BaseType::capacity,
		BaseType::allocator,
		BaseType::empty
	;

	/// @brief Pair key comparator.
	struct KeyCompare {
		/// @brief Returns whether a pair's key is equal to another.
		/// @param a Pair to compare.
		/// @param b Pair to compare to.
		/// @return Whether they're equal.
		constexpr static bool equals(PairType const& a, PairType const& b)
		requires Type::Comparator::Equals<KeyType, KeyType> {
			return SimpleComparator<KeyType>::equals(a.front(), b.front());
		}

		/// @brief Returns the order between two pairs' keys.
		/// @param a Pair to compare.
		/// @param b Pair to compare to.
		/// @return Order between keys.
		constexpr static auto compare(PairType const& a, PairType const& b)
		requires Type::Comparator::Threeway<KeyType, KeyType> {
			return SimpleComparator<KeyType>::compare(a.front(), b.front());
		}
	};

	/// @brief Default constructor.
	constexpr BaseListMap(): BaseType() {}

	/// @brief Constructs the container with a preallocated capacity.
	/// @param size Size to allocate.
	constexpr BaseListMap(SizeType const size): BaseType(size) {}

	/// @brief Constructs the container from a set of key-value pairs.
	/// @param values Pairs to add.
	/// @note
	///		After insertion, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	constexpr BaseListMap(ArgumentListType const& values): BaseType(values) {
		clean();
		update();
	}

	/// @brief Constructs the container form a set of key-value pairs.
	/// @param values Pairs to add.
	/// @note
	///		After insertion, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	template<SizeType N>
	constexpr explicit BaseListMap(As<PairType[N]> const& values): BaseType(values) {
		clean();
		update();
	}

	/// @brief Constructs the container form a set of key-value pairs.
	/// @tparam Args... Argument types.
	/// @param args... Pairs to add.
	/// @note
	///		After insertion, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	template<typename... Args>
	constexpr BaseListMap(Args const&... args)
	requires (... && Type::Convertible<Args, PairType>)
	: BaseType(args...) {
		clean();
		update();
	}

	/// @brief Constructs the container form a list-like container of key-value pairs.
	/// @param other List-like of key-value pairs to copy from.
	/// @note
	///		After insertion, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	constexpr BaseListMap(BaseType const& other): BaseType(other) {
		clean();
		update();
	}

	/// @brief Constructs the container form a list-like of key-value pairs.
	/// @param other List-like of key-value pairs to construct from.
	/// @note
	///		After insertion, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	constexpr BaseListMap(BaseType&& other): BaseType(CTL::move(other)) {
		clean();
		update();
	}

	/// @brief Constructs the container from another of the same type.
	/// @param other Container to copy from.
	constexpr BaseListMap(SelfType const& other): BaseType(other) {}

	/// @brief Constructs the container from another of the same type.
	/// @param other Container to move from.
	constexpr BaseListMap(SelfType&& other): BaseType(CTL::move(other)) {}

	/// @brief Constructs the container from a range of key-value pairs.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	constexpr BaseListMap(IteratorType const& begin, IteratorType const& end): BaseType(begin, end) {
		clean();
		update();
	}

	/// @brief Constructs the container from a range of key-value pairs.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator to end of range.
	constexpr BaseListMap(ReverseIteratorType const& begin, ReverseIteratorType const& end): BaseType(begin, end) {
		clean();
		update();
	}

	/// @brief Gets the value of the element that matches the given key.
	/// @param key Key to look for.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when key does not exist.
	constexpr ValueType const& at(KeyType const& key) const {
		if (empty()) throw OutOfBoundsException("Key does not exist!");
		IndexType i = search(key);
		if (i == -1)	throw OutOfBoundsException("Key does not exist!");
		else			return (data() + i)->value;
	}

	/// @brief Allows access to the value of the element that matches the given key. Creates one if key doesn't exist.
	/// @param key Key to look for.
	/// @return Reference to element's value.
	constexpr ValueType& operator[](KeyType const& key) {
		if (empty()) return BaseType::pushBack(PairType(key)).back().back();
		IndexType i = search(key);
		while ((i = search(key)) == -1)
			insert(PairType(key, ValueType()));
		return BaseType::at(i).back();
	}

	/// @brief Searches for the index of a given key. If key doesn't exist, returns -1.
	/// @param key Key to look for.
	/// @return Index of key, or -1 if not found.
	constexpr IndexType search(KeyType const& key) const
	requires (SORTED) {
		return ::CTL::bsearch<ConstIteratorType, IndexType, PairType, KeyCompare>(begin(), end(), PairType(key));
	}

	/// @brief Searches for the index of a given key. If key doesn't exist, returns -1.
	/// @param key Key to look for.
	/// @return Index of key, or -1 if not found.
	constexpr IndexType search(KeyType const& key) const
	requires (!SORTED) {
		return ::CTL::fsearch<ConstIteratorType, IndexType, PairType, KeyCompare>(begin(), end(), PairType(key));
	}

	/// @brief Returns all keys in the container.
	/// @return `List` of keys.
	constexpr List<KeyType, SizeType> keys() const {
		List<KeyType, SizeType> result;
		result.reserve(size());
		for (auto& i: *this)
			result.pushBack(i.front());
		return result;
	}

	/// @brief Returns all values in the container.
	/// @return `List` of values.
	constexpr List<ValueType, SizeType> values() const {
		List<ValueType, SizeType> result;
		result.reserve(size());
		for (auto& i: *this)
			result.pushBack(i.back());
		return result;
	}

	/// @brief Returns all key-value pairs in the container.
	/// @return `List` of key-value pairs.
	constexpr List<PairType, SizeType> items() const {
		List<PairType, SizeType> result;
		result.reserve(size());
		for (auto& i: *this)
			result.pushBack(i);
		return result;
	}

	/// @brief Gets the value of the element that matches the given key.
	/// @param key Key to look for.
	/// @return Value of the element.
	/// @throw OutOfBoundsException when key does not exist.
	constexpr ValueType const& operator[](KeyType const& index) const {return at(index);}

	/// @brief Copy assignment operator.
	/// @param other Container to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other)	{BaseType::operator=(other); return *this;				}
	/// @brief Copy assignment operator.
	/// @param other Container to move from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other)			{BaseType::operator=(CTL::move(other)); return *this;	}

	/// @brief Returns whether key exists in container.
	/// @param key Key to search for.
	/// @return Whether key exists.
	constexpr bool contains(KeyType const& key) const {
		if (empty()) return false;
		return search(key) != -1;
	}

	/// @brief Erases an element that matches the given key.
	/// @param key Key to search for.
	/// @return Reference to self.
	constexpr SelfType& erase(KeyType const& key) {
		IndexType i = search(key);
		if (i == -1) return *this;
		BaseType::erase(i);
		return *this;
	}

	/// @brief Erases elements that match a given predicate.
	/// @param predicate Predicate to use as check.
	/// @return Reference to self.
	constexpr SelfType& eraseIf(PredicateType const& predicate) {
		BaseType::eraseIf(predicate);
		return *this;
	}

	/// @brief Erases elements that do not match a given predicate.
	/// @param predicate Predicate to use as check.
	/// @return Reference to self.
	constexpr SelfType& eraseIfNot(PredicateType const& predicate) {
		BaseType::eraseIfNot(predicate);
		return *this;
	}

	/// @brief Inserts a key-value pair into the container, if key does not exist.
	/// @param pair Key-value pair to insert.
	/// @return Reference to self.
	constexpr SelfType& insert(PairType const& pair) requires (SORTED) {
		if (!contains(pair.key)) {
			auto const pos = Nearest::bsearch(begin(), end(), pair).lowest;
			if (static_cast<SizeType>(pos) < size())
				BaseType::insert(pair, Nearest::bsearch(begin(), end(), pair).lowest);
			else BaseType::pushBack(pair);
		}
		return *this;
	}

	/// @brief Inserts a key-value pair into the container, if key does not exist.
	/// @param pair Key-value pair to insert.
	/// @return Reference to self.
	constexpr SelfType& insert(PairType const& pair) requires (!SORTED) {
		if (!contains(pair.key))
			BaseType::pushBack(pair);
		return *this;
	}

	/// @brief Adds another container's items to this one.
	/// @param other Container to copy from.
	/// @return Reference to self.
	/// @note
	///		After appending, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	constexpr SelfType& append(SelfType const& other) {
		BaseType::appendBack(other);
		clean();
		update();
		return *this;
	}

	/// @brief Adds a set of key-value pairs to the container.
	/// @param values Pairs to add.
	/// @return Reference to self.
	///		After appending, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	constexpr SelfType& insert(ArgumentListType const& values) {
		return append(SelfType(values));
	}

	/// @brief Adds a range of key-value pairs to the container.
	/// @param begin Iterator to beginning of range.
	/// @param end Iterator to end of range.
	/// @return Reference to self.
	/// @note
	///		After appending, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	constexpr SelfType& insert(IteratorType const& begin, IteratorType const& end) {
		return append(SelfType(begin, end));
	}

	/// @brief Adds a range of key-value pairs to the container.
	/// @param begin Reverse iterator to beginning of range.
	/// @param end Reverse iterator to end of range.
	/// @return Reference to self.
	/// @note
	///		After appending, the container filters itself and removes duplicates keys.
	///		Most recent key-value pair is kept.
	constexpr SelfType& insert(ReverseIteratorType const& begin, ReverseIteratorType const& end) {
		return append(SelfType(begin, end));
	}

	/// @brief Adds a set of key-value pairs to the container.
	/// @param values Pairs to add.
	/// @return Reference to self.
	template<SizeType S>
	constexpr SelfType& insert(As<PairType[S]> const& values) {
		return append(SelfType(values));
	}

private:
	/*
	constexpr static CompareType UNIQUE_VALUES(PairType const& a, PairType const& b) {
		return a.key != b.key;
	}
	//*/

	//*
	constexpr static auto const	UNIQUE_VALUES	= [](PairType const& a, PairType const& b){return a.front() != b.front();};
	
	constexpr bool notInMap(PairType const& pair) const {
		return !contains(pair.front());
	}
	//*/

	constexpr void update() requires (SORTED)	{sort();	}
	constexpr void update() requires (!SORTED)	{			}

	constexpr void clean() {filter(UNIQUE_VALUES);}
};

#undef NOT_IN_MAP

/// @brief Container-specific type constraints.
namespace Type::Container {
	/// @brief Implementation of type constraints.
	namespace Impl {
		template<class T>
		struct IsSimpleMap;

		template<
			template <class, class, class, bool, template <class, class> class> class T0,
			class T1,
			class T2,
			class T3,
			bool B4,
			template <class, class> class T5
		>
		struct IsSimpleMap<T0<T1, T2, T3, B4, T5>>:
			BooleanConstant<
				Type::Equal<T0<T1, T2, T3, B4, T5>,
				::CTL::BaseListMap<T1, T2, T3, B4, T5>
			>> {};
	}

	/// @brief Type must be `BaseSimpleMap`.
	template<class T>
	concept SimpleMap = Impl::IsSimpleMap<T>::value;
}

/*
template<class TKey, class TValue, Type::Integer TIndex = usize, class TPair = Pair<TKey, TValue>, class THasher = Hasher>
requires (
	Type::Comparator::Threeway<TKey, TKey>
&&	PairType<TPair>
&&	Hashable<TKey, THasher>
) struct HashMap:
	List<TPair<TKey, TValue>, TIndex>,
	Collected<TKey, TValue, TPair>,
	SelfIdentified<HashMap<TKey, TValue, TIndex, TPair>> {

}
*/

/// @brief `BaseSimpleMap` analog for an unsorted map that remembers order of insertion.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TIndex Index type.
template<class TKey, class TValue, Type::Integer TIndex = usize>
using OrderedMap	= BaseListMap<TKey, TValue, TIndex, false>;

/// @brief `BaseSimpleMap` analog for a sorted map.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TIndex Index type.
template<class TKey, class TValue, Type::Integer TIndex = usize>
using ListMap		= BaseListMap<TKey, TValue, TIndex, true>;

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_MAP_H
