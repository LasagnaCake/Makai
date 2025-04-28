#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_SERVERSPAWNER_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_SERVERSPAWNER_H

#include "requestable.hpp"
#include "../server.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct ServerSpawner: ANamedRequestable {
		Random::Generator& rng;
		AServer& server;

		using ObjectHandle = Reference<AServerObject>;

		ServerSpawner(AServer& server, usize const id, Random::Generator& rng): ANamedRequestable(id), server(server), rng(rng) {}

		bool onRequest(Parameters const& params) override final {
			if (!params.contains(ConstHasher::hash("count"))) return;
			usize count = 0;
			try {
				auto const& param = params[ConstHasher::hash("count")];
				if (!param.empty())
					count = toInt64(param.front());
			} catch (...) {
				auto const& param = params[ConstHasher::hash("count")];
				throw Error::InvalidValue("Invalid count of [", param.front(), "]!");
			}
			for (usize i = 0; i < count; ++i)
				if (auto object = server.acquire()) {
					onObjectRequest(i, count, object.as<AServerObject>(), params);
					return true;
				}
			return false;
		}

		virtual void onObjectRequest(usize const id, usize const count, ObjectHandle const& object, Parameters const& params) {
			if (!object) return;
			for (auto const& param: params) {
				if (param.key == ConstHasher::hash("at")) {
					setParameter<Math::Vector2>(object, object->trans.position, param, 0);
					continue;
				}
				switch (param.key) {
					default: continue;
					case (ConstHasher::hash("color")):			setParameter(object, object->color, param, Graph::Color::WHITE); 	continue;
					case (ConstHasher::hash("scale")):			setParameter<Math::Vector2>(object, object->scale, param, 1); 		continue;
					case (ConstHasher::hash("discardable")):	setParameter(object, object->discardable, param, false);		 	continue;
					case (ConstHasher::hash("spawn")):			{object->spawn(); if (param.value.empty()) continue;	}
					case (ConstHasher::hash("spawn-time")):		setParameter<usize>(object, object->spawnTime, param, 5);			continue;
					case (ConstHasher::hash("despawn")):		{object->despawn(); if (param.value.empty()) continue;	}
					case (ConstHasher::hash("despawn-time")):	setParameter<usize>(object, object->despawnTime, param, 10);	 	continue;
					case (ConstHasher::hash("can-collide")): {
						bool tmp = false;
						setParameter<bool>(object, tmp, param, true);
						object->setCollisionState(tmp);
					} continue;
				}
			}
		}

		template<Type::Ex::Math::Vector::Vector T>
		void setParameter(
			ObjectHandle const& object,
			T& prop,
			Parameter const& param,
			T const& fallback = 0
		) {
			prop = getVector(object, param, fallback);
		}

		template<Type::Ex::Math::Vector::Vectorable T>
		void setParameter(
			ObjectHandle const& object,
			Property<T>& prop,
			Parameter const& param,
			T const& fallback = 0
		) {
			prop = getProperty(object, param, fallback);
		}

		template<Type::Primitive T>
		void setParameter(
			ObjectHandle const& object,
			T& prop,
			Parameter const& param,
			T const& fallback = 0
		) {
			prop = getPrimitive<D>(object, param, fallback);
		}

		template<usize D>
		Property<Math::Vector<D>> getProperty(ObjectHandle const& object, Parameter const& param, Math::Vector<D> const& fallback = 0) {
			Property<Math::Vector<D>> prop;
			if (param.value.empty()) return prop;
			if (param.value.size() < 2) {
				prop.value = convert<D>(param.value.front(), fallback);
				prop.start = prop.value;
				return prop;
			}
			usize current = 1;
			props.interpolate = true;
			if (param.value[current].front() == '@')
				preprocess(prop.start, param.key, object, param.value[current++]);
			else prop.start	= convert<D>(param.value[current++], fallback);
			if (current >= param.value.size()) return prop;
			if (param.value[current].front() == '@')
				preprocess(prop.stop, param.key, object, param.value[current++]);
			prop.stop	= convert<D>(param.value[current++], fallback);
			if (current >= param.value.size()) return prop;
			if (param.value[current].front() == '@')
				preprocess(prop.speed, param.key, object, param.value[current++]);
			prop.speed	= convert<1>(param.value[current++], fallback);
			if (current >= param.value.size()) return prop;
			if (param.value[current].front() == '@')
				preprocess(prop.ease, param.key, object, param.value[current++]);
			prop.ease	= getEase(param.value[current++]);
			return prop;
		}

		template <class T>
		T getPrimitive(ObjectHandle const& object, Parameter const& param, T const& fallback = 0) {
			if (params.empty()) return fallback;
			try {
				if (param.value.front().front() == '@') {
					auto out = fallback;
					preprocess(out, param.key, object, param.value.front());
					return out;
				}
				else return String::toNumber<T>(params.front());
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [", params.front(), "] for ", String(nameof<T>()), "!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
		}

		template <class T>
		static T getVector(ObjectHandle const& object, Parameter const& param, T const& fallback = 0) {
			if (params.empty()) return fallback;
			if (params.front().front() == '@') {
				auto out = fallback;
				preprocess(out, param.key, object, param.value.front());
				return out;
			}
			return convert(params.front(), fallback);
		}

		static Math::Ease::Mode getEase(String const& param) {
			if (param.empty()) return Math::Ease::linear;
			StringList ease = param.splitAtFirst('.');
			if (param.size() == 1) return Math::Ease::getMode(ease.front(), "linear");
			return Math::Ease::getMode(ease.front(), ease.back());
		}
		
		virtual bool preprocess(bool& value, usize const id, ObjectHandle const& object, String const& param)				{return processRNG(value, param);	}
		virtual bool preprocess(usize& value, usize const id, ObjectHandle const& object, String const& param)				{return processRNG(value, param);	}
		virtual bool preprocess(ssize& value, usize const id, ObjectHandle const& object, String const& param)				{return processRNG(value, param);	}
		virtual bool preprocess(float& value, usize const id, ObjectHandle const& object, String const& param)				{return processRNG(value, param);	}
		virtual bool preprocess(Vector2& value, usize const id, ObjectHandle const& object, String const& param)			{return processRNG(value, param);	}
		virtual bool preprocess(Vector3& value, usize const id, ObjectHandle const& object, String const& param)			{return processRNG(value, param);	}
		virtual bool preprocess(Vector4& value, usize const id, ObjectHandle const& object, String const& param)			{return processRNG(value, param);	}
		virtual bool preprocess(Math::Ease::Mode& value, usize const id, ObjectHandle const& object, String const& param)	{return false;						}

		template <class T>
		bool processRNG(T& value, String const& param) {
			if (param.empty()) return false;
			StringList const params = param.value.split(':');
			if (params.front() != "@rng") return false;
			if (params.size() == 1) value = rng.template number<T>();
			else if (params.size() > 2) try {
				value = rng.template number<T>(String::toNumber<T>(params[1]), String::toNumber<T>(params[2]));
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [", str, "] for ", String(nameof<T>()), "!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
			return true;
		}

	protected:
		template<usize D>
		static Math::Vector<D> convert(String const& str, Math::Vector<D> const& fallback = 0)
		requires (D == 1) {
			if (str.empty()) return fallback;
			try {
				return String::toNumber<T>(str);
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [", str, "] for ", String(nameof<Vector<D>>()), "!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
		}

		template<usize D>
		static Math::Vector<D> convert(String const& str, Math::Vector<D> const& fallback = 0)
		requires (D > 1) {
			if (str.empty()) return fallback;
			if (D == 4 && str.front() == '#')
				return Graph::Color::fromHexCodeString(str);
			StringList components = str.split(',');
			Math::Vector<D> out;
			usize const end = (components.size() < D ? components.size() : D);
			for (usize i = 0; i < end; ++i) {
				try {
					if (end == 1) out = toFloat(components[i]);
					else out.data[i] = toFloat(components[i]);
				} catch (...) {
					throw Error::InvalidValue(
						toString("Invalid value of [", str, "] for ", String(nameof<Math::Vector<D>>()), " property!"),
						CTL_CPP_PRETTY_SOURCE
					);
				}
			}
			return out;
		}
	};
}

#endif