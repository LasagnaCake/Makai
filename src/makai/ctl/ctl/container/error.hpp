#ifndef CTL_CUSTOM_RUNTIME_ERRORS_H
#define CTL_CUSTOM_RUNTIME_ERRORS_H

#include "string.hpp"
#include "../cpperror.hpp"
#include "../cpp/sourcefile.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Detailed errors.
namespace Error {
	/// @brief Basic error type.
	using Generic = DetailedException<String>;

	#define DEFINE_ERROR_TYPE(NAME)\
		struct NAME: public Generic {\
			NAME (\
				String const&			message,\
				CPP::SourceFile const&	src			= CTL_CPP_DEFAULT_SOURCE\
			): Generic (#NAME, message, src.file, src.lineName(), src.function, "none", "none") {}\
			NAME (\
				String const&			message,\
				String const&			info,\
				CPP::SourceFile const&	src			= CTL_CPP_DEFAULT_SOURCE\
			): Generic (#NAME, message, src.file, src.lineName(), src.function, info, "none") {}\
			NAME (\
				String const&			message,\
				String const&			info,\
				String const&			callerInfo,\
				CPP::SourceFile const&	src			= CTL_CPP_DEFAULT_SOURCE\
			): Generic (#NAME, message, src.file, src.lineName(), src.function, info, callerInfo) {}\
		}

	// "Invalid X" errors
	DEFINE_ERROR_TYPE(InvalidAction);
	DEFINE_ERROR_TYPE(InvalidValue);
	DEFINE_ERROR_TYPE(InvalidType);
	DEFINE_ERROR_TYPE(InvalidCall);
	DEFINE_ERROR_TYPE(InvalidCast);
	// Value errors
	DEFINE_ERROR_TYPE(OutOfBounds);
	DEFINE_ERROR_TYPE(NonexistentValue);
	DEFINE_ERROR_TYPE(DuplicateValue);
	DEFINE_ERROR_TYPE(NullPointer);
	DEFINE_ERROR_TYPE(NotFound);
	// Other errors
	DEFINE_ERROR_TYPE(FailedAction);
	DEFINE_ERROR_TYPE(Unimplemented);
	DEFINE_ERROR_TYPE(UserIsAnIdiot);
	DEFINE_ERROR_TYPE(Other);
	DEFINE_ERROR_TYPE(NotAnError);

	#undef DEFINE_ERROR_TYPE

	/// @brief Pointer to an exception.
	typedef Exception::Pointer ErrorPointer;

	/// @brief Returns a pointer to the current exception.
	/// @return Pointer to the current exception. 
	inline ErrorPointer current() {return Exception::current();}
}

CTL_NAMESPACE_END

/// @brief Defines a custom error type.
/// @param NAME Class name.
#define DEFINE_ERROR_TYPE(NAME)\
	struct NAME: public ::CTL::Error::Generic {\
		NAME (\
			::CTL::String const&			message,\
			::CTL::CPP::SourceFile const&	src			= CTL_CPP_DEFAULT_SOURCE\
		): ::CTL::Error::Generic (#NAME, message, src.file, src.lineName(), src.function, "none", "none") {}\
		NAME (\
			::CTL::String const&			message,\
			::CTL::String const&			info,\
			::CTL::CPP::SourceFile const&	src			= CTL_CPP_DEFAULT_SOURCE\
		): ::CTL::Error::Generic (#NAME, message, src.file, src.lineName(), src.function, info, "none") {}\
		NAME (\
			::CTL::String const&			message,\
			::CTL::String const&			info,\
			::CTL::String const&			callerInfo,\
			::CTL::CPP::SourceFile const&	src			= CTL_CPP_DEFAULT_SOURCE\
		): ::CTL::Error::Generic (#NAME, message, src.file, src.lineName(), src.function, info, callerInfo) {}\
	}

#endif // CTL_CUSTOM_RUNTIME_ERRORS_H
