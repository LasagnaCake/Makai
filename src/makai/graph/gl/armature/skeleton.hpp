#ifndef MAKAILIB_GRAPH_ARMATURE_SKELETON_H
#define MAKAILIB_GRAPH_ARMATURE_SKELETON_H

#include "../../../compat/ctl.hpp"

/// @brief Skeletal animation facilites.
namespace Makai::Graph::Armature {
	struct Skeleton {
		using Bone = Transform3D;

		constexpr static usize const MAX_BONES = 64;

		template<class T>
		using Container = Array<T, MAX_BONES>;

		constexpr Skeleton() {
			for (usize i = 0; i < MAX_BONES; ++i)
				bones[i] = {};
		}
		constexpr Skeleton(Skeleton const& other)	= default;
		constexpr Skeleton(Skeleton&& other)		= default;

		Container<Transform3D> bones;

		constexpr Skeleton& addChild(usize const bone, usize const child) {
			if (bone >= MAX_BONES || child >= MAX_BONES) return *this;
			if (bone == child) return *this;
			if (hasChild(child, bone) || hasChild(bone, child)) return *this;
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
			if (!forward[bone].empty()) {
				children.resize(forward[bone].size());
				for (auto const& child : forward[bone])
					if (child.value) children.pushBack(child.key);
			}
			return children;
		}

		constexpr List<usize> parentOf(usize const bone) const {
			if (bone >= MAX_BONES) return -1;
			if (!reverse[bone].empty()) {
				for (auto const& child : reverse[bone])
					if (child.value) return child.key;
			}
			return -1;
		}

		constexpr usize childrenCount(usize const bone) const {
			if (bone >= MAX_BONES) return 0;
			usize count = 0;
			if (!forward[bone].empty()) {
				for (auto const& child : forward[bone])
					if (child.value) ++count;
			}
			return count;
		}

		constexpr bool isRootBone(usize const bone) const {
			if (bone >= MAX_BONES) return 0;
			if (!reverse[bone].empty()) {
				for (auto const& child : reverse[bone])
					if (child.value) return false;
			}
			return true;
		}

		constexpr bool isLeafBone(usize const bone) const {
			if (bone >= MAX_BONES) return 0;
			if (!forward[bone].empty()) {
				for (auto const& child : reverse[bone])
					if (child.value) return false;
			}
			return false;
		}

		constexpr Container<Matrix4x4> matrices() const {
			Container<Matrix4x4> matrices;
			for (usize i = 0; i < MAX_BONES; ++i) {
				matrices[i] = bones[i];
			}
			List<usize> stack = roots();
			usize current;
			while (!stack.empty()) {
				current = stack.popBack();
				if (!stack.empty())
					matrices[current] = matrices[stack.back()] * matrices[current];
				stack.appendBack(childrenOf(current));
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

		Container<Map<usize, bool>> forward;
		Container<Map<usize, bool>> reverse;
	};
}

#endif