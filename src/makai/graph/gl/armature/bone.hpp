#ifndef MAKAILIB_GRAPH_ARMATURE_BONE_H
#define MAKAILIB_GRAPH_ARMATURE_BONE_H

#include "../../../compat/ctl.hpp"

/// @brief Skeletal animation facilites.
namespace Makai::Graph::Armature {
	/// @brief 3D animation bone.
	struct Bone {
		constexpr Bone()										= default;
		constexpr Bone(Transform3D const& trans): trans(trans)	{}
		constexpr Bone(Bone const& other)						= default;
		constexpr Bone(Bone&& other)							= default;

		constexpr ref<Bone> getParent() const {
			return parent;
		}

		constexpr Bone& setParent(ref<Bone> const newParent) {
			if (newParent == this) return *this;
			if (newParent) {
				auto predecessor = newParent->getParent();
				while (predecessor && newParent != this)
					predecessor = predecessor->parent;
				if (predecessor) return *this;
			}
			parent = newParent;
			return *this;
		}

		constexpr Matrix4x4 globalized() const {
			Matrix4x4 global = trans;
			if (parent) {
				auto predecessor = parent->getParent();
				while (predecessor && parent != this) {
					global = predecessor->localized() * global;
					predecessor = predecessor->parent;
				}
			}
			return global;
		}

		constexpr Matrix4x4 localized() const {return trans;}

		Transform3D trans;

	private:
		ref<Bone> parent = nullptr;
	};
}

#endif