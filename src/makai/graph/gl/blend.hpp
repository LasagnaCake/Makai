#ifndef MAKAILIB_GRAPH_BLEND_H
#define MAKAILIB_GRAPH_BLEND_H

#include "../../compat/ctl.hpp"
#include "core.hpp"

/// @brief Graphical facilities.
namespace Makai::Graph {
	/// @brief Blend function.
	enum class BlendFunction {
		BF_ZERO,
		BF_ONE,
		BF_SRC_COLOR,
		BF_ONE_MINUS_SRC_COLOR,
		BF_DST_COLOR,
		BF_ONE_MINUS_DST_COLOR,
		BF_SRC_ALPHA,
		BF_ONE_MINUS_SRC_ALPHA,
		BF_DST_ALPHA,
		BF_ONE_MINUS_DST_ALPHA,
		BF_SRC_ALPHA_SATURATE,
		BF_CONST_COLOR,
		BF_ONE_MINUS_CONST_COLOR,
		BF_CONST_ALPHA,
		BF_ONE_MINUS_CONST_ALPHA
	};

	/// @brief Blend equation.
	enum class BlendEquation {
		BE_ZERO,
		BE_ONE,
		BE_ADD,
		BE_SUB,
		BE_REV_SUB,
		BE_MIN,
		BE_MAX
	};

	/// @brief Blend mode.
	struct BlendMode {
		/// @brief Blend function setting.
		struct FunctionOption {
			/// @brief Blend function to use for the source color.
			BlendFunction srcColor = BlendFunction::BF_SRC_ALPHA;
			/// @brief Blend function to use for the destination color.
			BlendFunction dstColor = BlendFunction::BF_ONE_MINUS_SRC_ALPHA;
			/// @brief Blend function to use for the source alpha.
			BlendFunction srcAlpha = BlendFunction::BF_SRC_ALPHA_SATURATE;
			/// @brief Blend function to use for the destination alpha.
			BlendFunction dstAlpha = BlendFunction::BF_ONE;
		};
		/// @brief Blend function to use.
		FunctionOption func = {};
		/// @brief Blend equation setting.
		struct EquationOption {
			/// @brief Blend equation to use for the color.
			BlendEquation color = BlendEquation::BE_ADD;
			/// @brief Blend equation to use for the alpha.
			BlendEquation alpha = BlendEquation::BE_MAX;
		};
		/// @brief Blend equation to use.
		EquationOption eq = {};
	};

	/// @brief Blendable object.
	struct Blendable {
		/// @brief Default blend mode.
		inline static BlendMode DEFAULT_MODE = {};

		/// @brief Empty constructor.
		Blendable(): blend(DEFAULT_MODE) {}

		/// @brief Constructs the blendable with a given mode.
		/// @param blend Blend mode to use.
		constexpr Blendable(BlendMode const& blend): blend(blend) {}

		/// @brief Sets the blend function to use.
		/// @param srcColor Function to use for the source color.
		/// @param dstColor Function to use for the destination color.
		/// @param srcAlpha Function to use for the source alpha.
		/// @param dstAlpha Function to use for the destination alpha.
		/// @return Reference to self.
		constexpr Blendable& setBlendFunction(
			BlendFunction const& srcColor,
			BlendFunction const& dstColor,
			BlendFunction const& srcAlpha,
			BlendFunction const& dstAlpha
		) {
			blend.func = {srcColor, dstColor, srcAlpha, dstAlpha};
			return *this;
		}

		/// @brief Sets the blend function to use.
		/// @param src Function to use for the source.
		/// @param dst Function to use for the destination.
		/// @return Reference to self.
		constexpr Blendable& setBlendFunction(
			BlendFunction const& src,
			BlendFunction const& dst
		) {
			blend.func = {src, dst, src, dst};
			return *this;
		}

		/// @brief Sets the blend equation to use.
		/// @param color Equation to use for the color.
		/// @param alpha Equation to use for the alpha.
		/// @return Reference to self.
		constexpr Blendable& setBlendEquation(
			BlendEquation const& color,
			BlendEquation const& alpha
		) {
			blend.eq = {color, alpha};
			return *this;
		}

		/// @brief Sets the blend equation to use.
		/// @param eq Equation to use.
		/// @return Reference to self.
		constexpr Blendable& setBlendEquation(
			BlendEquation const& eq
		) {
			blend.eq = {eq, eq};
			return *this;
		}

		/// @brief Blend mode to use.
		BlendMode blend;

	protected:
		void useBlendMode(usize const drawBuffer = 0) const;
	};
}

#endif // MAKAILIB_GRAPH_BLEND_H
