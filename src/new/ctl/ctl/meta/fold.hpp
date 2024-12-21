#ifndef CTL_META_FOLD_H
#define CTL_META_FOLD_H

#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Fold expressions.
namespace Fold {
	/// @brief Logical `and`.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename... Args>
	constexpr bool land(Args const&... args) {
		return (... && args);
	}

	/// @brief Logical `or`.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename... Args>
	constexpr bool lor(Args const&... args) {
		return (... || args);
	}

	/// @brief Bitwise `and`.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T band(Args const&... args) {
		return (... & args);
	}

	/// @brief Bitwise `or`.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T bor(Args const&... args) {
		return (... | args);
	}

	/// @brief Bitwise `xor`.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T bxor(Args const&... args) {
		return (... ^ args);
	}

	/// @brief Addition.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T add(Args const&... args) {
		return (... + args);
	}

	/// @brief Subtraction.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T sub(Args const&... args) {
		return (... - args);
	}

	/// @brief Multiplication.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T mul(Args const&... args) {
		return (... * args);
	}

	/// @brief Division.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T div(Args const&... args) {
		return (... / args);
	}

	/// @brief Stream insertion.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the expression.
	template<typename T, typename... Args>
	constexpr T& strins(T& stream, Args const&... args) {
		(stream << ... << args);
		return stream;
	}

	/// @brief Average between values.
	/// @tparam ...Args Argument types.
	/// @param ...args Arguments.
	/// @return Result of the operation.
	template<typename T, typename... Args>
	constexpr T avg(Args const&... args) {
		return (add(args...)) / sizeof...(Args);
	}

	template<typename T, typename... Args>
	constexpr bool many(T const& first, Args const&... rest) {
		return (... || (first == rest));
	}

	template<typename T, typename... Args>
	constexpr bool mall(T const& first, Args const&... rest) {
		return (... && (first == rest));
	}
}

CTL_NAMESPACE_END

#endif // CTL_META_FOLD_H
