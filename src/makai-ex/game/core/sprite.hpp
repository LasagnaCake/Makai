#ifndef MAKAILIB_EX_GAME_SPRITE_H
#define MAKAILIB_EX_GAME_SPRITE_H

#include <makai/makai.hpp>

namespace Makai::Ex::Game {
	using Sprite = Graph::AnimatedPlaneRef;

	using SpriteInstance	= Instance<Sprite>;
	using SpriteHandle		= Handle<Sprite>;

	struct ThreePatchRef: Graph::ShapeRef<6> {
		ThreePatchRef(
			List<Graph::Triangle*> const& triangles,
			Graph::ReferenceHolder& parent
		): ShapeRef<6>(triangles, parent) {
			init(head, {triangles[0], triangles[1]});
			init(body, {triangles[2], triangles[3]});
			init(tail, {triangles[4], triangles[5]});
		}

		Handle<Graph::IReference> transform() override {
			for (usize i = 0; i < 4; ++i) {
				head[i]->uv		= uv.head[i];
				head[i]->color	= color.head[i];
			}
			for (usize i = 0; i < 4; ++i) {
				body[i]->uv		= uv.body[i];
				body[i]->color	= color.body[i];
			}
			for (usize i = 0; i < 4; ++i) {
				tail[i]->uv		= uv.tail[i];
				tail[i]->color	= color.tail[i];
			}
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
			*head[0]							= patch[0];
			*head[1]	= *head[3]	= *body[0]	= patch[1];
			*head[2]	= *head[4]				= patch[4];
			*head[5]	= *body[2]	= *body[4]	= patch[5];
			*body[1]	= *body[3]	= *tail[0]	= patch[2];
			*body[5]	= *tail[2]	= *tail[4]	= patch[6];
			*tail[1]	= *tail[3]				= patch[3];
			*tail[5]							= patch[7];
			return this;
		}

		Handle<Graph::IReference> reset() override {
			head[0]->position											= origin[0];
			head[1]->position	= head[3]->position	= body[0]->position	= origin[1];
			head[2]->position	= head[4]->position						= origin[4];
			head[5]->position	= body[2]->position	= body[4]->position	= origin[5];
			body[1]->position	= body[3]->position	= tail[0]->position	= origin[2];
			body[5]->position	= tail[2]->position	= tail[4]->position	= origin[6];
			tail[1]->position	= tail[3]->position						= origin[3];
			tail[5]->position											= origin[7];
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
		constexpr static void init(As<ref<Graph::Vertex>[6]>& list, As<ref<Graph::Triangle>[2]> const& tris) {
			list[0] = &tris[0]->verts[0];
			list[1] = &tris[0]->verts[1];
			list[2] = &tris[0]->verts[2];
			list[3] = &tris[1]->verts[0];
			list[4] = &tris[1]->verts[2];
			list[5] = &tris[1]->verts[1];
		}

		constexpr static void doVertex(Graph::Vertex& vertex, Matrix4x4 const& mat, Matrix3x3 const& nmat) {
				vertex.position	= Vector4(mat * Vector4(vertex.position, 1)).compensated().xyz();
				vertex.normal	= nmat * vertex.normal;
		}

		As<ref<Graph::Vertex>[6]> head;
		As<ref<Graph::Vertex>[6]> body;
		As<ref<Graph::Vertex>[6]> tail;
	};

	using ThreePatchInstance	= Instance<ThreePatchRef>;
	using ThreePatchHandle		= Handle<ThreePatchRef>;
}

#endif