#ifndef CTL_EX_DATA_RECORD_H
#define CTL_EX_DATA_RECORD_H

#include "../../ctl/container/lists/lists.hpp"
#include "../../ctl/exnamespace.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Data {
	template <Type::DefaultConstructible TData>
	struct Record {
		using DataType = TData;

		using StackType = List<DataType>;

		constexpr Record() {
			stack.pushBack({});
		}

		template <Type::CanBecome<DataType> T>
		constexpr Record(T value) {stack.pushBack(value);}

		constexpr Record(Record const&)	= default;
		constexpr Record(Record&&)		= default;

		constexpr Record& undo() {
			if (current > 1)
				--current;
			return *this;
		}

		constexpr Record& redo() {
			if (current < stack.size())
				++current;
			return *this;
		}

		StackType	history() const	{return stack.withoutRange(current, -1);	}
		StackType	state() const	{return stack;								}
		usize		size() const	{return current;							}

		template <Type::CanBecome<DataType> T>
		constexpr Record& operator=(T const& value) {return set(value);}

		template <Type::CanBecome<DataType> T>
		constexpr Record& set(T const& value) {
			if (current != stack.size())
				stack.resize(current++);
			stack.pushBack(value);
			return *this;
		}

		constexpr DataType get() const {
			return stack[current];
		}

		constexpr operator DataType() const {return get();}

		constexpr Record& operator=(Record const&)	= default;
		constexpr Record& operator=(Record&&)		= default;
	private:
		usize		current = 1;
		StackType	stack;
	};
}

CTL_EX_NAMESPACE_END

#endif