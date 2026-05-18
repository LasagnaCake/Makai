import os
import subprocess


class Step:
    class Library:
        class Type:
            pre: str
            post: str

            def __init__(self, pre: str, post: str = "") -> None:
                self.pre = pre
                self.post = post

            def apply(self, lib: str) -> str:
                return self.pre + lib + self.post

        SLT_OS = Type("-l")
        SLT_STATIC = Type("-L", ".a")
        SLT_SHARED_WINDOWS = Type("-L", ".dll.a")
        SLT_SHARED_UNIX = Type("-l:", ".so")

        include: str
        library: str
        type: Type

        def __init__(self, include: str, library: str, type: Type = SLT_STATIC) -> None:
            self.include = include
            self.library = library
            self.type = type

    class Target:
        file: str
        output: str
        extra_args: list[str]

    targets: list[Target]

    @staticmethod
    def build(program: str, input: str, output: str, args: list[str]):
        subprocess.run(executable=program, args=["-o", output, input] + args)

    def add_target(self, target: Target):
        self.targets.append(target)
        pass  # away

    def process(self, program: str, args: list[str]):
        for target in self.targets:
            Step.build(
                program,
                target.file,
                target.output,
                args.copy() + target.extra_args.copy(),
            )
