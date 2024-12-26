#ifndef CTL_EX_MATH_MATRIX_H
#define CTL_EX_MATH_MATRIX_H

#include "vector.hpp"
#include "../../ctl/exnamespace.hpp"
#include "../../ctl/math/core.hpp"

CTL_EX_NAMESPACE_BEGIN

/// @brief Math extensions.
namespace Math {
	template<usize R, usize C, CTL::Type::Math::Operatable T>
	class Matrix;
}

/// @brief Matrix-specific type constraints.
namespace Type::Ex::Math::Matrix {
	/// @brief Type must be a `Matrix`.
	template <class T>
	concept Matrix = requires {
		T::ROWS;
		T::COLUMNS;
		typename T::DataType;
	}
	&&	CTL::Type::Equal<T, CTL::Ex::Math::Matrix<T::ROWS, T::COLUMNS, typename T::DataType>>
	;

	/// @brief Type must be operatable, and must be convertible to `TCompat`.
	template <class T, class TCompat>
	concept Compatitble = CTL::Type::Math::Operatable<TCompat> && CTL::Type::Convertible<T, TCompat>;

	/// @brief Matrix must be a valid `Transform3D`.
	template <usize R, usize C>
	concept ValidTransform3D = (R == 4) && (R == C);

	/// @brief Matrix must be a valid `Transform2D`.
	template <usize R, usize C>
	concept ValidTransform2D = (R == 3) && (R == C);
}

/// @brief Math extensions.
namespace Math {

/// @brief Mathematical matrix. Column-major.
/// @tparam R Row count.
/// @tparam C Column count.
/// @tparam T Matrix element type.
template<usize R, usize C, CTL::Type::Math::Operatable TData>
class Matrix {
public:
	/// @brief Row count.
	constexpr static usize ROWS		= R;
	/// @brief Column count.
	constexpr static usize COLUMNS	= C;

	/// @brief Matrix element type.
	using DataType	= TData;

	/// @brief Single element array type.
	/// @tparam T Element type.
	template<class T = DataType>
	using SingleElementType	= Decay::AsType<TData[1]>;
	/// @brief Matrix storage type.
	/// @tparam T Element type.
	template<class T = DataType>
	using MatrixType		= Decay::AsType<TData[R][C]>;
	/// @brief Value array type.
	/// @tparam T Element type.
	template<class T = DataType>
	using ArrayType			= Decay::AsType<TData[R*C]>;
	/// @brief Single matrix column type.
	/// @tparam T Element type.
	template<class T = DataType>
	using SingleColumnType	= Decay::AsType<TData[C]>;

	/// @brief Self type.
	using SelfType	= Matrix<ROWS, COLUMNS, DataType>;

	static_assert(R > 0, "Matrix row size must not be zero!");
	static_assert(C > 0, "Matrix column size must not be zero!");

	/// @brief Empty constructor.
	constexpr Matrix() {}

	/// @brief Constructs the matrix's main diagonal with a value.
	/// @param v Value to use.
	constexpr Matrix(DataType const& v) {
		usize const start = ::CTL::Math::min(C, R);
		for (usize i = 0; i < start; i++)
			data[start-1-i][start-1-i] = v;
	}

	/// @brief Constructs the matrix's main diagonal with a value.
	/// @param v Value to use.
	constexpr Matrix(SingleElementType<> const& v) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = v[0];
	}

	/// @brief Constructs the matrix with a series of values.
	/// @param v Values to use.
	constexpr Matrix(MatrixType<> const& v) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = v[j][i];
	}

	/// @brief Constructs the matrix with a series of values.
	/// @param v Values to use.
	constexpr Matrix(ArrayType<> const& v) {
		DataType hack[R][C];
		for (usize i = 0; i < R*C; i++)
			((DataType*)hack)[i] = v[i];
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = hack[j][i];
	}

	/// @brief Constructs the matrix's columns with a series of values.
	/// @param v Values to use.
	constexpr Matrix(SingleColumnType<> const& v)
	requires (C > 1) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = v[i];
	}

	/// @brief Constructs the matrix's main diagonal with a value.
	/// @tparam T2 Value type.
	/// @param v Value to use.
	template<Type::Ex::Math::Matrix::Compatitble<DataType> T2>
	constexpr Matrix(SingleElementType<T2> const& v) {
		DataType rv = DataType(v[0]);
		usize const start = ::CTL::Math::min(C, R);
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = rv;
	}

	/// @brief Constructs the matrix with a series of values.
	/// @tparam T2 Value type.
	/// @param v Values to use.
	template<Type::Ex::Math::Matrix::Compatitble<DataType> T2>
	constexpr Matrix(MatrixType<T2> const& v) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = DataType(v[j][i]);
	}

	/// @brief Constructs the matrix with a series of values.
	/// @tparam T2 Value type.
	/// @param v Values to use.
	template<Type::Ex::Math::Matrix::Compatitble<DataType> T2>
	constexpr Matrix(ArrayType<T2> const& v) {
		T2 hack[R][C];
		for (usize i = 0; i < R*C; i++)
			((T2*)hack)[i] = v[i];
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = DataType(hack[j][i]);
	}

	/// @brief Constructs the matrix's columns with a series of values.
	/// @tparam T2 Value type.
	/// @param v Values to use.
	template<Type::Ex::Math::Matrix::Compatitble<DataType> T2>
	constexpr Matrix(SingleColumnType<T2> const& v)
	requires (C > 1) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = DataType(v[i]);
	}

	/// @brief Constructs the matrix's main diagonal from a vector.
	/// @param vec Vector to use.
	constexpr Matrix(Vector2 const& vec) {
		static_assert(R >= 2 && C >= 1, "Matrix is not a valid representation of a 2D vector!");
		data[C-1][R-1] = 1;
		data[C-1][0] = vec.x;
		data[C-1][1] = vec.y;
	}

	/// @brief Constructs the matrix's main diagonal from a vector.
	/// @param vec Vector to use.
	constexpr Matrix(Vector3 const& vec) {
		static_assert(R >= 3 && C >= 1, "Matrix is not a valid representation of a 3D vector!");
		data[C-1][R-1] = 1;
		data[C-1][0] = vec.x;
		data[C-1][1] = vec.y;
		data[C-1][2] = vec.z;
	}

	/// @brief Constructs the matrix's main diagonal from a vector.
	/// @param vec Vector to use.
	constexpr Matrix(Vector4 const& vec) {
		static_assert(R >= 4 && C >= 1, "Matrix is not a valid representation of a 4D vector!");
		data[C-1][R-1] = 1;
		data[C-1][0] = vec.x;
		data[C-1][1] = vec.y;
		data[C-1][2] = vec.z;
		data[C-1][3] = vec.w;
	}

	/// @brief Constructs the matrix from a 3D transformation.
	/// @param trans Transformation to use.
	constexpr Matrix(Transform3D const& trans)
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		compose(trans);
	}

	/// @brief Constructs the matrix from a 2D transformation.
	/// @param trans Transformation to use.
	constexpr Matrix(Transform2D const& trans)
	requires Type::Ex::Math::Matrix::ValidTransform2D<R, C> {
		*this = identity().transform(trans);
	}

	/// @brief Constructs the matrix from a transformation.
	/// @param pos Position.
	/// @param rot Rotation.
	/// @param scale Scale.
	constexpr Matrix(
		Vector3 const& pos,
		Vector3 const& rot,
		Vector3 const& scale
	) requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		compose(pos, rot, scale);
	}

	/// @brief Constructs the matrix from a transformation.
	/// @param trans Transformation to use.
	/// @param perspective Perspective.
	/// @param skew Skew.
	constexpr Matrix(
		Transform3D const& trans,
		Vector4 const& perspective,
		Vector3 const& skew
	) requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		compose(trans, perspective, skew);
	}

	/// @brief Constructs the matrix from a transformation.
	/// @param pos Position.
	/// @param rot Rotation.
	/// @param scale Scale.
	/// @param perspective Perspective.
	/// @param skew Skew.
	constexpr Matrix(
		Vector3 const& pos,
		Vector3 const& rot,
		Vector3 const& scale,
		Vector4 const& perspective,
		Vector3 const& skew
	) requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		compose(pos, rot, scale, perspective, skew);
	}

	/// @brief Copy constructor (`Matrix`).
	/// @param other `Matrix` to copy from.
	constexpr Matrix(SelfType const& other) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = other.data[i][j];
	}

	/// @brief Copy constructor (other `Matrix` type).
	/// @param other `Matrix` to copy from.
	template <typename T2>
	constexpr Matrix(Matrix<R, C, T2> const& other) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = DataType(other.data[i][j]);
	}

	/// @brief Destructor.
	constexpr ~Matrix() {}
	
	/// @brief Fills all cells with a given value.
	/// @param v Value to fill with.
	/// @return Reference to self.
	constexpr SelfType& fill(DataType const& v) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] = v;
		return *this;
	}

	/// @brief Returns the transposed version of the matrix.
	/// @return Transposed matrix.
	constexpr Matrix<C, R, DataType> transposed() const {
		Matrix<C, R, DataType> res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = data[i][j];
		return res;
	}

	/// @brief Transposes the matrix.
	/// @return Reference to self.
	/// @note Requires matrix to be a square matrix.
	constexpr SelfType& transpose() requires (R == C) {
		(*this) = transposed();
		return *this;
	}

	/// @brief Creates an identity matrix.
	/// @return Identity matrix.
	/// @note Requires matrix to be a square matrix.
	constexpr static SelfType identity() requires (R == C) {
		static_assert(C == R, "Matrix is not a square matrix!");
		return SelfType(1);
	}

	/// @brief Creates a matrix, with its bottom-right corner cell set to 0.
	/// @return Prototype matrix.
	/// @note Requires matrix to be a square matrix.
	constexpr static SelfType prototype() requires (R == C) {
		static_assert(C == R, "Matrix is not a square matrix!");
		SelfType res(0);
		res.data[R-1][C-1] = 1;
		return res;
	}

	/// @brief Creates a matrix with its secondary diagonal set to 1, and all other cells set to 0.
	/// @return Mirror matrix.
	/// @note Requires matrix to be a square matrix.
	constexpr static SelfType mirror() requires (R == C) {
		static_assert(C == R, "Matrix is not a square matrix!");
		SelfType res(0);
		for(usize i = 0; i < R; i++)
			res.data[(R-1)-i][i] = 1;
		return res;
	}

	/// @brief Converts a 3D rotation into a rotation matrix.
	/// @param angle Rotation to convert.
	/// @return Rotation matrix.
	/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/gtx/euler_angles.inl
	constexpr static Matrix<4, 4, DataType> fromEulerXYZ(Vector3 const& angle) {
		// Get sines and cosines
		float c1, c2, c3, s1, s2, s3;
		CTL::Math::sincos<float>(-angle.x, s1, c1);
		CTL::Math::sincos<float>(-angle.y, s2, c2);
		CTL::Math::sincos<float>(-angle.z, s3, c3);
		// Formulate matrix
		Matrix<4, 4, DataType> result;
		result[0][0] = c2 * c3;
		result[0][1] =-c1 * s3 + s1 * s2 * c3;
		result[0][2] = s1 * s3 + c1 * s2 * c3;
		result[0][3] = DataType(0);
		result[1][0] = c2 * s3;
		result[1][1] = c1 * c3 + s1 * s2 * s3;
		result[1][2] =-s1 * c3 + c1 * s2 * s3;
		result[1][3] = DataType(0);
		result[2][0] =-s2;
		result[2][1] = s1 * c2;
		result[2][2] = c1 * c2;
		result[2][3] = DataType(0);
		result[3][0] = DataType(0);
		result[3][1] = DataType(0);
		result[3][2] = DataType(0);
		result[3][3] = DataType(1);
		// Return result
		return result;
	}

	/// @brief Converts a 3D rotation into a rotation matrix.
	/// @param angle Rotation to convert.
	/// @return Rotation matrix.
	/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/gtx/euler_angles.inl
	constexpr static Matrix<4, 4, DataType> fromEulerYXZ(Vector3 const& angle) {
		// Get sines and cosines
		float tmp_ch, tmp_sh, tmp_cp, tmp_sp, tmp_cb, tmp_sb;
		CTL::Math::sincos<float>(-angle.y, tmp_sh, tmp_ch);
		CTL::Math::sincos<float>(-angle.x, tmp_sp, tmp_cp);
		CTL::Math::sincos<float>(-angle.z, tmp_sb, tmp_cb);
		// Compute matrix
		Matrix<4, 4, DataType> result;
		result[0][0] = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
		result[0][1] = tmp_sb * tmp_cp;
		result[0][2] = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
		result[0][3] = DataType(0);
		result[1][0] = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
		result[1][1] = tmp_cb * tmp_cp;
		result[1][2] = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
		result[1][3] = DataType(0);
		result[2][0] = tmp_sh * tmp_cp;
		result[2][1] = -tmp_sp;
		result[2][2] = tmp_ch * tmp_cp;
		result[2][3] = DataType(0);
		result[3][0] = DataType(0);
		result[3][1] = DataType(0);
		result[3][2] = DataType(0);
		result[3][3] = DataType(1);
		// Return result
		return result;
	}

	/// @brief Euler function type.
	typedef decltype(fromEulerXYZ)	EulerFunction;

	/// @brief Converts a 3D translation into a translation matrix.
	/// @param vec Translation to convert.
	/// @return Translation matrix.
	constexpr static Matrix<4, 4, DataType> fromTranslation(Vector3 const& vec) {
		return Matrix(1).translated(vec);
	}

	/// @brief Converts a 3D scaling into a translation matrix.
	/// @param vec Scaling to convert.
	/// @return Scaling matrix.
	constexpr static Matrix<4, 4, DataType> fromScale(Vector3 const& vec) {
		return Matrix(1).scaled(vec);
	}

	/// @brief Returns the inverted version of the matrix.
	/// @return Inverted matrix.
	/// @note Requires matrix to be a square matrix.
	constexpr SelfType inverted() const requires (R == C) {
		static_assert(C == R, "Matrix is not a square matrix!");
		DataType det = determinant();
		if (det == DataType(0)) return Matrix(1);
		SelfType res = cofactors().transposed() * (DataType(1) / det);
		return res;
	}

	/// @brief Inverts the matrix.
	/// @return Reference to self.
	/// @note Requires matrix to be a square matrix.
	constexpr SelfType& invert() requires (R == C) {
		static_assert(C == R, "Matrix is not a square matrix!");
		(*this) = inverted();
		return *this;
	}

	/// @brief Returns a translated version of the matrix.
	/// @param vec Translation to apply.
	/// @return Translated matrix.
	/// @note Matrix must be a valid 3D transformation matrix.
	constexpr Matrix<4, 4, DataType> translated(Vector3 const& vec) const
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return Matrix<4, 4, DataType>(data).translate(vec);
	}

	/// @brief Translates the matrix.
	/// @param vec Translation to apply.
	/// @return Reference to self.
	/// @note Matrix must be a valid 3D transformation matrix.
	/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
	constexpr Matrix<4, 4, DataType>& translate(Vector3 const& vec)
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		Vector4 calc =
			(Vector4(data[0]) * vec[0])
		+	(Vector4(data[1]) * vec[1])
		+	(Vector4(data[2]) * vec[2])
		+	Vector4(data[3])
		;
		data[3][0] = calc[0];
		data[3][1] = calc[1];
		data[3][2] = calc[2];
		data[3][3] = calc[3];
		return (*this);
	}

	/// @brief Returns a rotated version of the matrix.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param vec Rotation to apply.
	/// @return Rotated matrix.
	/// @note Matrix must be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr Matrix<4, 4, DataType> rotated(Vector3 const& vec) const
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return (*this) * ANGLE_FUNC(vec);
	}

	/// @brief Rotates the matrix.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param vec Rotation to apply.
	/// @return Reference to self.
	/// @note Matrix must be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr Matrix<4, 4, DataType>& rotate(Vector3 const& vec)
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		(*this) *= ANGLE_FUNC(vec);
		return (*this);
	}

	/// @brief Returns a scaled version of the matrix.
	/// @param vec Scaling to apply.
	/// @return Scaled matrix.
	/// @note Matrix must be a valid 3D transformation matrix.
	/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl 
	constexpr Matrix<4, 4, DataType> scaled(Vector3 const& vec) const
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		Vector4 result[4];
		result[0] = Vector4(data[0]) * vec[0];
		result[1] = Vector4(data[1]) * vec[1];
		result[2] = Vector4(data[2]) * vec[2];
		result[3] = Vector4(data[3]);
		return Matrix<4, 4, DataType>({
			result[0][0], result[1][0], result[2][0], result[3][0],
			result[0][1], result[1][1], result[2][1], result[3][1],
			result[0][2], result[1][2], result[2][2], result[3][2],
			result[0][3], result[1][3], result[2][3], result[3][3]
		});
	}

	/// @brief Scales the matrix.
	/// @param vec Scaling to apply.
	/// @return Reference to self.
	/// @note Matrix must be a valid 3D transformation matrix.
	/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl 
	constexpr Matrix<4, 4, DataType>& scale(Vector3 const& vec)
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		*this = scaled(vec);
		return (*this);
	}

	/// @brief Returns the matrix's cofactors.
	/// @return Matrix's cofactors.
	/// @note Matrix must be a square matrix.
	constexpr SelfType cofactors() const requires (R == C) {
		static_assert(C == R, "Matrix is not a square matrix!");
		SelfType res;
		for (usize i = 0; i < R; i++)
			for (usize j = 0; j < C; j++)
				res[i][j] = cofactor(i, j);
		return res;
	}

	/// @brief Returns the determinant.
	/// @return Matrix's determinant.
	/// @note Matrix must be a square matrix.
	constexpr DataType determinant() const requires (R == C) {
		static_assert(C == R, "Matrix is not a square matrix!");
		if constexpr(C == 1)		return data[0][0];
		else if constexpr(C == 2)	return data[0][0] * data[1][1] - data[1][0] * data[0][1];
		else if constexpr(C == 3)	return (
			(data[0][0] * data[1][1] * data[2][2])
		+	(data[1][0] * data[2][1] * data[0][2])
		+	(data[2][0] * data[0][1] * data[1][2])
		-	(data[2][0] * data[1][1] * data[0][2])
		-	(data[0][0] * data[2][1] * data[1][2])
		-	(data[1][0] * data[0][1] * data[2][2])
		);
		else {
			DataType res = 0;
			for (usize i = 0; i < R; i++) {
				if (data[i][0] == 0) continue;
				res += data[i][0] * cofactor(i, 0);
			}
			return res;
		}
	}

	/// @brief Returns the matrix, with both a given row and a given column removed.
	/// @param row Row to remove.
	/// @param col Column to remove.
	/// @return Truncated matrix.
	constexpr Matrix<R-1, C-1, DataType> truncated(usize const row, usize const col) const {
		static_assert(R > 1 && C > 1, "Cannot truncate a 1-dimensional matrix!");
		Matrix<R-1, C-1, DataType> res;
		int ro = 0, co = 0;
		for (usize i = 0; i < C; i++) {
			ro = 0;
			if (i == col || i == (C-1)) {co--; continue;}
			for (usize j = 0; j < R; j++) {
				if (j == row || j == (R-1)) {ro--; continue;}
				res[i+co][j+ro] = data[i][j];
			}
		}
		return res;
	}
	
	/// @brief Returns the matrix, with a set of rows and columns removed.
	/// @tparam RF Row count to remove.
	/// @tparam CF Column count to remove.
	/// @param rowStart Row to start copying data from.
	/// @param colStart Column to start copying data from.
	/// @return
	///		Shrunk matrix.
	///		Returns a zero matrix
	///		if `rowStart` is bigger than row count minus `RF`,
	///		or if `columnStart` is bigger than column count minus `CF`.
	template<usize RF = 1, usize CF = 1>
	constexpr Matrix<R-RF, C-CF, DataType> shrunkBy(usize const rowStart = 0, usize const colStart = 0) const {
		static_assert(R > 1 && C > 1, "Cannot shrink a 1-dimensional matrix!");
		static_assert(RF >= R && CF >= C, "Shrinking factor(s) are bigger than the matrix!");
		if (!(rowStart < (R-RF) && colStart < (C-CF))) Matrix<R-RF, C-CF, DataType>(0);
		Matrix<R-RF, C-CF, DataType> res;
		for (usize i = 0; i < C-CF; i++)
			for (usize j = 0; j < R-RF; j++)
				res[i][j] = data[i+colStart][j+rowStart];
		return res;
	}

	/// @brief Returns the matrix, with an added set of rows and columns.
	/// @tparam RF Row count to add.
	/// @tparam CF Column count to add.
	/// @param rowStart Row to start inserting data in the new matrix.
	/// @param colStart Column to start inserting data in the new matrix.
	/// @return Expanded matrix. 
	/// @return
	///		Shrunk matrix.
	///		Returns a zero matrix
	///		if `rowStart` is bigger than `RF`,
	///		or if `columnStart` is bigger than `CF`.
	template<usize RF = 1, usize CF = 1>
	constexpr Matrix<R+RF, C+CF, DataType> expandedBy(usize const rowStart = 0, usize const colStart = 0) const {
		if (!(rowStart < RF && colStart < CF)) return Matrix<R+RF, C+CF, DataType>(0);
		Matrix<R+RF, C+CF, DataType> res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[i+colStart][j+rowStart] = data[i][j];
		return res;
	}

	/// @brief Unary plus operator.
	/// @return Copy of self.
	constexpr SelfType operator+() const {return *this;}

	/// @brief Addition operator (value).
	/// @param val Value to add to all cells.
	/// @return Result of the operation.
	constexpr SelfType operator+(DataType const& val) const {
		SelfType res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = data[i][j] + val;
		return res;
	}

	/// @brief Addition operator (`Matrix`).
	/// @param mat `Matrix` to add.
	/// @return Result of the operation.
	constexpr SelfType operator+(SelfType const& mat) const {
		SelfType res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = data[i][j] + mat.data[i][j];
		return res;
	}

	/// @brief Addition operator (`Vector2`)
	/// @param vec Vector to add.
	/// @return Result of the operation.
	/// @brief Requires matrix to be 2 x 1.
	constexpr SelfType operator+(Vector2 const& vec) const {
		static_assert(R == 2 && C == 1, "Matrix size is invalid!");
		return (*this) + SelfType(vec);
	}

	/// @brief Addition operator (`Vector3`)
	/// @param vec Vector to add.
	/// @return Result of the operation.
	/// @brief Requires matrix to be 3 x 1.
	constexpr SelfType operator+(Vector3 const& vec) const {
		static_assert(R == 3 && C == 1, "Matrix size is invalid!");
		return (*this) + SelfType(vec);
	}

	/// @brief Addition operator (`Vector4`)
	/// @param vec Vector to add.
	/// @return Result of the operation.
	/// @brief Requires matrix to be 4 x 1.
	constexpr SelfType operator+(Vector4 const& vec) const {
		static_assert(R == 4 && C == 1, "Matrix size is invalid!");
		return (*this) + SelfType(vec);
	}

	/// @brief Unary minus operator.
	/// @return Negated matrix.
	constexpr SelfType operator-() const {
		SelfType res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = -data[i][j];
		return res;
	}

	/// @brief Subtraction operator (value).
	/// @param val Value to subtract from all cells.
	/// @return Result of the operation.
	constexpr SelfType operator-(DataType const& val) const {
		SelfType res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = data[i][j] - val;
		return res;
	}

	/// @brief Subtraction operator (`Matrix`).
	/// @param mat `Matrix` to subtract.
	/// @return Result of the operation.
	constexpr SelfType operator-(SelfType const& mat) const {
		SelfType res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = data[i][j] - mat.data[i][j];
		return res;
	}

	/// @brief Subtraction operator (`Vector2`)
	/// @param vec Vector to subtract.
	/// @return Result of the operation.
	/// @brief Requires matrix to be 2 x 1.
	constexpr SelfType operator-(Vector2 const& vec) const {
		static_assert(R == 2 && C == 1, "Matrix size is invalid!");
		return (*this) - SelfType(vec);
	}

	/// @brief Subtraction operator (`Vector3`)
	/// @param vec Vector to subtract.
	/// @return Result of the operation.
	/// @brief Requires matrix to be 3 x 1.
	constexpr SelfType operator-(Vector3 const& vec) const {
		static_assert(R == 3 && C == 1, "Matrix size is invalid!");
		return (*this) - SelfType(vec);
	}

	/// @brief Subtraction operator (`Vector4`)
	/// @param vec Vector to subtract.
	/// @return Result of the operation.
	/// @brief Requires matrix to be 4 x 1.
	constexpr SelfType operator-(Vector4 const& vec) const {
		static_assert(R == 4 && C == 1, "Matrix size is invalid!");
		return (*this) - SelfType(vec);
	}

	/// @brief Multiplication operator (value).
	/// @param val Value to multiply to all cells.
	/// @return Result of the operation.
	constexpr SelfType operator*(DataType const& val) const {
		SelfType res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = data[i][j] * val;
		return res;
	}

	/// @brief Multiplication operator (`Matrix`)
	/// @tparam C2 Column count of the matrix to multiply with.
	/// @param mat Matrix to multiply.
	/// @return Result of the operation.
	template<usize C2>
	constexpr Matrix<R, C2, DataType> operator*(Matrix<C, C2, DataType> const& mat) const {
		Matrix<R, C2, DataType> res;
		for (usize i = 0; i < R; i++)
			for (usize j = 0; j < C2; j++) {
				res.data[j][i] = 0;
				for (usize k = 0; k < C; k++)
					res.data[j][i] += data[k][i] * mat.data[j][k];
			}
		return res;
	}

	/// @brief Multiplication operator (`Vector2`)
	/// @param vec Vector to multiply.
	/// @return Result of the operation.
	/// @note Requires row count to be 2.
	constexpr Matrix<R, 1, DataType> operator*(Vector2 const& vec) const {
		static_assert(R == 2, "Matrix row count is invalid!");
		return (*this) * Matrix<R, 1, DataType>(vec);
	}

	/// @brief Multiplication operator (`Vector3`)
	/// @param vec Vector to multiply.
	/// @return Result of the operation.
	/// @note Requires row count to be 3.
	constexpr Matrix<R, 1, DataType> operator*(Vector3 const& vec) const {
		static_assert(R == 3, "Matrix row count is invalid!");
		return (*this) * Matrix<R, 1, DataType>(vec);
	}

	/// @brief Multiplication operator (`Vector4`)
	/// @param vec Vector to multiply.
	/// @return Result of the operation.
	/// @note Requires row count to be 4.
	constexpr Matrix<R, 1, DataType> operator*(Vector4 const& vec) const {
		static_assert(R == 4, "Matrix row count is invalid!");
		return (*this) * Matrix<R, 1, DataType>(vec);
	}

	/// @brief Multiplication operator (`Transform3D`)
	/// @param vec Vector to multiply.
	/// @return Result of the operation.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	constexpr SelfType operator*(Transform3D const& trans) const
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C>  {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return (*this) * SelfType(trans);
	}

	/// @brief Division operator (value).
	/// @param val Value to divide from all cells.
	/// @return Result of the operation.
	constexpr SelfType operator/(DataType const& val) const {
		SelfType res;
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				res[j][i] = data[i][j] / val;
		return res;
	}

	/*template<usize C2>
	constexpr Matrix<R, C2, DataType> operator/(Matrix<C, C2, DataType> const& mat) const {
		return (*this) * mat.inverted();
	}*/

	/// @brief Assignment operator (matrix-like).
	/// @param v Values to assign.
	/// @return Reference to self.
	constexpr SelfType& operator=(MatrixType<> const& v) {
		for (usize i = 0; i < C; i++) {
			for (usize j = 0; j < R; j++)
				data[i][j] = DataType(v[i][j]);
		}
	}

	/// @brief Assignment operator (array of values).
	/// @param v Values to assign.
	/// @return Reference to self.
	constexpr SelfType& operator=(ArrayType<> const& v) {
		for (usize i = 0; i < R*C; i++)
			((DataType*)data)[i] = DataType(v[i]);
	}

	/// @brief Addition assignment operator (value).
	/// @param val Value to add to all cells.
	/// @return Reference to self.
	constexpr SelfType& operator+=(DataType const& val) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] += val;
		return *this;
	}

	/// @brief Addition assignment operator (`Matrix`).
	/// @param mat Matrix to add.
	/// @return Reference to self.
	constexpr SelfType& operator+=(SelfType const& mat) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] += mat.data[i][j];
		return *this;
	}

	/// @brief Addition assignment operator (`Vector2`).
	/// @param vec Vector to add.
	/// @return Reference to self.
	/// @note Requires matrix to be 2 x 1.
	constexpr SelfType& operator+=(Vector2 const& vec) {
		static_assert(R == 2 && C == 1, "Matrix size is invalid!");
		return (*this) += SelfType(vec);
	}

	/// @brief Addition assignment operator (`Vector3`).
	/// @param vec Vector to add.
	/// @return Reference to self.
	/// @note Requires matrix to be 3 x 1.
	constexpr SelfType& operator+=(Vector3 const& vec) {
		static_assert(R == 3 && C == 1, "Matrix size is invalid!");
		return (*this) += SelfType(vec);
	}

	/// @brief Addition assignment operator (`Vector4`).
	/// @param vec Vector to add.
	/// @return Reference to self.
	/// @note Requires matrix to be 4 x 1.
	constexpr SelfType& operator+=(Vector4 const& vec) {
		static_assert(R == 4 && C == 1, "Matrix size is invalid!");
		return (*this) += SelfType(vec);
	}

	/// @brief Subtraction assignment operator (value).
	/// @param val Value to subtract from all cells.
	/// @return Reference to self.
	constexpr SelfType& operator-=(DataType const& val) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] -= val;
		return *this;
	}

	/// @brief Subtraction assignment operator (`Matrix`).
	/// @param mat Matrix to subtract.
	/// @return Reference to self.
	constexpr SelfType& operator-=(SelfType const& mat) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] -= mat.data[i][j];
		return *this;
	}

	/// @brief Subtraction assignment operator (`Vector2`).
	/// @param vec Vector to subtract.
	/// @return Reference to self.
	/// @note Requires matrix to be 2 x 1.
	constexpr SelfType& operator-=(Vector2 const& vec) {
		static_assert(R == 2 && C == 1, "Matrix size is invalid!");
		return (*this) -= SelfType(vec);
	}

	/// @brief Subtraction assignment operator (`Vector3`).
	/// @param vec Vector to subtract.
	/// @return Reference to self.
	/// @note Requires matrix to be 3 x 1.
	constexpr SelfType operator-=(Vector3 const& vec) {
		static_assert(R == 3 && C == 1, "Matrix size is invalid!");
		return (*this) -= SelfType(vec);
	}

	/// @brief Subtraction assignment operator (`Vector4`).
	/// @param vec Vector to subtract.
	/// @return Reference to self.
	/// @note Requires matrix to be 4 x 1.
	constexpr SelfType& operator-=(Vector4 const& vec) {
		static_assert(R == 4 && C == 1, "Matrix size is invalid!");
		return (*this) -= SelfType(vec);
	}

	/// @brief Multiplication assignment operator (value).
	/// @param val Value to mulitply to all cells.
	/// @return Reference to self.
	constexpr SelfType& operator*=(DataType const& val) {
		for (usize i = 0; i < C; i++)
			for (usize j = 0; j < R; j++)
				data[i][j] *= val;
		return *this;
	}

	/// @brief Multiplication assignment operator (`Matrix`).
	/// @param mat Matrix to multiply.
	/// @return Reference to self.
	/// @note Requires a square matrix.
	constexpr SelfType& operator*=(SelfType const& mat)
	requires (R == C) {
		(*this) = (*this) * mat;
		return *this;
	}

	/// @brief Multiplication assignment operator (`Transform3D`).
	/// @param trans Tranform to multiply.
	/// @return Reference to self.
	/// @note Requires matrix to be a valid 3D transformation matrix. 
	constexpr SelfType& operator*=(Transform3D const& trans)
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return (*this) *= SelfType(trans);
	}

	/// @brief Division assignment operator (value).
	/// @param val Value to divide from all cells.
	/// @return Reference to self.
	constexpr SelfType& operator/=(DataType const& val) {
		for (usize i = 0; i < R; i++)
			for (usize j = 0; j < C; j++)
				data[i][j] /= val;
		return *this;
	}

	/*template<usize C2>
	constexpr SelfType& operator/=(Matrix<R, C2, DataType> const& mat)
	requires (R == C) {
		(*this) = (*this) / mat;
		return *this;
	}*/

	/// @brief Array subscription operator.
	/// @param idx Index to match.
	/// @return Row view.
	/// @throws Error::InvalidValue if `idx` is bigger than column count.
	constexpr Span<DataType, R> operator[](usize const idx)	{
		if (idx > C)
			throw Error::InvalidValue(
				toString("Index of [", idx, "] is larger than column count of [",C,"]!"),
				CTL_CPP_PRETTY_SOURCE
			);
		return Span<DataType, R>(data[idx]);
	}

	
	/// @brief Array subscription operator.
	/// @param idx Index to match.
	/// @return Row view.
	/// @throws Error::InvalidValue if `idx` is bigger than column count.
	constexpr Span<const DataType, R> operator[](usize const idx) const	{
		if (idx > C)
			throw Error::InvalidValue(
				toString("Index of [", idx, "] is larger than column count of [",C,"]!"),
				CTL_CPP_PRETTY_SOURCE
			);
		return Span<const DataType, R>(data[idx]);
	}

	/// @brief Returns an iterator to the beginning of the matrix.
	/// @return Iterator to beginning of matrix.
	constexpr Iterator<DataType> begin()	{return &data[0][0];		}
	/// @brief Returns an iterator to the end of the matrix.
	/// @return Iterator to end of matrix.
	constexpr Iterator<DataType> end()		{return &data[C-1][R-1];	}

	/// @brief Returns an iterator to the beginning of the matrix.
	/// @return Iterator to beginning of matrix.
	constexpr Iterator<const DataType> begin() const	{return &data[0][0];		}
	/// @brief Returns an iterator to the end of the matrix.
	/// @return Iterator to end of matrix.
	constexpr Iterator<const DataType> end() const		{return &data[C-1][R-1];	}

	/// @brief Converts the matrix to a matrix of different element type.
	/// @tparam T2 Matrix element type.
	template <Type::Ex::Math::Matrix::Compatitble<DataType> T2>
	constexpr operator Matrix<R, C, T2>() const {return Matrix<R, C, T2>(data);}

	/// @brief Converts the matrix to a `Vector2`.
	constexpr operator Vector2() const {return toVector2();}
	/// @brief Converts the matrix to a `Vector3`.
	constexpr operator Vector3() const {return toVector3();}
	/// @brief Converts the matrix to a `Vector4`.
	constexpr operator Vector4() const {return toVector4();}

	/// @brief Return the last column of the matrix as a `Vector2`.
	/// @return Last column.
	/// @note Requires matrix row count to be at least 2.
	constexpr Vector2 toVector2() const {
		static_assert(R >= 2, "Matrix is not a valid representation of a 2D vector!");
		if constexpr(R == 4) return Vector2(data[C-1][0], data[C-1][1]) / data[C-1][3];
		else return Vector2(data[C-1][0], data[C-1][1]);
	}

	/// @brief Return the last column of the matrix as a `Vector3`.
	/// @return Last column.
	/// @note Requires matrix row count to be at least 3.
	constexpr Vector3 toVector3() const {
		static_assert(R >= 3, "Matrix is not a valid representation of a 3D vector!");
		if constexpr(R == 4) return Vector3(data[C-1][0], data[C-1][1], data[C-1][2]) / data[C-1][3];
		else return Vector3(data[C-1][0], data[C-1][1], data[C-1][2]);
	}

	/// @brief Return the last column of the matrix as a `Vector4`.
	/// @return Last column.
	/// @note Requires matrix row count to be at least 4.
	constexpr Vector4 toVector4() const {
		static_assert(R >= 4, "Matrix is not a valid representation of a 4D vector!");
		return Vector4(data[C-1][0], data[C-1][1], data[C-1][2], data[C-1][3]);
	}

	/// @brief Returns the matrix, transformed by a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param position Translation.
	/// @param rotation Rotation.
	/// @param scale Scaling.
	/// @return Transformed matrix.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType transformed(
		Vector3 const& position,
		Vector3 const& rotation,
		Vector3 const& scale
	) const requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		// Transform
		return translated(position).template rotated<ANGLE_FUNC>(rotation).scaled(scale);
	}

	/// @brief Returns the matrix, transformed by a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param trans Transformation to apply.
	/// @return Transformed matrix.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType transformed(Transform3D const& trans)
	const requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return transformed<ANGLE_FUNC>(trans.position, trans.rotation, trans.scale);
	}

	/// @brief Returns the matrix, transformed by a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param trans Transformation to apply.
	/// @return Transformed matrix.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	constexpr SelfType transformed(Transform2D const& trans)
	const requires Type::Ex::Math::Matrix::ValidTransform2D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return Matrix(*this).transform(trans);
	}

	/// @brief Transforms the matrix by a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param position Translation.
	/// @param rotation Rotation.
	/// @param scale Scaling.
	/// @return Reference to self.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType& transform(
		Vector3 const& position,
		Vector3 const& rotation,
		Vector3 const& scale
	) requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		// Transform
		return translate(position).template rotate<ANGLE_FUNC>(rotation).scale(scale);
	}

	/// @brief Transforms the matrix by a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param trans Transformation to apply.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType& transform(Transform3D const& trans)
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return transform<ANGLE_FUNC>(trans.position, trans.rotation, trans.scale);
	}

	/// @brief Transforms the matrix by a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param trans Transformation to apply.
	/// @brief Requires matrix to be a valid 2D transformation matrix.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	constexpr SelfType& transform(Transform2D const& trans)
	requires Type::Ex::Math::Matrix::ValidTransform2D<R, C> {
		static_assert(R == 3, "Matrix is not a valid representation of a 2D transform!");
		SelfType
			pos		= Matrix::identity(),
			rot		= Matrix::identity(),
			scale	= Matrix::identity()
		;
		// Scale
		scale[0][0] = trans.scale.x;
		scale[1][1] = trans.scale.y;
		// Position
		pos[2][0] = trans.position.x;
		pos[2][1] = trans.position.y;
		// Rotation
		DataType s, c;
		CTL::Math::sincos(trans.rotation, s, c);
		rot[0][0] = c;
		rot[1][0] = -s;
		rot[0][1] = s;
		rot[1][1] = c;
		// Result
		(*this) = pos * rot * scale * (*this);
		return *this;
	}

	/// @brief Builds the matrix from a set of transforms.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param position Position.
	/// @param rotation Rotation.
	/// @param scale Scale.
	/// @return Reference to self.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType& compose(
		Vector3 const& position,
		Vector3 const& rotation,
		Vector3 const& scale
	) requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		// Fill & Transform
		(*this) = Matrix(1);
		return transform<ANGLE_FUNC>(position, rotation, scale);
	}

	/// @brief Builds the matrix from a set of transforms.
	/// @tparam ANGLE_FUNC Angle conversion function
	/// @param trans Transformations to build from.
	/// @return Reference to self.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType& compose(Transform3D const& trans)
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return compose<ANGLE_FUNC>(trans.position, trans.rotation, trans.scale);
	}

	/// @brief Builds the matrix from a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param position Position.
	/// @param rotation Rotation.
	/// @param scale Scale.
	/// @param perspective Perspective.
	/// @param skew Skew.
	/// @return Reference to self.
	/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/gtx/matrix_decompose.inl
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType& compose(
		Vector3 const& position,
		Vector3 const& rotation,
		Vector3 const& scale,
		Vector4 const& perspective,
		Vector3 const& skew
	) requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		Matrix<4, 4, DataType> result(Matrix<4, 4, DataType>(1));
		// Apply perspective
		result.data[3][0] = perspective.x;
		result.data[3][1] = perspective.y;
		result.data[3][2] = perspective.z;
		result.data[3][3] = perspective.w;
		// Translate & rotate
		result.translate(position).template rotate<ANGLE_FUNC>(rotation);
		Matrix<4, 4, DataType> tmp = Matrix<4, 4, DataType>(1);
		// Skew
		if (skew.x) {
			tmp = 1;
			tmp[2][1] = skew.x;
			result *= tmp;
		}
		if (skew.y) {
			tmp = 1;
			tmp[2][0] = skew.y;
			result *= tmp;
		}
		if (skew.z) {
			tmp = 1;
			tmp[1][0] = skew.z;
			result *= tmp;
		}
		// Scale
		result.scale(scale);
		// Apply result
		(*this) = result;
		return *this;
	}

	/// @brief Builds the matrix from a set of transformations.
	/// @tparam ANGLE_FUNC Angle conversion function.
	/// @param trans Transformations to build from.
	/// @param perspective Perspective.
	/// @param skew Skew.
	/// @return Reference to self.
	/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/gtx/matrix_decompose.inl
	/// @note Requires matrix to be a valid 3D transformation matrix.
	template<EulerFunction ANGLE_FUNC = Matrix::fromEulerYXZ>
	constexpr SelfType& compose(
		Transform3D const&	trans,
		Vector4 const&		perspective,
		Vector3 const&		skew
	) requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		return compose<ANGLE_FUNC>(trans.position, trans.rotation, trans.scale, perspective, skew);
	}

	/// @brief Builds a 3D transform from the matrix.
	/// @param perspective Perspective transformation output.
	/// @param skew Skew transformation output.
	/// @return 3D transformation.
	/// @note Based off of https://opensource.apple.com/source/WebCore/WebCore-514/platform/graphics/transforms/TransformationMatrix.cpp 
	/// @note Requires matrix to be a valid 3D transformation matrix.
	/// @warning Doesn't currently work!
	[[gnu::unavailable("Not working as intended!")]]
	constexpr Transform3D decompose(
			Vector4& perspective,
			Vector3& skew
		) const requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		static_assert(R == 4, "Matrix is not a valid representation of a 3D transform!");
		Transform3D result;
		// if identity value is 0, return
		if (data[3][3] == 0) return Transform3D();
		// Copy & normalize matrix
		SelfType local(data);
		local /= local.data[3][3];
		// Copy normalized
		SelfType pMatrix(local);
		// Remove translation
		pMatrix.data[0][3] =
		pMatrix.data[1][3] =
		pMatrix.data[2][3] = DataType(0);
		pMatrix.data[3][3] = DataType(1);
		// If perspective's determinant is zero, return
		if (pMatrix.determinant() == DataType(0))
			return Transform3D();
		// Isolate perspective
		if (
			local.data[0][3] != DataType(0)
		||	local.data[1][3] != DataType(0)
		||	local.data[2][3] != DataType(0)
		) {
			// Get right-hand side of the equation
			Vector4 rhs;
			for (usize i = 0; i < 4; i++)
				rhs.data[i] = local.data[i][3];
			SelfType tipMatrix = pMatrix.inverted().transposed();
			perspective = tipMatrix * Matrix<4, 1, DataType>(rhs);
			// Clear perspective
			local.data[0][3] =
			local.data[1][3] =
			local.data[2][3] = DataType(0);
			local.data[3][3] = DataType(1);
		} else {
			perspective = Vector4(0, 0, 0, 1);
		}
		// Isolate translation
		result.position = Vector3(
			local.data[3][0],
			local.data[3][1],
			local.data[3][2]
		);
		local.data[3][0] =
		local.data[3][1] =
		local.data[3][2] = DataType(0);
		// Isolate scale & shear
		Vector3 row[3];
		for (usize i = 0; i < 3; i++)
			for (usize j = 0; i < 3; i++)
				row[i][j] = local.data[i][j];
		// Compute X scale & normalize first row
		result.scale.x = row[0].length();
		row[0].normalize();
		// Compute XY shear factor & orthogonalize 2nd row to 1st
		skew.z = row[0].dot(row[1]);
		row[1] = (row[1] * 1.0) + (row[0] * -skew.z);
		// Compute Y scale & normalize second row
		result.scale.y = row[1].length();
		row[1].normalize();
		skew.z /= result.scale.y;
		// Compute remaining shears & orthogonalize 3rd row
		skew.y = row[0].dot(row[2]);
		row[2] = (row[2] * 1.0) + (row[0] * -skew.y);
		skew.x = row[1].dot(row[2]);
		row[2] = (row[2] * 1.0) + (row[1] * -skew.x);
		// Get Z scale & normalize third row
		result.scale.z = row[2].length();
		row[2].normalize();
		skew.y /= result.scale.z;
		skew.x /= result.scale.z;
		// Check for coordinate flip
		if (row[0].mix(row[1], row[2]) < 0)
			for (usize i = 0; i < 3; i++) {
				result.scale[i] *= DataType(-1);
				row[i] *= DataType(-1);
			}
		// Get euler angles
		result.rotation.y = asin(-row[0][2]);
		if (cos(result.rotation.x) != 0) {
			result.rotation.x = CTL::Math::atan2(row[1][2], row[2][2]);
			result.rotation.z = CTL::Math::atan2(row[0][1], row[0][0]);
		} else {
			result.rotation.x = CTL::Math::atan2(-row[2][0], row[1][1]);
			result.rotation.z = 0;
		}
		// Return result
		return result;
	}

	/// @brief Builds a 3D transform from the matrix.
	/// @return 3D transformation.
	/// @note Requires matrix to be a valid 3D transformation matrix.
	/// @warning Doesn't currently work!
	[[gnu::unavailable("Not working as intended!")]]
	constexpr Transform3D decompose() const
	requires Type::Ex::Math::Matrix::ValidTransform3D<R, C> {
		Vector4 _p;
		Vector3 _s;
		return decompose(_p, _s);
	}

	/// @brief Returns the cofactor for a given cell.
	/// @param row Cell row.
	/// @param col Cell column.
	/// @return Cell's cofactor.
	constexpr DataType cofactor(usize const row, usize const col) const {
		return ((((row + col) % 2) == 0) ? DataType(+1) : DataType(-1)) * truncated(row, col).determinant();
	}

private:
	template<usize R2, usize C2, CTL::Type::Math::Operatable T2> friend class Matrix;
	template<typename T2> friend Matrix<4, 4, T2> translate(Matrix<4, 4, T2>, Vector3 const&);
	/// The matrix's data.
	Decay::AsType<DataType[C][R]> data = {};
};

/// @brief Template matrix shorthand.
/// @tparam R Row count.
/// @tparam C Column count.
/// @tparam T Matrix element type.
template <usize R, usize C, CTL::Type::Math::Operatable T> using Mat = Matrix<R, C, T>;

/// @brief Template 2 x 1 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat2x1 = Matrix<2, 1, T>;
/// @brief Template 3 x 1 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat3x1 = Matrix<3, 1, T>;
/// @brief Template 4 x 1 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat4x1 = Matrix<4, 1, T>;

/// @brief Template 2 x 2 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat2x2 = Matrix<2, 2, T>;
/// @brief Template 3 x 2 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat3x2 = Matrix<3, 2, T>;
/// @brief Template 4 x 2 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat4x2 = Matrix<4, 2, T>;

/// @brief Template 2 x 3 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat2x3 = Matrix<2, 3, T>;
/// @brief Template 3 x 3 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat3x3 = Matrix<3, 3, T>;
/// @brief Template 4 x 3 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat4x3 = Matrix<4, 3, T>;

/// @brief Template 2 x 4 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat2x4 = Matrix<2, 4, T>;
/// @brief Template 3 x 4 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat3x4 = Matrix<3, 4, T>;
/// @brief Template 4 x 4 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat4x4 = Matrix<4, 4, T>;

/// @brief Template 2 x 2 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat2 = TMat2x2<T>;
/// @brief Template 3 x 3 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat3 = TMat3x3<T>;
/// @brief Template 4 x 4 matrix.
/// @tparam T Matrix element type.
template <CTL::Type::Math::Operatable T> using TMat4 = TMat4x4<T>;

/// @brief 2 x 1 `float` matrix.
typedef TMat2x1<float> Matrix2x1;
/// @brief 3 x 1 `float` matrix.
typedef TMat3x1<float> Matrix3x1;
/// @brief 4 x 1 `float` matrix.
typedef TMat4x1<float> Matrix4x1;

/// @brief 2 x 2 `float` matrix.
typedef TMat2x2<float> Matrix2x2;
/// @brief 3 x 2 `float` matrix.
typedef TMat3x2<float> Matrix3x2;
/// @brief 4 x 2 `float` matrix.
typedef TMat4x2<float> Matrix4x2;

/// @brief 2 x 3 `float` matrix.
typedef TMat2x3<float> Matrix2x3;
/// @brief 3 x 3 `float` matrix.
typedef TMat3x3<float> Matrix3x3;
/// @brief 4 x 3 `float` matrix.
typedef TMat4x3<float> Matrix4x3;

/// @brief 2 x 4 `float` matrix.
typedef TMat2x4<float> Matrix2x4;
/// @brief 3 x 4 `float` matrix.
typedef TMat3x4<float> Matrix3x4;
/// @brief 4 x 4 `float` matrix.
typedef TMat4x4<float> Matrix4x4;

/// @brief 2 x 2 `float` matrix.
typedef TMat2<float> Matrix2;
/// @brief 3 x 3 `float` matrix.
typedef TMat3<float> Matrix3;
/// @brief 4 x 4 `float` matrix.
typedef TMat4<float> Matrix4;

/// @brief 2 x 1 `double` matrix.
typedef TMat2x1<double> Matrix2x1d;
/// @brief 3 x 1 `double` matrix.
typedef TMat3x1<double> Matrix3x1d;
/// @brief 4 x 1 `double` matrix.
typedef TMat4x1<double> Matrix4x1d;

/// @brief 2 x 2 `double` matrix.
typedef TMat2x2<double> Matrix2x2d;
/// @brief 3 x 2 `double` matrix.
typedef TMat3x2<double> Matrix3x2d;
/// @brief 4 x 2 `double` matrix.
typedef TMat4x2<double> Matrix4x2d;

/// @brief 2 x 3 `double` matrix.
typedef TMat2x3<double> Matrix2x3d;
/// @brief 3 x 3 `double` matrix.
typedef TMat3x3<double> Matrix3x3d;
/// @brief 4 x 3 `double` matrix.
typedef TMat4x3<double> Matrix4x3d;

/// @brief 2 x 4 `double` matrix.
typedef TMat2x4<double> Matrix2x4d;
/// @brief 3 x 4 `double` matrix.
typedef TMat3x4<double> Matrix3x4d;
/// @brief 4 x 4 `double` matrix.
typedef TMat4x4<double> Matrix4x4d;

/// @brief 2 x 2 `double` matrix.
typedef TMat2<double> Matrix2d;
/// @brief 3 x 3 `double` matrix.
typedef TMat3<double> Matrix3d;
/// @brief 4 x 4 `double` matrix.
typedef TMat4<double> Matrix4d;

/// @brief 2 x 1 `long double` matrix.
typedef TMat2x1<long double> Matrix2x1ld;
/// @brief 3 x 1 `long double` matrix.
typedef TMat3x1<long double> Matrix3x1ld;
/// @brief 4 x 1 `long double` matrix.
typedef TMat4x1<long double> Matrix4x1ld;

/// @brief 2 x 2 `long double` matrix.
typedef TMat2x2<long double> Matrix2x2ld;
/// @brief 3 x 2 `long double` matrix.
typedef TMat3x2<long double> Matrix3x2ld;
/// @brief 4 x 2 `long double` matrix.
typedef TMat4x2<long double> Matrix4x2ld;

/// @brief 2 x 3 `long double` matrix.
typedef TMat2x3<long double> Matrix2x3ld;
/// @brief 3 x 3 `long double` matrix.
typedef TMat3x3<long double> Matrix3x3ld;
/// @brief 4 x 3 `long double` matrix.
typedef TMat4x3<long double> Matrix4x3ld;

/// @brief 2 x 4 `long double` matrix.
typedef TMat2x4<long double> Matrix2x4ld;
/// @brief 3 x 4 `long double` matrix.
typedef TMat3x4<long double> Matrix3x4ld;
/// @brief 4 x 4 `long double` matrix.
typedef TMat4x4<long double> Matrix4x4ld;

/// @brief 2 x 2 `long double` matrix.
typedef TMat2<long double> Matrix2ld;
/// @brief 3 x 3 `long double` matrix.
typedef TMat3<long double> Matrix3ld;
/// @brief 4 x 4 `long double` matrix.
typedef TMat4<long double> Matrix4ld;

/// @brief 2 x 1 `int` matrix.
typedef TMat2x1<int> Matrix2x1i;
/// @brief 3 x 1 `int` matrix.
typedef TMat3x1<int> Matrix3x1i;
/// @brief 4 x 1 `int` matrix.
typedef TMat4x1<int> Matrix4x1i;

/// @brief 2 x 2 `int` matrix.
typedef TMat2x2<int> Matrix2x2i;
/// @brief 3 x 2 `int` matrix.
typedef TMat3x2<int> Matrix3x2i;
/// @brief 4 x 2 `int` matrix.
typedef TMat4x2<int> Matrix4x2i;

/// @brief 2 x 3 `int` matrix.
typedef TMat2x3<int> Matrix2x3i;
/// @brief 3 x 3 `int` matrix.
typedef TMat3x3<int> Matrix3x3i;
/// @brief 4 x 3 `int` matrix.
typedef TMat4x3<int> Matrix4x3i;

/// @brief 2 x 4 `int` matrix.
typedef TMat2x4<int> Matrix2x4i;
/// @brief 3 x 4 `int` matrix.
typedef TMat3x4<int> Matrix3x4i;
/// @brief 4 x 4 `int` matrix.
typedef TMat4x4<int> Matrix4x4i;

/// @brief 2 x 2 `int` matrix.
typedef TMat2<int> Matrix2i;
/// @brief 3 x 3 `int` matrix.
typedef TMat3<int> Matrix3i;
/// @brief 4 x 4 `int` matrix.
typedef TMat4<int> Matrix4i;

/// @brief 2 x 1 `float` matrix.
typedef Matrix2x1 Mat2x1;
/// @brief 3 x 1 `float` matrix.
typedef Matrix3x1 Mat3x1;
/// @brief 4 x 1 `float` matrix.
typedef Matrix4x1 Mat4x1;

/// @brief 2 x 2 `float` matrix.
typedef Matrix2x2 Mat2x2;
/// @brief 3 x 2 `float` matrix.
typedef Matrix3x2 Mat3x2;
/// @brief 4 x 2 `float` matrix.
typedef Matrix4x2 Mat4x2;

/// @brief 2 x 3 `float` matrix.
typedef Matrix2x3 Mat2x3;
/// @brief 3 x 3 `float` matrix.
typedef Matrix3x3 Mat3x3;
/// @brief 4 x 3 `float` matrix.
typedef Matrix4x3 Mat4x3;

/// @brief 2 x 4 `float` matrix.
typedef Matrix2x4 Mat2x4;
/// @brief 3 x 4 `float` matrix.
typedef Matrix3x4 Mat3x4;
/// @brief 4 x 4 `float` matrix.
typedef Matrix4x4 Mat4x4;

/// @brief 2 x 2 `float` matrix.
typedef Matrix2 Mat2;
/// @brief 3 x 3 `float` matrix.
typedef Matrix3 Mat3;
/// @brief 4 x 4 `float` matrix.
typedef Matrix4 Mat4;

/// @brief 2 x 1 `double` matrix.
typedef Matrix2x1d Mat2x1d;
/// @brief 3 x 1 `double` matrix.
typedef Matrix3x1d Mat3x1d;
/// @brief 4 x 1 `double` matrix.
typedef Matrix4x1d Mat4x1d;

/// @brief 2 x 2 `double` matrix.
typedef Matrix2x2d Mat2x2d;
/// @brief 3 x 2 `double` matrix.
typedef Matrix3x2d Mat3x2d;
/// @brief 4 x 2 `double` matrix.
typedef Matrix4x2d Mat4x2d;

/// @brief 2 x 3 `double` matrix.
typedef Matrix2x3d Mat2x3d;
/// @brief 3 x 3 `double` matrix.
typedef Matrix3x3d Mat3x3d;
/// @brief 4 x 3 `double` matrix.
typedef Matrix4x3d Mat4x3d;

/// @brief 2 x 4 `double` matrix.
typedef Matrix2x4d Mat2x4d;
/// @brief 3 x 4 `double` matrix.
typedef Matrix3x4d Mat3x4d;
/// @brief 4 x 4 `double` matrix.
typedef Matrix4x4d Mat4x4d;

/// @brief 2 x 2 `double` matrix.
typedef Matrix2d Mat2d;
/// @brief 3 x 3 `double` matrix.
typedef Matrix3d Mat3d;
/// @brief 4 x 4 `double` matrix.
typedef Matrix4d Mat4d;

/// @brief 2 x 1 `long double` matrix.
typedef Matrix2x1ld Mat2x1ld;
/// @brief 3 x 1 `long double` matrix.
typedef Matrix3x1ld Mat3x1ld;
/// @brief 4 x 1 `long double` matrix.
typedef Matrix4x1ld Mat4x1ld;

/// @brief 2 x 2 `long double` matrix.
typedef Matrix2x2ld Mat2x2ld;
/// @brief 3 x 2 `long double` matrix.
typedef Matrix3x2ld Mat3x2ld;
/// @brief 4 x 2 `long double` matrix.
typedef Matrix4x2ld Mat4x2ld;

/// @brief 2 x 3 `long double` matrix.
typedef Matrix2x3ld Mat2x3ld;
/// @brief 3 x 3 `long double` matrix.
typedef Matrix3x3ld Mat3x3ld;
/// @brief 4 x 3 `long double` matrix.
typedef Matrix4x3ld Mat4x3ld;

/// @brief 2 x 4 `long double` matrix.
typedef Matrix2x4ld Mat2x4ld;
/// @brief 3 x 4 `long double` matrix.
typedef Matrix3x4ld Mat3x4ld;
/// @brief 4 x 4 `long double` matrix.
typedef Matrix4x4ld Mat4x4ld;

/// @brief 2 x 2 `long double` matrix.
typedef Matrix2ld Mat2ld;
/// @brief 3 x 3 `long double` matrix.
typedef Matrix3ld Mat3ld;
/// @brief 4 x 4 `long double` matrix.
typedef Matrix4ld Mat4ld;

/// @brief 2 x 1 `int` matrix.
typedef Matrix2x1i Mat2x1i;
/// @brief 3 x 1 `int` matrix.
typedef Matrix3x1i Mat3x1i;
/// @brief 4 x 1 `int` matrix.
typedef Matrix4x1i Mat4x1i;

/// @brief 2 x 2 `int` matrix.
typedef Matrix2x2i Mat2x2i;
/// @brief 3 x 2 `int` matrix.
typedef Matrix3x2i Mat3x2i;
/// @brief 4 x 2 `int` matrix.
typedef Matrix4x2i Mat4x2i;

/// @brief 2 x 3 `int` matrix.
typedef Matrix2x3i Mat2x3i;
/// @brief 3 x 3 `int` matrix.
typedef Matrix3x3i Mat3x3i;
/// @brief 4 x 3 `int` matrix.
typedef Matrix4x3i Mat4x3i;

/// @brief 2 x 4 `int` matrix.
typedef Matrix2x4i Mat2x4i;
/// @brief 3 x 4 `int` matrix.
typedef Matrix3x4i Mat3x4i;
/// @brief 4 x 4 `int` matrix.
typedef Matrix4x4i Mat4x4i;

/// @brief 2 x 2 `int` matrix.
typedef Matrix2i Mat2i;
/// @brief 3 x 3 `int` matrix.
typedef Matrix3i Mat3i;
/// @brief 4 x 4 `int` matrix.
typedef Matrix4i Mat4i;

/// @brief Creates a camera (view) matrix.
/// @param eye Camera position.
/// @param at Camera target.
/// @param up Camera "up" direction.
/// @return Resulting matrix.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
constexpr Matrix4x4 lookAt(Vector3 const& eye, Vector3 const& at, Vector3 const& up) {
	Vector3 const f((at - eye).normalized());
	Vector3 const s(f.cross(up).normalized());
	Vector3 const u(s.cross(f));
	Matrix4x4 result(1);
	result[0][0] = +s.x;
	result[1][0] = +s.y;
	result[2][0] = +s.z;
	result[0][1] = +u.x;
	result[1][1] = +u.y;
	result[2][1] = +u.z;
	result[0][2] = -f.x;
	result[1][2] = -f.y;
	result[2][2] = -f.z;
	result[3][0] = -s.dot(eye);
	result[3][1] = -u.dot(eye);
	result[3][2] = +f.dot(eye);
	return result;
}

/// @brief Creates an orthographic projection matrix.
/// @tparam T Matrix element type.
/// @param left Leftmost edge of orthographic frustum.
/// @param right Rightmost edge of orthographic frustum.
/// @param bottom Bottommost edge of orthographic frustum.
/// @param top Topmost edge of orthographic frustum.
/// @param zNear Near clip plane.
/// @param zFar Far clip plane.
/// @return Resulting matrix.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl
template<CTL::Type::Math::Operatable T>
constexpr Matrix<4, 4, T> ortho(
	T const& left,
	T const& right,
	T const& bottom,
	T const& top,
	T const& zNear,
	T const& zFar
) {
	Matrix<4, 4, T> result(1);
	result[0][0] = T(2) / (right - left);
	result[1][1] = T(2) / (top - bottom);
	result[2][2] = - T(2) / (zFar - zNear);
	result[3][0] = - (right + left) / (right - left);
	result[3][1] = - (top + bottom) / (top - bottom);
	result[3][2] = - (zFar + zNear) / (zFar - zNear);
	return result;
}

/// @brief Creates an orthographic projection matrix.
/// @tparam T Matrix element type.
/// @param left Leftmost edge of orthographic frustum.
/// @param right Rightmost edge of orthographic frustum.
/// @param bottom Bottommost edge of orthographic frustum.
/// @param top Topmost edge of orthographic frustum.
/// @return Resulting matrix.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl
template<CTL::Type::Math::Operatable T>
constexpr Matrix<4, 4, T> infiniteOrtho(
	T const& left,
	T const& right,
	T const& bottom,
	T const& top
) {
	Matrix<4, 4, T> result(1);
	result[0][0] = T(2) / (right - left);
	result[1][1] = T(2) / (top - bottom);
	result[2][2] = - T(1);
	result[3][0] = - (right + left) / (right - left);
	result[3][1] = - (top + bottom) / (top - bottom);
	return result;
}

/// @brief Creates a generic view frustum projection matrix.
/// @tparam T Matrix element type.
/// @param left Leftmost edge of the frustum.
/// @param right Rightmost edge of the frustum.
/// @param bottom Bottommost edge of the frustum.
/// @param top Topmost edge of the frustum.
/// @param zNear Near clip plane.
/// @param zFar Far clip plane.
/// @return Resulting matrix.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl
template<CTL::Type::Math::Operatable T>
constexpr Matrix<4, 4, T> frustum(
	T const& left,
	T const& right,
	T const& bottom,
	T const& top,
	T const& zNear,
	T const& zFar
) {
	Matrix<4, 4, T> result(T(0));
	result[0][0] = (T(2) * zNear) / (right - left);
	result[1][1] = (T(2) * zNear) / (top - bottom);
	result[2][0] = (right + left) / (right - left);
	result[2][1] = (top + bottom) / (top - bottom);
	result[2][2] = - (zFar + zNear) / (zFar - zNear);
	result[2][3] = T(-1);
	result[3][2] = - (T(2) * zFar * zNear) / (zFar - zNear);
	return result;
}

/// @brief Creates a perspective projection matrix.
/// @tparam T Matrix element type.
/// @param fovy Field of view.
/// @param aspect Aspect ratio (width/height).
/// @param zNear Near clip plane.
/// @param zFar Far clip plane.
/// @return Resulting matrix.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl
template<CTL::Type::Math::Operatable T>
constexpr Matrix<4, 4, T> perspective(
	T const& fovy,
	T const& aspect,
	T const& zNear,
	T const& zFar
) {
	Matrix<4, 4, T> result(T(0));
	T const tanHalfFovy = tan(fovy / T(2));
	result[0][0] = T(1) / (aspect * tanHalfFovy);
	result[1][1] = T(1) / (tanHalfFovy);
	result[2][2] = - (zFar + zNear) / (zFar - zNear);
	result[2][3] = - T(1);
	result[3][2] = - (T(2) * zFar * zNear) / (zFar - zNear);
	return result;
}

/// @brief Creates a perspective projection matrix.
/// @tparam T Matrix element type.
/// @param fov Field of view.
/// @param width Screen width.
/// @param height Screen height.
/// @param zNear Near clip plane.
/// @param zFar Far clip plane.
/// @return Resulting matrix.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl
template<CTL::Type::Math::Operatable T>
constexpr Matrix<4, 4, T> perspectiveFOV(
	T const& fov,
	T const& width,
	T const& height,
	T const& zNear,
	T const& zFar
) {
	Matrix<4, 4, T> result(T(0));
	T const rad	= fov;
	T const h	= cos(T(0.5) * rad) / sin(T(0.5) * rad);
	T const w	= h * height / width;
	result[0][0] = w;
	result[1][1] = h;
	result[2][2] = - (zFar + zNear) / (zFar - zNear);
	result[2][3] = - T(1);
	result[3][2] = - (T(2) * zFar * zNear) / (zFar - zNear);
	return result;
}

/// @brief Creates a perspective projection matrix.
/// @tparam T Matrix element type.
/// @param fovy Field of view.
/// @param zNear Near clip plane.
/// @return Resulting matrix.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl 
template<CTL::Type::Math::Operatable T>
constexpr Matrix<4, 4, T> infinitePerspective(
	T const& fovy,
	T const& aspect,
	T const& zNear
) {
	T const range	= tan(fovy / T(2)) * zNear;
	T const left	= -range * aspect;
	T const right	= range * aspect;
	T const bottom	= -range;
	T const top		= range;
	Matrix<4, 4, T> result(T(0));
	result[0][0] = (T(2) * zNear) / (right - left);
	result[1][1] = (T(2) * zNear) / (top - bottom);
	result[2][2] = - T(1);
	result[2][3] = - T(1);
	result[3][2] = - T(2) * zNear;
	return result;
}

/// @brief Returns the euler angles for a matrix.
/// @tparam T Matrix element type.
/// @param mat Matrix to get euler angles from.
/// @return Euler angles.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/gtx/euler_angles.inl
template<CTL::Type::Math::Operatable T>
constexpr Vector3 getEulerAnglesYXZ(Matrix<4, 4, T> const& mat) {
	float T1 = CTL::Math::atan2(mat[2][0], mat[2][2]);
	float C2 = CTL::Math::sqrt(mat[0][1]*mat[0][1] + mat[1][1]*mat[1][1]);
	float T2 = CTL::Math::atan2(-mat[2][1], C2);
	float S1, C1;
	CTL::Math::sincos<float>(T1, S1, C1);
	float T3 = CTL::Math::atan2(S1*mat[1][2] - C1*mat[1][0], C1*mat[0][0] - S1*mat[0][2]);
	return Vector3(T1, T2, T3);
}

/// @brief Returns the euler angles for a matrix.
/// @tparam T Matrix element type.
/// @param mat Matrix to get euler angles from.
/// @return Euler angles.
/// @note Based off of https://github.com/g-truc/glm/blob/master/glm/gtx/euler_angles.inl
template<CTL::Type::Math::Operatable T>
constexpr Vector3 getEulerAnglesXYZ(Matrix<4, 4, T> const& mat) {
	float T1 = CTL::Math::atan2(mat[2][1], mat[2][2]);
	float C2 = CTL::Math::sqrt(mat[0][0]*mat[0][0] + mat[1][0]*mat[1][0]);
	float T2 = CTL::Math::atan2(-mat[2][0], C2);
	float S1, C1;
	CTL::Math::sincos<float>(T1, S1, C1);
	float T3 = CTL::Math::atan2(S1*mat[0][2] - C1*mat[0][1], C1*mat[1][1] - S1*mat[1][2]);
	return Vector3(T1, T2, T3);
}

/// @brief Interpolates between two matrices.
/// @tparam T Matrix element type.
/// @param from Start matrix.
/// @param to End matrix.
/// @param by Interpolation factor.
/// @return Interpolation between matrices.
template <Type::Ex::Math::Matrix::Matrix T>
constexpr T lerp(T const& from, T const& to, typename T::DataType const& by) {
	T result(0);
	for (usize i = 0; i < T::COLUMNS; i++)
		for (usize j = 0; j < T::ROWS; j++)
			result[i][j] =CTL::Math::lerp<typename T::DataType>(from[i][j], to[i][j], by);
	return result;
}

/// @brief Interpolates between two matrices.
/// @tparam T Matrix element type.
/// @param from Start matrix.
/// @param to End matrix.
/// @param by Interpolation factor.
/// @return Interpolation between matrices.
template <Type::Ex::Math::Matrix::Matrix T>
constexpr T lerp(T const& from, T const& to, T const& by) {
	T result(0);
	for (usize i = 0; i < T::COLUMNS; i++)
		for (usize j = 0; j < T::ROWS; j++)
			result[i][j] = CTL::Math::lerp<typename T::DataType>(from[i][j], to[i][j], by[i][j]);
	return result;
}

}

CTL_EX_NAMESPACE_END

#endif // CTL_EX_MATH_MATRIX_H
