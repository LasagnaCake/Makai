#ifndef CTL_ASYNC_DEFER_H
#define CTL_ASYNC_DEFER_H

#include "../exnamespace.hpp"
#include "../container/functor.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Function to be called at the end of a scope.
/// @note Cannot be directly heap-allocated.
struct Defer {
	/// @brief Empty constructor.
	constexpr Defer() {}
	
	/// @brief Sets a function to be called at the end of the scope.
	/// @param f Function to call.
	constexpr Defer(Functor<void()> const& f): func(f) {}
	
	/// @brief Sets a function to be called at the end of the scope.
	/// @param f Function to call.
	/// @return Reference to self.
	constexpr Defer& operator=(Functor<void()> const& f) {func = f; return *this;}

	/// @brief Destructor.
	constexpr ~Defer() {func();}

private:
	void* operator new(usize);
	void operator delete(pointer);
	void* operator new[](usize);
	void operator delete[](pointer);

	/// @brief Function to call at the end of the scope.
	Functor<void()> func;
};

CTL_NAMESPACE_END

#endif // CTL_ASYNC_DEFER_H
