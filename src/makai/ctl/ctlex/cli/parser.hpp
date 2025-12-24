#ifndef CTL_EX_CLI_PARSER_H
#define CTL_EX_CLI_PARSER_H

#include "../../ctl/ctl.hpp"
#include "../data/value.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace CLI {
	struct Parser {
		using Translation = Dictionary<String>;

		Translation tl;

		struct OptionStream {
			using Option = KeyValuePair<String, Data::Value>;

			constexpr OptionStream(usize const argc, ref<cstring> const argv):
				count(argc),
				args(argv),
				current(1) {}

			constexpr bool next() {
				if (current >= count)
					return false;
				++current;
				return true;
			}

			constexpr Option value() {
				Option opt;
				opt.key = args[current];
				if (opt.key.size() < 2) return opt;
				if (opt.key.front() == '-') {
					if (isLowercaseChar(opt.key[1]) || opt.key[1] == '-') {
						opt.key = opt.key.substring(1 + (opt.key[1] == '-'));
						++current;
						if (current >= count)
							opt.value = Data::Value::undefined();
						else
							opt.value = Data::Value::StringType(args[current]);
					} else {
						opt.key = opt.key.substring(1);
						opt.value = true;
					}
				}
				return opt;
			}

		private:
			usize			count;
			ref<cstring>	args;
			usize			current;
		};

		constexpr Parser(OptionStream const& stream): stream(stream) {}

		constexpr Parser(usize const argc, ref<cstring> const argv): stream(argc, argv) {}

		constexpr Data::Value parse(Data::Value const& base = Data::Value::object()) {
			Data::Value result = base.isObject() ? base : Data::Value::object();
			if (!(result.contains("__args") && result["__args"].isArray()))
				result["__args"] = Data::Value::array();
			while (stream.next()) {
				auto [key, value] = stream.value();
				while (tl.contains(key))
					key = tl[key];
				if (value.empty()) {
					auto& args = result["__args"];
					args[args.size()] = key;
				} else result[key] = value;
			}
			return result;
		}

	private:
		OptionStream stream;
	};
}

CTL_EX_NAMESPACE_END

#endif