ifdef openmp
export openmp
endif

ifdef debug-release
export debug-release
endif

ifdef no-buffers
export no-buffers
endif

ifdef gl-loader
export gl-loader
else
gl-loader:=glad
export gl-loader
endif

ifdef omp-threads
export omp-threads
else
omp-threads:=128
export omp-threads
endif

ifdef os
export os
else
export os:=win
endif

o?=2
export o

math?=fast
export math

ifdef compiler
export compiler
else
compiler:=msys2-gcc
export compiler
endif

define HELP_MESSAGE
Supported options:
>   openmp        = [ flag ]         : Whether openmp should be enabled        ( DEF: undefined    )
>   omp-threads   = [ number ]       : How many threads openmp should use      ( DEF: 128          )

>   deug-release  = [ flag ]         : Whether enable debugging in release lib ( DEF: undefined    )

>   gl-loader     = [ string ]       : Which GL loading API to use             ( DEF: glad         )

>   o             = [ 0..3|g|s ]     : Optimization level for release lib      ( DEF: 2            )
>   math          = [ string ]       : Math mode                               ( DEF: fast         )

>   subsystem     = [ string ]       : Which subsystem/file to compile         ( DEF: undefined    )

Supported [gl-loader] values:
> glad
> gl3w

Supported [math] values (any other value will be interpreted as 'normal'):
> fast
> safe
> normal

On the [subsystem] option:
If not defined, compiles all subsystems.
MUST be a (dot-separated) path to a file.
To compile EVERYTHING in a subsystem, use "*".
Examples:
	graph.gl.renderer.renderable
	embed.shader
	audio.*
endef
export HELP_MESSAGE

MAKAISRC:=src/makai
MAKAIEXSRC:=src/makai-ex

export MAKAISRC
export MAKAIEXSRC

ifdef gmake
GNU_MAKE	?=$(gmake)
else
GNU_MAKE	?=make
endif

export GNU_MAKE
