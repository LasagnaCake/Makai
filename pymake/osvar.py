class OSDependentValue[T]:
    windows: T
    linux: T

    def __init__(self, windows: T, linux: T) -> None:
        self.windows = windows
        self.linux = linux

    def __getitem__(self, name: str) -> T:
        if name in ["windows", "win", "nt"]:
            return self.windows
        if name in ["linux", "unix", "posix"]:
            return self.linux
        return self.windows
