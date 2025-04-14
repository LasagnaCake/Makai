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
		[[nodiscard]] static constexpr Instance<Entry> create(Args&&... args) {
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

	/// @brief Object entry type.
	using Object		= Instance<Entry>;
	/// @brief "Handle-to-object-entry" type.
	using ObjectHandle	= Handle<Entry>;

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