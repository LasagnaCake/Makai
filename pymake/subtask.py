import asyncio
import os
from collections.abc import Callable
from types import CoroutineType
from typing import ClassVar, Generic, Self, TypeVar

from pymake.synchro import Group

TYield = TypeVar("TYield")
TWhoKnows = TypeVar("TWhoKnows")
TReturn = TypeVar("TReturn")

Type = CoroutineType[TYield, TWhoKnows, TReturn]

class Subtask(Generic[TYield, TWhoKnows, TReturn]):
    in_parallel = True

    Promise = Type
    Executor = Callable[..., Type]

    fn: Callable[..., Type]

    _queue: ClassVar[list[Promise]] = []

    def __init__(self, fn: Executor):
        self.fn = fn

    def __call__(self: Self, *args, **kwargs) -> Self:
        Subtask._queue.append(self.fn(*args, **kwargs))
        return self

    @staticmethod
    async def sync():
        for task in Subtask._queue:
            try:
                await task
            except RuntimeError:
                continue
        await asyncio.sleep(1)
        Subtask._queue.clear()

def subtask[TYield, TWhoKnows, TReturn](fn: Subtask[TYield, TWhoKnows, TReturn].Executor):
    return Subtask[TYield, TWhoKnows, TReturn](fn)

def checkpoint[TYield, TWhoKnows, TReturn](fn: Subtask[TYield, TWhoKnows, TReturn].Executor):
    async def new_fn(*args, **kwargs):
        await Subtask.sync()
        await fn(*args, **kwargs)
    return new_fn
