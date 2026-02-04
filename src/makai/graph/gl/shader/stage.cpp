#include "../glapiloader.cc"

#include "stage.hpp"

using namespace Makai; using namespace Makai::Graph;

namespace SLF = Makai::SLF;

using ShaderType = SLF::ShaderType;

using namespace Literals::Text;

constexpr ShaderType fromFileExtension(String const& str) {
	if (str == "frag") return ShaderType::ST_FRAGMENT;
	if (str == "vert") return ShaderType::ST_VERTEX;
	if (str == "comp") return ShaderType::ST_COMPUTE;
	if (str == "geom") return ShaderType::ST_GEOMETRY;
	if (str == "tsct") return ShaderType::ST_TESS_CTRL;
	if (str == "tsev") return ShaderType::ST_TESS_EVAL;
	return ShaderType::ST_INVALID;
}

constexpr GLuint getGLShaderType(ShaderType const type) {
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


struct Pipeline::Stage::StageProgram {
	StageProgram()						{				}
	~StageProgram()						{destroy();		}

	StageProgram& destroy()				{
		if (id != 0) glDeleteProgram(id);
		id = 0;
		created = false;
		return *this;
	}

	StageProgram& create(String const& path) {
		if (created) return *this;
		auto const ext	= CTL::OS::FS::fileExtension(path);
		auto const type = fromFileExtension(ext);
		if (type == ShaderType::ST_INVALID)
			throw Error::FailedAction(
				"Failed to create shader stage!",
				"Invalid shader stage type \"" + ext + "\" !",
				CTL_CPP_PRETTY_SOURCE
			);
		return create(File::getText(path), type);
	}

	StageProgram& create(String const& source, ShaderType const type) {
		if (created) return *this;
		if (type == ShaderType::ST_INVALID)
			throw Error::FailedAction(
				"Failed to create shader stage!",
				"Invalid shader stage type!",
				CTL_CPP_PRETTY_SOURCE
			);
		int success;
		char infoLog[2048];
		auto const shaderCode = source.cstr();
		id = glCreateShaderProgramv(getGLShaderType(type), 1, &shaderCode);
		// Log compile errors if any
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(id, 2048, NULL, infoLog);
			throw Error::FailedAction("Could not compile Shader!\n"s, infoLog, CTL_CPP_PRETTY_SOURCE);
		};
		glGetShaderiv(id, GL_LINK_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(id, 2048, NULL, infoLog);
			throw Error::FailedAction("Could not compile Shader!\n"s, infoLog, CTL_CPP_PRETTY_SOURCE);
		};
		this->type	= type;
		created		= true;
		return *this;
	}

	ShaderType	type	= ShaderType::ST_INVALID;
	GLuint		id		= 0;
	bool		created	= false;
};

uint32 Pipeline::Stage::id() const			{return instance->id;		}
ShaderType Pipeline::Stage::type() const	{return instance->type;		}
bool Pipeline::Stage::exists() const		{return instance->created;	}

Pipeline::Stage::Stage() {
	instance = new StageProgram();
}

Pipeline::Stage::~Stage() {}

Pipeline::Stage& Pipeline::Stage::create(String const& path) {
	if (!instance) instance = new StageProgram();
	instance->create(path);
	return *this;
}

Pipeline::Stage& Pipeline::Stage::create(String const& source, ShaderType const type) {
	if (!instance) instance = new StageProgram();
	instance->create(source, type);
	return *this;
}

Pipeline::Stage& Pipeline::Stage::make(String const& path) {
	destroy().create(path);
	return *this;
}

Pipeline::Stage& Pipeline::Stage::make(String const& source, ShaderType const type) {
	destroy().create(source, type);
	return *this;
}

Pipeline::Stage& Pipeline::Stage::destroy() {
	instance->destroy();
}

Pipeline::Stage& Pipeline::Stage::unbind() {
	instance = new StageProgram();
}
