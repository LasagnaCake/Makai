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

	/// @brief Constructs a dynamic matrix from a `Vector2`.
	/// @param vec Vector to construct from.
	constexpr DynamicMatrix(Vector2 const& vec): DynamicMatrix(2, 1) {
		at(0)[0] = vec.x;
		at(0)[1] = vec.y;
	}

	/// @brief Constructs a dynamic matrix from a `Vector3`.
	/// @param vec Vector to construct from.
	constexpr DynamicMatrix(Vector3 const& vec): DynamicMatrix(3, 1) {
		at(0)[0] = vec.x;
		at(0)[1] = vec.y;
		at(0)[2] = vec.z;
	}

	/// @brief Constructs a dynamic matrix from a `Vector4`.
	/// @param vec Vector to construct from.
	constexpr DynamicMatrix(Vector4 const& vec): DynamicMatrix(4, 1) {
		at(0)[0] = vec.x;
		at(0)[1] = vec.y;
		at(0)[2] = vec.z;
		at(0)[3] = vec.w;
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
	constexpr SingleColumnType<DataType> operator[](usize const index) {
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

	/// @brief Returns the amount of cells in the matrix.
	/// @return Cell count.
	constexpr auto size() const			{return matrix.size();	}
	
	/// @brief Returns whether the matrix does not contain cells.
	/// @return Whether matrix does not contain cells.
	constexpr bool empty() const		{return matrix.empty();	}

	/// @brief Returns the amount of cells in each row.
	/// @return Row size.
	constexpr usize rowSize() const		{return rows;			}
	/// @brief Returns the amount of cells in each column.
	/// @return Column size.
	constexpr usize columnSize() const	{return columns;		}

	/// @brief Return the last column of the matrix as a `Vector2`.
	/// @return Last column.
	/// @throw Error::FailedAction if row count is less than 2.
	constexpr Vector2 toVector2() const {
		if (rows < 2)
			throw Error::FailedAction("Matrix is not a valid representation of a 2D vector!", CTL_CPP_PRETTY_SOURCE);
		if (rows == 4) return Vector2((*this)[columns-1][0], (*this)[columns-1][1]) / (*this)[columns-1][3];
		else return Vector2((*this)[columns-1][0], (*this)[columns-1][1]);
	}

	/// @brief Return the last column of the matrix as a `Vector3`.
	/// @return Last column.
	/// @throw Error::FailedAction if row count is less than 3.
	constexpr Vector3 toVector3() const {
		if (rows < 3)
			throw Error::FailedAction("Matrix is not a valid representation of a 3D vector!", CTL_CPP_PRETTY_SOURCE);
		if (rows == 4) return Vector3((*this)[columns-1][0], (*this)[columns-1][1], (*this)[columns-1][2]) / (*this)[columns-1][3];
		else return Vector3((*this)[columns-1][0], (*this)[columns-1][1], (*this)[columns-1][2]);
	}

	/// @brief Return the last column of the matrix as a `Vector4`.
	/// @return Last column.
	/// @throw Error::FailedAction if row count is less than 4.
	constexpr Vector4 toVector4() const {
		if (rows < 4)
			throw Error::FailedAction("Matrix is not a valid representation of a 4D vector!", CTL_CPP_PRETTY_SOURCE);
		return Vector4((*this)[columns-1][0], (*this)[columns-1][1], (*this)[columns-1][2], (*this)[columns-1][3]);
	}

	constexpr explicit operator Vector2() const {return toVector2();}
	constexpr explicit operator Vector3() const {return toVector3();}
	constexpr explicit operator Vector4() const {return toVector4();}

	constexpr DynamicMatrix operator+(DynamicMatrix const& other) const {
		if (!canAddOrSubtractWith(other))
			throw Error::FailedAction(
				"Cannot add matrices of different sizes!",
				toString(
					"Matrix A(r, c) -> [",rows,", ",columns,"]\n"
					"Matrix B(r, c) -> [",other.rows,", ",other.columns,"]"
				)
			);
		DynamicMatrix result = DynamicMatrix(rows, columns);
		for (usize i = 0; i < size(); ++i)
			result.matrix[i] = matrix[i] + other.matrix[i];
	}

	constexpr bool canAddOrSubtractWith(DynamicMatrix const& other) const {
		return size() == other.size();
	}

	constexpr bool canMultiplyWith(DynamicMatrix const& other) const {
		return rows == other.columns;
	}

private:
	/// @brief Row count.
	usize			rows	= 0;
	/// @brief Column count.
	usize			columns	= 0;
	/// @brief underlying matrix.
	MatrixType<>	matrix;
};

}

CTL_EX_NAMESPACE_END

#endif