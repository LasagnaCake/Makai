	.global mkEmbed_MVSX_OCLDecoder_V1
	.global mkEmbed_MVSX_OCLDecoder_V1_Size
	.section .rodata
mkEmbed_MVSX_OCLDecoder_V1:
	.incbin "decoder.v1.ocl"
1:
mkEmbed_MVSX_OCLDecoder_V1_Size:
	.int 1b - mkEmbed_MVSX_OCLDecoder_V1
