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
		~Referend();

		/// @brief Creates a shape reference bound to this object.
		/// @tparam T Reference type.
		/// @return Reference instance.
		template<AShapeType T>
		[[nodiscard]]
		Unique<T> create() {
			constexpr usize count = T::SIZE;
			if (lockState) throw Error::InvalidAction("Base object is locked!", CTL_CPP_PRETTY_SOURCE);
			triangles.expand(count, {});
			// Create shape
			Unique<T> shape;
			shape.bind(new T({&triangles, triangles.size()-count, count}, *this));
			// Add to reference list
			references.pushBack(shape.raw());
			// return shape
			return shape;
		}
		/// @brief Creates a shape reference bound to this object.
		/// @tparam T Reference type.
		/// @return Reference instance.
		template<AShapeType T>
		[[nodiscard, deprecated("Function is deprecated! Please use `create` instead!")]]
		Unique<T> createReference() {
			return create<T>();
		}

		/// @brief Unbinds all bound references.
		void clear();

		/// @brief Transforms all bound references.
		void transformAll();
		/// @brief Resets applied transformations in all bound references.
		void resetAll();

	private:
		void removeReference(AReference& ref);
		void unbindReference(AReference& ref);

		/// @brief References bound to this object.
		List<ref<AReference>>	references;

		friend struct AReference;

		/// @brief Triangle bank to store triangles into. 
		TriangleBank&	triangles;
		/// @brief Triangle bank lock state.
		bool&			lockState;
	};
}

#endif