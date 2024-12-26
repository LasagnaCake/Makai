#ifndef MAKAILIB_GRAPH_RENDERER_RENDERABLE_H
#define MAKAILIB_GRAPH_RENDERER_RENDERABLE_H

#include "../../../file/json.hpp"
#include "../../../data/encdec.hpp"
#include "drawable.hpp"
#include "reference.hpp"
#include "../material/material.hpp"
#include "../vertex.hpp"

/// @brief Graphical facilities.
namespace Makai::Graph {
	class Scene;

	/// @brief Renderable object.
	class Renderable: public IGLDrawable {
	public:
		/// @brief Latest renderable object definition file version supported.
		constexpr static usize VERSION = 0;

		/// @brief Constructs the renderable object.
		/// @param layer Layer to register the object to. By default, it is layer zero.
		/// @param manual Whether the object is manually rendered. By default, it is `false`.
		Renderable(usize const layer = 0, bool const manual = false);

		/// @brief Shader material to use.
		Material::ObjectMaterial material;

		/// @brief Constructs the renderable.
		/// @param triangles Triangles to use. Takes ownership of them.
		/// @param layer Layer to register the object to. By default, it is layer zero.
		/// @param manual Whether the object is manually rendered. By default, it is `false`.
		Renderable(
			List<Triangle*>&& triangles,
			usize const layer = 0,
			bool const manual = false
		);

		/// @brief Constructs the renderable.
		/// @param vertices Vertices to copy from.
		/// @param count Vertex count.
		/// @param layer Layer to register the object to. By default, it is layer zero.
		/// @param manual Whether the object is manually rendered. By default, it is `false`.
		/// @throw Error::InvalidValue if no vertices were passed, or vertex count is not a multiple of 3. 
		Renderable(
			Vertex* const vertices,
			usize const count,
			usize const layer = 0,
			bool const manual = false
		);

		/// @brief Destructor.
		virtual ~Renderable();

		/// @brief Creates a shape reference bound to this object.
		/// @tparam T Reference type.
		/// @return Reference instance.
		template<ShapeRefType T>
		[[nodiscard]]
		Instance<T> createReference() {
			constexpr usize count = T::SIZE;
			if (locked) throw Error::InvalidAction("Renderable object is locked!", CTL_CPP_PRETTY_SOURCE);
			List<Triangle*> tris(count);
			// Create triangles
			for (Triangle*& t: tris)
				t = new Triangle();
			triangles.appendBack(tris);
			// Create shape
			T* shape = new T(tris, *this);
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
		template <ShapeRefType T>
		void removeReference(Instance<T> const& ref) {
			if (!ref) return;
			if (locked) return;
			removeReference(*ref);
			ref.destroy();
		}

		/// @brief
		///		Destroys a reference while keeping its associated triangles.
		///		Will only execute if reference is associated with this object.
		/// @tparam T Reference type.
		/// @param ref Reference to remove.
		/// @note If successful, also destroys the reference.
		template <ShapeRefType T>
		void unbindReference(Instance<T> const& ref) {
			if (!ref) return;
			if (locked) return;
			unbindReference(*ref);
			ref.destroy();
		}

		/// @brief IRREVERSIBLE. bakes and locks the object.
		void bakeAndLock();
		
		/// @brief Extends the renderable with a series of vertices.
		/// @param vertices Vertices to copy and extend from.
		/// @param size Vertex count.
		/// @throw Error::InvalidValue if no vertices were passed, or vertex count is not a multiple of 3. 
		void extend(Vertex* const vertices, usize const size);

		/// @brief Extends the renderable with another renderable.
		/// @param other Renderable to copy and extend from.
		void extend(Renderable const& other);

		/// @brief Extends the renderable from a binary file.
		/// @param path Path to file.
		/// @throw Error::FailedAction If file size does not constitute a valid vertex set.
		/// @throw Error::FileLoadError If file is empty.
		void extendFromBinaryFile(String const& path);

		/// @brief Extends the renderable from a renderable object definition file.
		/// @param path Path to file.
		/// @throw Error::FailedAction For any JSON-related error.
		/// @throw Error::InvalidValue For any file-related eror.
		void extendFromDefinitionFile(String const& path);

		/// @brief Bakes the object.
		/// @details
		///		Pre-processes all references, and copies the triangles' vertices
		///		To the internal buffer that is used when rendering.
		///
		///		Any reference transformation no longer affects the object.
		///		In return, speeds up render time substantially.
		///		
		///		If you need speed, use this.
		void bake();

		/// @brief Unbakes the object.
		void unbake();

		/// @brief
		///		Destroys all references and deletes all triangles associated with the object.
		///		Also deletes the internal vertex buffer.
		void clearData();

		/// @brief Saves the object's triangles to a binary file.
		/// @param path Path to file.
		void saveToBinaryFile(String const& path);

		/// @brief Saves the object to an renderable object definition file.
		/// @param folder Folder to save the object in.
		/// @param name Object name.
		/// @param texturesFolder Folder to save textures in (if not integrated into file).
		/// @param integratedBinary Whether to integrate the object's triangles into the file.
		/// @param integratedTextures Whether to integrate the object's textures into the file.
		/// @param pretty Whether to make the file "human-readable".
		void saveToDefinitionFile(
			String const& folder,
			String const& name				= "object",
			String const& texturesFolder	= "tx",
			bool const integratedBinary		= false,
			bool const integratedTextures	= false,
			bool const pretty				= false
		);

		/// @brief Triangles bound to this object.
		List<Triangle*> triangles;

	private:
		void removeReference(IReference& ref);
		void unbindReference(IReference& ref);

		friend class Scene;
		friend class IReference;

		/// @brief Vertices used in rendering.
		Vertex* vertices = nullptr;

		/// @brief Whether the object has been baked.	
		bool baked	= false;
		/// @brief Whether the object is locked.
		bool locked	= false;

		void copyVertices();

		void draw() override;

		/// @brief References bound to this object.
		List<IReference*>	references;

		/// @brief Vertex count.
		usize vertexCount = 0;

		void extendFromDefinition(
			JSON::JSONData def,
			String const& sourcepath
		);

		void extendFromDefinitionV0(
			JSON::JSONData def,
			String const& sourcepath
		);

		JSON::JSONData getObjectDefinition(
			String const& encoding			= "base64",
			bool const integratedBinary		= true,
			bool const integratedTextures	= true
		);
	};
}

#endif // MAKAILIB_GRAPH_RENDERER_RENDERABLE_H
