#ifndef CTL_CPPERROR_H
#define CTL_CPPERROR_H

#include "namespace.hpp"
#include "staticvalue.hpp"
#include "templates.hpp"
#include "cppfailure.hpp"

CTL_NAMESPACE_BEGIN

struct Exception;

/// @brief Basic exception.
struct Exception:
	Failure,
	SelfIdentified<Exception>,
	Typed<const char*>,
	CStringable<char> {
public:
	using Typed				= ::CTL::Typed<const char*>;
	using SelfIdentified	= ::CTL::SelfIdentified<Exception>;

	using
		typename Typed::DataType,
		typename Typed::ConstReferenceType
	;

	using
		typename SelfIdentified::SelfType
	;

	/// @brief Constructs an exception with an associated message.
	/// @param message Exception message.
	Exception(ConstReferenceType message) noexcept: message(message), prev(ex) {
		ex = this;
	}

	/// @brief Copy constructor.
	/// @param other `Exception` to copy from.
	Exception(SelfType const& other) noexcept: Exception(other.message) {}

	/// @brief Returns the exception's message.
	/// @return Message.
	virtual char const* what() const noexcept {return message;}

	/// @brief Returns the current exception.
	/// @return Current exception.
	static Exception* current() noexcept 	{return ex;		}
	/// @brief Returns the previous exception.
	/// @return Previous exception.
	Exception* previous() const noexcept	{return prev;	}

private:
	/// @brief Message.
	char const* const message;

	/// @brief Previous exception.
	Exception* const prev		= nullptr;
	/// @brief Current exception.
	inline static Exception* ex	= nullptr;
	 
	template <class TString> friend class DetailedError;
};

/// @brief Out-of-bounds index.
struct OutOfBoundsException:		Exception {using Exception::Exception;};
/// @brief Bad function call.
struct BadCallException:			Exception {using Exception::Exception;};
/// @brief Invalid value.
struct InvalidValueException:		Exception {using Exception::Exception;};
/// @brief Nonexistent value.
struct NonexistentValueException:	Exception {using Exception::Exception;};
/// @brief Invalid type conversion.
struct InvalidConversionException:	Exception {using Exception::Exception;};
/// @brief Invalid type cast.
struct InvalidCastException:		Exception {using Exception::Exception;};
/// @brief Failed action.
struct FailedActionException:		Exception {using Exception::Exception;};
/// @brief Missing stream.
struct MissingStreamException:		Exception {using Exception::Exception;};

namespace {
	template<Type::Convertible<char const*> T>
	constexpr const char* strlit(T const& value) {
		return (const char*)value;
	}

	template<class T>
	constexpr const char* strlit(T const& value)
	requires requires (T t) {
		{t.cstr()} -> Type::Convertible<const char*>;
	} {
		return value.cstr();
	}

	template<class T>
	constexpr const char* strlit(T const& value)
	requires requires (T t) {
		{t.c_str()} -> Type::Convertible<const char*>;
	} {
		return value.c_str;
	}
}

/// @brief Error-specific type constraints.
namespace Type::Error {
	/// @brief Type must be a valid string type.
	template<typename T>
	concept ErrorStringType =
		Type::Constructible<T, const char*>
	&&	(Type::Constructible<T, T const&> || Type::Constructible<T, T>)
	&&	Type::Addable<T, T>
	&&	Type::Addable<T, const char*>
	&&	Type::Addable<const char*, T>
	&&	requires (T a, const char* b) {
			{strlit(a)};
			{a + a} -> Type::Equal<T>;
			{a + b} -> Type::Equal<T>;
			{b + a} -> Type::Equal<T>;
		}
	;
}

/// @brief Detailed exception.
/// @tparam TString String type.
template <Type::Error::ErrorStringType TString>
struct DetailedException:
	Exception,
	Derived<Exception>,
	Typed<TString>,
	SelfIdentified<DetailedException<TString>> {
public:
	using Typed				= ::CTL::Typed<TString>;
	using Derived			= ::CTL::Derived<Exception>;
	using SelfIdentified	= ::CTL::SelfIdentified<DetailedException<TString>>;

	using
		typename Typed::DataType,
		typename Typed::ConstReferenceType
	;

	using
		typename SelfIdentified::SelfType
	;

	using
		typename Derived::BaseType
	;

	using
		typename BaseType::CStringType
	;

	/// @brief Type of the exception.
	const DataType type;
	/// @brief Exception message.
	const DataType message;
	/// @brief File it was thrown in.
	const DataType file;
	/// @brief Line it was thrown in.
	const DataType line;
	/// @brief Caller that threw it.
	const DataType caller;
	/// @brief Information about the exception.
	const DataType info;
	/// @brief Information about the caller's exception.
	const DataType callerInfo;

	/// @brief Constructs the exception.
	/// @param type Type of the exception.
	/// @param message Exception message.
	/// @param file File it was thrown in.
	/// @param line Line it was thrown in
	/// @param caller Caller that threw it.
	/// @param info Information about the exception.
	/// @param callerInfo Information about the caller's exception.
	DetailedException(
		ConstReferenceType type			= "Unknown",
		ConstReferenceType message		= "none",
		ConstReferenceType file			= "unspecified",
		ConstReferenceType line			= "unspecified",
		ConstReferenceType caller		= "unspecified",
		ConstReferenceType info			= "none",
		ConstReferenceType callerInfo	= "none"
	) noexcept:
		BaseType(strlit(message)),
		type(type),
		message(message),
		file(file),
		line(line),
		caller(caller),
		info(info),
		callerInfo(callerInfo)
	{sumbuf = summary();}

	/// @brief Copy constructor.
	/// @param other `DetailedException` to copy from.
	DetailedException(SelfType const& other) noexcept:
		SelfType(
			other.type,
			other.message,
			other.file,
			other.line,
			other.caller,
			other.info,
			other.callerInfo
		)
	{}

	/// @brief Constructs the exception, based off of another.
	/// @param . Ignored. Necessary to avoid constructor conflicts.
	/// @param other `DetailedException` to extend from.
	/// @param type Type of the exception.
	/// @param file File it was thrown in.
	/// @param line Line it was thrown in
	/// @param caller Caller that threw it.
	/// @param info Information about the exception.
	/// @param callerInfo Information about the caller's exception.
	DetailedException(
		bool,
		BaseType const& other,
		ConstReferenceType type			= "Unknown",
		ConstReferenceType file			= "unspecified",
		ConstReferenceType line			= "unspecified",
		ConstReferenceType caller		= "unspecified",
		ConstReferenceType info			= "none",
		ConstReferenceType callerInfo	= "none"
	) noexcept:
		DetailedException(
			type,
			other.what(),
			file,
			line,
			caller,
			info,
			callerInfo
		)
	{}

	/// @brief Generates a detailed report of the exception.
	/// @return Detailed report.
	DataType report() const noexcept {
		DataType result = (
			"!!! AN ERROR HAS OCCURRED !!!\n\n"
			"<error>\n\n"
		+	type + ": " + message + "\n\n"
			"Caller: " + caller + "\n"
			"File: " + file + "\n"
			"Line: " + line + "\n"
			"\n[General Information]\n" + info + "\n"
			"\n[Caller Information]\n" + callerInfo + "\n"
		);
		result += "\n</error>";
		return result;
	}

	/// @brief Generates a summary of the exception.
	/// @return Summary.
	DataType summary() const noexcept {
		DataType sum = (type + ": " + message + (!info.empty() ? ("\n" + info) : ""));
		return sum;
	}

	/// @brief Returns a summary of the exception.
	/// @return Summary. 
	CStringType what() const noexcept override {
		return strlit(sumbuf);
	}

private:
	/// @brief Summary of the exception.
	DataType sumbuf = "";
};

CTL_NAMESPACE_END

#endif // CTL_CPPERROR_H
