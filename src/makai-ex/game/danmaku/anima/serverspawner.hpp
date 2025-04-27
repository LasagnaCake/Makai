#ifndef MAKAILIB_EX_GAME_DANMAKU_ANIMA_SERVERSPAWNER_H
#define MAKAILIB_EX_GAME_DANMAKU_ANIMA_SERVERSPAWNER_H

#include "requestable.hpp"
#include "../server.hpp"

/// @brief Anima-specific danmaku facilities.
namespace Makai::Ex::Game::Danmaku::Anima {
	struct ServerSpawner: ANamedRequestable {
		AServer& server;

		bool onRequest(Parameters const& params) override final {
			if (auto object = server.acquire()) {
				onObjectRequest(object.as<AServerObject>(), params);
				return true;
			}
			return false;
		}

		virtual void onObjectRequest(Reference<AServerObject> const& object, Parameters const& params) {
			if (!object) return;
			for (auto const& param: params) {
				switch (param.key) {
					default: break;
					case (ConstHasher::hash("at")):				setParameter<Math::Vector2>(object->trans.position, param.value, 0);	break;
					case (ConstHasher::hash("color")):			setParameter(object->color, param.value, Graph::Color::WHITE); 			break;
					case (ConstHasher::hash("scale")):			setParameter<Math::Vector2>(object->scale, param.value, 1); 			break;
					case (ConstHasher::hash("discardable")):	setParameter(object->discardable, param.value, false);		 			break;
					case (ConstHasher::hash("spawn")):			{object->spawn(); if (param.value.empty()) break;	}
					case (ConstHasher::hash("spawn-time")):		setParameter<usize>(object->spawnTime, param.value, 5);			 		break;
					case (ConstHasher::hash("despawn")):		{object->despawn(); if (param.value.empty()) break;	}
					case (ConstHasher::hash("despawn-time")):	setParameter<usize>(object->despawnTime, param.value, 10);			 	break;
					case (ConstHasher::hash("can-collide")): {
						bool tmp = false;
						setParameter<bool>(tmp, param.value, true);
						object->setCollisionState(tmp);
					} break;
				}
			}
		}

		template<class T>
		static void setParameter(
			T& prop,
			StringList const& params,
			T const& fallback = 0
		) requires (Type::Ex::Math::Vector::Vector<T> || Type::Equal<T, float>) {
			prop = getVector(params, fallback);
		}

		template<Type::Ex::Math::Vector::Vectorable T>
		static void setParameter(
			Property<T>& prop,
			StringList const& params,
			T const& fallback = 0
		) {
			prop = getProperty(params, fallback);
		}

		template<Type::Primitive T>
		static void setParameter(
			T& prop,
			StringList const& params,
			T const& fallback = 0
		) {
			prop = getPrimitive<D>(param, fallback);
		}

		template<usize D>
		static Property<Math::Vector<D>> getProperty(StringList const& params, Math::Vector<D> const& fallback = 0) {
			Property<Math::Vector<D>> prop;
			if (params.empty()) return prop;
			if (params.size() < 2) return {convert<D>(params[0], fallback)};
			usize current = 1;
			props.interpolate = true;
			prop.start	= convert<D>(params[current++], fallback);
			if (current >= params.size()) return prop;
			prop.stop	= convert<D>(params[current++], fallback);
			if (current >= params.size()) return prop;
			prop.speed	= convert<1>(params[current++], fallback);
			if (current >= params.size()) return prop;
			prop.ease	= getEase(params[current++]);
			return prop;
		}

		template <class T>
		static T getPrimitive(StringList const& params, T const& fallback) {
			if (params.empty()) return fallback;
			try {
				return String::toNumber<T>(params[0]);
			} catch (...) {
				throw Error::InvalidValue(
					toString("Invalid value of [", params[0], "] for ", String(nameof<T>()), "!"),
					CTL_CPP_PRETTY_SOURCE
				);
			}
		}

		template <class T>
		static T getVector(StringList const& params, T const& fallback) {
			if (params.empty()) return fallback;
			return convert(params[0], fallback);
		}

		static Math::Ease::Mode getEase(String const& param) {
			if (param.empty()) return Math::Ease::linear;
			StringList ease = param.split('.');
			if (param.size() == 1) return Math::Ease::getMode(ease[0], "linear");
			return Math::Ease::getMode(ease[0], ease[1]);
		}

	private:
		template<usize D>
		constexpr static Math::Vector<D> convert(String const& str, Math::Vector<D> const& fallback = 0)
		requires (D == 1) {
			if (str.empty()) return fallback;
			return getPrimitive<float>(str.split(':'), fallback);
		}

		template<usize D>
		constexpr static Math::Vector<D> convert(String const& str, Math::Vector<D> const& fallback = 0)
		requires (D > 1) {
			if (str.empty()) return fallback;
			StringList components = str.split(':');
			usize const end = (components.size() < D ? components.size() : D);
			Math::Vector<D> out;
			for (usize i = 0; i < end; ++i) {
				try {
					out.data[i] = toFloat(components[i]);
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