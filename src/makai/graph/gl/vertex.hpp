#ifndef MAKAILIB_GRAPH_VERTEX_H
#define MAKAILIB_GRAPH_VERTEX_H

#include "../../compat/ctl.hpp"
#include "armature/armature.hpp"

/// @brief Graphical facilites.
namespace Makai::Graph {
	/// @brief Vertex mapping.
	using VertexMap = Map<String, float>;
	
	#pragma pack(push, 1)

	/// @brief Base classes.
	namespace Base {
		/// @brief Basic vertex structure.
		struct BasicVertex {
			/// @brief Bone indices type.
			using BoneIndices	= Array<int32, 4>;
			/// @brief Bone weights type.
			using BoneWeights	= Array<float, 4>;
			/// @brief Bone indices default ID.
			constexpr static int32 const BONE_DEFAULT_ID = -2;
			/// @brief Vertex position.
			Vector3		position	= 0;
			/// @brief Vertex UV.
			Vector2		uv			= 0;
			/// @brief Vertex color.
			Vector4		color		= 1;
			/// @brief Vertex normal.
			Vector3		normal		= 0;
			/// @brief Vertex bone indices.
			BoneIndices	bones		= BoneIndices::withFill(BONE_DEFAULT_ID);
			/// @brief Vertex bone weights.
			BoneWeights	weights		= BoneWeights::withFill(0);
		};
	}

	namespace {
		constexpr usize const REQUIRED_COMPONENT_COUNT = (3+2+4+3+4+4);
	}

	/// @brief 3D vertex.
	struct Vertex: public Base::BasicVertex {
		/// @brief Base type.
		using BaseType = Base::BasicVertex;

		using BaseType::position;
		using BaseType::uv;
		using BaseType::color;
		using BaseType::normal;
		using BaseType::bones;
		using BaseType::weights;

		/// @brief Vertex component count.
		constexpr static usize COMPONENT_COUNT = (sizeof(BaseType) / sizeof(float));

		static_assert(COMPONENT_COUNT == REQUIRED_COMPONENT_COUNT, "Vertex size is off...");

		/// @brief Returns the default vertex mapping.
		static VertexMap defaultMap();

		/// @brief Constructs the vertex from a vertex mapping.
		/// @param vm Vertex mapping to construct from.
		Vertex(VertexMap const& vm);

		/// @brief Constructs the vertex from a series of values.
		/// @param x X position.
		/// @param y Y position. By default, it is zero.
		/// @param z Z position. By default, it is zero.
		/// @param u UV X position. By default, it is zero.
		/// @param v UV Y position. By default, it is zero.
		/// @param r Red channel color. By default, it is one.
		/// @param g Green channel color. By default, it is one.
		/// @param b Blue channel color. By default, it is one.
		/// @param a Alpha channel color. By default, it is one.
		/// @param nx Normal X position. By default, it is zero.
		/// @param ny Normal Y position. By default, it is zero.
		/// @param nz Normal Z position. By default, it is zero.
		/// @param i0 First bone index. By default, it is `DEFAULT_BONE_ID`.
		/// @param w0 First bone weight. By default, it is zero.
		/// @param i1 Second bone index. By default, it is `DEFAULT_BONE_ID`.
		/// @param w1 Second bone weight. By default, it is zero.
		/// @param i2 Third bone index. By default, it is `DEFAULT_BONE_ID`.
		/// @param w2 Third bone weight. By default, it is zero.
		/// @param i3 Fourth bone index. By default, it is `DEFAULT_BONE_ID`.
		/// @param w3 Fourth bone weight. By default, it is zero.
		constexpr Vertex(
			float const x,
			float const y	= 0,
			float const z	= 0,
			float const u	= 0,
			float const v	= 0,
			float const r	= 1,
			float const g	= 1,
			float const b	= 1,
			float const a	= 1,
			float const nx	= 0,
			float const ny	= 0,
			float const nz	= 0,
			int32 const i0	= BONE_DEFAULT_ID,
			float const w0	= 0,
			int32 const i1	= BONE_DEFAULT_ID,
			float const w1	= 0,
			int32 const i2	= BONE_DEFAULT_ID,
			float const w2	= 0,
			int32 const i3	= BONE_DEFAULT_ID,
			float const w3	= 0
		): Vertex(
			Vector3(x, y, z),
			Vector2(u, v),
			Vector4(r, g, b, a),
			Vector3(nx, ny, nz),
			{i0, i1, i2, i3},
			{w0, w1, w2, w3}
		) {}

		/// @brief Constructs the vertex from a series of vectors & arrays.
		/// @param position Vertex position.
		/// @param uv Vertex UV.
		/// @param color Vertex color.
		/// @param normal Vertex normal.
		/// @param bones Vertex bone indices.
		/// @param weights Vertex bone weights.
		constexpr Vertex(
			Vector3 const& position 	= 0,
			Vector2 const& uv			= 0,
			Vector4 const& color		= 1,
			Vector3 const& normal		= 0,
			BoneIndices const& bones	= BoneIndices::withFill(BONE_DEFAULT_ID),
			BoneWeights const& weights	= BoneWeights::withFill(0)
		):	BaseType{position, uv, color, normal, bones, weights} {}

		/// @brief Copy construtor.
		/// @param other `Vertex` to copy from.
		constexpr Vertex(
			Vertex const& other
		):	Vertex(
			other.position, 
			other.uv, 
			other.color, 
			other.normal, 
			other.bones,
			other.weights
		) {}

		/// @brief Sets the vertex attributes in the active vertex array.
		static void setAttributes();
		/// @brief Enables the vertex attributes.
		static void enableAttributes();
		/// @brief Disables the vertex attributes.
		static void disableAttributes();
	};

	static_assert(sizeof(Vertex) == sizeof(Base::BasicVertex), "Vertex size is off...");
	static_assert(sizeof(Vertex) == (sizeof(float) * REQUIRED_COMPONENT_COUNT), "Vertex size is off...");

	/// @brief 3D triangle.
	struct Triangle {
		/// @brief Triangle vertices.
		Vertex verts[3];
	};

	static_assert(sizeof(Triangle) == (sizeof(Vertex) * 3), "Triangle size is off...");
	#pragma pack(pop)

	/// @brief Initial vertex.
	constexpr Vertex INITIAL_VERTEX = Vertex();
}

#endif // MAKAILIB_GRAPH_VERTEX_H
