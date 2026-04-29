#include <makai/makai.hpp>
#include "base.cc"

using namespace Makai::Anima::V2;

using namespace Toolchain;

constexpr auto const VER = Makai::Data::Version{1};

constinit auto const METAPASS = Makai::obfuscate("Moriarty and the Unnamed Catharsis ~ Microcosm Genesis of Ars Poetica");

constinit auto const PACKAGEKEY = Makai::obfuscate("Binary Interloper of Esoteric Dreams ~ In Another Angelic Devil");

constexpr auto const METAINFO = R"###(
	{
		key		**{{key}}
		source	"concerto.animart.dev"
		version	**{{version}}
	}
)###";

// TODO: This (again)
