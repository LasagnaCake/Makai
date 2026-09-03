	.global mkEmbed_MVSX_OCLDecoder
	.global mkEmbed_MVSX_OCLDecoder_Size
	.section .rodata
mkEmbed_MVSX_OCLDecoder:
	.incbin "decoder.ocl"
1:
mkEmbed_MVSX_OCLDecoder_Size:
	.int 1b - mkEmbed_MVSX_OCLDecoder
