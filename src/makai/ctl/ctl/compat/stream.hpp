#ifndef CTL_COMPAT_STREAM_H
#define CTL_COMPAT_STREAM_H

#include <istream>
#include <ostream>
#include "../namespace.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Output STL stream.
/// @tparam T Character type.
template<class T>
using STLOutputStream	= std::basic_ostream<T>;
/// @brief Input STL stream.
/// @tparam T Character type.
template<class T>
using STLInputStream	= std::basic_istream<T>;

/// @brief Tags the deriving class as supporting STL stream operations.
/// @tparam TData Character type.
template<class TData>
struct STLStreamable {
	/// @brief Input stream type.
	typedef STLInputStream<TData>	InputSTLStreamType;
	/// @brief Output stream type.
	typedef STLOutputStream<TData>	OutputSTLStreamType;
};

CTL_NAMESPACE_END

#endif // CTL_IO_STREAM_H
