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
struct Failure: std::exception {
	cstring what() const noexcept override {return "Something happened!";}
};

/// @brief Irrecoverable failure. Catastrophic.
struct CatastrophicFailure: Crash {
	virtual cstring what() const noexcept {return "Something REALLY bad happened!";}
};

/// @brief Allocation failure. Catastrophic.
struct AllocationFailure: CatastrophicFailure {
	cstring what() const noexcept override {return "Memory allocation failed!";}
};

/// @brief Maximum size possible reached failure. Catastrophic.
struct MaximumSizeFailure: CatastrophicFailure {
	cstring what() const noexcept override {return "Maximum size reached!";}
};

/// @brief Object construction failure. Catastrophic.
struct ConstructionFailure: CatastrophicFailure {
	cstring what() const noexcept override {return "Failed to construct type!";}
};

/// @brief Crashes the program.
[[noreturn]] inline void panic() {
	throw Crash();
}

CTL_NAMESPACE_END

#endif // CTL_CPPFAILURE_H