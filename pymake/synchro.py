import subprocess as sp
from asyncio.threads import to_thread
from types import CoroutineType
from typing import Any, Callable, Generic, Never, TypeVar, cast

__async_allowed = True

def block_until_done[A, B, R](fn: CoroutineType[A, B, R], *args, **kwargs) -> R:
    ret: R|None = None
    while True:
        try:
            ret = cast(R, fn.send(*args, **kwargs))
        except StopIteration:
            break
    return ret or cast(R, None)

class Buddy[Ax, Bx, Rx]:
    fn: Callable[..., CoroutineType[Ax, Bx, Rx]]

    def __init__(self, fn: Callable[..., CoroutineType[Ax, Bx, Rx]]):
        self.fn = fn

    def __call__(self, *args, **kwargs) -> Rx:
        if val := block_until_done(self.fn(*args, **kwargs), *args, **kwargs):
            return val
        return cast(Rx, None)

def buddy[A, B, R](fn: Callable[..., CoroutineType[A, B, R]]) -> Callable[..., R]:
    return Buddy[A, B, R](fn)

async def spawn[A, B, R](*args, **kwargs) -> CoroutineType[A, B, sp.CompletedProcess[str]]:
    return to_thread(sp.run, *args, **kwargs)

class Group:
    group: list[CoroutineType[Any, Any, sp.CompletedProcess[str]]]

    def __init__(self):
        self.group = []

    def add(self, coro: CoroutineType[Any, Any, sp.CompletedProcess[str]]):
        if __async_allowed:
            self.group.append(coro)
        else:
            _ = block_until_done(coro)

    def spawn(self, *args, **kwargs):
        self.add(to_thread(sp.run, *args, **kwargs))

    async def all(self):
        [await item for item in self.group]
