#ifndef CTL_IO_STREAM_H
#define CTL_IO_STREAM_H

#include <istream>
#include <ostream>
#include "../namespace.hpp"
#include "buffer.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Output stream.
/// @tparam T Character type.
template<class T>
using OutputStream	= std::basic_ostream<T>;
/// @brief Input stream.
/// @tparam T Character type.
template<class T>
using InputStream	= std::basic_istream<T>;

/// @brief Generic stream.
using DataStream = std::iostream;

/// @brief File stream.
using FileStream = std::fstream;

/// @brief In-memory stream.
struct MemoryStream: DataStream {
	/// @brief Memory buffer associated with the stream.
	MemoryBuffer buffer;

	/// @brief Constructor.
	/// @param data Memory to reference.
	/// @param size Size of memory pointed to.
	MemoryStream(char* const data, usize const size):
		DataStream(&buffer),
		buffer(data, size) {}
};

/// @brief Tags the deriving class as supporting stream operations.
/// @tparam TData Character type.
template<class TData>
struct Streamable {
	/// @brief Input stream type.
	typedef InputStream<TData>	InputStreamType;
	/// @brief Output stream type.
	typedef OutputStream<TData>	OutputStreamType;
};

CTL_NAMESPACE_END

#endif // CTL_IO_STREAM_H
