from enum import Enum
import re
import os
import sys
from typing import Any
from array import array
from pathlib import Path
import codecs

# TODO: Proper errors
# TODO: Hex color processing

anychr = "[\\S\\s]"

names	= f"[0-z\\-_]"
nonames	= f"[^0-z\\-_]"

tokens	= f"[\\w&!@#$%&><+\\-_\']"
unaries	= f"[*.,;]"
strings	= f"(?<!\\\\)\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\""
parens	= f"\\({anychr}*?\\)"
bracks	= f"\\[{anychr}*?\\]"
lcoms	= f"\\/\\/.*"
bcoms	= f"\\/\\*{anychr}*?\\*\\/"

all_tokens = f"{tokens}+|{unaries}|{strings}|{parens}|{bracks}|{lcoms}|{bcoms}"
all_params = f"[^,]+|{strings}|{parens}|{bracks}|{lcoms}|{bcoms}"

class OperationType(Enum):
	NO_OP		= 0
	HALT		= 1
	ACTOR		= 2
	LINE		= 3
	EMOTION		= 4
	ACTION		= 5
	COLOR		= 6
	WAIT		= 7
	SYNC		= 8
	USER_INPUT	= 9
	NAMED_CALL	= 10
	JUMP		= 11

class Token:
	type: OperationType
	name: str | None
	params: list[Any]
	mode: int  = 0

	def __init__(self, type: OperationType, name: str | None = None, params: list[Any] = [], mode: int = 0):
		self.name	= name
		self.params	= params
		self.type	= type
		self.mode	= mode
	
	def as_operation(self, mode: int = 0) -> int:
		val = int(self.type.value)
		if mode == 0:
			return val | (self.mode << 12)
		else:
			return val | (mode << 12)

def serialize(s: str):
	return s.encode("ascii")

def bytes_to_long(bytes):
	length = len(bytes)
	if length < 8:
		extra = 8 - length
		bytes = b'\000' * extra + bytes
	assert len(bytes) == 8
	return sum((b << (k * 8) for k, b in enumerate(bytes)))

# From here: https://stackoverflow.com/a/28207808
def murmur64(data):
	seed = len(data)
	m = 0xc6a4a7935bd1e995
	r = 47

	MASK = 2 ** 64 - 1

	data_as_bytes = bytearray()

	data_as_bytes.extend(serialize(data))

	h = seed ^ ((m * len(data_as_bytes)) & MASK)

	off = int(len(data_as_bytes)/8)*8
	for ll in range(0, off, 8):
		k = bytes_to_long(data_as_bytes[ll:ll + 8])
		k = (k * m) & MASK
		k = k ^ ((k >> r) & MASK)
		k = (k * m) & MASK
		h = (h ^ k)
		h = (h * m) & MASK

	l = len(data_as_bytes) & 7

	if l >= 7:
		h = (h ^ (data_as_bytes[off+6] << 48))

	if l >= 6:
		h = (h ^ (data_as_bytes[off+5] << 40))

	if l >= 5:
		h = (h ^ (data_as_bytes[off+4] << 32))

	if l >= 4:
		h = (h ^ (data_as_bytes[off+3] << 24))

	if l >= 3:
		h = (h ^ (data_as_bytes[off+2] << 16))

	if l >= 2:
		h = (h ^ (data_as_bytes[off+1] << 8))

	if l >= 1:
		h = (h ^ data_as_bytes[off])
		h = (h * m) & MASK

	h = h ^ ((h >> r) & MASK)
	h = (h * m) & MASK
	h = h ^ ((h >> r) & MASK)

	return h

def strip(s: str) -> str:
	return s[1:-1]

# From: https://stackoverflow.com/a/24519338
def decode_escapes(s: str) -> str:
	ESCAPE_SEQUENCE_RE = re.compile(r'''
		( \\U........      # 8-digit hex escapes
		| \\u....          # 4-digit hex escapes
		| \\x..            # 2-digit hex escapes
		| \\[0-7]{1,3}     # Octal escapes
		| \\N\{[^}]+\}     # Unicode characters by name
		| \\[\\'"abfnrtv]  # Single-character escapes
	)''', re.UNICODE | re.VERBOSE)

	def decode_match(match):
		try:
			return codecs.decode(match.group(0), 'unicode-escape')
		except UnicodeDecodeError:
			# In case we matched the wrong thing after a double-backslash
			return match.group(0)

	return ESCAPE_SEQUENCE_RE.sub(decode_match, s)

def normalize(s: str) -> str:
	return decode_escapes(s.strip())

def unpack_recurse(l: tuple[Any] | list[Any]) -> list[str]:
	out : list[str] = []
	for m in l:
		if m is tuple or m is list:
			out.extend(unpack_recurse(m))
		elif m != "":
			out.append(str(m))
	return out


def unpack(tl: list[Any]) -> list[str]:
	out : list[str] = []
	for m in tl:
		if m is tuple or m is list:
			out.extend(unpack_recurse(m))
		elif m != "":
			out.append(str(m))
	return out

def separate(script: str) -> list[str]:
	matches = re.findall(all_tokens, script)
	if matches is None:
		raise "ERROR"
	return unpack(matches)

def param_separate(script: str) -> list[str]:
	matches = re.findall(all_params, script)
	if matches is None:
		raise "ERROR"
	out = []
	for m in unpack(matches):
		out.append(m.strip())
	print(f"Params: {out}")
	return out

def param_pack(s: str) -> list[str]:
	s = strip(s)
	pp = param_separate(s)
	out = []
	for p in pp:
		match p[0]:
			case '"':
				out.append(normalize(strip(p)))
			case '(' | '[':
				out.extend(param_pack(p))
			case _:
				if len(re.findall("\\s", p)):
					raise "ERROR"
				else:
					out.append(p)
	return out

def tokenize(nodes: list[str]) -> list[Token]:
	tokens: list[Token] = []
	if nodes is None or len(nodes) == 0:
		raise "ERROR"
	skip_next = False
	for i in range(len(nodes)):
		print(nodes[i])
		if nodes[i] is None:
			continue
		if skip_next:
			skip_next = False
			continue
		node = nodes[i].strip()
		nextNode = nodes[i+1] if i+1 < len(nodes) else None
		match (node[0]):
			case '/':
				continue
			case '"':
				tokens.append(Token(OperationType.LINE, None, [normalize(strip(node))]))
			case '@':
				if len(node) < 2:
					raise "ERROR"
				if nextNode is not None and nextNode[0] == '(':
					tokens.append(Token(OperationType.ACTION, node[1:], param_pack(nextNode)))
					skip_next = True
					continue
				else:
					tokens.append(Token(OperationType.ACTION, node[1:]))
			case '$':
				if len(node) < 2:
					raise "ERROR"
				if nextNode is not None:
					if nextNode[0] == '(':
						tokens.append(Token(OperationType.NAMED_CALL, node[1:], param_pack(nextNode)))
					elif nextNode[0] == '"':
						tokens.append(Token(OperationType.NAMED_CALL, node[1:], [normalize(strip(nextNode))]))
					elif not len(re.findall(nonames, nextNode)):
						tokens.append(Token(OperationType.NAMED_CALL, node[1:], [nextNode]))
					else:
						raise "ERROR"
					skip_next = True
					continue
				else:
					raise "ERROR"
			case '!':
				if len(node) < 2:
					raise "ERROR"
				tokens.append(Token(OperationType.EMOTION, node[1:]))
			case '\'':
				if len(node) < 2:
					raise "ERROR"
				tokens.append(Token(OperationType.WAIT, None, [int(node[1:])]))
			case '#':
				if len(node) < 4:
					raise "ERROR"
				if node[1] == '#':
					tokens.append(Token(OperationType.COLOR, None, [murmur64(node[2:])]))
				else:
					continue
				#	raise "Unimplemented!"
				#	tokens.append(Token(OperationType.COLOR, None, []))
			case '[':
				tokens.append(Token(OperationType.ACTOR, None, param_pack(node)))
			case '*':
				tokens.append(Token(OperationType.NO_OP, None, [], 1))
			case '.':
				tokens.append(Token(OperationType.SYNC))
			case ';':
				tokens.append(Token(OperationType.USER_INPUT))
			case '+' | '-':
				tokens.append(Token(OperationType.NAMED_CALL, node[1:], [int(node[0] == '+') + 1]))
			case '(':
				continue
			case _:
				raise "ERROR"
	if len(tokens) == 0:
		raise "ERROR"
	return tokens

class Section:
	start: int
	size: int

	def serialize(self) -> bytearray:
		out = bytearray()
		out.extend(self.start.to_bytes(8))
		out.extend(self.size.to_bytes(8))
		return out

	def offset(self) -> int:
		return self.start + self.size

	@staticmethod
	def size() -> int:
		return 8 * 2

	def __str__(self):
		return f"{{{self.start}, {self.size}}}"

class FileHeader:
	headerSize: int
	version: int	= 0
	minVersion: int	= 0
	flags: int		= 0
	data: Section
	jumps: Section
	code: Section

	def serialize(self) -> bytearray:
		out = bytearray()
		out.extend(self.headerSize.to_bytes(8))
		out.extend(self.version.to_bytes(8))
		out.extend(self.minVersion.to_bytes(8))
		out.extend(self.flags.to_bytes(8))
		out.extend(self.data.serialize())
		out.extend(self.jumps.serialize())
		out.extend(self.code.serialize())
		return out

	@staticmethod
	def size() -> int:
		return 8 * 4 + Section.size() * 3
	
	def __str__(self):
		return f"{{{self.headerSize}, {self.version}, {self.minVersion}, {self.flags}, {self.data}, {self.jumps}, {self.code}}}"

class Program:
	data: list[str]
	jumps: dict[int, int]
	code: bytearray

	def __init__(self):
		self.data	= ["false", "true"]
		self.jumps	= dict[int, int]()
		self.code	= bytearray()

	def add_operation(self, op: int):
		self.code.extend(op.to_bytes(2))

	def add_operand(self, value: int):
		self.code.extend(value.to_bytes(8))

	def add_string_operand(self, value: str):
		self.add_operand(len(self.data)+1)
		self.data.append(value)

	def add_named_operand(self, value: str):
		self.add_operand(murmur64(value))

	def add_param_pack_operand(self, values: list[str]):
		self.add_operand(len(self.data)+1)
		self.add_operand(len(values))
		self.data.extend(values)

	def serialize(self) -> bytearray:
		out = bytearray()
		for entry in self.data:
			out.extend(serialize(entry))
			out.append(0)
		for jump in self.jumps.keys():
			out.extend(jump.to_bytes(8))
			out.extend(jumps[jump].to_bytes(8))
		out.extend(self.code)
		return out

	def get_data_section(self) -> Section:
		section = Section()
		section.start	= FileHeader.size()
		section.size	= 0
		for v in self.data:
			section.size += len(v)
		return section
		
	
	def get_jump_section(self) -> Section:
		section = Section()
		section.start	= self.get_data_section().offset()
		section.size	= len(self.jumps) * 16
		return section


	def get_code_section(self) -> Section:
		section = Section()
		section.start	= self.get_jump_section().offset()
		section.size	= len(self.code)
		return section

def package(prog: Program) -> bytearray:
	out = bytearray()
	fh = FileHeader()
	print(prog.data)
	fh.headerSize = FileHeader.size()
	fh.data		= prog.get_data_section()
	fh.jumps	= prog.get_jump_section()
	fh.code		= prog.get_code_section()
	print(fh)
	out.extend(fh.serialize())
	out.extend(prog.serialize())
	return out

def assemble(tokens: list[Token]) -> Program:
	prog = Program()
	for token in tokens:
		print(f"{{{token.type}, {token.name}, {token.params}, {token.mode}}}")
		match token.type:
			case OperationType.NO_OP | OperationType.HALT | OperationType.SYNC | OperationType.USER_INPUT:
				prog.add_operation(token.as_operation())
			case OperationType.LINE:
				prog.add_operation(token.as_operation())
				prog.add_string_operand(token.params[0])
			case OperationType.ACTOR:
				if token.params is not None and len(token.params) > 0:
					for i in range(len(token.params)):
						if token.params[i] == "...":
							if i > 0:
								raise "ERROR"
							prog.add_operation(token.as_operation(2))
							continue
						if str(token.params[i]).find('.') != -1:
							raise "ERROR"
						prog.add_operation(token.as_operation(i > 0))
						prog.add_named_operand(token.params[i])
				else:
					prog.add_operation(token.as_operation())
					prog.add_operand(0)
			case OperationType.EMOTION:
				prog.add_operation(token.as_operation())
				prog.add_named_operand(token.name)
			case OperationType.COLOR:
				prog.add_operation(token.as_operation())
				prog.add_operand(token.params[0])
			case OperationType.ACTION:
				prog.add_operation(token.as_operation(len(token.params) > 0))
				if len(token.params) > 0:
					prog.add_param_pack_operand(token.params)
			case OperationType.NAMED_CALL:
				prog.add_operation(token.as_operation(len(token.params) >= 2))
				if len(token.params) >= 2:
					prog.add_param_pack_operand(token.params)
				elif token.params[0] is str:
					prog.add_string_operand(token.params[0])
				elif token.params[0] is bool:
					prog.add_operand(int(token.params[0]))
			case OperationType.JUMP | OperationType.WAIT:
				prog.add_operation(token.as_operation())
				prog.add_operand(token.params[0])
	return prog


def compile(path: str, outpath: str):
	src = ""
	with open(path, 'r') as f:
		src = f.read()
	out = package(assemble(tokenize(separate(src))))
	Path(outpath).write_bytes(out)
#	with open(outpath, 'w+b') as f:
#		f.write(out)

if __name__ == "__main__":
	print(all_tokens)
	print(all_params)
	compile(sys.argv[1], sys.argv[2])