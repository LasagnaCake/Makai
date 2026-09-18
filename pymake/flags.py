from copy import copy, deepcopy
from typing import Self


class Flags:
	flags: list[str]
	def __init__(self: Self, *args: str):
		self.flags = list[str]([*args])

	def add(self: Self, *args: str|Self|list[str]) -> Self:
		for arg in args:
			if arg is str:
				self.flags.append(arg)
			elif arg is Flags:
				self.add(arg.flags)
			elif arg is list[str]:
				self.flags.extend(arg)
		return self

	def unpack(self: Self) -> list[str]:
		return deepcopy(self.flags)

	def clone(self: Self) -> Self:
		return deepcopy(self)

	def __add__(self: Self, args: str|Self|list[str]) -> Self:
		return self.clone().add(args)

def concat(*args: str|Flags|list[str]) -> Flags:
	return Flags().add(*args)
