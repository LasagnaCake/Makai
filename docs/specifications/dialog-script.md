# Dialog Scripting Language

## Overview

Comments are done like C/C++ comments.

Comprised of four basic commands:

| Comand | Usage |
|:-:|:-|
| `[<character>, ...]` | For specifying characters. See ahead for more details. |
| `@<action>` | For character actions. Will apply to previous `[...]` command. |
| `!<emotion>` | For specifying character emotes. Will apply to previous `[...]` command. |
| `"<text>"` | For specifying character lines. Will apply to previous `[...]` command. |
| `#<hex>` | For specifying text colour. Will apply to previous `"<text>"` comand. Must be a valid hex color. |
| `.<number>` | For waiting. MUST be a whole number. User cannot skip this wait. |
| `+<flag>` | For enabling flags. |
| `-<flag>` | For disabling flags. |
| `$<name> <value>` | For setting values. For setting strings, use double quotes. |
| `,` | For waiting for previous commands to finish. |
| `;` | For waiting for user input to proceed. |

Text color will always be white, unless changed for the specified line.

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

// Alice enters the scene, says something, steps out, then waits
[alice] !neutral @enter @step-in "I'm currently talking!" ; @step-out ,
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
[charlie] !angry @enter @step-in , "Can you two just SHUT UP!?" #f00 ; @step-out , .240 ,
// Bob and alice are now sad :(
[alice, bob] !sad @step-in , "Sorry..." ;
// Disable autoplay
-autoplay
// Reset wait
$delay 600
// All three exit the scene
[alice, bob, charlie] @exit ,

```