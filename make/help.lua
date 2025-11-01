HELP_MESSAGE = [[
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
]]