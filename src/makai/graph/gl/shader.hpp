#ifndef MAKAILIB_GRAPH_SHADER_H
#define MAKAILIB_GRAPH_SHADER_H

#include "../../file/file.hpp"
#include "../../compat/ctl.hpp"

#include "uniform.hpp"
#include "core.hpp"

/// @brief Graphical facilites.
namespace Makai::Graph {
	using ShaderType = SLF::ShaderType;

	/// @brief Shader program container.
	class Shader {
	private:
		struct ShaderProgram;

		using ShaderInstance = Instance<ShaderProgram>;

		/// @brief Underlying shader program instance.
		ShaderInstance instance;

		/// @brief Whether shader exists.
		bool created = false;

		void attach(String const& code, ShaderType const& shaderType);
	public:
		/// @brief Empty constructor.
		Shader();

		/// @brief Creates a shader and associates it to the object.
		/// @param vertexCode Vertex shader code.
		/// @param fragmentCode Fragment shader code.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		Shader(String const& vertexCode, String const& fragmentCode);

		/// @brief Creates a shader and associates it to the object.
		/// @param slfData Shader layout to construct from.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		Shader(SLF::SLFData const& slfData);

		/// @brief Creates a shader and associates it to the object.
		/// @param code Shader code.
		/// @param shaderType Shader type.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		Shader(String const& code, ShaderType const& shaderType);

		/// @brief Copy constructor (`Shader`).
		/// @param other Shader to copy from.
		/// @return Reference to self.
		Shader(Shader const& other);

		/// @brief Move constructor (`Shader`).
		/// @param other Shader to move.
		/// @return Reference to self.
		Shader(Shader&& other);

		/// @brief Default shader programs.
		struct Program {
			/// @brief Default vertex shader program for objects.
			static String const DEFAULT_MAIN_VERT;
			/// @brief Default fragment shader program for objects.
			static String const DEFAULT_MAIN_FRAG;
			/// @brief Default vertex shader program for framebuffers.
			static String const DEFAULT_BUFFER_VERT;
			/// @brief Default fragment shader program for framebuffers.
			static String const DEFAULT_BUFFER_FRAG;
		};

		/// @brief Destructor.
		~Shader();

		/// @brief Returns whether this object has a shader associated with it.
		/// @return Whether shader exists.
		inline bool exists() const;

		/// @brief Creates a shader and associates it to the object, if none exists.
		/// @param vertexCode Vertex shader code.
		/// @param fragmentCode Fragment shader code.
		/// @return If shader already doesn't exist.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		bool create(String const& vertexCode, String const& fragmentCode);

		/// @brief Creates a shader and associates it to the object, if none exists.
		/// @param slfData Shader layout to construct from.
		/// @return If shader already doesn't exist.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		bool create(SLF::SLFData const& slfData);

		/// @brief Creates a shader and associates it to the object, if none exists.
		/// @param code Shader code.
		/// @param shaderType Shader type.
		/// @return If shader already doesn't exist.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		bool create(String const& code, ShaderType const& shaderType);

		/// @brief Creates a shader and associates it to the object.
		/// @param vertexCode Vertex shader code.
		/// @param fragmentCode Fragment shader code.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		void make(String const& vertexCode, String const& fragmentCode);

		/// @brief Creates a shader and associates it to the object.
		/// @param slfData Shader layout to construct from.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		void make(SLF::SLFData const& slfData);

		/// @brief Creates a shader and associates it to the object.
		/// @param code Shader code.
		/// @param shaderType Shader type.
		/// @throw Error::FailedAction if shader compilation/linkage fails.
		void make(String const& code, ShaderType const& shaderType);

		/// @brief Disconnects the shader associated with this object, if any.
		void destroy();

		/// @brief Enables the shader.
		void operator()() const;

		/// @brief Enables the shader.
		void enable() const;

		/// @brief Returns a shader uniform.
		/// @param name Uniform name.
		/// @return Shader uniform.
		Uniform uniform(String const& name) const;

		/// @brief Returns a shader uniform.
		/// @param name Uniform name.
		/// @return Shader uniform.
		Uniform operator[](String const& name) const;

		/// @brief Copy assignment operator (`Shader`).
		/// @param other Shader to copy from.
		/// @return Reference to self.
		Shader& operator=(Shader const& other);

		/// @brief Move assignment operator (`Shader`).
		/// @param other Shader to move.
		/// @return Reference to self.
		Shader& operator=(Shader&& other);

		/// @brief Default shader for (most) 3D objects.
		static Shader DEFAULT;
	};
}

#endif // MAKAILIB_GRAPH_SHADER_H
