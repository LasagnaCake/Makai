	.global mkEmbed_MVSXShaderFrag
	.global mkEmbed_MVSXShaderFrag_Size
	.section .rodata
mkEmbed_MVSXShaderFrag:
	.incbin "mvsx.frag"
1:
mkEmbed_MVSXShaderFrag_Size:
	.int 1b - mkEmbed_MVSXShaderFrag
