#ifndef MAKAILIB_GRAPH_RENDERER_SCENE_H
#define MAKAILIB_GRAPH_RENDERER_SCENE_H

#include "drawable.hpp"
#include "renderable.hpp"
#include "../material/material.hpp"

/// @brief Graphical facilities.
namespace Makai::Graph {
	/// @brief Type must be a valid camera type.
	template<class T>
	concept CameraType = Makai::Type::Convertible<T, Camera3D>;

	/// @brief Base classes.
	namespace Base {
		/// @brief Collection of drawable objects.
		/// @tparam TDraw 
		template<GLDrawableType TDraw>
		class Collection: public IDrawable {
		public:
			/// @brief Drawable object type.
			using DrawableType	= TDraw;
			
			/// @brief Drawable object entry type.
			using DrawableEntry	= KeyValuePair<String, DrawableType*>;
			/// @brief Drawable object database type.
			using DrawableBank	= Dictionary<DrawableType*>;

			/// @brief Constructs the collection.
			/// @param layer Layer to register the object to. By default, it is layer zero.
			/// @param manual Whether the object is manually rendered. By default, it is `false`.
			Collection(usize const layer = 0, bool const manual = false): IDrawable(layer, manual) {}

			/// @brief Destructor.
			virtual ~Collection() {
				destroy();
			}
			
			/// @brief Global transform.
			Transform3D	space;

			/// @brief Deletes all drawable objects in the collection.
			void destroy() {
				auto objs = objects;
				for (auto& [n, o]: objs)
					delete o;
				objects.clear();
			}

			/// @brief Creates a drawable object.
			/// @param name Name of the object.
			/// @return Key-value pair containing its name, and a pointer to it.
			/// @note Pointer returned is a non-owning pointer.
			/// @throw Error::InvalidValue if name is invalid, or if name is duplicate (if enabled).
			DrawableEntry createObject(String name = "") {
				#ifdef MAKAILIB_SCENE_ERROR_IF_DUPLICATE_NAME
				if (objects.contains(name))
					throw Error::InvalidValue("Object of name '"+name+"' already exists!", CTL_CPP_PRETTY_SOURCE);
				#endif // MAKAILIB_SCENE_ERROR_IF_DUPLICATE_NAME
				DrawableType* r = nullptr;
				if (name.empty())	name = validateName("unnamed");
				else				name = validateName(name);
				objects[name] = (r = new DrawableType(true));
				return DrawableEntry{name, r};
			}

			/// @brief Deletes a drawable object.
			/// @param obj Object to delete.
			void deleteObject(DrawableType* const obj) {
				if (obj)
					for (auto& [k, v]: objects)
						if (obj == v) {
							objects.erase(k);
							delete obj;
							break;
						}
			}

			/// @brief Deletes a drawable object.
			/// @param name Name of object to delete.
			void deleteObject(String const& name = "") {
				if (!objects.contains(name)) return;
				delete objects[name];
				objects.erase(name);
			}

			/// @brief Returns all objects this collection has.
			/// @return All objects.
			/// @note Pointers returned are non-owning pointers. 
			inline DrawableBank getObjects() {
				return objects;
			}

			/// @brief Returns an object's name.
			/// @param obj Object to get the name for.
			/// @return Name of object.
			String getNameOfObject(DrawableType* const obj) {
				String name = "";
				if (obj)
					for (auto& [k, v]: objects)
						if (obj == v)
							name = k;
				return name;
			}

			/// @brief Gets an object by its name.
			/// @param name Name of object.
			/// @return Requested object, or `nullptr` if not found.
			/// @note Pointer returned is a non-owning pointer.
			DrawableType* getObject(String const& name) {
				if (!objects.contains(name))
					return nullptr;
				return objects[name];
			}

			/// @brief Gets an object by its name.
			/// @param name Name of object.
			/// @return Requested object, or `nullptr` if not found.
			/// @note Pointer returned is a non-owning pointer.
			inline DrawableType* operator[](String const& name) {
				return getObject(name);
			}

			/// @brief Returns if a given name is a valid name.
			/// @param name Name to check.
			/// @return Whether it is valid.
			static bool isValidName(String const& name) {
				return !Regex::contains(name, "([\\cA-\\cZ]|[ \\t\"\\\\/?*<>:|])");
			}

		protected:
			String validateName(String const& name) {
				if (!isValidName(name))
					throw Error::InvalidValue(
						"Name must not contain control and/or specific characters!",
						"Name must not contain control characters, and the following:"
						"\n- newlines, spaces or tabs"
						"\n- \\, /, ?, *, <, >, :, \" and/or |",
						CTL_CPP_PRETTY_SOURCE
					);
				String newName = name;
				usize i = 0;
				while (objects.contains(newName))
					newName = name + toString(i++);
				return newName;
			}

			/// @brief Objects associated with collection.
			DrawableBank objects;
		};
	}

	/// @brief Renderable scene.
	class Scene: public Base::Collection<Renderable> {
	public:
		using BaseType = Base::Collection<Renderable>;

		using BaseType::BaseType;

		/// @brief Scene world material.
		Material::WorldMaterial	world;
		/// @brief Scene camera.
		GimbalCamera3D			camera;

		/// @brief Latest scene definition file version supported.
		constexpr static usize VERSION = 0;

		/// @brief Constructs the scene from a file.
		/// @param layer Layer to register the scene to.
		/// @param path Path to scene definition file.
		/// @param manual Whether the object is manually rendered. By default, it is `false`.
		Scene(usize const layer, String const& path, bool manual = false);

		/// @brief Extends the scene from a scene definition file.
		/// @param path Path to scene definition file.
		void extendFromSceneFile(String const& path);

		/// @brief Saves the scene to an scene definition file.
		/// @param folder Folder to save definition in.
		/// @param name Name of the scene.
		/// @param integratedObjects Whether to integrate the scene objects into the file.
		/// @param integratedObjectBinaries Whether to integrate the scene objects' triangles into the file.
		/// @param integratedObjectTextures Whether to integrate the scene objects' textures into the file.
		/// @param pretty Whether to make the file "human-readable".
		void saveToSceneFile(
			String const& folder,
			String const& name,
			bool const integratedObjects		= false,
			bool const integratedObjectBinaries	= false,
			bool const integratedObjectTextures	= false,
			bool const pretty = false

		);

		/// @brief Extends the scene from another scene.
		/// @param other Scene to extend from.
		void extend(Scene const& other);

	private:
		void draw() override;

		void extendFromDefinition(JSON::JSONData const& def, String const& sourcepath);

		void extendFromDefinitionV0(JSON::JSONData def, String const& sourcepath);

		JSON::JSONData getSceneDefinition(
			bool const integratedObjects			= true,
			bool const integratedObjectBinaries	= true,
			bool const integratedObjectTextures	= true
		);
	};
}

#endif // MAKAILIB_GRAPH_RENDERER_SCENE_H
