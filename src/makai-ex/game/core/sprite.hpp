#ifndef MAKAILIB_EX_GAME_SPRITE_H
#define MAKAILIB_EX_GAME_SPRITE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {
	using Sprite = Graph::AnimatedPlaneRef;

	using SpriteInstance	= Makai::Instance<Sprite>;
	using SpriteHandle		= Makai::Handle<Sprite>;

	struct ThreePatchRef: Graph::ShapeRef<6> {
		ThreePatchRef(
			BoundRange const& triangles,
			Graph::ReferenceHolder& parent
		): ShapeRef<6>(triangles, parent) {
			init(head, {0, 1});
			init(body, {2, 3});
			init(tail, {4, 5});
		}

		Handle<Graph::IReference> transform() override {
			if (!fixed) return this;
			// Calculate transformed vertices
			Graph::Vertex patch[8] = {
				origin[0], origin[1], origin[2], origin[3],
				origin[4], origin[5], origin[6], origin[7]
			};
			if (visible) {
				patch[0].position.x -= size.head;
				patch[1].position.x -= size.head;
				patch[2].position.x += size.body;
				patch[6].position.x += size.body;
				patch[3].position.x += size.body + size.tail;
				patch[7].position.x += size.body + size.tail;
				Matrix4x4 mat(local);
				Matrix3x3 normalMat(mat.transposed().invert().truncated(3, 3));
				for(auto& vert: patch)
					doVertex(vert, mat, normalMat);
			} else for (auto& vert: patch)
				vert.position = 0;
			// Apply transformations
			at(head[0])									= patch[0];
			at(head[1])	= at(head[3])	= at(body[0])	= patch[1];
			at(head[2])	= at(head[4])					= patch[4];
			at(head[5])	= at(body[2])	= at(body[4])	= patch[5];
			at(body[1])	= at(body[3])	= at(tail[0])	= patch[2];
			at(body[5])	= at(tail[2])	= at(tail[4])	= patch[6];
			at(tail[1])	= at(tail[3])					= patch[3];
			at(tail[5])									= patch[7];
			setGroup(head, uv.head, color.head);
			setGroup(body, uv.body, color.body);
			setGroup(tail, uv.tail, color.tail);
			return this;
		}

		Handle<Graph::IReference> reset() override {
			at(head[0]).position													= origin[0];
			at(head[1]).position	= at(head[3]).position	= at(body[0]).position	= origin[1];
			at(head[2]).position	= at(head[4]).position							= origin[4];
			at(head[5]).position	= at(body[2]).position	= at(body[4]).position	= origin[5];
			at(body[1]).position	= at(body[3]).position	= at(tail[0]).position	= origin[2];
			at(body[5]).position	= at(tail[2]).position	= at(tail[4]).position	= origin[6];
			at(tail[1]).position	= at(tail[3]).position							= origin[3];
			at(tail[5]).position													= origin[7];
			return this;
		}

		Vector3	origin[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		struct PatchSize {
			float head = 0;
			float body = 0;
			float tail = 0;
		} size;

		struct PatchUV {
			Vector2 head[4] = {0, 0, 0, 0};
			Vector2 body[4] = {0, 0, 0, 0};
			Vector2 tail[4] = {0, 0, 0, 0};
		} uv;

		struct PatchColor {
			Vector4 head[4] = {1, 1, 1, 1};
			Vector4 body[4] = {1, 1, 1, 1};
			Vector4 tail[4] = {1, 1, 1, 1};
		} color;

	private:
		constexpr Graph::Vertex& at(As<usize[2]> const& place) {
			return triangles[place[0]].verts[place[1]];
		}

		constexpr void setGroup(As<usize[6][2]>& list, As<Vector2[4]>& uv, As<Vector4[4]>& color) {
			set(at(list[0]), uv[0], color[0]);
			set(at(list[1]), uv[1], color[1]);
			set(at(list[3]), uv[1], color[1]);
			set(at(list[2]), uv[2], color[2]);
			set(at(list[4]), uv[2], color[2]);
			set(at(list[5]), uv[3], color[3]);
		}

		constexpr static void set(Graph::Vertex& vtx, Vector2 const& uv, Vector4 const& color) {
			vtx.uv		= uv;
			vtx.color	= color;
		}

		constexpr static void init(As<usize[6][2]>& list, As<usize[2]> const& tris) {
			list[0][0] =
			list[1][0] =
			list[2][0] = tris[0];
			list[3][0] =
			list[4][0] =
			list[5][0] = tris[1];
			list[0][1] = 0;
			list[1][1] = 1;
			list[2][1] = 2;
			list[3][1] = 0;
			list[4][1] = 2;
			list[5][1] = 1;
		}

		constexpr static void doVertex(Graph::Vertex& vertex, Matrix4x4 const& mat, Matrix3x3 const& nmat) {
				vertex.position	= Vector4(mat * Vector4(vertex.position, 1)).compensated().xyz();
				vertex.normal	= nmat * vertex.normal;
		}

		As<usize[6][2]> head;
		As<usize[6][2]> body;
		As<usize[6][2]> tail;
	};

	using ThreePatchInstance	= Instance<ThreePatchRef>;
	using ThreePatchHandle		= Handle<ThreePatchRef>;
}

#endif