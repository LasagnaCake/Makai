#ifndef CTL_CPPFAILURE_H
#define CTL_CPPFAILURE_H

#include "namespace.hpp"
#include "ctypes.hpp"
#include <stdexcept>

CTL_NAMESPACE_BEGIN

/// @brief Program crash. Catastrophic.
struct Crash {};

/// @brief Crash used purely for debug purposes. Catastrophic.
/// @tparam I Line of the crash.
template<usize I>
struct DebugCrash: Crash {};

/// @brief Generic, potentially-recoverable failure.
struct Failure {
	virtual char const* what() const noexcept {return "Something happened!";}
};

/// @brief Irrecoverable failure. Catastrophic.
struct CatastrophicFailure: Crash {
	virtual char const* what() const noexcept {return "Something REALLY bad happened!";}
};

/// @brief Allocation failure. Catastrophic.
struct AllocationFailure: CatastrophicFailure {
	virtual char const* what() const noexcept {return "Memory allocation failed!";}
};

/// @brief Maximum size possible reached failure. Catastrophic.
struct MaximumSizeFailure: CatastrophicFailure {
	virtual char const* what() const noexcept {return "Maximum size reached!";}
};

/// @brief Object construction failure. Catastrophic.
struct ConstructionFailure: CatastrophicFailure {
	virtual char const* what() const noexcept {return "Failed to construct type!";}
};

/// @brief Crashes the program.
inline void panic() {
	throw Crash();
}

CTL_NAMESPACE_END

#endif // CTL_CPPFAILURE_H