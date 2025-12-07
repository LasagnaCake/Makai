#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_PATCH_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_PATCH_H

#include "shape.hpp"
#include "plane.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Implementation details.
	namespace Impl {
		/// @brief Builds a patch.
		/// @param triangle Triangles to build patch on.
		/// @param offset Offset to apply to the patch's position.
		/// @param sizes Patch sizing details.
		/// @param rows Patch row count.
		/// @param columns Patch column count.
		void makePatch(
			AReference::BoundRange const&	triangles,
			Vector3 const&					offset,
			ref<Vector2 const> const		sizes,
			usize const						rows,
			usize const						columns
		);

		/// @brief Sets a patch's UVs.
		/// @param triangle Triangles in the patch.
		/// @param uvs Patch UV details.
		/// @param rows Patch row count.
		/// @param columns Patch column count.
		void setPatchUVs(
			AReference::BoundRange const&	triangles,
			ref<Vector2 const> const		uvs,
			usize const						rows,
			usize const						columns
		);

		/// @brief Sets a patch's colors.
		/// @param triangle Triangles in the patch.
		/// @param colors Patch color details.
		/// @param rows Patch row count.
		/// @param columns Patch column count.
		void setPatchColors(
			AReference::BoundRange const&	triangles,
			ref<Vector4 const> const		colors,
			usize const						rows,
			usize const						columns
		);
	}

	/// @brief Patch reference base.
	/// @tparam R Row count.
	/// @tparam C Column count.
	template <usize R, usize C>
	struct PatchBase: AShape<(R + C) * 2> {
		using AShape<(R + C) * 2>::AShape;

		/// @brief Row count.
		constexpr static usize const ROWS		= R;
		/// @brief Column count.
		constexpr static usize const COLUMNS	= C;

		static_assert(ROWS  > 0 && COLUMNS > 0, "Row & column count must not be zero!");
		static_assert((ROWS != COLUMNS) || (ROWS > 1), "Patch cannot be 1 x 1!");

		/// @brief Patch parameters.
		/// @tparam T Parameter type.
		template <class T>
		using Parameters = As<T[COLUMNS*2][ROWS*2]>;

		/// @brief Patch scale type.
		using ScaleType = Meta::If<(ROWS == 1), float, Vector2>;
		
		/// @brief Patch positions.
		using Positions = As<Vector3[(COLUMNS+1)][(ROWS+1)]>;
		
		/// @brief Patch sizes.
		using Sizes		= ScaleType[C];
		/// @brief Patch UVs.
		using UVs		= Parameters<Vector2>;
		/// @brief Patch colors.
		using Colors	= Parameters<Vector4>;

		/// @brief Patch size for single-row types.
		struct ShapeSize1D {
			Sizes sizes		= {1};
			float height	= 1;
		};
		
		/// @brief Patch size for multi-row types.
		struct ShapeSize2D {
			Sizes sizes		= {1};
		};

		/// @brief Patch shape details.
		struct Shape: Meta::If<(ROWS > 1), ShapeSize2D, ShapeSize1D> {
			/// @brief Patch alignment against local origin.
			ScaleType	align	= 0;
			/// @brief Patch UVs.
			UVs			uvs		= {0};
			/// @brief Patch colors.
			Colors		colors	= {1};
		};

		/// @brief Patch shape details.
		Shape shape;
		
		/// @brief Returns the patch's total size.
		/// @return Total size.
		ScaleType size() const {
			ScaleType totalSize = 0;
			for (auto const& size: shape.sizes)
				totalSize += size;
			return totalSize;
		}

		/// @brief Builds the patch.
		/// @param triangles Triangles in the patch.
		/// @param size Total patch size.
		/// @param shape Patch shape details.
		static void build(
			AReference::BoundRange const&	triangles,
			Makai::Vector3 const&			size,
			Shape const&					shape
		) {
			auto const su = Cast::rewrite<ref<Vector2 const>>(shape.uvs);
			auto const sc = Cast::rewrite<ref<Vector4 const>>(shape.colors);
			if constexpr (Type::Derived<Shape, ShapeSize1D>) {
				As<Vector2[C]> sizes;
				for (usize i = 0; i < C; ++i) {
					sizes[i].y = shape.height;
					sizes[i].x = shape.sizes[i];
					Impl::makePatch(triangles, -size * shape.align, sizes, ROWS, COLUMNS);
				}
			} else {
				auto const ss = Cast::rewrite<ref<Vector2 const>>(shape.sizes);
				Impl::makePatch(triangles, -size * shape.align, ss, ROWS, COLUMNS);
			}
			Impl::setPatchUVs(triangles, su, ROWS, COLUMNS);
			Impl::setPatchColors(triangles, sc, ROWS, COLUMNS);
		}
		
		using AShape<(R + C) * 2>::fixed;
		using AShape<(R + C) * 2>::visible;
		
	private:
		/// @brief Applies transformations to the bound triangles.
		void onTransform() override {
			if (fixed) return;
			setBaseShape();
			applyTransform();
		}

		/// @brief Resets transformations applied to the bound triangles.
		void onReset() override final {
			if (fixed) return;
			for (auto& triangle: triangles)
				for (auto& vert: triangle.verts) 
					vert.position = 0;
		}

		void setBaseShape() {
			build(triangles, size(), shape);
		}

		using AShape<(R + C) * 2>::applyTransform;
		using AShape<(R + C) * 2>::triangles;
	};

	/// @brief Patch reference.
	template <usize R, usize C> struct Patch;

	/// @brief Patch row reference.
	template <usize N> using PatchRow		= Patch<1, N>;
	/// @brief Patch square reference.
	template <usize N> using PatchSquare	= Patch<N, N>;

	/// @brief 1x1 patch reference.
	template <> struct Patch<1, 1>: Plane {using Plane::Plane;};

	/// @brief 2x1 patch reference.
	template <> struct Patch<1, 2>: PatchBase<1, 2> {using PatchBase<1, 2>::PatchBase;};
	/// @brief 3x1 patch reference.
	template <> struct Patch<1, 3>: PatchBase<1, 3> {using PatchBase<1, 3>::PatchBase;};
	/// @brief 2x2 patch reference.
	template <> struct Patch<2, 2>: PatchBase<2, 2> {using PatchBase<2, 2>::PatchBase;};
	/// @brief 3x3 patch reference.
	template <> struct Patch<3, 3>: PatchBase<3, 3> {using PatchBase<3, 3>::PatchBase;};

	/// @brief "Two-Patch" reference.
	using TwoPatch1D	= PatchRow<2>;
	/// @brief "Three-Patch" reference.
	using ThreePatch1D	= PatchRow<3>;
	/// @brief "Four-Patch" reference.
	using FourPatch2D	= PatchSquare<2>;
	/// @brief "Nine-Patch" reference.
	using NinePatch2D	= PatchSquare<3>;
}

#endif