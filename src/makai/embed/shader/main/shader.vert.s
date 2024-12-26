	.global mkEmbed_MainShaderVert
	.global mkEmbed_MainShaderVert_Size
	.section .rodata
mkEmbed_MainShaderVert:
	.incbin "shader.vert"
1:
mkEmbed_MainShaderVert_Size:
	.int 1b - mkEmbed_MainShaderVert
