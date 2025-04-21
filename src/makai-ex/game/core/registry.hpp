#ifndef MAKAILIB_EX_GAME_CORE_REGISTRY_H
#define MAKAILIB_EX_GAME_CORE_REGISTRY_H

#include <makai/makai.hpp>

/// @brief Game extensions.
namespace Makai::Ex::Game {

/// @brief Object registry ("Garbage Collector"-ish structure).
/// @tparam T Object type.
/// @tparam ID Unique identifier. By default, it is `0`.
template<class T, usize ID = 0>
struct Registry {
	/// @brief Registry member.
	struct Member;

	/// @brief Registry entry.
	using Entry = Unique<Member>;

	/// @brief Object container type.
	/// @tparam TShared<class> Shared pointer type.
	template<template <class> class TShared>
	struct Container: private TShared<Entry> {
		/// @brief Base class type.
		using BaseType = TShared<Entry>;
		static_assert(Type::OneOf<BaseType, Instance<Entry>, Handle<Entry>>, "T must be a shared pointer type!");
		/// @brief Data type.
		using DataType		= typename Entry::DataType;
		/// @brief Reference type.
		using ReferenceType	= typename Entry::ReferenceType;
		/// @brief Pointer type.
		using PointerType	= typename Entry::PointerType;

		using BaseType::BaseType;
		using BaseType::operator=;
		using BaseType::operator==;
		using BaseType::operator<=>;
		
		/// @brief Copy constructor (defaulted).
		constexpr Container(Container const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Container(Container&& other)		= default;

		/// @brief Copy assignment operator (defaulted).
		constexpr Container& operator=(Container const& other)	= default;
		/// @brief Move assignment operator (defaulted).
		constexpr Container& operator=(Container&& other)		= default;

		/// @brief Returns whether the underlying object exists.
		/// @return Whether the underlying object exists.
		constexpr bool exists() const		{return BaseType::exists() && (*this).exists();	}
		/// @brief Returns whether the underlying object exists.
		/// @return Whether the underlying object exists.
		constexpr operator bool() const		{return exists();								}
		/// @brief Returns whether the underlying object does not exist.
		/// @return Whether the underlying object does not exist.
		constexpr bool operator!() const	{return	!exists();								}

		/// @brief Returns a raw pointer to the underlying object.
		/// @return Raw pointer to underlying object.
		constexpr ref<DataType> raw() const {
			if (exists()) return (*this).raw();
			return nullptr;
		}
		/// @brief Returns a `Reference` to the underlying object.
		/// @return `Reference` to underlying object.
		constexpr Reference<DataType> reference() const {
			if (exists()) return (*this).reference();
			return nullptr;
		}

		/// @brief Statically casts the object to a new type.
		/// @tparam TNew New object type.
		/// @return Reference to new object type.
		template<class TNew>
		constexpr Reference<TNew> as() const			{return (*this).reference().template as<TNew>();			}
		/// @brief Dynamically casts the object to a new type.
		/// @tparam TNew New object type.
		/// @return Reference to new object type.
		template<class TNew>
		constexpr Reference<TNew> polymorph() const		{return (*this).reference().template polymorph<TNew>();		}
		/// @brief Reinterprets the object as an object type with different constness and volatileness.
		/// @tparam TNew New object type.
		/// @return Reference to new object type.
		template<class TNew>
		constexpr Reference<TNew> mutate() const		{return (*this).reference().template mutate<TNew>();		}
		/// @brief Reinterprets the object as a different object type.
		/// @tparam TNew New object type.
		/// @return Reference to new object type.
		template<class TNew>
		constexpr Reference<TNew> reinterpret() const	{return (*this).reference().template reinterpret<TNew>();	}

		/// @brief Pointer member access operator.
		/// @return Pointer to underlying object.
		/// @warning Will throw `Error::NullPointer` if object does not exist!
		constexpr PointerType operator->()				{return (*this).operator->();		}
		/// @brief Pointer member access operator.
		/// @return Pointer to underlying object.
		/// @warning Will throw `Error::NullPointer` if object does not exist!
		constexpr PointerType const operator->() const	{return (*this).operator->();		}

		/// @brief Returns the value of the underlying object.
		/// @return Reference to underlying object.
		/// @warning Will throw `Error::NullPointer` if object does not exist!
		constexpr DataType& value() const 				{return BaseType::value().value();	}
		/// @brief Dereference operator.
		/// @return Reference to underlying object.
		/// @warning Will throw `Error::NullPointer` if object does not exist!
		constexpr ReferenceType operator*() const		{return value();					}
	};

	/// @brief "Instance-to-entry" type.
	using Object		= Container<Instance>;
	/// @brief "Handle-to-entry" type.
	using ObjectHandle	= Container<Handle>;

	/// @brief Registry member. All classes that are part of this registry must derive from this one.
	/// @warning
	///		Normal instantiation is STRICTLY forbidden for this class!
	///		If you need to create an instance of a derived class, please use the `create(...)` function
	///		in here, or its associated `Registry`.
	struct Member {
		/// @brief Immediately deletes the member.
		constexpr void destroy() {
			if (destroying) return;
			destroying = true;
			self->unbind();
		}

		/// @brief Queues the member for deletion.
		constexpr Member& queueDestroy() {
			Registry::queue(this);
			queued = QueueState::QS_QUEUED;
			return *this;
		}

		/// @brief Instantiates a registry member.
		/// @tparam ...Args Constructor argument types.
		/// @tparam TSub Registry member type.
		/// @param ...args Constructor arguments.
		/// @return Reference to member entry.
		template<Type::Subclass<Member> TSub, class... Args>
		[[nodiscard]] static constexpr Object create(Args&&... args) {
			return (new TSub(forward<Args>(args)...))->self.raw();
		}

		/// @brief Destructor.
		constexpr virtual ~Member() {
			Registry::remove(this);
			if (queued == QueueState::QS_QUEUED)
				Registry::unqueue(this);
		}

		/// @brief Returns whether the member is queued for deletion.
		/// @return Whether member is queued for deletion.
		constexpr bool isQueued() const {
			return queued != QueueState::QS_UNQUEUED;
		}

	protected:
		/// @brief Constructor.
		constexpr Member() {
			self->bind(this);
			Registry::add(this);
		}
	
	private:
		/// @brief Reference to own entry.
		Handle<Entry> self = new Entry();

		constexpr void destroy(bool const) {
			queued = QueueState::QS_HANDLED;
			destroy();
		}

		friend class Registry;

		enum class QueueState {
			QS_UNQUEUED,
			QS_QUEUED,
			QS_HANDLED
		};

		/// @brief Whether the member is being destroyed via a `destroy()` call.
		bool		destroying 	= false;
		/// @brief Current queue state.
		QueueState	queued		= QueueState::QS_UNQUEUED;
	};

	/// @brief Search predicate type.
	using FindPredicate = bool(Member const&);

	/// @brief Query result type.
	using QueryResult = List<ObjectHandle>;

	/// @brief Finds all members that match the given predicate.
	/// @tparam TPred Predicate type.
	/// @param predicate Predicate to check for.
	/// @return Matched members.
	/// @note May return members queued for deletion, so it's not recommended to cache it!
	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult find(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (*m && predicate(**m))
				res.pushBack(m);
		return res;
	}

	/// @brief Finds all members that do not match the given predicate.
	/// @tparam TPred Predicate type.
	/// @param predicate Predicate to check for.
	/// @return Excluded members.
	/// @note May return members queued for deletion, so i't i's not recommended to cache it!
	template <Type::Functional<FindPredicate> TPred>
	constexpr static QueryResult findNot(TPred const& predicate) {
		QueryResult res;
		for (auto const& m: members)
			if (*m && !predicate(**m))
				res.pushBack(m);
		return res;
	}

	/// @brief Returns all existing members in the registry.
	/// @return All existing registry members.
	/// @note May return members queued for deletion, so it's not recommended to cache it!
	constexpr static QueryResult all() {
		QueryResult res;
		for (auto const& m: members)
			if (*m) res.pushBack(m);
		return res;
	}

	/// @brief Destroys all queued members.
	constexpr static void destroyQueued() {
		auto qc = queued;
		for (auto& q: qc)
			if (q) q->destroy(true);
		queued.clear();
	}

	/// @brief Instantiates a registry member.
	/// @tparam ...Args Constructor argument types.
	/// @tparam TSub Registry member type.
	/// @param ...args Constructor arguments.
	/// @return Reference to member entry.
	template<Type::Subclass<Member> TSub, class... Args>
	[[nodiscard]] static constexpr Object create(Args&&... args) {
		return Member::template create<TSub>(forward<Args>(args)...);
	}
	
private:
	friend class Member;

	constexpr static void queue(owner<Member> const& member)	{queued.pushBack(member);	}
	constexpr static void unqueue(owner<Member> const& member)	{queued.eraseLike(member);	}

	constexpr static void add(owner<Member> const& member)		{members.pushBack(member->self);	}
	constexpr static void remove(owner<Member> const& member)	{members.eraseLike(member->self);	}

	/// @brief Existing registry members.
	inline static List<owner<Entry>>	members;
	/// @brief Members queued for deletion.
	inline static List<owner<Member>>	queued;
};

}
#endif