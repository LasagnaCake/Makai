#ifndef GRAPHICAL_RENDERER_H
#define GRAPHICAL_RENDERER_H

#include "../anchors.hpp"
#include "gl_shader.hpp"

#define GET_GL_POINTER(start, offset) (void*)((start) + (offset) * sizeof(float))
#define GET_GL_OFFSET(offset) (void*)((offset) * sizeof(float))

#define glSetClearColor(COLOR) glClearColor(COLOR.x, COLOR.y, COLOR.z, COLOR.w);

#include "gl_color.hpp"

#ifndef DEFAULT_BLEND_FUNC
#define DEFAULT_BLEND_FUNC GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO
//#define DEFAULT_BLEND_FUNC GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA
//#define DEFAULT_BLEND_FUNC GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
//#define DEFAULT_BLEND_FUNC GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
//#define DEFAULT_BLEND_FUNC GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
//#define DEFAULT_BLEND_FUNC GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE
//#define DEFAULT_BLEND_FUNC GL_ONE, GL_ONE, GL_ONE, GL_ONE
//#define DEFAULT_BLEND_FUNC GL_SRC_ALPHA, GL_SRC_ALPHA, GL_DST_ALPHA, GL_DST_ALPHA
//#define DEFAULT_BLEND_FUNC GL_SRC_ALPHA, GL_ONE, GL_SRC_ALPHA, GL_ONE
//#define DEFAULT_BLEND_FUNC GL_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE, GL_ONE
#endif // DEFAULT_BLEND_FUNC

#ifndef DEFAULT_BLEND_EQUATION
//#define DEFAULT_BLEND_EQUATION GL_FUNC_ADD, GL_FUNC_REVERSE_SUBTRACT
//#define DEFAULT_BLEND_EQUATION GL_FUNC_ADD, GL_FUNC_ADD
#define DEFAULT_BLEND_EQUATION GL_FUNC_ADD, GL_MAX
#endif // DEFAULT_BLEND_EQUATION

#ifndef GRAPHICAL_PARALLEL_THREAD_COUNT
#define GRAPHICAL_PARALLEL_THREAD_COUNT PARALLEL_THREAD_COUNT
#endif // GRAPHICAL_PARALLEL_THREAD_COUNT

#ifndef GRAPHICAL_PARALLEL_FOR
#define GRAPHICAL_PARALLEL_FOR PRAGMA_PARALLEL_FOR(GRAPHICAL_PARALLEL_THREAD_COUNT)
#endif // GRAPHICAL_PARALLEL_FOR

namespace VecMath {
	Vector2 fromJSONArrayV2(JSONData const& json, Vector2 const& defaultValue = 0) {
		try {
			if (json.is_array())
				return Vector2(
					json[0],
					json[1]
				);
			else if (json.is_number())
				return json.get<float>();
			else return defaultValue;
		} catch (JSON::exception const& e) {
			return defaultValue;
		}
	}

	Vector3 fromJSONArrayV3(JSONData const& json, Vector3 const& defaultValue = 0) {
		try {
			if (json.is_array())
				return Vector3(
					json[0],
					json[1],
					json[2]
				);
			else if (json.is_number())
				return json.get<float>();
			else return defaultValue;
		} catch (JSON::exception const& e) {
			return defaultValue;
		}
	}

	Vector4 fromJSONArrayV4(JSONData const& json, Vector4 const& defaultValue = 0) {
		try {
			if (json.is_array())
				return Vector4(
					json[0],
					json[1],
					json[2],
					json[3]
				);
			else if (json.is_number())
				return json.get<float>();
			else return defaultValue;
		} catch (JSON::exception const& e) {
			return defaultValue;
		}
	}
}

namespace Decoder {
	#define _ENCDEC_CASE(T, F) if (encoding == T) return F(data)
	List<ubyte> decodeData(String const& data, String const& encoding) try {
		_ENCDEC_CASE	("base32",	cppcodec::base32_rfc4648::decode);
		_ENCDEC_CASE	("base64",	cppcodec::base64_rfc4648::decode);
		throw Error::InvalidValue(
			"Invalid encoding: " + encoding,
			__FILE__,
			toString(__LINE__),
			"decodeData"
		);
	} catch (cppcodec::parse_error const& e) {
		throw Error::FailedAction(
			"Failed at decoding byte data!",
			__FILE__,
			toString(__LINE__),
			"decodeData",
			e.what()
		);
	}

	String encodeData(List<ubyte> const& data, String const& encoding) try {
		_ENCDEC_CASE	("base32",	cppcodec::base32_rfc4648::encode);
		_ENCDEC_CASE	("base64",	cppcodec::base64_rfc4648::encode);
		throw Error::InvalidValue(
			"Invalid encoding: " + encoding,
			__FILE__,
			toString(__LINE__),
			"decodeData"
		);
	} catch (cppcodec::parse_error const& e) {
		throw Error::FailedAction(
			"Failed at encoding byte data!",
			__FILE__,
			toString(__LINE__),
			"encodeData",
			e.what()
		);
	}
	#undef _ENCDEC_CASE
}

namespace Drawer {
	namespace {
		//GLuint defBackBuffer

		using
		VecMath::Transform3D,
		VecMath::Transform2D,
		VecMath::srpTransform,
		std::vector;

		using namespace std;
		using namespace Decoder;
	}

	#define RAW_VERTEX_SIZE (sizeof(RawVertex) / sizeof(float))
	#define RAW_VERTEX_BYTE_SIZE sizeof(RawVertex)
	#define RAW_VERTEX_COMPONENTS "x,y,z,u,v,r,g,b,a,nx,ny,nz"
	struct RawVertex {
		float
			x	= 0,
			y	= 0,
			z	= 0,
			u	= 0,
			v	= 0,
			r	= 1,
			g	= 1,
			b	= 1,
			a	= 1,
			nx	= 0,
			ny	= 0,
			nz	= 1;
	};

	typedef HashMap<String, float> VertexMap;

	const VertexMap baseVertexMap = {
		{"x",0},
		{"y",0},
		{"z",0},
		{"u",0},
		{"v",0},
		{"r",1},
		{"g",1},
		{"b",1},
		{"a",1},
		{"nx",0},
		{"ny",0},
		{"nz",1}
	};

	RawVertex toRawVertex(VertexMap vm = baseVertexMap) {
		return RawVertex {
			vm["x"],
			vm["y"],
			vm["z"],
			vm["u"],
			vm["v"],
			vm["r"],
			vm["g"],
			vm["b"],
			vm["a"],
			vm["nx"],
			vm["ny"],
			vm["nz"]
		};
	}

	RawVertex toRawVertex(Vector3 pos, Vector2 uv, Vector4 col = Vector4(1), Vector3 norm = Vector3(0)) {
		RawVertex res;
		res.x = pos.x;
		res.y = pos.y;
		res.z = pos.z;
		res.u = uv.x;
		res.v = uv.y;
		res.r = col.x;
		res.g = col.y;
		res.b = col.z;
		res.a = col.w;
		res.nx = norm.x;
		res.ny = norm.y;
		res.nz = norm.z;
		return res;
	}

	typedef function<void()> const DrawFunc;

	Group::Group<DrawFunc*> layers;

	void renderLayer(size_t const& layerID) {
		for (auto rFunc: layers[layerID]) {
			(*rFunc)();
		}
	}

	void setVertexAttributes() {
		// Position
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			RAW_VERTEX_BYTE_SIZE,
			GET_GL_OFFSET(0)
		);
		// UV
		glVertexAttribPointer(
			1,
			2,
			GL_FLOAT,
			GL_FALSE,
			RAW_VERTEX_BYTE_SIZE,
			GET_GL_OFFSET(3)
		);
		// Color
		glVertexAttribPointer(
			2,
			4,
			GL_FLOAT,
			GL_FALSE,
			RAW_VERTEX_BYTE_SIZE,
			GET_GL_OFFSET(5)
		);
		// Normal
		glVertexAttribPointer(
			3,
			3,
			GL_FLOAT,
			GL_FALSE,
			RAW_VERTEX_BYTE_SIZE,
			GET_GL_OFFSET(8)
		);
	}

	inline void enableVertexAttributes() {
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
	}

	inline void disableVertexAttributes() {
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
	}

	inline void setFrontFace(bool const& clockwise = true) {
		glFrontFace(clockwise ? GL_CW : GL_CCW);
	}

	inline void clearColorBuffer(Vector4 const& color) {
		glSetClearColor(color);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	inline void clearDepthBuffer() {
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	// Vertex Setters

	constexpr void vertexSetPosition(RawVertex& v, Vector3 const& pos) {
		v.x = pos.x;
		v.y = pos.y;
		v.z = pos.z;
	}

	constexpr void vertexSetUV(RawVertex& v, Vector2 const& uv) {
		v.u = uv.x;
		v.v = uv.y;
	}

	constexpr void vertexSetColor(RawVertex& v, Vector4 const& color) {
		v.r = color.x;
		v.g = color.y;
		v.b = color.z;
		v.a = color.w;
	}

	constexpr void vertexSetNormal(RawVertex& v, Vector3 const& n) {
		v.nx = n.x;
		v.ny = n.y;
		v.nz = n.z;
	}

	// Vertex Getters

	constexpr Vector3 vertexGetPosition(RawVertex& v) {
		return Vector3(v.x, v.y, v.z);
	}

	constexpr Vector2 vertexGetUV(RawVertex& v) {
		return Vector2(v.u, v.v);
	}

	constexpr Vector4 vertexGetColor(RawVertex& v) {
		return Vector4(v.r, v.g, v.b, v.a);
	}

	constexpr Vector3 vertexGetNormal(RawVertex& v) {
		return Vector3(v.nx, v.ny, v.nz);
	}

	struct BlendData {
		struct BlendFunctionData {
			GLenum
				srcColor = GL_SRC_ALPHA,
				dstColor = GL_ONE_MINUS_SRC_ALPHA,
				srcAlpha = GL_ONE,
				dstAlpha = GL_ZERO
			;
		} func = {DEFAULT_BLEND_FUNC};
		struct BlendEquationData {
			GLenum
				color = GL_FUNC_ADD,
				alpha = GL_MAX
			;
		} eq = {DEFAULT_BLEND_EQUATION};
	};

	inline void setBlendMode(BlendData const& blend, unsigned int const& drawBuffer = 0) {
		glBlendFuncSeparatei(
			drawBuffer,
			blend.func.srcColor,
			blend.func.dstColor,
			blend.func.srcAlpha,
			blend.func.dstAlpha
		);
		glBlendEquationSeparatei(
			drawBuffer,
			blend.eq.color,
			blend.eq.alpha
		);
	}

	struct Blendable {
		Blendable& setBlendFunction(
			GLenum const& srcColor,
			GLenum const& dstColor,
			GLenum const& srcAlpha,
			GLenum const& dstAlpha
		) {
			blend.func = {srcColor, dstColor, srcAlpha, dstAlpha};
			return *this;
		}

		Blendable& setBlendFunction(
			GLenum const& src,
			GLenum const& dst
		) {
			blend.func = {src, dst, src, dst};
			return *this;
		}

		Blendable& setBlendEquation(
			GLenum const& color,
			GLenum const& alpha
		) {
			blend.eq = {color, alpha};
			return *this;
		}

		Blendable& setBlendEquation(
			GLenum const& eq
		) {
			blend.eq = {eq, eq};
			return *this;
		}

		BlendData blend;

	protected:
		Blendable& setBlend() {
			setBlendMode(blend);
			return *this;
		}
	};

	Vector4 colorFromJSON(JSONData const& json) {
		try {
			if (json.is_array())
				return Vector4(
					json[0],
					json[1],
					json[2],
					json[3]
				);
			else return Vector4(1);
		} catch (JSON::exception const& e) {
			return Vector4(1);
		}
	}

	#include "gl_texture.hpp"
}

namespace VecMath {
	using Drawer::RawVertex;

	void srpTransform(RawVertex& vtx, Matrix4x4 const& tmat) {
		// Position
		Vector3 res = (tmat * Vector4(vtx.x, vtx.y, vtx.z, 1.0f)).toVector3();
		Drawer::vertexSetPosition(vtx, res);
		// Normal
		res = (tmat * Vector4(vtx.nx, vtx.ny, vtx.nz, 1.0f)).toVector3();
		Drawer::vertexSetNormal(vtx, res);
	}

	inline void srpTransform(RawVertex& vtx, Transform3D const& trans) {
		srpTransform(vtx, Matrix4x4(trans));
	}
}

namespace RenderData {
	namespace {
		using
		VecMath::Transform,
		VecMath::Transform2D,
		VecMath::Transform3D,
		VecMath::srpTransform,
		Drawer::RawVertex,
		Drawer::toRawVertex,
		Drawer::DrawFunc,
		Drawer::Texture2D,
		Drawer::vertexGetPosition,
		std::function,
		std::vector,
		std::string;
		using namespace Decoder;
	}

	/// Base triangle data structure.
	struct Triangle {
		RawVertex verts[3];
	};

	class Renderable;

	namespace Material {
		#include "gl_material.hpp"
	}

	namespace Base {
		#include "gl_drawable.hpp"
	}

	namespace Reference3D {
		#include "gl_reference.hpp"
	}

	#include "gl_dummy.hpp"
	#include "gl_renderable.hpp"

	namespace Text {
		#include "gl_text.hpp"
	}

	namespace Bar {
		#include "gl_progressbar.hpp"
	}

	#include "gl_scene.hpp"
}

#include "gl_framebuffer.hpp"

#endif // GRAPHICAL_RENDERER_H