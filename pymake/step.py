import os
import subprocess


class Step:
    class Target:
        file: str
        extra_args: list[str]

    def process(self, program: str, input: str, output: str, args: list[str]):
        subprocess.run(executable=program, args=["-o", output, input] + args)

    def add_target(self, target: Target):
        pass  # away
