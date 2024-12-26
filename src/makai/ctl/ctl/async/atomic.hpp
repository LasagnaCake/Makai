#ifndef CTL_ASYNC_ATOMIC_H
#define CTL_ASYNC_ATOMIC_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../templates.hpp"
#include "../container/function.hpp"
#include "../container/view.hpp"
#include "../adapter/comparator.hpp"
#include "thread.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Thread-safe value container.
/// @tparam T Value type.
/// @note
///		This only guarantees that acquisition,
///		assignment (and assignment-like) and `modify()` operations
///		are thread-safe.
/// @note
///		If you need the value to not be accessed by any other thread
///		while the current thread operates on it,
///		or for an indeterminate amount of time,
///		Use a `Box` instead.
template <class T>
class Atomic:
	Async::Base::Yieldable,
	Typed<T>,
	SelfIdentified<Atomic<T>>,
	Derived<Async::Base::Yieldable>,
	Ordered {
public:
	using Typed				= Typed<T>;
	using SelfIdentified	= SelfIdentified<Atomic<T>>;
	using Derived			= Derived<Async::Base::Yieldable>;

	using
		typename Typed::DataType,
		typename Typed::ConstantType,
		typename Typed::ReferenceType,
		typename Typed::TemporaryType,
		typename Typed::ConstReferenceType
	;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Derived::BaseType
	;

	/// @brief Thread-safe view.
	using ViewType = View<Atomic<DataType>>;

	/// @brief Thread-safe operation.
	using OperationType = Decay::AsFunction<void(ReferenceType)>;

	/// @brief Empty constructor.
	constexpr Atomic() noexcept											{}
	/// @brief Copy constructor.
	/// @param data Value to copy from.
	constexpr Atomic(ConstReferenceType data):	data(data)				{}
	/// @brief Move constructor.
	/// @param data Value to move.
	constexpr Atomic(TemporaryType data):		data(CTL::move(data))	{}

	/// @brief Returns whether the stored value is currently being accessed.
	/// @return Whether it is being currently accessed.
	inline bool isLocked() const {return locked;}
	
	/// @brief Safe copy assignment operator.
	/// @param data Value to copy from.
	inline SelfType& operator=(ConstReferenceType val) {
		capture();
		data = val;
		release();
		return *this;
	}

	/// @brief Safe move assignment operator.
	/// @param data Value to move.
	inline SelfType& operator=(TemporaryType val) {
		capture();
		data = CTL::move(val);
		release();
		return *this;
	}

	/// @brief Safely accesses and modifies the stored value.
	/// @tparam TFunction Operation type.
	/// @param op Operation to perform.
	/// @return Reference to self.
	/// @details Guarantees that, while the operation is being performed, no other thread will be able to access the value.
	template<Type::Functional<OperationType> TFunction>
	inline SelfType& modify(TFunction const& op) {
		capture();
		op(data);
		release();
		return *this;
	}

	/// @brief Safely accesses and modifies the stored value.
	/// @tparam TFunction Operation type.
	/// @param op Operation to perform.
	/// @return Reference to self.
	/// @details Guarantees that, while the operation is being performed, no other thread will be able to access the value.
	template<Type::Functional<OperationType> TFunction>
	inline SelfType& operator()(TFunction const& op) {return modify(op);}

	/// @brief Returns a view to this object.
	/// @return View to object.
	inline ViewType view() {
		return ViewType(*this);
	}

	/// @brief Safely retrieves the stored value.
	/// @return Stored value.
	inline operator DataType() const {return value();}

	/// @brief Safely retrieves the stored value.
	/// @return Stored value.
	inline DataType value() const {
		capture();
		DataType v = data;
		release();
		return v;
	}

	/// @brief Safely compares values.
	/// @param other Value to compare with.
	/// @return Whether they're equal.
	/// @note Requires value type to be equally comparable.
	/// @sa Comparator::equals().
	inline bool operator==(T const& other) const
	requires Type::Comparator::Equals<DataType, DataType>	{return SimpleComparator<T>::equals(value(), other);			}
	/// @brief Safely compares values.
	/// @param other Value to compare with.
	/// @return Whether they're equal.
	/// @note Requires value type to be equally comparable.
	/// @sa Comparator::equals().
	inline bool operator==(Atomic const& other) const
	requires Type::Comparator::Equals<DataType, DataType>	{return SimpleComparator<T>::equals(value(), other.value());	}

	/// @brief Safely performs a threeway comparison.
	/// @param other Value to compare with.
	/// @return Order between values.
	/// @note Requires value type to be threeway comparable.
	/// @sa Comparator::compare().
	inline OrderType operator<=>(T const& other) const
	requires Type::Comparator::Threeway<DataType, DataType>	{return SimpleComparator<T>::compare(value(), other);			}
	/// @brief Safely performs a threeway comparison.
	/// @param other Atomic to compare with.
	/// @return Order between values.
	/// @note Requires value type to be threeway comparable.
	/// @sa Comparator::compare().
	inline OrderType operator<=>(Atomic const& other) const
	requires Type::Comparator::Threeway<DataType, DataType>	{return SimpleComparator<T>::compare(value(), other.value());	}

	/// @brief Safe pre-increment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator++() requires Type::PreIncrementable<DataType>									{capture(); ++data; return release();					}
	/// @brief Safe post-increment operator.
	/// @return Previous value.
	/// @note Only implemented if type supports it.
	DataType operator++(int) requires Type::PostIncrementable<DataType>									{capture(); DataType v = data++; release(); return v;	}
	/// @brief Safe pre-decrement operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator--() requires Type::PreDecrementable<DataType>									{capture(); --data; return release();					}
	/// @brief Safe post-decrement operator.
	/// @return Previous value.
	/// @note Only implemented if type supports it.
	DataType operator--(int) requires Type::PostDecrementable<DataType>									{capture(); DataType v = data--; release(); return v;	}
	/// @brief Safe add-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator+=(DataType const& v) requires Type::AddAssignable<DataType, DataType>			{capture(); data += v; return release();				}
	/// @brief Safe subtract-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator-=(DataType const& v) requires Type::SubAssignable<DataType, DataType>			{capture(); data -= v; return release();				}
	/// @brief Safe multiply-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator*=(DataType const& v) requires Type::MulAssignable<DataType, DataType>			{capture(); data *= v; return release();				}
	/// @brief Safe divide-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator/=(DataType const& v) requires Type::DivAssignable<DataType, DataType>			{capture(); data /= v; return release();				}
	/// @brief Safe bitwise-xor-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator^=(DataType const& v) requires Type::Bitwise::XorAssignable<DataType, DataType>	{capture(); data ^= v; return release();				}
	/// @brief Safe bitwise-and-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator&=(DataType const& v) requires Type::Bitwise::AndAssignable<DataType, DataType>	{capture(); data &= v; return release();				}
	/// @brief Safe bitwise-or-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator|=(DataType const& v) requires Type::Bitwise::OrAssignable<DataType, DataType>	{capture(); data |= v; return release();				}
	/// @brief Safe modulo-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator%=(DataType const& v) requires Type::ModAssignable<DataType, DataType>			{capture(); data %= v; return release();				}
	/// @brief Safe bitwise-shift-left-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator<<=(DataType const& v) requires Type::Stream::InsAssignable<DataType, DataType>	{capture(); data <<= v; return release();				}
	/// @brief Safe bitwise-shift-right-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator>>=(DataType const& v) requires Type::Stream::ExtAssignable<DataType, DataType>	{capture(); data >>= v; return release();				}

private:
	inline SelfType& capture()	{acquireLock(); return *this;	}
	inline SelfType& release()	{releaseLock(); return *this;	}

	inline void capture() const	{acquireLock();					}
	inline void release() const	{releaseLock();					}

	inline void acquireLock() const {
		do {
			while (getLock())
				BaseType::asyncYield();
		} while (__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE));
	}

	inline void releaseLock() const {
		__atomic_clear(&locked, __ATOMIC_RELEASE);
	}

	inline bool getLock() const {
		return __atomic_load_n(&locked, __ATOMIC_RELAXED);
	}

	/// @brief Underlying stored data.
	DataType data;
	/// @brief Whether the stored value is currently being accessed.
	volatile bool mutable locked = false;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_ATOMIC_H
