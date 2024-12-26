#ifndef CTL_COMPAT_ARGUMENTS_H
#define CTL_COMPAT_ARGUMENTS_H

#include "../namespace.hpp"
#include "../ctypes.hpp"
#include "../templates.hpp"
#include "../cpperror.hpp"
#include "../typeinfo.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Argument list.
/// @tparam T Argument type.
/// @tparam T Size limit.
template <class T, usize L = NumberLimit<usize>::HIGHEST>
struct Arguments:
	Typed<T>,
	SelfIdentified<Arguments<T>> {
	using Typed				= CTL::Typed<T>;
	using SelfIdentified	= CTL::SelfIdentified<Arguments<T>>;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Typed::DataType,
		typename Typed::ConstantType,
		typename Typed::PointerType,
		typename Typed::ConstPointerType,
		typename Typed::ReferenceType,
		typename Typed::ConstReferenceType
	;

	/// @brief Destructor.
	constexpr ~Arguments() noexcept {delete[] start;}

	/// @brief Empty constructor.
	constexpr Arguments() noexcept: start(nullptr), length(0) {}

	/// @brief Constructs the argument list with a fixed array of arguments.
	/// @tparam N Array size.
	/// @param data Fixed array of arguments to use.
	template<usize N>
	constexpr Arguments(Decay::AsType<ConstantType[N]> const& data)
	requires (N <= L): start(::new DataType[N]), length(N) {
		MX::memcpy(start, data, N);
	}

	/// @brief Constructs the argument list with a given parameter pack.
	/// @tparam ...Args Parameter pack.
	/// @param first First item on pack.
	/// @param ...rest Other pack elements.
	template<class... Args>
	constexpr Arguments(DataType const& first, Args const&... rest)
	requires (sizeof...(Args) <= L):
		start(::new DataType[sizeof...(Args) + 1]{first, rest...}), length(sizeof...(Args) + 1) {}

	/// @brief Gets the argument at the specified index.
	/// @param index Argument index.
	/// @return Reference to argument.
	/// @throw OutOfBoundsException when index is bigger than argument count.
	constexpr ReferenceType operator[](ssize index) {
		while (index < 0)
			index += length;
		if (index > (length-1))
			throw OutOfBoundsException("Argument index is out of bounds!");
		return start[index];
	}

	/// @brief Gets the argument at the specified index.
	/// @param index Argument index.
	/// @return Const reference to argument.
	/// @throw OutOfBoundsException when index is bigger than argument count.
	constexpr ConstReferenceType operator[](ssize index) const {
		while (index < 0)
			index += length;
		if (index > (length-1))
			throw OutOfBoundsException("Argument index is out of bounds!");
		return start[index];
	}

	/// @brief Copy assignment operator (deleted). 
	Arguments& operator=(Arguments&&)		= delete;
	/// @brief Move assignment operator (deleted).
	Arguments& operator=(Arguments const&)	= delete;

	/// @brief Gets a pointer to the beginning of the argument list.
	/// @return Pointer to the beginning of the argument list.
	constexpr ConstPointerType begin() const noexcept	{return start;			}
	/// @brief Gets a pointer to the end of the argument list.
	/// @return Pointer to the end of the argument list.
	constexpr ConstPointerType end() const noexcept		{return start + length;	}

	/// @brief Returns the size of the argument list.
	/// @return Argument list size.
	constexpr usize size() const noexcept	{return length;			}
	/// @brief Returns whether the argument list is empty.
	/// @return Whether the argument list is empty.
	constexpr usize empty() const noexcept	{return length == 0;	}

private:
	/// @brief "C-style array" constructor.
	/// @param data Pointer to beginning of array.
	/// @param size Array size.
	constexpr Arguments(ConstPointerType data, usize size):
	start(new DataType[size]), length(size) {
		MX::memcpy(start, data, size);
	}

	/// @brief Start of the argument list.
	PointerType		start;
	/// @brief Length of the argument list.
	usize			length;
};

template <class TData>
struct ListInitializable {
	typedef Arguments<TData>	ArgumentListType;
};

CTL_NAMESPACE_END

#endif // CTL_COMPAT_ARGUMENTS_H
