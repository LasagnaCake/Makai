#ifndef MAKAILIB_GRAPH_ARMATURE_SKELETON_H
#define MAKAILIB_GRAPH_ARMATURE_SKELETON_H

#include "../../../compat/ctl.hpp"

/// @brief Skeletal animation facilites.
namespace Makai::Graph::Armature {
	struct Skeleton {
		using Bone = Transform3D;

		constexpr static usize const MAX_BONES = 64;

		constexpr Skeleton() {
			for (usize i = 0; i < MAX_BONES; ++i)
				bones[i] = {};
		}
		constexpr Skeleton(Skeleton const& other)	= default;
		constexpr Skeleton(Skeleton&& other)		= default;

		Array<Transform3D, MAX_BONES> bones;

		constexpr Skeleton& addChild(usize const bone, usize const child) {
			if (bone >= MAX_BONES || child >= MAX_BONES) return *this;
			if (bone == child) return *this;
			if (hasChild(child, bone) || hasChild(bone, child)) return *this;
			relations[bone][child] = true;
			return *this;
		}

		constexpr Skeleton& removeChild(usize const bone, usize const child) {
			if (bone >= MAX_BONES || child >= MAX_BONES) return *this;
			if (bone == child) return *this;
			relations[bone][child] = false;
			return *this;
		}

		constexpr List<usize> childrenOf(usize const bone) const {
			if (bone >= MAX_BONES) return List<usize>();
			List<usize> children;
			if (!relations[bone].empty()) {
				children.resize(relations[bone].size());
				for (auto const& child : relations[bone])
					if (child.value) children.pushBack(child.key);
			}
			return children;
		}

		constexpr usize childrenCount(usize const bone) const {
			if (bone >= MAX_BONES) return 0;
			usize count = 0;
			if (!relations[bone].empty()) {
				for (auto const& child : relations[bone])
					if (child.value) ++count;
			}
			return count;
		}

		constexpr Array<Matrix4x4, MAX_BONES> matrices() const {
			Array<Matrix4x4, MAX_BONES> matrices;
			for (usize i = 0; i < MAX_BONES; ++i) {
				matrices[i] = bones[i];
			}
			List<usize> stack = roots();
			usize current	= -1;
			usize previous	= -1;
			while (!stack.empty()) {
				previous = current;
				current = stack.popBack();
				if (previous != -1)
					matrices[current] = Matrix4x4(matrices[previous]) * matrices[current];
				stack.appendBack(childrenOf(current));
			}
			return matrices;
		}

		constexpr List<usize> roots() const {
			List<usize> roots;
			for (usize i = 0; i < MAX_BONES; ++i) {
				bool hit = false;
				for (usize j = i+1; j < MAX_BONES; ++j) {
					if (relations[j].contains(i)) {
						hit = true;
						break;
					}
				}
				if (!hit) roots.pushBack(i);
			}
			return roots;
		}

		constexpr List<usize> leaves() const {
			List<usize> leaves;
			for (usize i = 0; i < MAX_BONES; ++i)
				if (!childrenCount(i))
					leaves.pushBack(i);
			return leaves;
		}

	private:
		constexpr bool bridge(usize const from, usize const to) const {
			if (from == to) return true;
			List<usize> stack;
			stack.pushBack(from);
			usize current;
			while (!stack.empty()) {
				if (stack.rfind(to) != -1) return true;
				current = stack.popBack();
				stack.appendBack(childrenOf(current));
			}
			return false;
		}

		constexpr bool hasChild(usize const bone, usize const child) const {
			return bridge(bone, child);
		}

		Array<Map<usize, bool>, 64> relations;
	};
}

#endif