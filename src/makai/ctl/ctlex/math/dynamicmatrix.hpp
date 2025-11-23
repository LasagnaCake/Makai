#ifndef CTL_EX_MATH_DYNAMICMATRIX_H
#define CTL_EX_MATH_DYNAMICMATRIX_H

#include "vector.hpp"
#include "matrix.hpp"
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
	
	/// @brief Constructs the matrix as a given size, with a given value in the diagonal.
	/// @param rows Row count.
	/// @param rows Column count.
	/// @param value Diagonal value. By default, it is the default-constructed element type.
	template<usize R, usize C>
	constexpr explicit DynamicMatrix(Matrix<R, C, DataType> const& matrix):
		rows(R),
		columns(C) {
		matrix.resize(rows * columns);
		for (usize r = 0; r < rows; ++r)
			for (usize c = 0; c < columns; ++c)
				at(c)[r] = matrix[c][r];
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

	/// @brief Returns the dynamic matrix as a `Matrix`.
	/// @tparam R Row count.
	/// @tparam C Column count.
	/// @return Matrix as static matrix.
	/// @throw Error::FailedAction if sizes do not match.
	template<usize R, usize C>
	constexpr Matrix<R, C, DataType> toMatrix() const {
		Matrix<R, C, DataType> result;
		if (rows < R || columns < C)
			throw Error::FailedAction(
				"Dynamic matrix cannot be converted to the given matrix type!",
				toString(
					"Matrix From(r, c) -> [",rows,", ",columns,"]\n"
					"Matrix To(r, c) -> [",R,", ",C,"]"
				),
				CTL_CPP_PRETTY_SOURCE
			);
		for (usize r = 0; r < rows; ++r)
			for (usize c = 0; c < columns; ++c)
				result[c][r] = at(c)[r];
		return result;
	}

	/// @brief Converts the matrix to a `Vector2`.
	constexpr explicit operator Vector2() const					{return toVector2();		}
	/// @brief Converts the matrix to a `Vector3`.
	constexpr explicit operator Vector3() const					{return toVector3();		}
	/// @brief Converts the matrix to a `Vector4`.
	constexpr explicit operator Vector4() const					{return toVector4();		}
	/// @brief Converts the matrix to a `Matrix`.
	/// @tparam R Row count.
	/// @tparam C Column count.
	template<usize R, usize C>
	constexpr explicit operator Matrix<R, C, DataType>() const	{return toMatrix<R, C>();	}

	constexpr DynamicMatrix& operator+=(DataType const& other) {
		for (auto& cell: matrix)
			cell += other;
		return *this;
	}
	
	constexpr DynamicMatrix& operator-=(DataType const& other) {
		for (auto& cell: matrix)
			cell -= other;
		return *this;
	}
	
	constexpr DynamicMatrix& operator*=(DataType const& other) {
		for (auto& cell: matrix)
			cell *= other;
		return *this;
	}
	
	constexpr DynamicMatrix& operator/=(DataType const& other) {
		for (auto& cell: matrix)
			cell /= other;
		return *this;
	}

	constexpr DynamicMatrix operator+(DataType const& other) const {return copy(*this) += other;}
	constexpr DynamicMatrix operator-(DataType const& other) const {return copy(*this) -= other;}
	constexpr DynamicMatrix operator*(DataType const& other) const {return copy(*this) *= other;}
	constexpr DynamicMatrix operator/(DataType const& other) const {return copy(*this) /= other;}

	constexpr DynamicMatrix operator+(DynamicMatrix const& other) const {
		if (!canAddOrSubtractWith(other))
			throw Error::FailedAction(
				"Cannot add matrices of different sizes!",
				toString(
					"Matrix A(r, c) -> [",rows,", ",columns,"]\n"
					"Matrix B(r, c) -> [",other.rows,", ",other.columns,"]"
				),
				CTL_CPP_PRETTY_SOURCE
			);
		DynamicMatrix result = DynamicMatrix(rows, columns);
		for (usize i = 0; i < size(); ++i)
			result.matrix[i] = matrix[i] + other.matrix[i];
		return result;
	}
	
	constexpr DynamicMatrix operator-(DynamicMatrix const& other) const {
		if (!canAddOrSubtractWith(other))
			throw Error::FailedAction(
				"Cannot subtract matrices of different sizes!",
				toString(
					"Matrix A(r, c) -> [",rows,", ",columns,"]\n"
					"Matrix B(r, c) -> [",other.rows,", ",other.columns,"]"
				),
				CTL_CPP_PRETTY_SOURCE
			);
		DynamicMatrix result = DynamicMatrix(rows, columns);
		for (usize i = 0; i < size(); ++i)
			result.matrix[i] = matrix[i] - other.matrix[i];
		return result;
	}
	
	constexpr DynamicMatrix operator*(DynamicMatrix const& other) const {
		if (!canMultiplyWith(other))
			throw Error::FailedAction(
				"Invalid matrix multiplication!",
				toString(
					"Matrix A(r, c) -> [",rows,", ",columns,"]\n"
					"Matrix B(r, c) -> [",other.rows,", ",other.columns,"]\n"
					"A(r) != B(c)!"
				),
				CTL_CPP_PRETTY_SOURCE
			);
		DynamicMatrix result = DynamicMatrix(rows, other.columns, 0);
		for (usize i = 0; i < rows; i++)
			for (usize j = 0; j < other.columns; j++) {
				result[j][i] = 0;
				for (usize k = 0; k < columns; k++)
					result[j][i] += at(k)[i] * other[j][k];
			}
		return result;
	}

	constexpr DynamicMatrix& operator+=(DynamicMatrix const& other) {return operator=(operator+(other));}
	constexpr DynamicMatrix& operator-=(DynamicMatrix const& other) {return operator=(operator-(other));}
	constexpr DynamicMatrix& operator*=(DynamicMatrix const& other) {return operator=(operator*(other));}

	constexpr bool canAddOrSubtractWith(DynamicMatrix const& other) const {
		return rows == other.rows && columns == other.columns;
	}

	constexpr bool canMultiplyWith(DynamicMatrix const& other) const {
		return rows == other.columns;
	}

	/// @brief Transposes the matrix.
	/// @return Reference to self.
	/// @note Requires matrix to be a square matrix.
	constexpr DynamicMatrix& transpose() {
		return operator=(transposed());
	}

	/// @brief Returns the transposed version of the matrix.
	/// @return Transposed matrix.
	constexpr DynamicMatrix transposed() const {
		DynamicMatrix tmat = DynamicMatrix(columns, rows, 0);
		for (usize r = 0; r < rows; ++r)
			for (usize c = 0; c < columns; ++c)
				tmat[r][c] = at(c)[r];
	}

	/// @brief Returns the determinant.
	/// @return Matrix's determinant.
	/// @throw Error::FailedAction if matrix is not a square matrix.
	constexpr DataType determinant() const {
		if (rows != columns)
			throw Error::FailedAction("Matrix is not a square matrix!", CTL_CPP_PRETTY_SOURCE);
		if (columns == 1)		return at(0)[0];
		else if (columns == 2)	return at(0)[0] * at(1)[1] - at(1)[0] * at(0)[1];
		else if (columns == 3)	return (
			(at(0)[0] * at(1)[1] * at(2)[2])
		+	(at(1)[0] * at(2)[1] * at(0)[2])
		+	(at(2)[0] * at(0)[1] * at(1)[2])
		-	(at(2)[0] * at(1)[1] * at(0)[2])
		-	(at(0)[0] * at(2)[1] * at(1)[2])
		-	(at(1)[0] * at(0)[1] * at(2)[2])
		);
		else {
			DataType res = 0;
			for (usize i = 0; i < rows; i++) {
				if (at(i)[0] == 0) continue;
				res += at(i)[0] * cofactor(i, 0);
			}
			return res;
		}
	}

	/// @brief Returns the matrix's cofactors.
	/// @return Matrix's cofactors.
	/// @throw Error::FailedAction if matrix is not a square matrix.
	constexpr DynamicMatrix cofactors() const {
		if (rows != columns)
			throw Error::FailedAction("Matrix is not a square matrix!", CTL_CPP_PRETTY_SOURCE);
		DynamicMatrix result;
		for (usize i = 0; i < rows; i++)
			for (usize j = 0; j < columns; j++)
				result[i][j] = cofactor(i, j);
		return result;
	}
	
	/// @brief Returns the cofactor for a given cell.
	/// @param row Cell row.
	/// @param col Cell column.
	/// @return Cell's cofactor.
	constexpr DataType cofactor(usize const row, usize const col) const {
		return ((((row + col) % 2) == 0) ? DataType(+1) : DataType(-1)) * truncated(row, col).determinant();
	}

	/// @brief Returns the matrix, with both a given row and a given column removed.
	/// @param row Row to remove.
	/// @param col Column to remove.
	/// @return Truncated matrix.
	/// @throw Error::FailedAction if matrix is 1-dimensional.
	constexpr DynamicMatrix truncated(usize const row, usize const col) const {
		if (!(rows > 1 && columns > 1))
			throw Error::FailedAction("Cannot truncate a 1-dimensional matrix!", CTL_CPP_PRETTY_SOURCE);
		DynamicMatrix result;
		int ro = 0, co = 0;
		for (usize i = 0; i < columns; i++) {
			ro = 0;
			if (i == col || i == (columns-1)) {co--; continue;}
			for (usize j = 0; j < rows; j++) {
				if (j == row || j == (rows-1)) {ro--; continue;}
				result[i+co][j+ro] = at(i)[j];
			}
		}
		return result;
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