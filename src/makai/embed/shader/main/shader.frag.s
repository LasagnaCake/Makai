	.global mkEmbed_MainShaderFrag
	.global mkEmbed_MainShaderFrag_Size
	.section .rodata
mkEmbed_MainShaderFrag:
	.incbin "shader.frag"
1:
mkEmbed_MainShaderFrag_Size:
	.int 1b - mkEmbed_MainShaderFrag
