#ifndef MAKAILIB_MP_OCL_IMAGE_H
#define MAKAILIB_MP_OCL_IMAGE_H

#include "component.hpp"

/// @brief Open Computing Language facilities
namespace Makai::MP::OpenCL {
	using Image = Context::Image;

	struct Context::Image: Component<Image>, Contextual<Image>, Clonable<Image> {
		friend struct Contextual<Image>;
		friend struct Component<Image>;
		friend struct Clonable<Image>;

		struct Impl;

		struct [[CTL_PACKED_STRUCT]] Format {
			enum class Order: uint32 {
				OCL_IFO_RGBA,
				OCL_IFO_ARGB,
				OCL_IFO_BGRA,
				OCL_IFO_ABGR,
			};

			enum class Layout: uint32 {
				OCL_IFL_I8,
				OCL_IFL_I16,
				OCL_IFL_U8,
				OCL_IFL_U16,
				OCL_IFL_U16_5R_6G_5B,
				OCL_IFL_U16_5R_5G_5B,
				OCL_IFL_U32_10R_10G_10B,
				OCL_IFL_U32_10R_10G_10B_2A,
				OCL_IFL_HALF_FLOAT,
				OCL_IFL_FLOAT,
			};

			enum class Mask: int32 {
				OCL_IFM_IGNORED		= -1,
				OCL_IFM_DISABLED	= 0,
				OCL_IFM_ENABLED		= 1,
			};

			enum class ValidationError: uint32 {
				OCL_IFVE_INVALID_DIMENSION,
				OCL_IFVE_INVALID_CHANNEL_MASK,
				OCL_IFVE_INVALID_CHANNEL_ORDER_FOR_MASK,
				OCL_IFVE_INVALID_CHANNEL_LAYOUT_FOR_MASK,
				OCL_IFVE_INVALID_MASK_FOR_SRGB,
				OCL_IFVE_INVALID_LAYOUT_FOR_NORMALIZED_IMAGE,
			};

			uint32		dimension:	2 = 2;
			Mask		red:		2 = Mask::OCL_IFM_ENABLED;
			Mask		green:		2 = Mask::OCL_IFM_ENABLED;
			Mask		blue:		2 = Mask::OCL_IFM_ENABLED;
			Mask		alpha:		2 = Mask::OCL_IFM_ENABLED;
			Order		order:		4 = Order::OCL_IFO_RGBA;
			Layout		layout:		4 = Layout::OCL_IFL_I8;
			uint32		srgb:		1 = false;
			uint32		normalized:	1 = false;

			static Nullable<ValidationError> validate(Format const& fmt);
		};

		struct [[CTL_PACKED_STRUCT]] Description {
			struct [[CTL_PACKED_STRUCT]] Pitch {
				usize row = 0;
				usize slice = 0;
			};
			usize width;
			usize height;
			usize depth = 0;
			Format format;
			usize count = 1;
			uint32 mipmaps = 0;
			uint32 samples = 0;
			Pitch pitch;

			enum class ValidationError: uint32 {
				OCL_IDVE_INVALID_FORMAT,
				OCL_IDVE_INVALID_DIMENSIONS,
				OCL_IDVE_INVALID_COUNT,
			};

			static Nullable<ValidationError> validate(Description const& desc);
		};

		struct [[CTL_PACKED_STRUCT]] Properties {
			enum class Access: uint8 {
				OCL_IPA_NONE,
				OCL_IPA_READ,
				OCL_IPA_WRITE,
				OCL_IPA_READ_WRITE,
			};

			enum class Memory: uint8 {
				OCL_IPM_COPY,
				OCL_IPM_ALLOCATE,
				OCL_IPM_REFERENCE,
			};

			Access	kernel:		2 = Access::OCL_IPA_READ_WRITE;
			Access	host:		2 = Access::OCL_IPA_READ_WRITE;
			Memory	memory:		2 = Memory::OCL_IPM_ALLOCATE;
			bool	constant:	1 = false;

			enum class ValidationError: uint32 {};

			static Nullable<ValidationError> validate(Properties const& props);
		};

		Image(Context const& context);

		enum class CreateError: usize {
			OCL_ICE_IMAGE_ALREADY_EXISTS,
			OCL_ICE_INVALID_IMAGE_PROPERTIES,
			OCL_ICE_INVALID_IMAGE_DESCRIPTION,
			OCL_ICE_OUT_OF_RESOURCES,
			OCL_ICE_OUT_OF_HOST_MEMORY,
		};

		Nullable<CreateError> create(Properties const& props, Description const& desc);
		Nullable<CreateError> create(Properties const& props, Description const& desc, ByteSpan<> const& data);
		Nullable<CreateError> create(Properties const& props, Description const& desc, ConstByteSpan<> const& data);
		Nullable<CreateError> create(Properties const& props, Description const& desc, Buffer const& data);
		Nullable<CreateError> create(Properties const& props, Description const& desc, MemorySlice const& data);

		Nullable<CreateError> make(Properties const& props, Description const& desc);
		Nullable<CreateError> make(Properties const& props, Description const& desc, ByteSpan<> const& data);
		Nullable<CreateError> make(Properties const& props, Description const& desc, ConstByteSpan<> const& data);
		Nullable<CreateError> make(Properties const& props, Description const& desc, Buffer const& data);
		Nullable<CreateError> make(Properties const& props, Description const& desc, MemorySlice const& data);
	};
}

#endif
