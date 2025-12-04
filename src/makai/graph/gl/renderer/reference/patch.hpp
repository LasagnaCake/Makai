#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_PATCH_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_PATCH_H

#include "shape.hpp"
#include "plane.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	template <usize R, usize C>
	struct APatchBase: AShape<(R + C) * 2> {
		static_assert(R  > 0 && C > 0, "Row & column count must not be zero!");
		template <class T>
		using Parameters = As<T[C+1][R+1]>;

		using ScaleType = Meta::DualType<(R == 1), float, Vector2>;

		ScaleType			align = 0;
		As<ScaleType[C]>	sizes;
		Parameters<Vector2>	uvs;
		Parameters<Vector4>	colors;

		Handle<AReference> transform() override final {
			setBaseShape();
			applyTransform();
			return this;
		}

		Handle<AReference> reset() override final {
			setBaseShape();
			applyTransform();
			return this;
		}

	protected:
		virtual void setBaseShape() = 0;
	
	private:
		using AShape<(R + C) * 2>::applyTransform;
	};

	template <usize R, usize C> struct Patch;

	template <usize N> using PatchRow		= Patch<1, N>;
	template <usize N> using PatchSquare	= Patch<N, N>;

	template <> struct Patch<1, 1>: Plane {};

	template <> struct Patch<1, 2>: APatchBase<1, 2> {
		float align	= 0;
	private:
		void setBaseShape() override final;
	};

	template <> struct Patch<1, 3>: APatchBase<1, 3> {
		float align	= 0;
	private:
		void setBaseShape() override final;
	};
	
	template <> struct Patch<2, 2>: APatchBase<2, 2> {
		Vector2 align = 0;
	private:
		void setBaseShape() override final;
	};
	
	template <> struct Patch<3, 3>: APatchBase<3, 3> {
		Vector2 align = 0;
	private:
		void setBaseShape() override final;
	};

	using TwoPatch1D	= PatchRow<2>;
	using ThreePatch1D	= PatchRow<3>;
	using FourPatch2D	= PatchSquare<2>;
	using NinePatch2D	= PatchSquare<3>;
}

#endif