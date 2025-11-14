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
template<Type::Safe TData, Type::Safe TError>
class Result:
public SelfIdentified<Result<TData, TError>>,
public Typed<TData>,
public Defaultable<TData> {
public:
	static_assert(Type::Different<TData, TError>, "TError type and TData must be different!");

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

	/// @brief Empty constructor (deleted).
	constexpr Result() = delete;

	/// @brief Copy constructor (`Result`).
	/// @param other Other `Result` to copy from.
	constexpr Result(SelfType const& other) {operator=(other);}
	/// @brief Copy constructor (value).
	/// @param other Result value to copy.
	constexpr Result(ConstReferenceType value)					{operator=(value);	}
	/// @brief Copy constructor (Error).
	/// @param other Error value to copy.
	/// @note Explicit if error type is implicitly convertible to result type.
	constexpr explicit(IMPLICIT) Result(ErrorType const& value)	{operator=(value);	}

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
	constexpr SelfType& operator=(DataType const& value)						{destruct(); MX::construct(&result.value, value); state = ResultState::RS_OK; return *this;		}
	/// @brief Copy assignment operator (error).
	/// @param error Error to store.
	/// @return Reference to self.
	/// @note Requires error type to not be implicitly convertible to result type.
	constexpr SelfType& operator=(ErrorType const& error) requires (!IMPLICIT)	{destruct(); MX::construct(&result.error, error); state = ResultState::RS_ERROR; return *this;	}
	/// @brief Copy assignment operator (error).
	/// @param other `Result` to copy from.
	/// @return Reference to self.
	constexpr SelfType& operator=(SelfType const& other) {
		switch (other.state) {
			case ResultState::RS_OK:	return operator=(other.result.value);
			case ResultState::RS_ERROR: return operator=(other.result.error);
			default: destruct(); break;
		}
		return *this;
	}

	/// @brief Equality comparison operator (value).
	/// @param value Value to compare.
	/// @return Whether `Result` is equal to it.
	constexpr bool operator==(DataType const& value) const {
		if (state == ResultState::RS_UNDEFINED) return false;
		return ok() ? result.value == value : false;
	}
	/// @brief Equality comparison operator (error).
	/// @param error Error value to compare.
	/// @return Whether `Result` is equal to it.
	/// @note Requires error type to not be implicitly convertible to result type.
	constexpr bool operator==(ErrorType const& error) const requires (!IMPLICIT) {
		if (state == ResultState::RS_UNDEFINED) return false;
		return ok() ? false : result.error == error;
	}
	/// @brief Equality comparison operator (`Result`).
	/// @param other Other `Result` to compare.
	/// @return Whether objects are equal.
	constexpr bool operator==(SelfType const& other) const {
		switch (state) {
			case ResultState::RS_ERROR:		return other == result.error;
			case ResultState::RS_VALUE:		return other == result.value;	
			default: return false;
		}
	}

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

	/// @brief Return type.
	/// @brief T Data type.
	template <class T>
	using ReturnType = Meta::DualType<Type::Constructible<T, nulltype>, T, Nullable<T>>;

	/// @brief Returns the stored value, or null if none.
	/// @return The stored value, or null if none.
	constexpr ReturnType<DataType>	value() const {using R = ReturnType<DataType>; return ok() ? R(result.value) : R(nullptr);		}
	/// @brief Returns the stored error, or null if none.
	/// @return The stored error, or null if none.
	constexpr ReturnType<ErrorType>	error() const {using R = ReturnType<ErrorType>; return !ok() ? R(result.error) : R(nullptr);	}

	/// @brief Destructor.
	constexpr ~Result() {destruct();}

private:
	constexpr void destruct() {
		switch (state) {
			case ResultState::RS_OK:	MX::destruct(&result.value); break;
			case ResultState::RS_ERROR:	MX::destruct(&result.error); break;
			default: break;
		}
		state = ResultState::RS_UNDEFINED;
	}

	enum class ResultState {
		RS_UNDEFINED,
		RS_OK,
		RS_ERROR
	};

	union ResultWrapper {
		DataType	value;
		ErrorType	error;

		constexpr ResultWrapper()	{}
		constexpr ~ResultWrapper()	{}
	};

	ResultWrapper	result;
	ResultState		state	= ResultState::RS_UNDEFINED;
};

CTL_NAMESPACE_END

#endif // CTL_CONTAINER_RESULT_H
