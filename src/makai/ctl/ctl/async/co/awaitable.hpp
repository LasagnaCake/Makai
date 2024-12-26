#ifndef CTL_ASYNC_CO_AWAITABLE_H
#define CTL_ASYNC_CO_AWAITABLE_H

#include "../../namespace.hpp"
#include "../../ctypes.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Cooperative routine facilities.
namespace Co {
	template<class T>
	struct IAwaitable {
		virtual bool await_ready()		= 0;
		virtual bool await_suspend()	= 0;
		virtual T await_resume()		= 0;
	};
}

CTL_NAMESPACE_END

#endif