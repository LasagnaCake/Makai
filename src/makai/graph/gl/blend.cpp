#include "glapiloader.cc"

#include "blend.hpp"

using namespace Makai; using namespace Makai::Graph;

/*BlendMode Blendable::DEFAULT_MODE = {
	{
		BlendFunction::BF_SRC_ALPHA
		BlendFunction::BF_ONE_MINUS_SRC_ALPHA
		BlendFunction::BF_SRC_ALPHA_SATURATE
		BlendFunction::BF_ONE
	},{
		BlendEquation::BE_ADD
		BlendEquation::BE_MAX
	}
};*/

GLenum convert(BlendFunction const& value) {
	using enum BlendFunction;
	switch (value) {
		case BF_ZERO:					return GL_ZERO;
		case BF_ONE:					return GL_ONE;
		case BF_SRC_COLOR:				return GL_SRC_COLOR;
		case BF_ONE_MINUS_SRC_COLOR:	return GL_ONE_MINUS_SRC_COLOR;
		case BF_DST_COLOR:				return GL_DST_COLOR;
		case BF_ONE_MINUS_DST_COLOR:	return GL_ONE_MINUS_DST_COLOR;
		case BF_SRC_ALPHA:				return GL_SRC_ALPHA;
		case BF_ONE_MINUS_SRC_ALPHA:	return GL_ONE_MINUS_SRC_ALPHA;
		case BF_DST_ALPHA:				return GL_DST_ALPHA;
		case BF_ONE_MINUS_DST_ALPHA:	return GL_ONE_MINUS_DST_ALPHA;
		case BF_SRC_ALPHA_SATURATE:		return GL_SRC_ALPHA_SATURATE;
		case BF_CONST_COLOR:			return GL_CONSTANT_COLOR;
		case BF_ONE_MINUS_CONST_COLOR:	return GL_ONE_MINUS_CONSTANT_COLOR;
		case BF_CONST_ALPHA:			return GL_CONSTANT_ALPHA;
		case BF_ONE_MINUS_CONST_ALPHA:	return GL_ONE_MINUS_CONSTANT_ALPHA;
		/*
		case 0x0F: return GL_SRC1_COLOR;
		case 0x10: return GL_ONE_MINUS_SRC1_COLOR;
		case 0x11: return GL_SRC1_ALPHA;
		case 0x12: return GL_ONE_MINUS_SRC1_ALPHA;
		*/
		default: return GL_ONE;
	}
}

GLenum convert(BlendEquation const& value) {
	using enum BlendEquation;
	switch (value) {
		case BE_ZERO:		return GL_ZERO;
		case BE_ONE:		return GL_ONE;
		case BE_ADD:		return GL_FUNC_ADD;
		case BE_SUB:		return GL_FUNC_SUBTRACT;
		case BE_REV_SUB:	return GL_FUNC_REVERSE_SUBTRACT;
		case BE_MIN:		return GL_MIN;
		case BE_MAX:		return GL_MAX;
		default: return GL_FUNC_ADD;
	}
}

void Blendable::useBlendMode(usize const drawBuffer) const {
	#ifdef MAKAILIB_DEBUG
	API::Debug::Context("Blendable::useBlendMode");
	#endif // MAKAILIB_DEBUG
	glBlendFuncSeparatei(
		drawBuffer,
		convert(blend.func.srcColor),
		convert(blend.func.dstColor),
		convert(blend.func.srcAlpha),
		convert(blend.func.dstAlpha)
	);
	glBlendEquationSeparatei(
		drawBuffer,
		convert(blend.eq.color),
		convert(blend.eq.alpha)
	);
}
