import asyncio
import subprocess as sp
from asyncio.threads import to_thread
from collections.abc import Callable, Iterator
from types import CoroutineType
from typing import Any, Self, cast

def block_until_done[A, B, R](fn: CoroutineType[A, B, R]) -> R:
	return asyncio.run(fn)

class Buddy[Ax, Bx, Rx]:
    fn: Callable[..., CoroutineType[Ax, Bx, Rx]]

    def __init__(self, fn: Callable[..., CoroutineType[Ax, Bx, Rx]]):
        self.fn = fn

    def __call__(self, *args, **kwargs) -> Rx:
        return asyncio.run(self.fn(*args, **kwargs))

def buddy[A, B, R](fn: Callable[..., CoroutineType[A, B, R]]) -> Callable[..., R]:
    return Buddy[A, B, R](fn)

async def spawn(*args, **kwargs) -> sp.CompletedProcess[str]:
    return await to_thread(sp.run, *args, **kwargs, shell=False, check=True)

class Group:
    in_parallel = True

    Process = CoroutineType[Any, Any, sp.CompletedProcess[str]]
    Processes = list[Process]

    group: Processes

    def __init__(self):
        self.group = []

    def add(self, coro: Process):
        if Group.in_parallel:
            self.group.append(coro)
        else:
            _ = block_until_done(coro)

    def join_with(self, other: Self) -> Self:
        self.group += other.group
        return self

    def spawn(self, *args, **kwargs) -> None:
        self.add(to_thread(sp.run, *args, **kwargs))

    def __iter__(self) -> Iterator[Process]:
        return self.iterate()

    def iterate(self) -> Iterator[Process]:
        return iter(self.group)

    async def await_all(self):
    	for item in self.group:
            try:
                await item
            except RuntimeError:
                continue

@staticmethod
def join_groups(*groups: Group) -> Group:
    procs = Group.Processes()
    for group in groups:
        procs.extend(group.group)
    new_group = Group()
    new_group.group = procs
    return new_group
