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
export gl-loader:=glad
endif

ifdef omp-threads
export omp-threads
else
export omp-threads:=128
endif

export o?=2

export math?=fast

define HELP_MESSAGE
Supported options:
>   openmp        = [ flag ]         : Whether openmp should be enabled        ( DEF: undefined    )
>   omp-threads   = [ number ]       : How many threads openmp should use      ( DEF: 128          )

>   deug-release  = [ flag ]         : Whether to not use buffers in App       ( DEF: undefined    )

>   gl-loader     = [ string ]       : Which GL loading API to use             ( DEF: glad         )

>   o             = [ 0..3|g|s ]     : Optimization level for release lib      ( DEF: 2            )
>   math          = [ string ]       : Math mode                               ( DEF: fast         )

Supported [gl-loader] values:
> glad
> gl3w

Supported [math] values (any other value will be interpreted as 'normal'):
> fast
> safe
> normal
endef
export HELP_MESSAGE

export MAKAISRC:=src/makai
export MAKAIEXSRC:=src/makai-ex