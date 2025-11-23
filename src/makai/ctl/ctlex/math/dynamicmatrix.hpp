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

	/// @brief Constructs the matrix from another matrix.
	/// @param matrix Matrix to construct from.
	constexpr DynamicMatrix(MatrixType<> const matrix): matrix(matrix) {}
	
	/// @brief Constructs the matrix as a given size, with a given value in the diagonal.
	/// @param rows Row count.
	/// @param rows Column count.
	/// @param value Diagonal value. By default, it is the default-constructed element type.
	constexpr explicit DynamicMatrix(usize const rows, usize const columns, DataType const& value = DataType()) {
		matrix.resize(rows * columns);
		for (usize i = 0; i < rows; ++i) {
			if (i >= columns) break;
			SingleColumnType<> col{matrix.data() + i * columns, matrix.data() + ((i + 1) * columns - 1)};
			col[i] = value;
		}
	}

	constexpr DynamicMatrix operator+(DynamicMatrix const& other) const {

	}

private:
	MatrixType<> matrix;
};

}

CTL_EX_NAMESPACE_END

#endif