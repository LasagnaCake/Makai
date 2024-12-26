#include "glapiloader.cc"

#include "shader.hpp"

using namespace Makai; using namespace Makai::Graph;

namespace SLF = Makai::SLF;

using ShaderType = SLF::ShaderType;

using namespace Literals::Text;

constexpr GLuint getGLShaderType(ShaderType const& type) {
	switch (type) {
		default:
		case ShaderType::ST_INVALID:	return GL_FALSE;
		case ShaderType::ST_FRAGMENT:	return GL_FRAGMENT_SHADER;
		case ShaderType::ST_VERTEX:		return GL_VERTEX_SHADER;
		case ShaderType::ST_COMPUTE:	return GL_COMPUTE_SHADER;
		case ShaderType::ST_GEOMETRY:	return GL_GEOMETRY_SHADER;
		case ShaderType::ST_TESS_CTRL:	return GL_TESS_CONTROL_SHADER;
		case ShaderType::ST_TESS_EVAL:	return GL_TESS_EVALUATION_SHADER;
	}
	return GL_FALSE;
}

struct Shader::ShaderProgram {
	ShaderProgram()		{}
	ShaderProgram(bool)	{create();	}
	~ShaderProgram()	{destroy();	}
	void create()	{if (id == 0) id = glCreateProgram();	}
	void destroy()	{if (id != 0) glDeleteProgram(id);		}
	GLuint id = 0;
};

/// Similar to create, but internal.
void Shader::attach(String const& code, ShaderType const& shaderType) {
	// Compile shaders
	GLuint shader;
	int success;
	char infoLog[2048];
	const char* shaderCode = code.cstr();
	// Vertex Shader
	shader = glCreateShader(getGLShaderType(shaderType));
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	// Log compile errors if any
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 2048, NULL, infoLog);
		throw Error::FailedAction("Could not compile Shader!\n"s, infoLog, CTL_CPP_PRETTY_SOURCE);
	};
	// Shader Program
	instance->create();
	glAttachShader(instance->id, shader);
	glLinkProgram(instance->id);
	// Log linking errors if any
	glGetProgramiv(instance->id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(instance->id, 2048, NULL, infoLog);
		throw Error::FailedAction("Could not link shader program!\n"s, infoLog, CTL_CPP_PRETTY_SOURCE);
	}
	glDeleteShader(shader);
}

Shader::Shader() {
	instance.bind(new ShaderProgram());
}

Shader::Shader(String const& vertexCode, String const& fragmentCode) {
	create(vertexCode, fragmentCode);
}

Shader::Shader(SLF::SLFData const& slfData) {
	create(slfData);
}

Shader::Shader(String const& code, ShaderType const& shaderType) {
	create(code, shaderType);
}

Shader::Shader(Shader const& other) {
	instance = other.instance;
}

Shader::Shader(Shader&& other) {
	instance = other.instance;
}

Shader::~Shader() {
	destroy();
}

/// Returns whether this object has a shader associated with it (i.e. "is created").
inline bool Shader::exists() const {
	return created;
}

/// Creates a shader and associates it to the object. Returns false if already created.
bool Shader::create(String const& vertexCode, String const& fragmentCode) {
	if (created) return false;
	created = true;
	attach(vertexCode, ShaderType::ST_VERTEX);
	attach(fragmentCode, ShaderType::ST_FRAGMENT);
	return true;
}

/// Creates a shader from an SLF file and associates it to the object. Returns false if already created.
bool Shader::create(SLF::SLFData const& slfData) {
	if (created) return false;
	String dir = slfData.folder;
	String shaderPath = "";
	String log = "";
	String code;
	for (SLF::ShaderEntry const& shader: slfData.shaders) {
		shaderPath = OS::FS::concatenate(dir, shader.path);
		DEBUGLN(shaderPath);
		if (shader.code.empty())
			code = File::getText(shaderPath);
		else
			code = shader.code;
		try {
			attach(code, shader.type);
		} catch (Error::Generic const& err) {
			log += "\n[[ Error on shader '"s + shaderPath + "'! ]]:\n";
			log += err.what();
		}
	}
	if (!log.empty())
		throw Error::FailedAction("Compilation failure!", log, CTL_CPP_PRETTY_SOURCE);
	created = true;
	return true;
}

/// Creates a shader from a given shader code, and a shader type  and associates it to the object. Returns false if already created.
bool Shader::create(String const& code, ShaderType const& shaderType) {
	if (created) return false;
	attach(code, shaderType);
	created = true;
	return true;
}

void Shader::make(String const& vertexCode, String const& fragmentCode) {
	destroy();
	create(vertexCode, fragmentCode);
}

void Shader::make(SLF::SLFData const& slfData) {
	destroy();
	create(slfData);
}

void Shader::make(String const& code, ShaderType const& shaderType) {
	destroy();
	create(code, shaderType);
}

/// Destroys the shader associated with this object, if any. Does not delete object.
void Shader::destroy() {
	if (created) {
		instance.unbind();
		created = false;
	}
}

/// Operator overload.
void Shader::operator()() const {
	enable();
}

/// Enables the shader object.
void Shader::enable() const {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("Shader::enable");
	#endif // MAKAILIB_DEBUG
	glUseProgram(instance->id);
}

/**
* The way to set uniforms.
* Done like this: SHADER.uniform(UNIFORM_NAME)(UNIFORM_VALUE);
*/
Uniform Shader::uniform(String const& name) const {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context ctx("Shader::uniform");
	#endif // MAKAILIB_DEBUG
	enable();
	Uniform su(name, instance->id);
	return su;
}

/**
* The way to set uniforms.
* Done like this: SHADER[UNIFORM_NAME](UNIFORM_VALUE);
*/
Uniform Shader::operator[](String const& name) const {
	return uniform(name);
}

Shader& Shader::operator=(Shader const& other) {
	instance = other.instance;
	return *this;
}

Shader& Shader::operator=(Shader&& other) {
	instance = other.instance;
	return *this;
}

Shader Shader::DEFAULT = Shader();

extern char mkEmbed_MainShaderVert[];
extern char mkEmbed_MainShaderFrag[];
extern char mkEmbed_BufferShaderVert[];
extern char mkEmbed_BufferShaderFrag[];

extern int mkEmbed_MainShaderVert_Size;
extern int mkEmbed_MainShaderFrag_Size;
extern int mkEmbed_BufferShaderVert_Size;
extern int mkEmbed_BufferShaderFrag_Size;

String const Shader::Program::DEFAULT_MAIN_VERT		= String(mkEmbed_MainShaderVert, mkEmbed_MainShaderVert_Size);
String const Shader::Program::DEFAULT_MAIN_FRAG		= String(mkEmbed_MainShaderFrag, mkEmbed_MainShaderFrag_Size);
String const Shader::Program::DEFAULT_BUFFER_VERT	= String(mkEmbed_BufferShaderVert, mkEmbed_BufferShaderVert_Size);
String const Shader::Program::DEFAULT_BUFFER_FRAG	= String(mkEmbed_BufferShaderFrag, mkEmbed_BufferShaderFrag_Size);
