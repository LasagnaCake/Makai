# Dialog Scripting Language

## Overview

Comments are done like C/C++ comments.

Default starting character scope is global.

Text color will always be white, unless changed for the specified line.

Comprised of the following commands:

| Comand | Usage |
|:-:|:-|
| `[<character>, ...]` | For specifying characters. See ahead for more details. To add a character to the previous character roster, use the `*` modifier. |
| `@<action>` | For character actions. Will apply to previous `[]` command. For passing parameters, surround the value with parentheses. For multiple parameters, separate them with commas. |
| `!<emotion>` | For character emotions. Will apply to previous `[]` command. |
| `"<text>"` | For character lines. Will apply to previous `[]` command. To add text to the previous spoken line, use the `*` modifier. |
| `#<hex>` | For specifying text colour. Will apply to previous `""` comand. Must be a valid hex colour. |
| `'<number>` | For waiting. MUST be a whole number. |
| `+<flag>` | For enabling flags. Effectively `$<flag> 1`. |
| `-<flag>` | For disabling flags. Effectively `$<flag> 0`. |
| `$<name> <value>` | For setting external values. For passing strings, use double quotes. For multiple parameters, surround them with parentheses, and separate them with commas.  |
| `.` | For waiting for previous commands to finish. User cannot skip this wait. |
| `;` | For waiting for user input to proceed. If autoplay is enabled, waits for the auto-timer to finish. |
| `*` | For modifying certain commands. |

And the following extended commands:

| Comand | Usage |
|:-:|:-|
|`*[<character>, ...]`| Adds characters to the previous `[]` command, instead of overwriting it. |
|`*"<text>"`| Adds text to the previous spoken line, instead of overwriting it. |

Under consideration:

| Comand | Usage |
|:-:|:-|
| `,` | For waiting for previous commands to finish. User can skip this wait. If done so, will proceed from next `;` command. |
| `&<act> {...}` | For defining acts (named blocks) in a file. These will only be executed when jumped to. |
| `:{...}` | For (possibly) interpreted, simple code, or extended command blocks. |
| `:goto <act>` | For performing different acts. Does not return to previous point. |
| `:do <act>` | For performing different acts. Returns to previous point. |

### On the `[]` command

Can be zero or more characters. Each character is separated by a comma.

May be empty, to specify global scope.

May be `[...]`, to specify all characters.

May be `[... : <character> ...]`, to specify all characters, except the ones listed.

Each `<character>` may be followed by an emotion.

## Example

```
// Example dialog:

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
[charlie] !angry @enter . @step-in !scream "Can you two just SHUT UP!?" #f00 ; !angry @step-out '240 .
// Bob and alice are now sad :(
[alice, bob] !sad @step-in . "Sorry..." ;
// Disable autoplay
-autoplay
// Reset wait
$delay 600
// All three exit the scene
[...] @exit .

```