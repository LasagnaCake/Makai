#ifndef MAKAILIB_GRAPH_ARMATURE_SKELETON_H
#define MAKAILIB_GRAPH_ARMATURE_SKELETON_H

#include "../../../compat/ctl.hpp"

/// @brief Skeletal animation facilites.
namespace Makai::Graph::Armature {
	template<usize MB = 64>
	struct Skeleton {
		using Bone = Transform3D;

		constexpr static usize const MAX_BONES = MB;

		template<class T>
		using Container	= Array<T, MAX_BONES>;

		using Bones		= Container<Transform3D>;
		using Matrices	= Container<Matrix4x4>;
		using Relations	= Map<usize, Map<usize, bool>>;

		constexpr Skeleton()						= default;
		constexpr Skeleton(Skeleton const& other)	= default;
		constexpr Skeleton(Skeleton&& other)		= default;

		Bones bones;

		constexpr Skeleton& addChild(usize const bone, usize const child) {
			if (bone >= MAX_BONES || child >= MAX_BONES) return *this;
			if (bone == child) return *this;
			if (connected(child, bone) || connected(bone, child)) return *this;
			forward[bone][child] = true;
			reverse[child][bone] = true;
			return *this;
		}

		constexpr Skeleton& removeChild(usize const bone, usize const child) {
			if (bone >= MAX_BONES || child >= MAX_BONES) return *this;
			if (bone == child) return *this;
			forward[bone][child] = false;
			reverse[child][bone] = false;
			return *this;
		}

		constexpr Skeleton& clearChildren(usize const bone) {
			if (bone >= MAX_BONES) return *this;
			for (auto const& child: forward[bone])
				reverse[child.key] = false;
			forward[bone].clear();
			return *this;
		}

		constexpr Skeleton& clearAllRelations() {
			forward = {};
			reverse = {};
			return *this;
		}

		constexpr List<usize> childrenOf(usize const bone) const {
			if (bone >= MAX_BONES) return List<usize>();
			List<usize> children;
			if (forward.contains(bone)) {
				children.resize(forward[bone].size());
				for (auto const& child : forward[bone])
					if (child.value) children.pushBack(child.key);
			}
			return children;
		}

		constexpr List<usize> parentOf(usize const bone) const {
			if (bone >= MAX_BONES) return -1;
			if (reverse.contains(bone)) {
				for (auto const& child : reverse[bone])
					if (child.value) return child.key;
			}
			return -1;
		}

		constexpr usize childrenCount(usize const bone) const {
			if (bone >= MAX_BONES) return 0;
			usize count = 0;
			if (forward.contains(bone)) {
				for (auto const& child : forward[bone])
					if (child.value) ++count;
			}
			return count;
		}

		constexpr bool isRootBone(usize const bone) const {
			if (bone >= MAX_BONES) return false;
			if (reverse.contains(bone)) {
				for (auto const& child : reverse[bone])
					if (child.value) return false;
			}
			return true;
		}

		constexpr bool isLeafBone(usize const bone) const {
			if (bone >= MAX_BONES) return false;
			if (forward.contains(bone)) {
				for (auto const& child : reverse[bone])
					if (child.value) return false;
			}
			return true;
		}

		constexpr Matrices matrices() const {
			Matrices matrices;
			for (usize i = 0; i < MAX_BONES; ++i) {
				matrices[i] = bones[i];
			}
			List<usize> const boneRoots = roots();
			for (auto const root : boneRoots) {
				List<usize> stack;
				stack.pushBack(root);
				usize current;
				while (!stack.empty()) {
					current = stack.popBack();
					if (!stack.empty())
						matrices[current] = matrices[stack.back()] * matrices[current];
					if (!isLeafBone(current))
						stack.appendBack(childrenOf(current));
				}
			}
			return matrices;
		}

		constexpr List<usize> roots() const {
			List<usize> roots;
			for (usize i = 0; i < MAX_BONES; ++i)
				if (isRootBone(i))
					roots.pushBack(i);
			return roots;
		}

		constexpr List<usize> leaves() const {
			List<usize> leaves;
			for (usize i = 0; i < MAX_BONES; ++i)
				if (isLeafBone(i))
					leaves.pushBack(i);
			return leaves;
		}

	private:
		constexpr bool connected(usize const from, usize const to) const {
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

		Relations forward;
		Relations reverse;
	};
}

#endif