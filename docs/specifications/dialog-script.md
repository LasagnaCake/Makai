# Dialog Scripting Language

## Overview

Comments are done like C/C++ comments.

Text color will always be white, unless changed for the specified line.

Comprised of a few commands:

| Comand | Usage |
|:-:|:-|
| `[<character>, ...]` | For specifying characters. See ahead for more details. |
| `@<action>` | For character actions. Will apply to previous `[...]` command. |
| `!<emotion>` | For specifying character emotes. Will apply to previous `[...]` command. |
| `"<text>"` | For specifying character lines. Will apply to previous `[...]` command. |
| `#<hex>` | For specifying text colour. Will apply to previous `"<text>"` comand. Must be a valid hex color. |
| `'<number>` | For waiting. MUST be a whole number. |
| `+<flag>` | For enabling flags. Effectively `$<flag> 1`. |
| `-<flag>` | For disabling flags. Effectively `$<flag> 0`. |
| `$<name> <value>` | For setting values. For setting strings, use double quotes. |
| `.` | For waiting for previous commands to finish. User cannot skip this wait. |
| `;` | For waiting for user input to proceed. If autoplay is enabled, waits for the auto-timer to finish. |

Under consideration:

| Comand | Usage |
|:-:|:-|
| `,` | For waiting for previous commands to finish. User can skip this wait. If done so, will proceed from next `;` command. |
| `:{...}` | For (possibly) interpreted, simple code. |
| `*<act> {...}` | For defining acts (named blocks) in a file. |
| `goto <act>` | For jumping to different acts. Does not return to previous point. |
| `do <act>` | For executing different acts. Returns to previous point. |

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
[alice, bob] !happy "And now we're both talking!" ; @step-out
// Enable autoplay
+autoplay
// Change wait
$delay 120
// Charlie steps in, angry
[charlie] !angry @enter @step-in . !scream "Can you two just SHUT UP!?" #f00 ; !angry @step-out '240 .
// Bob and alice are now sad :(
[alice, bob] !sad @step-in . "Sorry..." ;
// Disable autoplay
-autoplay
// Reset wait
$delay 600
// All three exit the scene
[...] @exit .

```