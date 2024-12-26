	.global mkEmbed_BufferShaderFrag
	.global mkEmbed_BufferShaderFrag_Size
	.section .rodata
mkEmbed_BufferShaderFrag:
	.incbin "shader.frag"
1:
mkEmbed_BufferShaderFrag_Size:
	.int 1b - mkEmbed_BufferShaderFrag
