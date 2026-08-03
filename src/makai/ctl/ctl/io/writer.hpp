#ifndef CTL_IO_WRITER_H
#define CTL_IO_WRITER_H

#include "write.hpp"
#include "../container/strings/strings.hpp"
#include "../algorithm/strconv.hpp"

CTL_NAMESPACE_BEGIN

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SUBOBJECTS
using IStringWriter = IWriter<String, CTL_IWRITER_WRAP(toString)>;

struct OutputStringWriter: IStringWriter {
	virtual ~OutputStringWriter() {}

	OutputStringWriter& display(String const& what) override	{IO::write(what); return *this;			}
	OutputStringWriter& newLine() override						{IO::write(IO::NEWLINE); return *this;	}
};
CTL_DIAGBLOCK_END

CTL_NAMESPACE_END

#endif // CTL_IO_WRITER_H
