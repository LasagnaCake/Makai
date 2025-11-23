#ifndef CTL_EX_MATH_DYNAMICMATRIX_H
#define CTL_EX_MATH_DYNAMICMATRIX_H

#include "vector.hpp"
#include "../../ctl/exnamespace.hpp"
#include "../../ctl/math/core.hpp"
#include "../../ctl/container/lists/list.hpp"
#include "../../ctl/container/error.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace Math {

/// @brief Dynamic matrix. Column-major.
/// @tparam TData Matrix element type.
template<CTL::Type::Math::Operatable TData>
struct DynamicMatrix {
	/// @brief Matrix element type.
	using DataType	= TData;

	/// @brief Matrix storage type.
	/// @tparam T Element type.
	template<class T = DataType>
	using MatrixType		= As<List<TData>>;
	/// @brief Single matrix column type.
	/// @tparam T Element type.
	template<class T = DataType>
	using SingleColumnType	= As<Span<TData>>;

	/// @brief Default constructor.
	constexpr DynamicMatrix() {}
	
	/// @brief Copy constructor (defaulted).
	constexpr DynamicMatrix(DynamicMatrix const&)	= default;
	/// @brief Move constructor (defaulted).
	constexpr DynamicMatrix(DynamicMatrix&&)		= default;
	
	/// @brief Constructs the matrix as a given size, with a given value in the diagonal.
	/// @param rows Row count.
	/// @param rows Column count.
	/// @param value Diagonal value. By default, it is the default-constructed element type.
	constexpr explicit DynamicMatrix(usize const rows, usize const columns, DataType const& value = DataType()):
		rows(rows),
		columns(columns) {
		matrix.resize(rows * columns);
		for (usize i = 0; i < rows; ++i) {
			if (i >= columns) break;
			auto col = at(i);
			col[i] = value;
		}
	}

	/// @brief Returns the row at the given column.
	/// @param index Index to match.
	/// @return Row view.
	/// @throws Error::InvalidValue if `index` is bigger than column count.
	constexpr SingleColumnType<DataType> at(usize const index) {
		if (index > columns)
			throw Error::InvalidValue(
				toString("Index of [", columns, "] is larger than column count of [",columns,"]!"),
				CTL_CPP_PRETTY_SOURCE
			);
		return {matrix.begin() + index * columns, matrix.begin() + ((index + 1) * columns - 1)};
	}

	/// @brief Returns the row at the given column.
	/// @param index Index to match.
	/// @return Row view.
	/// @throws Error::InvalidValue if `index` is bigger than column count.
	constexpr SingleColumnType<const DataType> at(usize const index) const {
		if (index > columns)
			throw Error::InvalidValue(
				toString("Index of [", columns, "] is larger than column count of [",columns,"]!"),
				CTL_CPP_PRETTY_SOURCE
			);
		return {matrix.begin() + index * columns, matrix.begin() + ((index + 1) * columns - 1)};
	}

	/// @brief Array subscription operator.
	/// @param index Index to match.
	/// @return Row view.
	/// @throws Error::InvalidValue if `index` is bigger than column count.
	constexpr SingleColumnType<DataType> operator[](usize const index)  {
		return at(index);
	}

	/// @brief Array subscription operator.
	/// @param index Index to match.
	/// @return Row view.
	/// @throws Error::InvalidValue if `index` is bigger than column count.
	constexpr SingleColumnType<const DataType> operator[](usize const index) const {
		return at(index);
	}

	/// @brief Returns an iterator to the beginning of the matrix.
	/// @return Iterator to beginning of matrix.
	constexpr auto begin()			{return matrix.begin();	}
	/// @brief Returns an iterator to the end of the matrix.
	/// @return Iterator to end of matrix.
	constexpr auto end()			{return matrix.end();	}
	/// @brief Returns an iterator to the beginning of the matrix.
	/// @return Iterator to beginning of matrix.
	constexpr auto begin() const	{return matrix.begin();	}
	/// @brief Returns an iterator to the end of the matrix.
	/// @return Iterator to end of matrix.
	constexpr auto end() const		{return matrix.end();	}

private:
	usize			rows;
	usize			columns;
	MatrixType<>	matrix;
};

}

CTL_EX_NAMESPACE_END

#endif