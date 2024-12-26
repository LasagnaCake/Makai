#ifndef MAKAILIB_COMPAT_CTL_H
#define MAKAILIB_COMPAT_CTL_H

#include "../ctl/ctl.hpp"

/// @brief Makai core API.
namespace Makai {
	using namespace CTL;
	using namespace CTL::Ex;
	/// @brief Math facilities.
	namespace Math {
		using namespace CTL::Ex::Math;
		using namespace CTL::Math;
	}
	/// @brief Type constraints.
	namespace Type {
		using namespace CTL::Ex::Type;
		using namespace CTL::Type;
	}
	namespace Type = Type;
    using Math::Vector2;
    using Math::Vector3;
    using Math::Vector4;
    using Math::Vec2;
    using Math::Vec3;
    using Math::Vec4;
    using Math::Matrix4x4;
    using Math::Matrix4;
    using Math::Mat4;
    using Math::Matrix3x3;
    using Math::Matrix3;
    using Math::Mat3;
    using Math::Transform2D;
    using Math::Transform3D;
	using namespace Literals::Math;
	using CTL::String, CTL::List, CTL::OrderedMap, CTL::SimpleMap, CTL::Map;
}

#endif // MAKAILIB_COMPAT_CTL_H