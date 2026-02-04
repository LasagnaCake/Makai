#ifndef MAKAILIB_GRAPH_RENDERER_VERTEBRATE_H
#define MAKAILIB_GRAPH_RENDERER_VERTEBRATE_H

#include "../../../compat/ctl.hpp"

#include "../../armature/armature.hpp"
#include "../vertex.hpp"
#include "../shader/shader.hpp"

namespace Makai::Graph {
	/// @brief Armature-containing object.
	/// @tparam MB Max number of bones. By default, it is `64`.
	template<usize MB = 64>
	struct Vertebrate;

	/// @brief Armature-containing object.
	/// @tparam MB Max number of bones. By default, it is `64`.
	template<usize MB>
	struct Vertebrate {
		/// @brief Armature type.
		using ArmatureType = Armature::Skeleton<MB>;

		/// @brief Maximum number of bones.
		constexpr static usize const MAX_BONES = ArmatureType::MAX_BONES;

		/// @brief Armature associated with the object.
		ArmatureType armature;

	protected:
		/// @brief Applies the armature to a shader.
		/// @param shader Shader to apply armature to.
		/// @param name Uniform struct associated with the armature. Defaults to `"armature"`.
		/// @note
		///		Uniform passed in `name` parameter MUST contain two fields:
		///
		///		- An array of `mat4`s of at least `MAX_BONES` entries, named `bones`.
		///
		///		- An integer of name `boneCount`.
		void applyArmature(Shader& shader, String const& name = "armature") const {
			auto const bones = armature.matrices();
			auto const uniform = shader[name + ".bones[0]"];
			uniform.setArray(bones.data(), bones.size());
			shader[name + ".boneCount"].set(static_cast<uint32>(bones.size()));
		}
	};
}

#endif
