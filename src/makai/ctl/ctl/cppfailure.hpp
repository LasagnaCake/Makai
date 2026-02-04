#ifndef CTL_CPPFAILURE_H
#define CTL_CPPFAILURE_H

#include "namespace.hpp"
#include "cpp/debug.hpp"
#include "ctypes.hpp"

CTL_NAMESPACE_BEGIN

namespace {struct Untraceable {};}

/// @brief Program crash. Catastrophic.
struct Crash {};

/// @brief Crash used purely for debug purposes. Catastrophic.
/// @tparam I Line of the crash.
template<usize I>
struct DebugCrash: Crash {};

/// @brief Generic, potentially-recoverable failure.
struct Failure: CPP::Debug::Traceable {
	constexpr virtual cstring what() const noexcept {return "Something happened!";}
};

/// @brief Irrecoverable failure. Catastrophic.
struct CatastrophicFailure: Crash, CPP::Debug::Traceable {
	constexpr virtual cstring what() const noexcept {return "Something REALLY bad happened!";}
};

/// @brief Allocation failure. Catastrophic.
struct AllocationFailure: CatastrophicFailure {
	constexpr cstring what() const noexcept override {return "Memory allocation failed!";}
};

/// @brief Maximum size possible reached failure. Catastrophic.
struct MaximumSizeFailure: CatastrophicFailure {
	constexpr cstring what() const noexcept override {return "Maximum size reached!";}
};

/// @brief Object construction failure. Catastrophic.
struct ConstructionFailure: CatastrophicFailure {
	constexpr cstring what() const noexcept override {return "Failed to construct type!";}
};

/// @brief Invalid memory access failure. Catastrophic.
struct InvalidAccessFailure: CatastrophicFailure {
	constexpr cstring what() const noexcept override {return "Invalid memory access";}
};

/// @brief Crashes the program. Effectively throws a catastrophic, non-recoverable crash.
/// @note If you somehow catch this, you're doing something VERY wrong.
[[noreturn]]
inline void panic() {
	// Parry this you fucking casual
	throw Untraceable{};
}

CTL_NAMESPACE_END

#endif // CTL_CPPFAILURE_H
