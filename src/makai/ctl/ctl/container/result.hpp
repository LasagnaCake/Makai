#ifndef CTL_CONTAINER_RESULT_H
#define CTL_CONTAINER_RESULT_H

#include "../templates.hpp"
#include "error.hpp"
#include "nullable.hpp"
#include "function.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Result of an operation, with an optional error type.
/// @tparam TData Data type.
/// @tparam TError Error type.
/// @note `TError` must be a different type than `TData`.
template<typename TData, Type::Different<TData> TError>
class Result:
public SelfIdentified<Result<TData, TError>>,
public Typed<TData>,
public Defaultable<TData> {
public:
	using Typed				= Typed<TData>;
	using SelfIdentified	= SelfIdentified<Result<TData, TError>>;

	using
		typename Typed::DataType,
		typename Typed::ConstReferenceType,
		typename Typed::ReferenceType,
		typename Typed::TemporaryType
	;

	using
		typename SelfIdentified::SelfType
	;

	using ErrorType = TError;

	using ErrorFunctionType = Decay::AsFunction<void(ErrorType const&)>;
	using ValueFunctionType = Decay::AsFunction<void(ConstReferenceType)>;

	/// @brief Whether `TError` can be implicitly convertible to `TData`.
	constexpr static bool IMPLICIT = Type::Convertible<TError, TData>;

	/// @brief Copy constructor (`Result`).
	/// @param other Other `Result` to copy from.
	constexpr Result(SelfType const& other)						{result = other.result; state = other.state;						}
	/// @brief Move constructor (`Result`).
	/// @param other Other `Result` to move.
	constexpr Result(SelfType&& other) 							{result = CTL::move(other.result); state = CTL::move(other.state);	}
	/// @brief Copy constructor (value).
	/// @param other Result value to copy.
	constexpr Result(ConstReferenceType value)					{result.value = value; state = ResultState::RS_OK;					}
	/// @brief Move constructor (value).
	/// @param other Result value to move.
	constexpr Result(TemporaryType value)						{result.value = CTL::move(value); state = ResultState::RS_OK;		}
	/// @brief Copy constructor (Error).
	/// @param other Error value to copy.
	/// @note Explicit if error type to not be implicitly convertible to result type.
	constexpr explicit(IMPLICIT) Result(ErrorType const& value)	{result.error = error; state = ResultState::RS_ERROR;				}
	/// @brief Move constructor (Error).
	/// @param other Error value to copy.
	/// @note Explicit if error type to not be implicitly convertible to result type.
	constexpr explicit(IMPLICIT) Result(ErrorType&& value)		{result.error = CTL::move(error); state = ResultState::RS_ERROR;	}

	/// @brief Runs the passed callable if there is a value.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the value type, and return void.
	template<Type::Functional<ValueFunctionType> TFunction>
	constexpr SelfType& then(TFunction const& proc)  				{if (ok()) proc(result.value); return *this;	}
	/// @brief Runs the passed callable if there is a value.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the value type, and return void.
	template<Type::Functional<ValueFunctionType> TFunction>
	constexpr SelfType const& then(TFunction const& proc) const 	{if (ok()) proc(result.value); return *this;	}

	/// @brief Runs the passed callable if there is an error.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the error type, and return void.
	template<Type::Functional<ErrorFunctionType> TFunction>
	constexpr SelfType& onError(TFunction const& proc)				{if (!ok()) proc(result.error); return *this;	}
	/// @brief Runs the passed callable if there is an error.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the error type, and return void.
	template<Type::Functional<ErrorFunctionType> TFunction>
	constexpr SelfType const& onError(TFunction const& proc) const	{if (!ok()) proc(result.error); return *this;	}

	/// @brief Copy assignment operator (value).
	/// @param value Value to store.
	/// @return Reference to self.
	constexpr SelfType& operator=(DataType const& value)						{result.value = value; state = ResultState::RS_OK; return *this;					}
	/// @brief Move assignment operator (value).
	/// @param value Value to store.
	/// @return Reference to self.
	constexpr SelfType& operator=(DataType&& value)								{result.value = CTL::move(value); state = ResultState::RS_OK; return *this;			}
	/// @brief Copy assignment operator (error).
	/// @param error Error to store.
	/// @return Reference to self.
	/// @note Requires error type to not be implicitly convertible to result type.
	constexpr SelfType& operator=(ErrorType const& error) requires (!IMPLICIT)	{result.error = error; state = ResultState::RS_ERROR; return *this;					}
	/// @brief Move assignment operator (error).
	/// @param error Error to store.
	/// @return Reference to self.
	/// @note Requires error type to not be implicitly convertible to result type.
	constexpr SelfType& operator=(ErrorType&& error) requires (!IMPLICIT)		{result.error = CTL::move(error); state = ResultState::RS_ERROR; return *this;		}
	/// @brief Copy assignment operator (error).
	/// @param other `Result` to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other)						{result = other.result; state = other.state; return *this;							}
	/// @brief Move assignment operator (error).
	/// @param other `Result` to move.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType&& other)								{result = CTL::move(other.result); state = CTL::move(other.state); return *this;	}

	/// @brief Equality comparison operator (value).
	/// @param value Value to compare.
	/// @return Whether `Result` is equal to it.
	constexpr bool operator==(DataType const& value) const							{if (!ok()) return false; return result.value == value;			}
	/// @brief Equality comparison operator (error).
	/// @param error Error value to compare.
	/// @return Whether `Result` is equal to it.
	/// @note Requires error type to not be implicitly convertible to result type.
	constexpr bool operator==(ErrorType const& error) const requires (!IMPLICIT)	{if (ok()) return false; return result.error == error;			}
	/// @brief Equality comparison operator (`Result`).
	/// @param other Other `Result` to compare.
	/// @return Whether objects are equal.
	constexpr bool operator==(SelfType const& other) const							{return ok() ? other == result.value : other == result.error;	}

	/// @brief Returns whether there is an non-error value.
	/// @return Whether there is an non-error value.
	constexpr bool ok()	const			{return state == ResultState::RS_OK;	}
	/// @brief Returns whether there is an non-error value.
	/// @return Whether there is an non-error value.
	constexpr operator bool() const		{return ok();							}
	/// @brief Returns whether there is an non-error value.
	/// @return Whether there is an non-error value.
	constexpr bool operator()() const	{return ok();							}

	/// @brief Runs the passed callable if there is a value.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the value type, and return void.
	template<Type::Functional<ValueFunctionType> TFunction>
	constexpr SelfType& operator()(TFunction const& proc)				{return then(proc);		}
	/// @brief Runs the passed callable if there is a value.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the value type, and return void.
	template<Type::Functional<ValueFunctionType> TFunction>
	constexpr SelfType const& operator()(TFunction const& proc) const	{return then(proc);		}

	/// @brief Runs the passed callable if there is an error.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the error type, and return void.
	/// @note Requires error type to not be implicitly convertible to result type.
	template<Type::Functional<ErrorFunctionType> TFunction>
	constexpr SelfType& operator()(TFunction const& proc)				
	requires (!IMPLICIT) {return onError(proc);	}
	/// @brief Runs the passed callable if there is an error.
	/// @tparam TFunction Callable type.
	/// @param proc Callable to run.
	/// @return Reference to self.
	/// @note `TFunction` must accept a const reference to the error type, and return void.
	/// @note Requires error type to not be implicitly convertible to result type.
	template<Type::Functional<ErrorFunctionType> TFunction>
	constexpr SelfType const& operator()(TFunction const& proc) const	
	requires (!IMPLICIT) {return onError(proc);	}

	/// @brief Returns the stored value, or null if none.
	/// @return The stored value, or null if none.
	constexpr Nullable<DataType>	value() const {return ok() ? result.value : nullptr;	}
	/// @brief Returns the stored error, or null if none.
	/// @return The stored error, or null if none.
	constexpr Nullable<ErrorType>	error() const {return !ok() ? result.error : nullptr;	}

private:
	enum class ResultState {
		RS_OK,
		RS_ERROR
	};

	union ResultWrapper {
		DataType	value;
		ErrorType	error;
	};

	ResultWrapper	result;
	ResultState		state;
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_RESULT_H
