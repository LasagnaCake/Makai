#ifndef MAKAILIB_GRAPH_ARMATURE_SKELETON_H
#define MAKAILIB_GRAPH_ARMATURE_SKELETON_H

#include "../../../compat/ctl.hpp"

/// @brief Skeletal animation facilites.
namespace Makai::Graph::Armature {
	/// @brief Armature skeleton.
	/// @tparam MB Maximum amount of bones.
	template<usize MB = 64>
	struct Skeleton {
		/// @brief Bone type.
		using Bone = Transform3D;

		/// @brief Maximum amount of bones the skeleton has.
		constexpr static usize const MAX_BONES = MB;

		/// @brief Container wrapper type.
		template<class T>
		using Container	= Array<T, MAX_BONES>;

		/// @brief Bone list container type.
		using Bones		= Container<Bone>;
		/// @brief Matrix list container type.
		using Matrices	= Container<Matrix4x4>;
		/// @brief Relation graph type.
		using Relations	= Map<usize, Map<usize, bool>>;

		/// @brief Empty constructor.
		constexpr Skeleton()						= default;
		/// @brief Copy constructor (defaulted).
		constexpr Skeleton(Skeleton const& other)	= default;
		/// @brief Move constructor (defaulted).
		constexpr Skeleton(Skeleton&& other)		= default;

		/// @brief Skeleton pose.
		Bones pose	= Bones::withFill(Bone::identity());
		/// @brief Rest pose.
		Bones rest	= Bones::withFill(Bone::identity());

		/// @brief Creates a parent-child relationship between two bones, if applicable.
		/// @param bone Bone to act as parent.
		/// @param child Bone to act as child.
		/// @return Reference to self.
		constexpr Skeleton& addChild(usize const bone, usize const child) {
			if (baked || locked) return *this;
			if (bone >= MAX_BONES || child >= MAX_BONES) return *this;
			if (bone == child) return *this;
			if (connected(child, bone) || connected(bone, child)) return *this;
			forward[bone][child] = true;
			reverse[child][bone] = true;
			return *this;
		}

		/// @brief Removes a parent-child relationship between two bones, if applicable.
		/// @param bone Bone acting as parent.
		/// @param child Bone acting as child.
		/// @return Reference to self.
		constexpr Skeleton& removeChild(usize const bone, usize const child) {
			if (baked || locked) return *this;
			if (bone >= MAX_BONES || child >= MAX_BONES) return *this;
			if (bone == child) return *this;
			forward[bone][child] = false;
			reverse[child][bone] = false;
			return *this;
		}

		/// @brief Clears all parent-child relations associated with the bone.
		/// @param bone Bone clear relations from.
		/// @return Reference to self.
		constexpr Skeleton& clearChildren(usize const bone) {
			if (baked || locked) return *this;
			if (bone >= MAX_BONES) return *this;
			if (forward.contains(bone))
				for (auto const& child: forward[bone])
					reverse[child.key][bone] = false;
			forward[bone].clear();
			return *this;
		}

		/// @brief Clears all relations for every bone.
		/// @return Reference to self.
		constexpr Skeleton& clearAllRelations() {
			if (baked || locked) return *this;
			forward.clear();
			reverse.clear();
			return *this;
		}

		/// @brief Returns all children of a given bone.
		/// @param bone Bone to get children.
		/// @return Children of bone.
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

		/// @brief Returns the parent of a given bone.
		/// @param bone Bone to get parent.
		/// @return Parent of bone.
		constexpr usize parentOf(usize const bone) const {
			if (bone >= MAX_BONES) return -1;
			if (reverse.contains(bone)) {
				for (auto const& child : reverse[bone])
					if (child.value) return child.key;
			}
			return -1;
		}

		/// @brief Returns the amount of children a given bone has.
		/// @param bone Bone to get children count.
		/// @return Children count of bone.
		constexpr usize childrenCount(usize const bone) const {
			if (bone >= MAX_BONES) return 0;
			usize count = 0;
			if (forward.contains(bone)) {
				for (auto const& child : forward[bone])
					if (child.value) ++count;
			}
			return count;
		}

		/// @brief Returns whether a bone is a "root" bone (i.e. has no parent).
		/// @param bone Bone to check.
		/// @return Whether bone is a root bone.
		constexpr bool isRootBone(usize const bone) const {
			if (bone >= MAX_BONES) return false;
			if (reverse.contains(bone)) {
				for (auto const& child : reverse[bone])
					if (child.value) return false;
			}
			return true;
		}

		/// @brief Returns whether a bone is a "leaf" bone (i.e. has no children).
		/// @param bone Bone to check.
		/// @return Whether bone is a leaf bone.
		constexpr bool isLeafBone(usize const bone) const {
			if (bone >= MAX_BONES) return false;
			if (forward.contains(bone)) {
				for (auto const& child : forward[bone])
					if (child.value) return false;
			}
			return true;
		}

		/// @brief Returns the computed matrices for all bones.
		/// @return Bone matrices.
		constexpr Matrices matrices() const {
			Matrices matrices, boneMatrix, inverse, poseMatrix;
			for (usize i = 0; i < MAX_BONES; ++i) {
				poseMatrix[i] = pose[i];
				if (baked)
					inverse[i] = bakedInverse[i];
				else {
					boneMatrix[i]	= rest[i];
					inverse[i]		= boneMatrix[i].inverted();
				}
			}
			dfsTraverse(
				[&] (usize const parent, usize const child) {
					if (parent != Limit::MAX<usize>) {
						if (!baked) {
							boneMatrix[child]	= boneMatrix[parent] * boneMatrix[child];
							inverse[child]		= boneMatrix[child].inverted();
						}
						poseMatrix[child] = poseMatrix[parent] * poseMatrix[child];
					}
					matrices[child] = inverse[child] * poseMatrix[child];
				}
			);
			return matrices;
		}

		/// @brief Returns all root bones.
		/// @return Root bones.
		constexpr List<usize> roots() const {
			if (baked || locked) return bakedRoots;
			List<usize> roots;
			for (usize i = 0; i < MAX_BONES; ++i)
				if (isRootBone(i))
					roots.pushBack(i);
			return roots;
		}

		/// @brief Returns all leaf bones.
		/// @return Leaf bones.
		constexpr List<usize> leaves() const {
			if (baked || locked) return bakedLeaves;
			List<usize> leaves;
			for (usize i = 0; i < MAX_BONES; ++i)
				if (isLeafBone(i))
					leaves.pushBack(i);
			return leaves;
		}

		/// @brief Returns whether a bone can be reached from another bone.
		/// @param from Bone to start from.
		/// @param from Bone to end in.
		/// @return Whether bones are connected.
		constexpr bool connected(usize const from, usize const to) const {
			if (from == to) return true;
			List<usize> stack;
			stack.pushBack(from);
			usize current;
			while (!stack.empty()) {
				if (stack.rfind(to) != -1) return true;
				current = stack.popBack();
				if (!isLeafBone(current))
					stack.appendBack(childrenOf(current));
			}
			return false;
		}

		/// @brief Bakes the armature.
		/// @details
		///		Pre-processes the rest poses and bone relations.
		///
		///		Any rest pose changes no longer affect the object.
		///		Any bone relation changes no longer affect the object.
		///		In return, speeds up calculations substantially.
		///		
		///		If you need speed, use this.
		/// @return Refeference to self.
		constexpr Skeleton& bake() {
			if (baked || locked) return *this;
			Matrices bone;
			for (usize i = 0; i < MAX_BONES; ++i) {
				bone[i]			= rest[i];
				bakedInverse[i]	= bone[i].inverted();
			}
			bakedRoots	= roots();
			bakedLeaves	= leaves();
			baked = true;
			dfsTraverse(
				[&] (usize const parent, usize const child) {
					if (parent != Limit::MAX<usize>) {
						bone[child]			= bone[parent] * bone[child];
						bakedInverse[child]	= bone[child].inverted();
					}
				}
			);
			return *this;
		}

		/// @brief Unbakes the armature.
		/// @return Refeference to self.
		constexpr Skeleton& unbake() {
			if (locked) return *this;
			baked = false;
			return *this;
		}

		/// @brief IRREVERSIBLE. bakes and locks the object.
		constexpr void bakeAndLock() {
			if (locked) return;
			bake();
			locked = true;
		}

		/// @brief Traverses via Depth-First Search across the bone tree.
		/// @tparam
		///		TFunction Function type.
		///		The first parameter it takes is the `parent` bone,
		///		and the second is the `child` bone.
		///		If bone does not have parent, its value is `Limit::MAX<usize>`.
		/// @param Function to execure for every bone in the tree.
		template<Type::Functional<void(usize const, usize const)> TFunction>
		constexpr void dfsTraverse(TFunction const& func) const {
			List<usize> boneRoots = baked ? bakedRoots : roots();
			for (auto const root : boneRoots) {
				List<KeyValuePair<usize, usize>> stack;
				stack.pushBack({Limit::MAX<usize>, root});
				usize current;
				usize parent;
				while (stack.size()) {
					auto relation = stack.popBack();
					parent	= relation.key;
					current	= relation.value;
					func(parent, current);
					if (!isLeafBone(current)) {
						for (auto& child: childrenOf(current))
							stack.pushBack({current, child});
					}
				}
			}
		}
	private:
		/// @brief Whether rest pose was baked.
		bool		baked		= false;
		/// @brief Whether the object is locked.
		bool		locked		= false;
		/// @brief Baked rest pose.
		Matrices	bakedInverse;
		/// @brief Baked root bones.
		List<usize>	bakedRoots;
		/// @brief Baked leaf bones.
		List<usize>	bakedLeaves;
		/// @brief Parent-child relations.
		Relations	forward;
		/// @brief Child-parent relations.
		Relations	reverse;
	};
}

#endif