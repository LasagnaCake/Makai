#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_SLICE_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_SLICE_H

#include "shape.hpp"

namespace Makai::Graph::Ref {
	template <usize N>
	struct ASliceBase: AShape<N * 2> {

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
	};

	template <usize N> struct Slice;

	template <> struct Slice<2>: ASliceBase<2> {
		float align	= 0;
		float width	= 1;
		float left	= 1;
		float right	= 1;
	private:
		void setBaseShape() override final;
	};

	template <> struct Slice<3>: ASliceBase<3> {
		float align	= 0;
		float head	= 1;
		float body	= 1;
		float tail	= 1;
		float width	= 1;
	private:
		void setBaseShape() override final;
	};
	
	template <> struct Slice<4>: ASliceBase<4> {
		Vector2 align = 0;
		Vector2 left;
		Vector2 right;
	private:
		void setBaseShape() override final;
	};
	
	template <> struct Slice<9>: ASliceBase<9> {
		Vector2 align = 0;
		Vector2 head;
		Vector2 body;
		Vector2 tail;
	private:
		void setBaseShape() override final;
	};
}

#endif