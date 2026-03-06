#  `@type`

```
@type <name> [<attributes>...]
```

Specifies an ART type with the given series of attributes.

> [!note]
> `array` and `derived` cannot be specified in the same type.

## Attributes

### `empty`

Specifies that the type is considered an empty type.

### `nil`

Specifies that the type can be a null value.

### `basic< <basic-type-name> >`

Specifies that the type is an analog for a ART basic type.

### `derived< <type-name> >`

Specifies that the type inherits attributes from another type.

### `array< <type-name> >`

Specifies that the type is an array of a given type.

### `dyn`

Specifies that the type's size is not known at creation time.

### `struct`

Specifies that the type contains sub-fields.

### `value`

Specifies that the type's contents should be stored sequentially in-memory, and not as an array of references.

If specified, any field/index access will always result in a copy of the value.

### `copy`

Specifies that the type's contents can be trivially-copied.

### `align( <size> )`

Specifies that the type's bite size must align to a multiple of integer `<size>`.

### `fields[ <types> ... ]`

Specifies that the type contains fields of the given types, in order of declaration.
