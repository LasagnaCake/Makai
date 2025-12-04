#ifndef MAKAILIB_GRAPH_RENDERER_REFERENCE_REFEREND_H
#define MAKAILIB_GRAPH_RENDERER_REFERENCE_REFEREND_H

#include "shape.hpp"

/// @brief Graphical object references.
namespace Makai::Graph::Ref {
	/// @brief Reference holder.
	struct Referend {
		/// @brief Triangle storage container type.
		using TriangleBank = List<Makai::Graph::Triangle>;

		/// @brief Constructs the reference holder.
		/// @param triangles Reference to triangle bank.
		/// @param lockState Reference to lock state.
		Referend(
			TriangleBank& triangles,
			bool& lockState
		): triangles(triangles), lockState(lockState) {}

		/// @brief Destructor.
		virtual ~Referend() {
			clearReferences();
		}

		/// @brief Creates a shape reference bound to this object.
		/// @tparam T Reference type.
		/// @return Reference instance.
		template<AShapeType T>
		[[nodiscard]]
		Instance<T> createReference() {
			constexpr usize count = T::SIZE;
			if (lockState) throw Error::InvalidAction("Renderable object is locked!", CTL_CPP_PRETTY_SOURCE);
			triangles.expand(count, {});
			// Create shape
			T* shape = new T({triangles, triangles.size()-count, count}, *this);
			// Add to reference list
			references.pushBack(shape);
			// return shape
			return shape;
		}

		/// @brief
		///		Destroys a reference and its associated triangles.
		///		Will only execute if reference is associated with this object.
		/// @tparam T Reference type.
		/// @param ref Reference to remove.
		/// @note If successful, also destroys the reference.
		template <AShapeType T>
		void removeReference(Instance<T> ref) {
			if (!ref) return;
			if (lockState) return;
			removeReference(*ref);
			auto const rp = ref.raw();
			ref.release();
			delete rp;
		}

		/// @brief
		///		Destroys a reference while keeping its associated triangles.
		///		Will only execute if reference is associated with this object.
		/// @tparam T Reference type.
		/// @param ref Reference to remove.
		/// @note If successful, also destroys the reference.
		template <AShapeType T>
		void unbindReference(Instance<T> ref) {
			if (!ref) return;
			if (lockState) return;
			unbindReference(*ref);
			auto const rp = ref.raw();
			ref.release();
			delete rp;
		}

		/// @brief Destroys all bound references.
		void clearReferences();

	protected:
		/// @brief Transforms all bound references.
		void transformReferences();
		/// @brief Resets applied transformation in all bound references.
		void resetReferenceTransforms();

	private:
		void removeReference(AReference& ref);
		void unbindReference(AReference& ref);

		/// @brief References bound to this object.
		List<ref<AReference>>	references;

		friend class AReference;

		/// @brief Triangle bank to store triangles into. 
		TriangleBank&	triangles;
		/// @brief Triangle bank lock state.
		bool&			lockState;
	};
}

#endif