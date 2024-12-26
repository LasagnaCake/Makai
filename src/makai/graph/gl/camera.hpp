#ifndef MAKAILIB_GRAPH_CAMERA_H
#define MAKAILIB_GRAPH_CAMERA_H

#include "../../compat/ctl.hpp"

/// @brief Graphical facilities.
namespace Makai::Graph {
	/// @brief Orthographic projection settings.
	struct OrthographicData {
		/// @brief Projection strength.
		float strength	= 0;
		/// @brief Orthographic origin.
		Vector2 origin	= Vector2(0);
		/// @brief Orthographic size.
		Vector2 size	= Vector2(1);
	};

	/// @brief Converts an orthographic projection into a perspective matrix.
	/// @param data Orthographic projection to convert.
	/// @param zNear Near clip plane.
	/// @param zFar Far clip plane.
	/// @return Resulting matrix.
	constexpr Matrix4x4 asMatrix(OrthographicData const& data, float const zNear, float const zFar) {
		return Math::ortho(
			data.origin.x,
			data.size.x,
			data.size.y,
			data.origin.y,
			zNear,
			zFar
		);
	}

	/// @brief Camera interface.
	struct ICamera {
		/// @brief Returns the camera matrix. Must be implemented.
		/// @return Camera matrix.
		constexpr virtual Matrix4x4 matrix()		const = 0;
		/// @brief Returns the projection matrix. Must be implemented.
		/// @return Projection matrix.
		constexpr virtual Matrix4x4 projection()	const = 0;

		/// @brief Destructor.
		constexpr virtual ~ICamera() {}
	};
	
	/// @brief 3D camera.
	struct Camera3D: ICamera {
		/// @brief Position.
		Vector3 eye;
		/// @brief View target.
		Vector3 at		= Vector3(0, 0, 1);
		/// @brief "Up" direction.
		Vector3 up		= Vector3(0, 1, 0);
		/// @brief Screen aspect ratio.
		Vector2 aspect	= Vector2(4.0, 3.0);
		/// @brief Field of view.
		float fov	= 2.0;
		/// @brief Near clip plane.
		float zNear	= 0.01;
		/// @brief Far clip plane.
		float zFar	= 100.0;
		/// @brief Orthographic setting.
		OrthographicData	ortho;
		/// @brief Whether most points given (3D Perspective) are relative to the camera's position.
		bool relativeToEye = false;

		constexpr virtual ~Camera3D() {}

		/// @brief Returns the camera matrix.
		/// @return Camera matrix.
		constexpr Matrix4x4 matrix() const override final {
			Vector3 camAt = at;
			if (relativeToEye)
				camAt += eye;
			return Math::lookAt(
				eye,
				camAt,
				up
			);
		}
		
		/// @brief Returns the projection matrix.
		/// @return Projection matrix.
		constexpr Matrix4x4 projection() const override final {
			if (ortho.strength == 0.0)
				return Math::perspective(
					fov,
					aspect.tangent(),
					zNear,
					zFar
				);
			if (ortho.strength == 1.0)
				return Math::ortho(
					ortho.origin.x,
					ortho.size.x,
					ortho.size.y,
					ortho.origin.y,
					zNear,
					zFar
				);
			Matrix4x4 om = Math::ortho(
				ortho.origin.x,
				ortho.size.x,
				ortho.size.y,
				ortho.origin.y,
				zNear,
				zFar
			);
			Matrix4x4 pm = Math::perspective(
				fov,
				aspect.tangent(),
				zNear,
				zFar
			);
			return Math::lerp<Matrix4x4>(pm, om, ortho.strength);
		}

		/// @brief Creates a "2D" camera.
		/// @param size Vector space size.
		/// @return "2D" camera.
		constexpr static Camera3D from2D(Vector2 const& size) {
			Camera3D camera;
			camera.eye				= Vec3(0, 0, -100);
			camera.at				= Vec3(0, 0, 1);
			camera.ortho.strength	= 1.0;
			camera.ortho.origin		= 0;
			camera.ortho.size		= size * -1;
			camera.zFar				= 1000.0;
			camera.relativeToEye	= true;
			return camera;
		}

		/// @brief Creates a "2D" camera.
		/// @param size Vector space size.
		/// @param raatio Aspect ratio.
		/// @return "2D" camera.
		constexpr static Camera3D from2D(float const scale, Vector2 const& ratio) {
			return Camera3D::from2D(ratio * scale);
		}
	};

	/// @brief Rotateable 3D camera.
	struct GimbalCamera3D: ICamera {
		/// @brief Position.
		Vector3 position;
		/// @brief Rotation.
		Vector3 rotation;
		/// @brief Screen aspect ratio.
		Vector2 aspect	= Vector2(4.0, 3.0);
		/// @brief Field of view.
		float fov	= 2.0;
		/// @brief Near clip plane.
		float zNear	= 0.01;
		/// @brief Far clip plane.
		float zFar	= 100.0;
		/// @brief Orthographic setting.
		OrthographicData	ortho;

		constexpr virtual ~GimbalCamera3D() {}

		constexpr Matrix4x4 matrix() const override final {
			return toCamera3D().matrix();
		}

		constexpr Matrix4x4 projection() const override final {
			return toCamera3D().projection();
		}

		/// @brief Converts the camera to a 3D camera the engine can use.
		/// @return 3D camera.
		constexpr Camera3D toCamera3D() const {
			Vector3 eye	= position;
			Vector3 at	= Vector3(0, -1, 0);
			Vector3 up	= Vector3(0, 0, 1);
			Matrix4x4 const rmat = Matrix4x4::fromEulerYXZ(rotation);
			Camera3D camera;
			camera.eye				= eye;
			camera.at				= rmat * Vector4(at, 1);
			camera.up				= rmat * Vector4(up, 1);
			camera.aspect			= aspect;
			camera.fov				= fov;
			camera.zNear			= zNear;
			camera.zFar				= zFar;
			camera.ortho			= ortho;
			camera.relativeToEye	= true;
			return camera;
		}

		/// @brief Converts the camera to a 3D camera the engine can use.
		/// @return 3D camera.
		constexpr operator Camera3D() const {
			return toCamera3D();
		}

		/// @brief Constructs the camera from a `Camera3D`.
		/// @param cam Camera to construct from.
		/// @return Reference to self.
		constexpr GimbalCamera3D& fromCamera3D(Camera3D const& cam) {
			auto crot	= Math::getEulerAnglesYXZ(cam.matrix());
			position	= cam.eye;
			rotation	= Vector3(crot.x, crot.y, crot.z);
			aspect		= cam.aspect;
			fov			= cam.fov;
			zNear		= cam.zNear;
			zFar		= cam.zFar;
			ortho		= cam.ortho;
			return (*this);
		}

		/// @brief Constructs the camera from a `Camera3D`.
		/// @param cam Camera to construct from.
		/// @return Reference to self.
		constexpr GimbalCamera3D& operator=(Camera3D const& cam) {
			return fromCamera3D(cam);
		}
	};
}

#endif // MAKAILIB_GRAPH_CAMERA_H
