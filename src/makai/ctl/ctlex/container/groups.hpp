#ifndef CTL_EX_CONTAINER_GROUPS_H
#define CTL_EX_CONTAINER_GROUPS_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/typetraits/traits.hpp"
#include "../../ctl/templates.hpp"
#include "../../ctl/container/map.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Collection of objects, grouped by an identifier.
/// @tparam TData Element type.
/// @tparam TIdentifier Identifier type.
/// @tparam TIndex Index type.
template<class TData, class TIdentifier = usize, CTL::Type::Integer TIndex = usize>
struct Groups:
	Collected<TIdentifier, TData, KeyValuePair>,
	Iteratable<KeyValuePair<TIdentifier, TData>, TIndex>,
	SelfIdentified<Groups<TData, TIdentifier, TIndex>> {
public:
	static_assert(Type::Comparator::Threeway<TIdentifier, TIdentifier>, "Identifier type must be comparable!");

	using Collected			= ::CTL::Collected<TIdentifier, TData>;
	using Iteratable		= ::CTL::Iteratable<KeyValuePair<TIdentifier, TData>, TIndex>;
	using SelfIdentified	= ::CTL::SelfIdentified<Groups<TData, TIdentifier, TIndex>>;

	using KeyType	= Collected::KeyType;
	using ValueType	= Collected::ValueType;
	using PairType	= Collected::PairType;

	using
		SizeType = Iteratable::SizeType
	;

	using GroupType				= List<ValueType, SizeType>;
	using IdentifierListType	= List<KeyType, SizeType>;

	using CollectionType = Map<KeyType, GroupType, SizeType>;

	using typename SelfIdentified::SelfType;

	/// @brief Gets a group by an identifier.
	/// @param group Group identifier.
	/// @return Reference to group.
	constexpr GroupType& get(KeyType const& group) {
		if (!groups.contains(group))
			flush(group);
		return groups[group];
	}

	/// @brief Gets a group by an identifier.
	/// @param group Group identifier.
	/// @return Reference to group.
	constexpr GroupType& operator[](KeyType const& group) {
		return get(group);
	}

	/// @brief Returns the groups an object is in.
	/// @param obj Object to search for.
	/// @return Groups it is in.
	constexpr IdentifierListType withObject(ValueType const& obj) {
		if (groups.empty()) return IdentifierListType();
		IdentifierListType gs;
		try {
			for (auto const& group: groups)
				if (group.value.find(obj) != -1)
					gs.pushBack(group.key);
		} catch(...) {}
		return gs;
	}

	/// @brief Adds an object to a group.
	/// @param obj Object to add.
	/// @param group Group to add in.
	/// @return Reference to self.
	constexpr SelfType& add(ValueType const& obj, KeyType const& group) {
		get(group).pushBack(obj);
		return *this;
	}

	/// @brief Removes an object from a group.
	/// @param obj Object to remove.
	/// @param group Group to remove from.
	/// @return Reference to self.
	constexpr SelfType& remove(ValueType const& obj, KeyType const& group) {
		DEBUGLN("Finding object group...");
		GroupType& gp = get(group);
		DEBUGLN("Removing object...");
		gp.eraseLike(obj);
		DEBUGLN("Done!");
		return *this;
	}

	/// @brief Removes an object from all groups it is in.
	/// @param obj Object to remove.
	/// @return Reference to self.
	constexpr SelfType& removeFromAll(ValueType const& obj) {
		IdentifierListType gs = withObject(obj);
		if (gs.empty()) return *this;
		for (auto group: gs)
			remove(obj, group);
		return *this;
	}

	/// @brief Creates/clears a group.
	/// @param group Group to flush.
	/// @return Reference to self.
	constexpr SelfType& flush(KeyType const& group) {
		groups[group] = GroupType();
		return *this;
	}

	/// @brief Returns whether an object is in a given group.
	/// @param obj Object to check.
	/// @param group Group to check in.
	/// @return Whether the object is in the group.
	constexpr bool contains(ValueType const& obj, KeyType const& group) const {
		return groups[group].find(obj) != -1;
	}

	/// @brief Returns the identifiers of all groups.
	/// @return All group identifiers.
	constexpr IdentifierListType all() const {
		return groups.keys();
	}
	
	/// @brief Returns an iterator to the beginning of the collection.
	/// @return Iterator to beginning of collection.
	constexpr auto begin()			{return groups.begin();		}
	/// @brief Returns an iterator to the end of the collection.
	/// @return Iterator to end of collection.
	constexpr auto end()			{return groups.end();		}
	/// @brief Returns a pointer to the beginning of the collection.
	/// @return Pointer to beginning of collection.
	constexpr auto cbegin()			{return groups.cbegin();	}
	/// @brief Returns a pointer to the end of the collection.
	/// @return Pointer to end of collection.
	constexpr auto cend()			{return groups.cend();		}
	/// @brief Returns a reverse iterator to the beginning of the collection.
	/// @return Reverse iterator to beginning of collection.
	constexpr auto rbegin()			{return groups.rbegin();	}
	/// @brief Returns a reverse iterator to the end of the collection.
	/// @return Reverse iterator to end of collection.
	constexpr auto rend()			{return groups.rend();		}

	/// @brief Returns an iterator to the beginning of the collection.
	/// @return Iterator to beginning of collection.
	constexpr auto begin() const	{return groups.begin();		}
	/// @brief Returns an iterator to the end of the collection.
	/// @return Iterator to end of collection.
	constexpr auto end() const		{return groups.end();		}
	/// @brief Returns a pointer to the beginning of the collection.
	/// @return Pointer to beginning of collection.
	constexpr auto cbegin() const	{return groups.cbegin();	}
	/// @brief Returns a pointer to the end of the collection.
	/// @return Pointer to end of collection.
	constexpr auto cend() const		{return groups.cend();		}
	/// @brief Returns a reverse iterator to the beginning of the collection.
	/// @return Reverse iterator to beginning of collection.
	constexpr auto rbegin() const	{return groups.rbegin();	}
	/// @brief Returns a reverse iterator to the end of the collection.
	/// @return Reverse iterator to end of collection.
	constexpr auto rend() const		{return groups.rend();		}

private:
	/// @brief The underlying group database.
	CollectionType groups;
};

CTL_EX_NAMESPACE_END

#endif // CTL_EX_CONTAINER_GROUPS_H