#ifndef CTL_ASYNC_BOX_H
#define CTL_ASYNC_BOX_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../typetraits/traits.hpp"
#include "mutex.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Thread-safe value container.
/// @tparam T Value type.
/// @tparam M Mutex type.
/// @note
///		This class is to be used together with a mutex lock.
/// @note
///		If you need a simpler way to store thread-safe data,
///		without worrying about handling ownership,
///		use an `Atomic` instead.
template<class T, Type::Derived<Mutex> M = Mutex>
class Box:
	public M,
	public Typed<T>,
	public SelfIdentified<Box<T, M>>,
	public Derived<M>,
	public Ordered {
public:
	using Typed				= Typed<T>;
	using SelfIdentified	= SelfIdentified<Box<T, M>>;
	using Derived			= Derived<M>;

	using
		typename Typed::DataType,
		typename Typed::ReferenceType,
		typename Typed::TemporaryType,
		typename Typed::ConstReferenceType
	;

	using
		typename SelfIdentified::SelfType
	;

	using typename Derived::BaseType;

	/// @brief Mutex type.
	using MutexType = BaseType;

	/// @brief Empty constructor.
	constexpr Box()	noexcept										{}
	/// @brief Copy constructor.
	/// @param data Data to copy from.
	constexpr Box(ConstReferenceType data):	data(data)				{}
	/// @brief Move constructor.
	/// @param data Data to move.
	constexpr Box(TemporaryType data):			data(move(data))	{}

	/// @brief Safe assignment operator.
	/// @param val Value to assign.
	/// @return Reference to self.
	inline SelfType& operator=(T const& val) {
		this->wait();
		data = val;
		return *this;
	}

	/// @brief Safely acquires the stored value.
	/// @return Stored value.
	operator T() {return value();}

	/// @brief Safely acquires the stored value.
	/// @return Stored value.
	DataType value() const {
		this->wait();
		return data;
	}
	
	/// @brief Safely compares values.
	/// @param other Value to compare with.
	/// @return Whether they're equal.
	/// @note Requires value type to be equally comparable.
	/// @sa Comparator::equals().
	inline bool operator==(DataType const& other) const
	requires Type::Comparator::Equals<DataType, DataType>	{return SimpleComparator<T>::equals(value(), other);			}
	/// @brief Safely compares values.
	/// @param other Value to compare with.
	/// @return Whether they're equal.
	/// @note Requires value type to be equally comparable.
	/// @sa Comparator::equals().
	inline bool operator==(SelfType const& other) const
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
	inline OrderType operator<=>(SelfType const& other) const
	requires Type::Comparator::Threeway<DataType, DataType>	{return SimpleComparator<T>::compare(value(), other.value());	}

	/// @brief Safe pre-increment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator++() requires Type::PreIncrementable<DataType>									{this->wait(); ++data;					}
	/// @brief Safe post-increment operator.
	/// @return Previous value.
	/// @note Only implemented if type supports it.
	DataType operator++(int) requires Type::PostIncrementable<DataType>									{this->wait(); T v = data++; return v;	}
	/// @brief Safe pre-decrement operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator--() requires Type::PreDecrementable<DataType>									{this->wait(); --data;					}
	/// @brief Safe post-decrement operator.
	/// @return Previous value.
	/// @note Only implemented if type supports it.
	DataType operator--(int) requires Type::PostDecrementable<DataType>									{this->wait(); T v = data--; return v;	}
	/// @brief Safe add-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator+=(DataType const& v) requires Type::AddAssignable<DataType, DataType>			{this->wait(); data += v;				}
	/// @brief Safe subtract-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator-=(DataType const& v) requires Type::SubAssignable<DataType, DataType>			{this->wait(); data -= v;				}
	/// @brief Safe multiply-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator*=(DataType const& v) requires Type::MulAssignable<DataType, DataType>			{this->wait(); data *= v;				}
	/// @brief Safe divide-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator/=(DataType const& v) requires Type::DivAssignable<DataType, DataType>			{this->wait(); data /= v;				}
	/// @brief Safe bitwise-xor-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator^=(DataType const& v) requires Type::Bitwise::XorAssignable<DataType, DataType>	{this->wait(); data ^= v;				}
	/// @brief Safe bitwise-and-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator&=(DataType const& v) requires Type::Bitwise::AndAssignable<DataType, DataType>	{this->wait(); data &= v;				}
	/// @brief Safe bitwise-or-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator|=(DataType const& v) requires Type::Bitwise::OrAssignable<DataType, DataType>	{this->wait(); data |= v;				}
	/// @brief Safe modulo-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator%=(DataType const& v) requires Type::ModAssignable<DataType, DataType>			{this->wait(); data %= v;				}
	/// @brief Safe bitwise-shift-left-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator<<=(DataType const& v) requires Type::Stream::InsAssignable<DataType, DataType>	{this->wait(); data <<= v;				}
	/// @brief Safe bitwise-shift-right-assignment operator.
	/// @return Reference to self.
	/// @note Only implemented if type supports it.
	SelfType& operator>>=(DataType const& v) requires Type::Stream::ExtAssignable<DataType, DataType>	{this->wait(); data >>= v;				}

private:
	/// @brief Underlying stored data.
	DataType data;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_BOX_H
