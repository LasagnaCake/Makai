#  `@type`

```
@type <name> [<attributes> ...]
```

Specifies an ART type with the given series of attributes.

> [!note]
> `array` and `derived` cannot be specified in the same type.

> [!note]
> If an attribute requires a specific type, that type must be previously-declared.

## Attributes

### `empty`

Specifies that the type is considered an empty type.

### `nil`

Specifies that the type can be a null value.

### `basic< <basic-type-name> >`

Specifies that the type is an analog for a ART basic type.

> [!note]
> Basic types also imply:
> - `value`
> - `copy`

### `derived< <type-name> >`

Specifies that the type inherits attributes from another type.

### `array< <type-name> >`

Specifies that the type is an array of a given type.

### `dyn`

Specifies that the type's size and contents aren't known at creation time.

### `struct`

Specifies that the type contains sub-fields.

### `value`

Specifies that the type's contents are stored sequentially in-memory, and not as an array of references.

If specified, any field/index access will always result in a copy of the value in that location. Thusly, `value array`s, while being more space-efficient, are also slower.

### `copy`

Specifies that the type's contents can be trivially-cloned.

### `align( <size> )`

Specifies that the type's bite size must align to a multiple of integer `<size>`.

### `fields[ <types> ... ]`

Specifies that the type contains fields of the given types, in order of declaration.

### `operators [ <op-name> : <fn> ... ]`

Specifies that the type contains a series of operators.

### `casts [ <type-name> : <fn> ... ]`

Specifies that the type contains a series of casts.

### `bound`

Specifies that the type is equivalent to an runtime-specified type.

### `final`

Specifies that the type cannot be derived from.

### `discard`

Specifies that the result of methods that return the given type can be ignored.
