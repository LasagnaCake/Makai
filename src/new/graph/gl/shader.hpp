#ifndef MAKAILIB_GRAPH_SHADER_H
#define MAKAILIB_GRAPH_SHADER_H

#include "../file/file.hpp"
#include "../ctl/ctl.hpp"

#include "uniform.hpp"
#include "global.hpp"

namespace Makai::Graph {
	class Shader {
	private:
		struct ShaderProgram;

		using ShaderInstance = Instance<ShaderProgram>;

		ShaderInstance instance;

		bool created = false;

		/// Similar to create, but internal.
		void attach(String const& code, GLuint const& shaderType);
	public:
		Shader();

		Shader(String const& vertexCode, String const& fragmentCode);

		Shader(SLF::SLFData const& slfData);

		Shader(String const& code, GLuint const& shaderType);

		Shader(Shader const& other);

		Shader(Shader&& other);

		~Shader();

		/// Returns whether this object has a shader associated with it (i.e. "is created").
		inline bool isCreated();

		/// Creates a shader and associates it to the object. Returns false if already created.
		bool create(String const& vertexCode, String const& fragmentCode);

		/// Creates a shader from an SLF file and associates it to the object. Returns false if already created.
		bool create(SLF::SLFData const& slfData);

		/// Creates a shader from a given shader code, and a shader type  and associates it to the object. Returns false if already created.
		bool create(String const& code, GLuint const& shaderType);

		void make(String const& vertexCode, String const& fragmentCode);

		void make(SLF::SLFData const& slfData);

		void make(String const& code, GLuint const& shaderType);

		/// Destroys the shader associated with this object, if any. Does not delete object.
		void destroy();

		/// Operator overload.
		void operator()();

		/// Enables the shader object.
		void enable();

		/**
		* The way to set uniforms.
		* Done like this: SHADER.uniform(UNIFORM_NAME)(UNIFORM_VALUE);
		*/
		Uniform uniform(String const& name);

		/**
		* The way to set uniforms.
		* Done like this: SHADER[UNIFORM_NAME](UNIFORM_VALUE);
		*/
		Uniform operator[](String const& name);

		Shader& operator=(Shader const& other);

		Shader& operator=(Shader&& other);
	} defaultShader;
}

#endif // MAKAILIB_GRAPH_SHADER_H