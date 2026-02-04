#ifndef MAKAILIB_GRAPH_MATERIAL_EFFECT_H
#define MAKAILIB_GRAPH_MATERIAL_EFFECT_H

#include "../../../ctl/ctl.hpp"
#include "../../../file/json.hpp"
#include "../texture.hpp"
#include "../../color/color.hpp"

/// @brief Shader materials.
namespace Makai::Graph::Material {
	/// @brief Material effects.
	namespace Effect {
		/// @brief Effect components.
		namespace Base {
			struct Toggleable {
				/// @brief Whether the effect is enabled.
				bool enabled = false;
			};

			struct Limitable {
				/// @brief Starting point.
				float start	= 0.0;
				/// @brief Stopping point.
				float stop	= 10.0;
			};

			struct Variable {
				/// @brief Strength.
				float	strength = 1;
			};

			struct Variable2D {
				/// @brief Strength.
				Vector2	strength = 1;
			};

			struct Variable3D {
				/// @brief Strength.
				Vector3	strength = 1;
			};

			struct Variable4D {
				/// @brief Strength.
				Vector4	strength = 1;
			};

			struct ColorableRGBA {
				/// @brief Color.
				Vector4	color = Color::WHITE;
			};

			struct ColorableRGB {
				/// @brief Color.
				Vector3	color = 1;
			};

			struct Imageable2D {
				/// @brief Texture.
				Texture2D image = nullptr;
			};

			struct Channelable {
				/// @brief Color channel.
				int channel = -1;
			};

			struct Transformable2D {
				/// @brief Transform.
				Transform2D trans;
			};

			struct Positionable2D {
				/// @brief Position.
				Vector2 position;
			};

			struct Invertible {
				/// @brief Whether to invert the effect.
				bool invert = false;
			};

			struct Tuneable {
				/// @brief Frequency.
				float frequency	= 0;
				/// @brief Amplitude.
				float amplitude	= 0;
				/// @brief Offset.
				float shift		= 0;
			};

			struct Tuneable2D {
				/// @brief Frequency.
				Vector2 frequency	= Vector2::ZERO();
				/// @brief Amplitude.
				Vector2 amplitude	= Vector2::ZERO();
				/// @brief Offset.
				Vector2 shift		= Vector2::ZERO();
			};

			struct Sizeable {
				/// @brief Size.
				float size = 0;
			};

			struct Sizeable2D {
				/// @brief Size.
				Vector2 size = 0;
			};

			struct Countable {
				/// @brief Count.
				usize count = 1;
			};
		}

		namespace {
			using namespace Base;
		}

		/// @brief Blend function to use in the effect.
		enum class EffectBlendFunction: unsigned int {
			EBF_ZERO = 0,
			EBF_ONE,
			EBF_SRC,
			EBF_ONE_MINUS_SRC,
			EBF_DST,
			EBF_ONE_MINUS_DST,
		};

		/// @brief Blend equation to use in the effect.
		enum class EffectBlendEquation: unsigned int {
			EBE_ADD,
			EBE_SUBTRACT,
			EBE_MULTIPLY,
			EBE_DIVIDE,
			EBE_REVERSE_SUBTRACT,
			EBE_REVERSE_DIVIDE,
			EBE_ADD_CLAMP,
			EBE_SUBTRACT_CLAMP,
			EBE_MULTIPLY_CLAMP,
			EBE_DIVIDE_CLAMP,
			EBE_REVERSE_SUBTRACT_CLAMP,
			EBE_REVERSE_DIVIDE_CLAMP,
			EBE_MAX,
			EBE_MIN,
		};

		/// @brief Blend source to use in the effect.
		enum class EffectBlendSource: unsigned int {
			EBS_ZERO,
			EBS_ONE,
			EBS_COLOR,
			EBS_ALPHA,
			EBS_NOISE,
			EBS_SOLID_COLOR,
			EBS_SOLID_ALPHA,
		};

		/// @brief Effect blend mode.
		struct EffectBlendMode {
			/// @brief Blend source (left-hand side of the operation).
			EffectBlendFunction source		= EffectBlendFunction::EBF_SRC;
			/// @brief Blend destination (right-hand side of the operation).
			EffectBlendFunction destination	= EffectBlendFunction::EBF_DST;
			/// @brief Blend equation (operation to perform).
			EffectBlendEquation equation	= EffectBlendEquation::EBE_MULTIPLY;
		};

		/// @brief Effect blend setting.
		struct EffectBlendSetting {
			/// @brief Color blend mode.
			EffectBlendMode	color	= {EffectBlendFunction::EBF_ONE, EffectBlendFunction::EBF_DST, EffectBlendEquation::EBE_MULTIPLY};
			/// @brief Alpha blend mode.
			EffectBlendMode	alpha	= {EffectBlendFunction::EBF_SRC, EffectBlendFunction::EBF_ONE, EffectBlendEquation::EBE_MULTIPLY};
		};

		/// @brief Gradient effect.
		struct Gradient: Toggleable, Channelable, Invertible {
			/// @brief Starting color.
			Vector4	begin	= Color::BLACK;
			/// @brief End color.
			Vector4	end		= Color::WHITE;
		};

		/// @brief Color inversion effect.
		struct Negative: Toggleable, Variable {};

		/// @brief Image effect.
		struct Image: Toggleable, Imageable2D {};

		/// @brief Displacement effect.
		struct Warp: Image, Transformable2D {
			/// @brief Color channel to use for horizontal displaement.
			uint32 channelX = 0;
			/// @brief Color channel to use for vertical displaement.
			uint32 channelY = 1;
		};

		// Object Material Effects

		/// @brief Texture effect.
		struct Texture: Image {
			/// @brief Texture alpha clip.
			float alphaClip = 0.1;
		};

		/// @brief Blend equation the blend texture uses.
		/// @details
		///		Layout (from LSB to MSB):
		///
		///		- First four bits: which equation to use (`*`, `/`, `+`, `-`, `max`, `min`).
		///
		///		- Next two bits: Which parameters to use the inverse from (none, destination, source, both).
		///			Due to the implementation, it is affected by the swap bit (i.e. If swapped,
		///			which parameter gets the inverse also swaps).
		///
		///		- Next two bits: Which parameters to replace with 1 (none, destination, source, both).
		///
		///		- Next bit: Whether to clamp the result between 0 and 1.
		///
		///		- Next bit: Whether to swap source and destination.
		enum class BlendTextureEquation: uint32 {
			BTE_MUL				= 0x00,
			BTE_DIV				= 0x01,
			BTE_ADD				= 0x02,
			BTE_SUB				= 0x03,
			BTE_MAX				= 0x04,
			BTE_MIN				= 0x05,
			BTE_INVERT_DST		= 0x10,
			BTE_INVERT_SRC		= 0x20,
			BTE_INVERT_BOTH		= BTE_INVERT_DST | BTE_INVERT_SRC,
			BTE_DST_ONE			= 0x40,
			BTE_SRC_ONE			= 0x80,
			BTE_BOTH_ONE		= BTE_DST_ONE | BTE_SRC_ONE,
			BTE_CLAMP_RESULT	= 0x100,
			BTE_SWAP_SRC_DST	= 0x200,
			BTE_DEFAULT			= BTE_MUL,
		};

		/// @brief Bitwise OR for `BlendTextureEquation`.
		constexpr BlendTextureEquation operator|(BlendTextureEquation const a, BlendTextureEquation const b) {
			return static_cast<BlendTextureEquation>(CTL::enumcast(a) | CTL::enumcast(b));
		}

		/// @brief Bitwise AND for `BlendTextureEquation`.
		constexpr BlendTextureEquation operator&(BlendTextureEquation const a, BlendTextureEquation const b)	{
			return static_cast<BlendTextureEquation>(CTL::enumcast(a) & CTL::enumcast(b));
		}

		/// @brief Bitwise XOR for `BlendTextureEquation`.
		constexpr BlendTextureEquation operator^(BlendTextureEquation const a, BlendTextureEquation const b)	{
			return static_cast<BlendTextureEquation>(CTL::enumcast(a) ^ CTL::enumcast(b));
		}

		/// @brief Bitwise NOT for `BlendTextureEquation`.
		constexpr BlendTextureEquation operator~(BlendTextureEquation const eq)	{
			return static_cast<BlendTextureEquation>(~CTL::enumcast(eq));
		}

		struct BlendTexture: Image, Variable3D {
			using Equation = BlendTextureEquation;
			/// @brief Blend equation to use.
			/// @note Source is the current color (texture * vertex color), destination is the blend texture.
			/// @note For ease of use, refer to the enum itself.
			Equation equation = Equation::BTE_DEFAULT;
		};

		/// @brief Emmisive texture effect.
		struct Emission: Image, Variable {
		};

		/// @brief Normal map effect.
		struct NormalMap: Image, Variable {
		};

		// Buffer Material Effects

		/// @brief Color channel mask effect.
		struct Mask: Toggleable, Imageable2D, Transformable2D, Invertible {
			/// @brief Mask albedo tint.
			Vector4	albedo		= 1;
			/// @brief Mask albedo accent.
			Vector4	accent		= 0;
			/// @brief Whether mask is relative to screen UV.
			bool	relative	= false;
		};

		/// @brief Wave shape.
		enum class WaveShape: unsigned int {
			WS_SQUARE = 0,
			WS_SINE,
			WS_BIN_SINE,
			WS_ABS_SINE,
			WS_ABS_BIN_SINE,
			WS_TRIANGLE,
			WS_BIN_TRIANGLE,
			WS_ABS_TRIANGLE,
			WS_ABS_BIN_TRIANGLE,
			WS_HALF_SINE,
			WS_BIN_HALF_SINE,
			WS_ABS_HALF_SINE,
			WS_ABS_BIN_HALF_SINE,
			WS_HALF_TRIANGLE,
			WS_BIN_HALF_TRIANGLE,
			WS_ABS_HALF_TRIANGLE,
			WS_ABS_BIN_HALF_TRIANGLE,
			WS_SIMPLE_NOISE,
			WS_BIN_SIMPLE_NOISE,
		};

		/// @brief Screen wave effect.
		struct Wave: Toggleable, Tuneable2D {
			/// @brief Distortion shape.
			WaveShape	shape	= WaveShape::WS_SQUARE;
		};

		/// @brief Rainbow effect.
		struct Rainbow: Toggleable, Variable {
			Vector2 frequency		= 0.0;
			Vector2 shift			= 0.0;
			bool	absoluteColor	= false;
			bool	polar			= false;
			float	polarShift		= 0.0;
		};

		/// @brief Blur effect.
		struct Blur: Toggleable, Variable2D {};

		/// @brief Outline effect.
		struct Outline: Toggleable, Sizeable2D, ColorableRGBA {
			/// @brief Whether the alpha is relative to the edge's alpha.
			bool relativeAlpha = true;
		};

		/// @brief Polar distortion effect.
		struct PolarWarp: Toggleable, Sizeable, Positionable2D, Variable2D, ColorableRGBA {
			/// @brief Polar tint strength.
			float	tintStrength	= 1;
			/// @brief Whether the distortion is fish-eye distortion.
			bool	fishEye			= true;
		};

		/// @brief Noise function.
		enum class NoiseType: unsigned int {
			NT_NOISE_SIMPLE = 0,
			NT_NOISE_GOLD,
			NT_NOISE_SUPER
		};

		/// @brief Noise blend mode.
		/// @note `SRC`: Pixel Color. `DST`: Noise.
		struct NoiseBlendMode: EffectBlendSetting {};

		/// @brief Noise effect.
		struct Noise: Toggleable, Variable, Transformable2D {
			/// @brief Noise seed.
			float			seed	= 1;
			/// @brief Noise function to use.
			NoiseType		type	= NoiseType::NT_NOISE_SUPER;
			/// @brief Blend mode.
			NoiseBlendMode	blend;
		};

		// World Material Effects

		/// @brief Fog effect.
		struct Fog: Toggleable, Limitable, ColorableRGBA, Variable {};

		/// @brief Ambient lighting effect.
		struct Ambient: ColorableRGB, Variable {};
	}

	/// @brief Saves an image effect.
	/// @param effect Effect to save.
	/// @param folder Folder to save to.
	/// @param path Path to image effect.
	/// @return JSON structure associated with the effect.
	JSON::Value saveImageEffect(Effect::Image& effect, CTL::String const& folder, String const& path);

	/// @brief Loads an image effect.
	/// @param effect Effect to load.
	/// @param sourcepath Folder of definition file.
	/// @param texture Texture to load image into.
	/// @return Image effect.
	Effect::Image loadImageEffect(JSON::Value effect, CTL::String const& sourcepath, Texture2D& texture);
}

namespace MkEffect = Makai::Graph::Material::Effect;

#endif // MAKAILIB_GRAPH_MATERIAL_EFFECT_H
