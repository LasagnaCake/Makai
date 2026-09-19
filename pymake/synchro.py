import asyncio
from os import waitpid
import subprocess as sp
from asyncio.threads import to_thread
from collections.abc import Callable, Iterator
from collections.abc import Awaitable
from typing import Any, Generic, Self, TypeVar, cast
from pymake.os import info


Popen = sp.Popen[str]

TYield = TypeVar("TYield")
TWhoKnows = TypeVar("TWhoKnows")
TReturn = TypeVar("TReturn")

Coro = Awaitable

def block_until_done(fn: Coro) -> TReturn:
	return asyncio.run(fn)

class Buddy(Generic[TYield, TWhoKnows, TReturn]):
    Wrapper = Coro

    fn: Callable[..., Wrapper]

    def __init__(self, fn: Callable[..., Wrapper]):
        self.fn = fn

    def __call__(self, *args, **kwargs) -> TReturn:
        return asyncio.run(self.fn(*args, **kwargs))

def buddy[A, B, R](fn: Callable[..., Coro[A, B, R]]) -> Callable[..., R]:
    return Buddy[A, B, R](fn)

async def wrap(*args, **kwargs) -> Popen:
    def waiter(proc: Popen):
        _ = proc.wait()
        return proc
    px = await to_thread(sp.Popen, *args, **kwargs)
    return await to_thread(waiter, px)

async def spawn(*args, **kwargs) -> Popen:
    return await wrap(*args, **kwargs, shell=False)

async def pipe_into(from_proc: list[str], to_proc: list[str]) -> Popen:
    pin = await wrap(from_proc, shell=False, stdout=sp.PIPE)
    return await wrap(to_proc, shell=False, stdin=pin.stdout)

class Group:
    in_parallel = True

    Process = Coro[Any, Any, Popen]
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
        self.add(wrap(*args, **kwargs, shell=False))

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

def join_groups(*groups: Group) -> Group:
    procs = Group.Processes()
    for group in groups:
        procs.extend(group.group)
    new_group = Group()
    new_group.group = procs
    return new_group
