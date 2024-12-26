#ifndef MAKAILIB_GRAPH_MATERIAL_MATERIALS_H
#define MAKAILIB_GRAPH_MATERIAL_MATERIALS_H

#include "../../../file/json.hpp"
#include "../../../compat/ctl.hpp"
#include "effect.hpp"
#include "debug.hpp"
#include "../color.hpp"
#include "../shader.hpp"
#include "../renderer/mode.hpp"

/// @brief Shader materials.
namespace Makai::Graph::Material {
	/// @brief Shader material interface.
	struct IMaterial {
		/// @brief Applies the material to the shader. Must be implemented.
		/// @param shader Shader to apply to.
		virtual void use(Shader const& shader) const = 0;
		/// @brief Destructor.
		constexpr virtual ~IMaterial() {}
	};

	/// @brief Object material interface.
	struct IObjectMaterial: IMaterial {
		/// @brief Albedo tint.
		Vector4	color = Color::WHITE;

		/// @brief Destructor.
		constexpr virtual ~IObjectMaterial() {}
	};

	/// @brief Default object material.
	struct ObjectMaterial final: IObjectMaterial {
		/// @brief Whether the has directional shading.
		bool shaded			= false;
		/// @brief Whether the object can recieve illumination.
		bool illuminated	= false;
		/// @brief Hue.
		float			hue			= 0;
		/// @brief Saturation.
		float			saturation	= 1;
		/// @brief Luminosity.
		float			luminosity	= 1;
		/// @brief Brightness.
		float			brightness	= 0;
		/// @brief Contrast.
		float			contrast	= 1;
		/// @brief Texture UV shift.
		Vector2				uvShift;
		/// @brief Texture.
		Effect::Texture		texture;
		/// @brief Normal map.
		Effect::NormalMap	normalMap;
		/// @brief Emmision texture.
		Effect::Emission	emission;
		/// @brief Displacement texture.
		Effect::Warp		warp;
		/// @brief Negative effect.
		Effect::Negative	negative;
		/// @brief Gradient effect.
		Effect::Gradient	gradient;
		/// @brief Instances.
		List<Vector3>	instances = {Vec3(0, 0, 0)};
		/// @brief Face culling mode.
		CullMode	culling		= CullMode::OCM_NONE;
		/// @brief Face fill mode.
		FillMode	fill		= FillMode::OFM_FILL;
		/// @brief Debug view mode.
		ObjectDebugView	debug	= ObjectDebugView::ODV_NONE;

		/// @brief Applies the material to the shader.
		/// @param shader Shader to apply to.
		void use(Shader const& shader) const override final;

		/// @brief Destructor.
		constexpr virtual ~ObjectMaterial() {}
	};

	/// @brief Framebuffer material interface.
	struct IBufferMaterial: IMaterial {
		/// @brief Background color.
		Vector4 background = Color::NONE;

		/// @brief Destructor.
		constexpr virtual ~IBufferMaterial() {}
	};

	/// @brief Default framebuffer material.
	struct BufferMaterial final: IBufferMaterial {
		Vector4
			/// @brief Albedo tint.
			color	= Color::WHITE,
			/// @brief Accent color.
			accent	= Color::NONE
		;
		/// @brief Hue.
		float			hue			= 0;
		/// @brief Saturation.
		float			saturation	= 1;
		/// @brief Luminosity.
		float			luminosity	= 1;
		/// @brief Brightness.
		float			brightness	= 0;
		/// @brief Contrast.
		float			contrast	= 1;
		/// @brief Screen shift.
		Vector2			uvShift;
		/// @brief Channel mask.
		Effect::Mask		mask;
		/// @brief Screen displacement texture.
		Effect::Warp		warp;
		/// @brief Negative effect.
		Effect::Negative	negative;
		/// @brief Blur effect.
		Effect::Blur		blur;
		/// @brief Outline effect.
		Effect::Outline		outline;
		/// @brief Screen wave effect.
		Effect::Wave		wave;
		/// @brief Screen distortion effect.
		Effect::Wave		prism;
		/// @brief Polar distirtion effect.
		Effect::PolarWarp	polarWarp;
		/// @brief Gradient effect.
		Effect::Gradient	gradient;
		/// @brief Rainbow effect.
		Effect::Rainbow		rainbow;
		/// @brief Noise effect.
		Effect::Noise		noise;
		/// @brief Debug view mode.
		BufferDebugView	debug	= BufferDebugView::BDV_NONE;

		/// @brief Applies the material to the shader.
		/// @param shader Shader to apply to.
		void use(Shader const& shader) const override final;

		/// @brief Destructor.
		constexpr virtual ~BufferMaterial() {}
	};

	/// @brief World material interface.
	struct IWorldMaterial: IMaterial {
		/// @brief Destructor.
		constexpr virtual ~IWorldMaterial() {}
	};

	/// @brief Default world material.
	struct WorldMaterial final: IWorldMaterial {
		/// @brief Near fog effect.
		Effect::Fog		nearFog;
		/// @brief Far fog effect.
		Effect::Fog		farFog;
		/// @brief Ambient lighting.
		Effect::Ambient	ambient;

		/// @brief Applies the material to the shader.
		/// @param shader Shader to apply to.
		void use(Shader const& shader) const override final;

		/// @brief Destructor.
		constexpr virtual ~WorldMaterial() {}
	};

	/// @brief Type must be a material of some kind.
	template<class T> concept ValidMaterial			= Makai::Type::Subclass<T, IMaterial>;
	/// @brief Type must be a object material of some kind.
	template<class T> concept ValidObjectMaterial	= Makai::Type::Subclass<T, IObjectMaterial>;
	/// @brief Type must be a framebuffer material of some kind.
	template<class T> concept ValidBufferMaterial	= Makai::Type::Subclass<T, IBufferMaterial>;
	/// @brief Type must be a world material of some kind.
	template<class T> concept ValidWorldMaterial	= Makai::Type::Subclass<T, IWorldMaterial>;
}

#endif // MAKAILIB_GRAPH_MATERIAL_MATERIALS_H
