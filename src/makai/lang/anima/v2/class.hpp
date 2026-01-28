#ifndef MAKAILIB_ANIMA_V2_CLASS_H
#define MAKAILIB_ANIMA_V2_CLASS_H

#include "../../../compat/ctl.hpp"

namespace Makai::Anima::V2 {
	struct Class {
		using Type = Instance<Class>;
		using TypeRef = Handle<Class>;
		using Database = Map<ID::VLUID, Type>;

		struct Function {
			using Instance = Instance<Function>;
			using Database = Map<ID::VLUID, Instance>;
			ID::VLUID			id;
			String				name;
			ID::VLUID			result;
			List<ID::VLUID>		args;
			Nullable<uint64>	location;
		};
		

		struct Method {
			using Instance = Instance<Method>;
			ID::VLUID			id;
			String				name;
			bool				isStatic = false;
		};
		
		struct Field {
			using Instance = Instance<Field>;
			ID::VLUID	id;
			String		name;
			ID::VLUID	type;
		};

		struct Property {
			using Instance = Instance<Property>;
			ID::VLUID			id;
			Nullable<ID::VLUID>	getter;
			Nullable<ID::VLUID>	setter;
		};

		ID::VLUID	base;

		ID::VLUID					id;
		String						name;
		Nullable<Data::Value::Kind>	underlying;
		List<Field::Instance>		fields;
		List<Method::Instance>		methods;
		List<Property::Instance>	properties;
		bool						isAny		= false;

		struct Context {
			Database			types;
			Function::Database	functions;
		};

		Data::Value create(Context& context, Data::Value obj = Data::Value::object()) {
			obj.append(context.types[base]->create(context, obj));
			obj["::type"] = name;
			obj["::base"] = context.types[base]->name;
			auto& f = obj["::fields"];
			auto& m = obj["::methods"];
			for (auto& field: fields)
				if (!f.contains(field->name)) {
					f[field->name]["id"]	= field->id;
					f[field->name]["type"]	= field->type;
				}
			for (auto& method: methods) {
				m[method->name]["id"]		= method->id;
				m[method->name]["static"]	= method->isStatic;
			}
			for (auto& prop: properties) {
				if (prop->getter && context.functions[prop->getter.value()]->location) {
					auto g = context.functions[prop->getter.value()];
					m["::get::" + g->name] = g->location.value();
				}
				if (prop->setter && context.functions[prop->setter.value()]->location) {
					auto s = context.functions[prop->setter.value()];
					m["::set::" + s->name] = s->location.value();
				}
			}
			return obj;
		}

		Data::Value serialize() const {
			Data::Value def = def.object();
			auto& f = def["fields"];
			auto& m = def["methods"];
			auto& p = def["properties"];
			for (auto& field: fields) {
				auto& ff = f[f.size()];
				ff["name"] = field->name; 
				ff["type"] = field->type; 
			}
			for (auto& method: methods) {
				m[f.size()] = method->id;
			}
			for (auto& property: properties) {
				auto& pp = p[f.size()];
				pp["id"] = property->id;
				if (property->getter)
					pp["getter"] = property->getter.value();
				if (property->setter)
					pp["setter"] = property->setter.value();
			}
			return def;
		}
	};
}

#endif