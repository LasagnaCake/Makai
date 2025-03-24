# Anima Language

## Overview

Meant to be a (mostly) gereral-purpose, (kinda) simple-to-use, (somewhat) extensible language, to be used for animation, entity control, dialog, and other uses.

It is compiled into bytecode, and run in a virtual machine (`AVM::Engine`).

All of its necessary components are located in the `AVM` namespace, inside the `Makai::Ex::Game` extension.

To utilize it, simply derive from `AVM::Engine` in your class.

## Philosophy

### The behaviour is defined by the programmer.

By default, all of the commands (except those without associated functions) do nothing. It is up to the programmer to choose whether to change that behaviour. To do so, simply override the associated `op*` function, and implement your own.

For all functions (except `opWait` and `opColor`), all integer values are hashes of the associated name.

Example: `opNamedCallSingle` gets called with an integer `X`, and a string `"Example"`. `X` is the hash for `do-thing`. In the anima source file, this would be seen as `$do-thing "Example"`.

### The language is designed to be used for *extending* behaviour.

It is not designed to be a replacement programming language, but rather, as an extension to the programmer — and the engine's — systems.

## Behaviour

- Default starting character scope is global.

### Dialog-specific behaviour

- Text color will always be white, unless changed for the specified line.

## File extensions

`anima`, `an` → Anima source files.

`anib`, `anb` → Compiled anima binaries.

## Source code structure

Comments are done like C/C++ line an block comments.

Comprised of the following commands, and their recommended use case:

| Command | Usage | Associated Function(s) | Can accept function arguments |
|:-:|:-|:-:|:-:|:-:|
| `[<characters>]` | For specifying the character roster. See ahead for more details. To add a character to the current roster, add the `*` modifier before it. | none | No |
| `@<action>` | For character actions. Will apply to previous `[]` command. For passing parameters, surround the value with parentheses. For multiple parameters, separate them with commas. | `opPerform` | Yes (parameters) |
| `!<emotion>` | For character emotions. Will apply to previous `[]` command. | `opEmote` | No |
| `"<text>"` | For character lines. Will apply to previous `[]` command. To add text to the previous spoken line, add the `*` modifier before it. | `opSay`, `opAdd` | Yes (string interpolation) |
| `#<hex>` | For specifying text colour. Will apply to previous `""` comand. Must be a valid hex colour. To use color references (names), use `##<name>`. | `opColor`, `opColorRef` | No |
| `'<number>` | For setting a time to wait for. MUST be a whole number. | `opDelay` | No |
| `+<flag>` | For enabling flags. Same as `$<flag> "true"`. | `opNamedCallSingle` | No |
| `-<flag>` | For disabling flags. Same as `$<flag> "false"`. | `opNamedCallSingle` | No |
| `$<name> <value>` | For setting external values, and executing named operations. For passing strings, use double quotes. For multiple parameters, surround them with parentheses, and separate them with commas.  | `opNamedCallSingle`, `opNamedCallMultiple` | Yes (parameters) |
| `.` | For waiting for previous commands to finish. User cannot skip this wait. | `opWaitForActions` | N/A |
| `;` | For waiting for user input to proceed. If autoplay is enabled, waits for the auto-timer to finish. | `opWaitForUser` | N/A |
| `*` | For modifying certain commands. | none | N/A |

Also contains the following keywords:

| Keyword | Usage | Associated Function(s) | Can accept function arguments |
|:-:|:-|:-:|:-:|
| `act <name> ... end` | For defining named blocks in a file. These will only be executed when jumped to. | none | No |
| `scene <name> ... end` | For defining named blocks in a file. These will only be executed when jumped to. | none | No |
| `function <name> (<args...>) ... end` | For defining functions. These will only be executed when called. | none | No |
| `next <block-name>` | For jumping to named blocks. **Does not return** to where it was called from, once the block is finished. | none | No |
| `perform <block-name>` | For jumping to named blocks. **Returns** to where it was called from, once the block is finished. | none | No |
| `finish` | Exits the current block early. | none | N/A |
| `terminate` | Exits the program early. | none | N/A |
| `<perform\|next> <params...> (<blocks>)` | Jumps one of the listed blocks, depending on a set of parameters. See ahead for further details. | see ahead | No |
| `call <function> (<args...>)` | For calling functions. See ahead for further details. | see ahead | Yes (Arguments) |

Under consideration:

| Comand | Usage |
|:-:|:-|
| `,` | For waiting for previous commands to finish. User can skip this wait. If done so, will proceed from next `;` command.|
| `<<type-name>\|value> <name> ` | For defining variables. Follows the same path name rules as a jump would. To utilize it in places, use `#` instead of `$`. |
| `<operation-name> <name> <<value>\|#<name>\|$<name>> ` | For setting/modifying variables. |

### On `act`s and `scene`s

Both acts and stories can have named blocks inside them. You can perform a **scene**'s sub-blocks from outside its scene, **but not an act**'s.

To access a scene's sub-block, append `:<sub-block>` to the name.

Jumps will be relative to the current block's scope. To jump to a different block in the **previous scope** (or global, if none), prepend the jump target with `~`. To jump via an **absolute path (starting from global scope)**, prepend the jump target with `:`.

In essence: An `act`'s contents are **private**, and a `scene`'s contents are **public**.

### On `function`s

Named blocks that can take in values.

To call a function, use `call <name> (<args...>)`. Scope rules that apply to `act`s and `scene`s also apply here.

To use an argument inside a function, do it via `%<arg-name>`.

To use string interpolation inside a function, do it via `%<arg-name>%`. To use the `%` character, use `%%` instead.

> [!note]
> You may also execute a function via a `perform` or `next` command, but this method is not recommended, as the compiler **does not check if the function takes arguments or not** in this case.

### On the `next` and `perform` commands

The state is comprised of the current actor state, global SP mode, and execution pointer.

`next` behaves more like how a `goto` would - immediately jumps execution to the block, without resetting or saving the current state, and **does not return to the previous execution point**.

`perform` behaves more like how a function call would. The current state gets pushed in the stack, and then reset to its starting value. Once the named block is done, the stack gets popped, and **the previous state is restored**.

If a `perform` is followed by any amount of `next`s, then the program will return execution to the `perform` statement's 
location.

### On `perform select`s

Associated command: `opGetInt`.

Command: `<perform|next> select <$<name>|random> (<blocks>)`

Jumps to one of the listed blocks. Behaves like a `perform`/`next` command would, depending on which is used.

Jump paths are separated by commas.

To jump depending on a value, use `$<name>`. The value will be clamped between the first choice (0) and the last (amount of choices - 1).

To jump at random, use `random`.

#### Example
```
perform select $ending (good-end, neutral-end, bad-end, none)
```

### On `perform choice`s

Associated command: `opGetChoice`.

Command: `<perform|next> choice <name> (<blocks>)`

Jumps to one of the listed blocks. Behaves like a `perform`/`next` command would, depending on which is used.

Jump paths are separated by commas.

Intended to be used like a dialog choice.

#### Example
```
perform choice choose-your-ending (good-end, neutral-end, bad-end, none)
```

### For any kind of jump list (`<perform|next> <type>`)

For any particular jump target:

- To jump to a given block, pass the name as you would for a `perform`/`next` command.
- To exit the block, use `terminate`/`finish`, depending on your need.
- To simply continue executing, use `none`.
- To repeat the block from the begginning, use `repeat`.

### On the `[]` command

Can be zero or more characters. Each character is separated by a comma.

May be empty, to specify global scope.

May be `[...]`, to specify all characters.

May be `[..., <characters>]`, to specify all characters, except the ones listed.

## Example

The following example is made to be used in conjunction with a `Dialog::AnimaPlayer`:

```
// Set frames to wait for user input before proceeding
$delay 600

// Alice enters the scene, says something, then steps out
[alice] !neutral @enter . @step-in "I'm currently talking!" ; @step-out .
// Bob enters the scene
[bob] !bored @enter @step-in "Now I'm talking..." ;
// Alice steps in
[alice] @step-in
// Both say the same line
*[bob] !happy "And now we're both talking!" ; @step-out
// Enable autoplay
+autoplay
// Change wait
$delay 120
// Charlie steps in, angry
[charlie] !angry @enter . @step-in !scream "Can you two just SHUT UP!?" #f00 ;
!angry *" I'm trying to sleep here." ; @step-out '240 .
// Bob and alice are now sad :(
[alice, bob] !sad @step-in . "Sorry..." ;
// Disable autoplay
-autoplay
// Reset wait
$delay 600
// All three leave the scene
[...] @leave .
```
