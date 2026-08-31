#ifndef CTL_EX_BINARY_ENCODER_H
#define CTL_EX_BINARY_ENCODER_H

#include "core.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Binary {
	template <class T>
	struct Encoder {
		IWriter&	writer;
		T			file;

		template <class T>
		Entry put(T const& value) {
			Entry entry;
			entry.start = writer.pointer;
			writer.put(value);
			entry.size = sizeof(T);
			return entry;
		}

		template <class T>
		Entry append(List<T> const& values) {
			Entry entry;
			entry.start = writer.pointer;
			writer.append(values);
			entry.size = writer.pointer - entry.start;
			return entry;
		}

		template <Type::OneOf<String, UTF8String, UTF32String> T>
		Entry append(T const& value) {
			Entry entry;
			entry.start = writer.pointer;
			String s = value;
			writer.write({(ref<byte const>)s.data(), s.size()});
			entry.size = writer.pointer - entry.start;
			return entry;
		}

		template <class T>
		Entry add(T const& value) {
			return put(put(value));
		}

		template <class T>
		Entry insert(T const& value) {
			return put(append(value));
		}

		template <class T>
		Entry include(List<T> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(put(value));
			return append(headers);
		}

		template <class T>
		Entry embed(List<T> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(append(value));
			return append(headers);
		}

		template <Type::NoneOf<String, UTF8String, UTF32String> T>
		Entry pack(List<T> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(add(value));
			return append(headers);
		}

		template <class T>
		Entry pack(List<List<T>> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(insert(value));
			return append(headers);
		}

		template <Type::OneOf<String, UTF8String, UTF32String> T>
		Entry pack(List<T> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(insert(value));
			return append(headers);
		}

		Encoder& begin() {
			put(Entry(0, sizeof(FileStructure)));
			return *this;
		}


		template <Type::Functional<void(Encoder&)> TFunc>
		Encoder& run(TFunc const& func) {
			func(*this);
			return *this;
		}

		template <Type::Functional<Entry(Encoder&)> TFunc>
		Entry process(TFunc const& func) {
			return func(*this);
		}

		template <Type::Functional<Entry(Encoder&)> TFunc>
		Entry processIf(bool const cond, TFunc const& func) {
			if (!cond) return {};
			return func(*this);
		}

		Encoder& end() {
			Entry fin = put(file);
			writer.go(0);
			writer.put(fin);
			return *this;
		}
	};
}

CTL_EX_NAMESPACE_END

#endif
