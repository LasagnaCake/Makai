import subprocess

from pymake.compiler import Compiler


def make_embed_script(file_path: str, name: str):
    return f"""
		.global {name}
		.global {name}_Size
		.section .rodata
	{name}:
		.incbin "{file_path}"
	1:
	{name}_Size:
		.int 1b - {name}
	"""


def embed(file_path: str, output_path: str, embed_as: str):
    script = make_embed_script(file_path, embed_as)
    subprocess.run(
        executable=Compiler.program,
        args=["-o", output_path, "-x", "assembler", "-", script],
    )
