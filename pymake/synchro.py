import subprocess as sp
from asyncio.threads import to_thread
from types import CoroutineType
from typing import Any, Generator

__async_allowed = True

def block_until_done(fn, *args, **kwargs):
	awaitable = fn()(*args, **kwargs)
	ret = None
	while True:
		try:
			ret = awaitable.send(*args, **kwargs)
		except StopIteration:
			break
	return ret

def buddy(fn):
	def new_fn(*args, **kwargs):
		block_until_done(fn, *args, **kwargs)
	return new_fn

async def spawn(*args, **kwargs):
	return to_thread(sp.run, *args, **kwargs)

class Group:
	def __init__(self):
		self.group = list[CoroutineType[Any, Any, Any]]()

	def add(self, coro):
		if __async_allowed:
			self.group.append(coro)
		else:
			block_until_done(coro)

	def spawn(self, *args, **kwargs):
		self.add(to_thread(sp.run, *args, **kwargs))

	async def all(self):
		[await item for item in self.group]
