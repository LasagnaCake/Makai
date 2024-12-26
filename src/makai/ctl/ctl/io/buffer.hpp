#ifndef CTL_IO_BUFFER_H
#define CTL_IO_BUFFER_H

#include <streambuf>
#include "../namespace.hpp"
#include "../ctypes.hpp"

CTL_NAMESPACE_BEGIN

/// @brief Generic buffer.
using DataBuffer	= std::streambuf;
/// @brief File buffer.
using FileBuffer	= std::filebuf;
/// @brief String buffer.
using StringBuffer	= std::stringbuf;

/// @brief In-memory buffer.
struct MemoryBuffer: public DataBuffer {
	/// @brief Constructor.
	/// @param data Memory to reference.
	/// @param size Size of memory pointed to.
	MemoryBuffer(char* const data, usize const size) {
		setg(data, data, data + size);
		setp(data, data + size);
	}
};

CTL_NAMESPACE_END

#endif // CTL_IO_BUFFER_H
