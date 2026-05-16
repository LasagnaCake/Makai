from pymake.osvar import OSDependentValue

MRI_SCRIPTS = OSDependentValue[str](
    """
	create obj/extern/extern.3p.a
	addlib obj/extern/$(THIRD_PARTY_PREFIX).sdl.a
	addlib obj/extern/$(THIRD_PARTY_PREFIX).sdl-net.a
	addlib obj/extern/$(THIRD_PARTY_PREFIX).cryptopp.a
	addlib obj/extern/$(THIRD_PARTY_PREFIX).curl.a
	save
	end
	""",
    """
	create obj/extern/extern.3p.a
	addlib obj/extern/$(THIRD_PARTY_PREFIX).cryptopp.a
	addlib obj/extern/$(THIRD_PARTY_PREFIX).sdl-net.a
	save
	end
	""",
)
