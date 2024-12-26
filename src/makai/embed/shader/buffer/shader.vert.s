	.global mkEmbed_BufferShaderVert
	.global mkEmbed_BufferShaderVert_Size
	.section .rodata
mkEmbed_BufferShaderVert:
	.incbin "shader.vert"
1:
mkEmbed_BufferShaderVert_Size:
	.int 1b - mkEmbed_BufferShaderVert
