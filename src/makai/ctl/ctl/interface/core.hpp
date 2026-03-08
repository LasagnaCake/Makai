#ifndef CTL_INTERFACE_CORE_H
#define CTL_INTERFACE_CORE_H

#include "../namespace.hpp"
#include "../typetraits/decay.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Interface for a compile-time-able stored value.
/// @tparam T Value type.
template<class T>
struct IConstValue {
	/// @brief Destructor.
	constexpr ~IConstValue() {}
	/// @brief Returns the stored value. Must be implemented.
	/// @return Stored value.
	constexpr virtual T value() const = 0;
};

/// @brief Interface for a stored value.
/// @tparam T Value type.
template<class T>
struct IValue {
	/// @brief Destructor.
	constexpr ~IValue() {}
	/// @brief Returns the stored value. Must be implemented.
	/// @return Stored value.
	constexpr virtual T value() const = 0;
};

/// @brief Interface for an object with toggleable visibility.
struct IVisible {
	/// @brief Destructor.
	constexpr ~IVisible() {}
	/// @brief Shows the object. Must be implemented.
	constexpr virtual void show() = 0;
	/// @brief Hides the object. Must be implemented.
	constexpr virtual void hide() = 0;
};

/// @brief Interface for a clonable object.
/// @tparam T Object instance type.
template<class T>
struct IClonable: T {
	/// @brief Destructor.
	constexpr ~IClonable() {}
	/// @brief Returns a new copy of the object.
	/// @return copy of object.
	constexpr virtual T clone() const = 0;
	/// @brief Returns a new copy of the object.
	/// @return copy of object.
	constexpr virtual T clone() = 0;
};

/// @brief Interface for an invokable object.
/// @tparam T Result type.
/// @tparam Args... Argument types.
template<class T, class... Args>
struct IInvokable {
	/// @brief Destructor.
	constexpr ~IInvokable() {}
	/// @brief Invokes some code.
	/// @param args... arguments.
	/// @return Result.
	constexpr virtual T invoke(Args... args) = 0;
};

/// @brief Interface for a const-invokable object.
/// @tparam T Result type.
/// @tparam Args... Argument types.
template<class T, class... Args>
struct IConstInvokable {
	/// @brief Destructor.
	constexpr ~IConstInvokable() {}
	/// @brief Invokes some code.
	/// @param args... arguments.
	/// @return Result.
	constexpr virtual T invoke(Args... args) const = 0;
};

CTL_NAMESPACE_END

#endif // CTL_INTERFACE_CORE_H
