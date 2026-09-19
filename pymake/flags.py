from copy import copy, deepcopy
from typing import Self


class Flags:
	flags: list[str]
	def __init__(self: Self, *args: str):
		self.flags = list[str]([*args])

	def add(self: Self, *args: str|Self|list[str]|None) -> Self:
		for arg in args:
			if type(arg) is str:
				self.flags.append(arg)
			elif type(arg) is Flags:
				_ = self.flags.extend(arg.flags)
			elif type(arg) is list:
				self.flags.extend(arg)
			elif arg is None:
				continue
			else:
				raise TypeError(str(type(arg)))
		return self

	def unpack(self: Self) -> list[str]:
		return deepcopy(self.flags)

	def clone(self: Self) -> Self:
		return deepcopy(self)

	def concat(self: Self, *args: str|Self|list[str]) -> Self:
		return self.clone().add(*args)

	def __add__(self: Self, args: str|Self|list[str]) -> Self:
		return self.concat(args)

def concat(*args: str|Flags|list[str]) -> Flags:
	return Flags().concat(*args)
