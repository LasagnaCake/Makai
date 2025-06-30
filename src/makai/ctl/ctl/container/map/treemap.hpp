#ifndef CTL_CONTAINER_MAP_TREEMAP_H
#define CTL_CONTAINER_MAP_TREEMAP_H

#include "../arguments.hpp"
#include "../tree/rbl.hpp"
#include "../lists/list.hpp"
#include "../../namespace.hpp"
#include "../../templates.hpp"
#include "../../typetraits/traits.hpp"
#include "../../algorithm/hash.hpp"
#include "../pair.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Tags the deriving class as a collection of key-value pairs, stored in a tree structure.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TPair<class, class> Pair type.
/// @tparam TTree<class, class> Tree type.
template<
	class TKey,
	class TValue,
	template <class, class> class							TPair	= Pair,
	template <class, class, template<class> class> class	TTree	= Tree::RBL
>
struct TreeCollected: Paired<TKey, TValue, TPair> {
	static_assert(Type::Container::PairLike<TPair<TKey, TValue>>, "Type is not a valid pair type!");
};

/// @brief Associative container comprised of key-value pairs.
/// @tparam TKey Key type.
/// @tparam TValue Value type.
/// @tparam TSize Size type.
/// @param TCompare<class> Comparator type.
/// @param TTree<class, class, class<class>> Tree type.
template<
	class TKey,
	class TValue,
	Type::Integer TSize = usize,
	template <class> class									TCompare	= SimpleComparator,
	template <class, class, template <class> class> class	TTree		= Tree::RBL
>
struct TreeMap:
	TreeCollected<TKey, TValue, KeyValuePair, TTree>,
	Derived<TTree<TKey, TValue, TCompare>>,
	SelfIdentified<TreeMap<TKey, TValue, TSize, TCompare, TTree>>,
	Indexed<TSize>,
	private TTree<TKey, TValue, SimpleComparator> {
		using Derived			= ::CTL::Derived<TTree<TKey, TValue, TCompare>>;
		using TreeCollected		= ::CTL::TreeCollected<TKey, TValue, KeyValuePair, TTree>;
		using SelfIdentified	= ::CTL::SelfIdentified<TreeMap<TKey, TValue, TSize, TCompare, TTree>>;
		using Indexed			= ::CTL::Indexed<TSize>;

		using typename Derived::BaseType;

		using
			typename TreeCollected::KeyType,
			typename TreeCollected::ValueType,
			typename TreeCollected::PairType
		;

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

		using
			typename BaseType::DataType,
			typename BaseType::ConstantType
		;

		/// @brief Empty constructor (defaulted).
		constexpr TreeMap()									= default;
		/// @brief Copy constructor (defaulted).
		constexpr TreeMap(TreeMap const& other)				= default;
		/// @brief Move constructor (defaulted).
		constexpr TreeMap(TreeMap&& other)					= default;
		/// @brief Copy assignment operator (defaulted).
		constexpr TreeMap& operator=(TreeMap const& other)	= default;
		/// @brief Move assignment operator (defaulted).
		constexpr TreeMap& operator=(TreeMap&& other)		= default;

		/// @brief Constructs the container from a list of pairs.
		/// @param values Values to add.
		constexpr TreeMap(List<PairType> const& values) {insert(values);}

		/// @brief Constructs the container from a range of key-value pairs.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		template <class TNode>
		constexpr TreeMap(
			typename BaseType::template NodeIterator<TNode, false> const& begin,
			typename BaseType::template NodeIterator<TNode, false> const& end
		) {insert(begin, end);}

		/// @brief Constructs the container from a range of key-value pairs.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		template <class TNode>
		constexpr TreeMap(
			typename BaseType::template NodeIterator<TNode, true> const& begin,
			typename BaseType::template NodeIterator<TNode, true> const& end
		) {insert(begin, end);}

		/// @brief Constructs the container from a range of key-value pairs.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		constexpr TreeMap(Iterator<PairType, false> const& begin, Iterator<PairType, false> const& end)	{insert(begin, end);}

		/// @brief Constructs the container from a range of key-value pairs.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		constexpr TreeMap(Iterator<PairType, true> const& begin, Iterator<PairType, true> const& end)	{insert(begin, end);}

		/// @brief Adds a set of key-value pairs to the container.
		/// @param values Pairs to add.
		/// @return Reference to self.
		template <SizeType S>
		constexpr TreeMap(As<PairType[S]> const& values) {insert(values);}

		/// @brief Gets the value of the element that matches the given key.
		/// @param key Key to look for.
		/// @return Value of the element.
		/// @throw OutOfBoundsException when key does not exist.
		constexpr ValueType const& at(KeyType const& key) const {
			if (auto const node = find(key))
				return node->value;
			throw OutOfBoundsException("Key does not exist!");
		}

		/// @brief Gets the value of the element that matches the given key.
		/// @param key Key to look for.
		/// @return Value of the element.
		constexpr ValueType& operator[](KeyType const& key) {
			if (auto const node = find(key))
				return node->value;
			return insert(key)->value;
		}

		/// @brief Gets the value of the element that matches the given key.
		/// @param key Key to look for.
		/// @return Value of the element.
		/// @throw OutOfBoundsException when key does not exist.
		constexpr ValueType const& operator[](KeyType const& key) const {
			return at(key);
		}

		/// @brief Inserts a key-value pair into the container, if key does not exist.
		/// @param pair Key-value pair to insert.
		/// @return Reference to self.
		constexpr SelfType& insert(PairType const& pair) {
			if (!contains(pair.key))
				insert(pair.key)->value = pair.value;
			++count;
			return *this;
		}

		/// @brief Adds another container's items to this one.
		/// @param other Container to copy from.
		/// @return Reference to self.
		/// @note Existing values are updated.
		constexpr SelfType& append(SelfType const& other) {
			BaseType::append(other);
			count += other.size();
			return *this;
		}
		
		/// @brief Adds a set of key-value pairs to the container.
		/// @param values Pairs to add.
		/// @return Reference to self.
		/// @note Only the most recent value for a key is kept.
		constexpr SelfType& insert(List<PairType> const& values) {
			for (auto const& v: values)
				insert(*v);
			return *this;
		}

		/// @brief Adds a range of key-value pairs to the container.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		/// @return Reference to self.
		template <class TNode>
		constexpr SelfType& insert(
			typename BaseType::template NodeIterator<TNode, false> const& begin,
			typename BaseType::template NodeIterator<TNode, false> const& end
		) {
			for (auto v = begin; v != end; ++v)
				insert(*v);
			return *this;
		}

		/// @brief Adds a range of key-value pairs to the container.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		/// @return Reference to self.
		template <class TNode>
		constexpr SelfType& insert(
			typename BaseType::template NodeIterator<TNode, true> const& begin,
			typename BaseType::template NodeIterator<TNode, true> const& end
		) {
			for (auto v = begin; v != end; ++v)
				insert(*v);
			return *this;
		}

		/// @brief Adds a range of key-value pairs to the container.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		/// @return Reference to self.
		constexpr SelfType& insert(Iterator<PairType, false> const& begin, Iterator<PairType, false> const& end) {
			for (auto v = begin; v != end; ++v)
				insert(*v);
			return *this;
		}

		/// @brief Adds a range of key-value pairs to the container.
		/// @param begin Iterator to beginning of range.
		/// @param end Iterator to end of range.
		/// @return Reference to self.
		constexpr SelfType& insert(Iterator<PairType, true> const& begin, Iterator<PairType, true> const& end) {
			for (auto v = begin; v != end; ++v)
				insert(*v);
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
			if (contains(key)) --count;
			BaseType::erase(key);
			return *this;
		}
		
		/// @brief Returns the amount of elements in the container.
		/// @return Element count.
		constexpr SizeType size() const {return count;}

		/// @brief Returns whether the container contains a given key.
		/// @param key Key to search for.
		/// @return Whether container has key.
		constexpr bool contains(KeyType const& key) const {return find(key);}

		/// @brief Returns all keys in the container.
		/// @return `List` of keys.
		constexpr List<KeyType, SizeType> keys() const {
			List<KeyType, SizeType> result;
			result.reserve(count);
			for (auto& i: *this)
				result.pushBack(i.front());
			return result;
		}

		/// @brief Returns all values in the container.
		/// @return `List` of values.
		constexpr List<ValueType, SizeType> values() const {
			List<ValueType, SizeType> result;
			result.reserve(count);
			for (auto& i: *this)
				result.pushBack(i.back());
			return result;
		}

		/// @brief Returns all key-value pairs in the container.
		/// @return `List` of key-value pairs.
		constexpr List<PairType, SizeType> items() const {
			List<PairType, SizeType> result;
			result.reserve(count);
			for (auto& i: *this)
				result.pushBack({i.front(), i.back()});
			return result;
		}

	private:
		/// @brief Amount of elements in the container.
		SizeType count = 0;
	};

CTL_NAMESPACE_END

#endif