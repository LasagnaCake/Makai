#ifndef CTL_INTERFACE_CORE_H
#define CTL_INTERFACE_CORE_H

#include "../namespace.hpp"
#include "../typetraits/decay.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Interface for a stored constant value.
/// @tparam T Value type.
template<class T>
struct IConstValue {
	/// @brief Destructor.
	constexpr virtual ~IConstValue() {}
	/// @brief Returns the stored value. Must be implemented.
	/// @return Stored value.
	constexpr virtual T value() const = 0;
};

/// @brief Interface for a stored mutable value.
/// @tparam T Value type.
template<class T>
struct IValue {
	/// @brief Destructor.
	constexpr virtual ~IValue() {}
	/// @brief Returns the stored value. Must be implemented.
	/// @return Stored value.
	constexpr virtual T value() = 0;
};

/// @brief Interface for an object with toggleable visibility.
struct IVisible {
	/// @brief Destructor.
	constexpr virtual ~IVisible() {}
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
	constexpr virtual ~IClonable() {}
	/// @brief Returns a new copy of the object.
	/// @return copy of object.
	constexpr virtual T clone() const = 0;
	/// @brief Returns a new copy of the object.
	/// @return copy of object.
	constexpr virtual T clone() = 0;
};

/// @brief Interface for an invokable object.
/// @tparam TInvoke Invocation type.
template<class TInvoke>
struct IInvokable;

/// @brief Interface for an invokable object.
/// @tparam TReturn Result type.
/// @tparam TArgs... Argument types.
template<class TReturn, class... TArgs>
struct IInvokable<TReturn(TArgs...)> {
	/// @brief Destructor.
	constexpr virtual ~IInvokable() {}
	/// @brief Invokes some code.
	/// @param args... arguments.
	/// @return Result.
	constexpr virtual TReturn invoke(TArgs... args) = 0;
};

/// @brief Interface for an invokable object.
/// @tparam TInvoke Invocation type.
template<class TInvoke>
struct IConstInvokable;

/// @brief Interface for a const-invokable object.
/// @tparam TReturn Result type.
/// @tparam TArgs... Argument types.
template<class TReturn, class... TArgs>
struct IConstInvokable<TReturn(TArgs...)> {
	/// @brief Destructor.
	constexpr virtual ~IConstInvokable() {}
	/// @brief Invokes some code.
	/// @param args... arguments.
	/// @return Result.
	constexpr virtual TReturn invoke(TArgs... args) const = 0;
};

#define CTL_IWRITER_WRAP(F) ([] <class _> (_ const& e) {return F(e);})

/// @brief Interface for a text output object.
template <class T, auto C = [] <class _> (_ const& e) {return T(e);}>
requires (Type::Equal<decltype(C(declval<T>())), T>)
struct IWriter {
	constexpr virtual ~IWriter() {}

	constexpr virtual IWriter& display(T const& what)	= 0;
	constexpr virtual IWriter& newLine()				= 0;

	template <class... Args>
	constexpr IWriter& write(Args const&... args)
	requires (... && Type::Equal<decltype(C(declval<Args>())), T>) {
		(..., display(C(args)));
		return *this;
	}

	template <class... Args>
	constexpr IWriter& writeLine(Args const&... args)
	requires (... && Type::Equal<decltype(C(declval<Args>())), T>)  {
		return write(args...).newLine();
	}
};

CTL_NAMESPACE_END

#endif // CTL_INTERFACE_CORE_H
