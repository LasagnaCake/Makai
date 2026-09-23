#ifndef CTL_CONTAINER_VARIANT_H
#define CTL_CONTAINER_VARIANT_H

#include "../namespace.hpp"
#include "../templates.hpp"
#include "../meta/pack.hpp"
#include "../typetraits/decay.hpp"
#include "../memory/core.hpp"
#include "../typetraits/typehash.hpp"
#include "../meta/pack.hpp"
#include "nullable.hpp"

CTL_NAMESPACE_BEGIN;

struct Variant {
	struct IResource {
		constexpr virtual ~IResource() {}
		constexpr virtual owner<IResource> clone() const = 0;
		constexpr virtual TypeHash hash() const = 0;
		constexpr virtual pointer value() const = 0;
	};

	template <class T>
	struct Resource: IResource {
		constexpr static TypeHash HASH = TypeHash::forType<T>();
		constexpr virtual ~Resource() {}
		constexpr owner<IResource> clone() const override	{return new Resource(data);	}
		constexpr TypeHash hash() const override			{return HASH;				}
		constexpr pointer value() const override			{return &data;				}

		constexpr Resource(Decay::Unwrap<T> value): data(value) {}
	private:
		T data;
	};

	template <class T>
	constexpr Nullable<T&> get() {
		if (!is<T>()) return null;
		return *(T*)resource->value();
	}

	template <class T>
	constexpr Nullable<T const&> get() const {
		if (!is<T>()) return null;
		return *(T const*)resource->value();
	}

	template <class T>
	constexpr void set(Decay::Unwrap<T> value) const {
		if (resource)
			delete resource;
		resource = new Resource<T>(value);
	}

	template <class T>
	constexpr bool is() const {
		if (!resource) return false;
		return TypeHash::forType<T>() == resource->hash();
	}

	constexpr ~Variant() {if (resource) delete resource;}

	constexpr Variant() {}

	template <Type::Different<Variant> T>
	constexpr Variant(Decay::Unwrap<T> value) {set(value);}

	constexpr Variant clone() const {
		if (!resource) return Variant();
		auto nv = Variant();
		nv.resource = resource->clone();
		return nv;
	}

	constexpr Variant(Variant&& other):			resource(move(other.resource))	{other.resource = nullptr;	}
	constexpr Variant(Variant const& other):	Variant(move(other.clone()))	{							}

	constexpr Nullable<TypeHash> type() const {
		if (!resource) return null;
		return resource->hash();
	}

	constexpr bool empty() const {
		return resource;
	}

	constexpr operator bool() const {
		return !empty();
	}

private:
	owner<IResource> resource = nullptr;
};

using Anything = Variant;

CTL_NAMESPACE_END

#endif
