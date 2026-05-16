sinclude options.make

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

ifdef arch
export arch
else
export arch:=arm64
endif

o?=2
export o

ifdef debug-tooling
export debug-tooling
endif

math?=fast
export math

ifdef compiler
export compiler
else
ifeq ($(os),win)
compiler:=msys2-gcc
else
compiler:=gcc
endif
export compiler
endif

define HELP_MESSAGE
Supported compilation targets:
> it      : Debug Library + Release Library + Tooling
> all     : Debug Library + Release Library
> debug   : Debug Library
> release : Release Library
> tooling : Tooling (Requires library to be built)

Supported options:
>   openmp        = [ flag ]         : Whether openmp should be enabled        ( DEF: undefined    )
>   omp-threads   = [ number ]       : How many threads openmp should use      ( DEF: 128          )

>   deug-release  = [ flag ]         : Whether enable debugging in release lib ( DEF: undefined    )
>   deug-tooling  = [ flag ]         : Whether enable debugging in tooling     ( DEF: undefined    )

>   gl-loader     = [ string ]       : Which GL loading API to use             ( DEF: glad         )

>   o             = [ 0..3|g|s ]     : Optimization level for release lib      ( DEF: 2            )
>   math          = [ string ]       : Math mode                               ( DEF: fast         )

>   subsystem     = [ string ]       : Which subsystem/file to compile         ( DEF: undefined    )

>   lite          = [ flag ]         : Whether to bundle specific libraries    ( DEF: undefined    )

>   os            = [ string ]       : Target OS to compile for                ( DEF: win          )
>   compiler      = [ string ]       : Compiler toolchain to use               ( DEF: auto         )
>   gmake         = [ string ]       : Name of GNU Make executable             ( DEF: make         )

Supported [gl-loader] values:
> glad
> gl3w

Supported [math] values (any other value will be interpreted as 'normal'):
> fast
> safe
> normal

Supported [os] values:
> win (Windows)
> linux (Linux)

Supported [compiler] values:
> msys2-gcc
> msys2-clang
> mingw-win
> mingw-linux
> auto

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

export LIBFILE_SRC

export GNU_MAKE
