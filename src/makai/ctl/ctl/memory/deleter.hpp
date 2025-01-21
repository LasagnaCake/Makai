#ifndef CTL_MEMORY_DELETER_H
#define CTL_MEMORY_DELETER_H

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Implementations.
namespace Impl {
	/// @brief Basic deleter.
	/// @tparam T Data type.
	template<class T>
	struct BasicDeleter {
		/// @brief Data type.
		using DataType = T;

		/// @brief Deletes the given object.
		/// @param obj Object to delete.
		constexpr void operator()(DataType* const obj) const {
			delete obj;
		}

		/// @brief Deletes the given object.
		/// @tparam U Object type.
		/// @param obj Object to delete.
		template<class U>
		constexpr void operator()(U* const obj) const
		requires Type::Convertible<As<U[]>*, As<DataType[]>*> {
			delete[] obj;
		}
	};
}

/// @brief Memory-specific type constraints.
namespace Type::Memory {
	/// @brief Type must be a deleter for `TData`.
	template <class T, class TData> concept Deleter = requires (T t, TData* data) {
		{t(data)};
	};
}

/// @brief Default deleter.
/// @tparam T Data type.
template<class T>
using Deleter = Impl::BasicDeleter<T>;

/// @brief Tags the deriving class as possessing a deleter of some kind.
/// @tparam D Deleter.
/// @tparam TData Data type deleted by deleter.
template<auto D, class TData>
requires Type::Memory::Deleter<decltype(D), TData>
struct Deletable {
	/// @brief Deleter.
	constexpr static auto deleter = D;
};

CTL_NAMESPACE_END

#endif