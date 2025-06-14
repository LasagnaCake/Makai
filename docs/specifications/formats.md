# Notes
* Undefined optional values are set to their specified defaults.
* Any 4D vector representing color can also be represented by a hex string.
* * **Example:** `[1.0, 1.0, 1.0, 1.0]` becomes `"#FFFFFFFF"` or `"0xFFFFFFFF"`.
* Any vector whose components are *equivalent* can be represented by a single number value.
* * **Example:** `[2.0, 2.0, 2.0]` becomes `2.0`, `[4.0, 4.0]` becomes `4.0`, and so forth.

## Valid Hex Strings
Each string *must* be prepended with either `#` or `0x`.

| String | Component Size | Components | Examples |
|:---:|:---:|:---:|:---:|
|`"RGB"`| 4-bit | `R`ed, `G`reen and `B`lue | `"#F00"` or `"0xF00"` |
|`"RGBA"`| 4-bit | `R`ed, `G`reen, `B`lue and `A`lpha | `"#0F0F"` or `"0x0F0F"` |
|`"RRGGBB"`| 8-bit | `R`ed, `G`reen and `B`lue | `"#0FF000"` or `"0x0FF000"` |
|`"RRGGBBAA"`| 8-bit | `R`ed, `G`reen, `B`lue and `A`lpha | `"#F0FF00F0"` or `"0xF0FF00F0"` |

# Formats

## Components

### Transform2D

#### Layout

```javascript
"trans": {
	// Required
	"position": [0.0, 0.0], // 2D vector representing position
	"rotation": 0.0,
	"scale": [1.0, 1.0], // 2D vector representing scale
}
```

### Transform3D

#### Layout

```javascript
"trans": {
	// Required
	"position": [0.0, 0.0, 0.0], // 3D vector representing position
	"rotation": [0.0, 0.0, 0.0], // 3D vector representing rotation
	"scale": [1.0, 1.0, 1.0], // 3D vector representing scale
}
```

---

### ImageData2D

#### Layout
If image is to be embedded in the file:

```javascript
"image": {
	// Required
	"data": "", // Image file data, as encoded string
	"encoding": "" // Image file data's encoding
	// Optional
	"minFilter": 0, // See Makai::Texture2D::FilterMode
	"magFilter": 0  // See Makai::Texture2D::FilterMode
}
```

If image is to be saved in a separate file:

```javascript
"image": {
	// Required
	"data": {
		"path": "" // Path to image file, relative to definition file
	},
	// Optional
	"minFilter": 0, // See Makai::Texture2D::FilterMode
	"magFilter": 0  // See Makai::Texture2D::FilterMode
}
```

### Camera3D

#### Layout
TODO: Add `"GIMBAL"` type
```javascript
"camera": {
	// Required
	"type": "DEFAULT"
	"eye": [0.0, 0.0, 0.0],
	"at": [0.0, 0.0, -1.0],
	"up": [0.0, 1.0, 0.0],
	"aspect": [4.0, 3.0],
	"fov": 2.0,
	"zNear": 0.01,
	"zFar": 100.0,
	// Optional
	"ortho": {
		// Required
		"origin": [0, 0],
		"size": [1, 1],
		"strength": 0.0
	},
	"relativeToEye": false
}
```

## ObjectMaterial

### Layout

```javascript
"material": {
	// Optional
	"color": [1.0, 1.0, 1.0, 1.0], // 4D vector representing the object's color, or a hex string representing the color value
	"shaded": false,
	"illuminated": false,
	"hue": 0.0,
	"saturation": 1.0, // Ranges from -1.0 to 1.0
	"luminosity": 1.0, // Ranges from -1.0 to 1.0
	"texture": {
		// Required
		"enabled": true,
		"image": {/* See Components:ImageData2D:Layout */},
		"alphaClip": 0.1, // Texture alpha clipping
	},
	"normalMap": {
		// Required
		"enabled": true,
		"image": {/* See Components:ImageData2D:Layout */},
		// Optional
		"strength": 1.0, // Normal map strength
	},
	"emission": {
		// Required
		"enabled": true,
		"image": {/* See Components:ImageData2D:Layout */},
		// Optional
		"strength": 1.0, // Emission strength
	},
	"warp": {
		// Required
		"enabled": true,
		"image": {/* See Components:ImageData2D:Layout */},
		"trans": {/* See Components:Transform2D:Layout */},
		// Optional
		"channelX": 0, // Index of the pixel's channel to be used for horizontal transformation
		"channelY": 1, // Index of the pixel's channel to be used for vertical transformation
	},
	"negative": {
		// Required
		"enabled": true,
		"strength": 1.0,
	},
	"gradient": {
		// Required
		"enabled": true,
		"channel": 0, // Index of the pixel's channel to be used for gradient transformation. If -1, an average of the RGB is used
		"invert": false,
		"begin": [0.0, 0.0, 0.0, 1.0], // 4D vector representing the start color, or a hex string representing the color value
		"end": [1.0, 1.0, 1.0, 1.0], // 4D vector representing the end color, or a hex string representing the color value
	},
	"instances": [
		// Required
		[0.0, 0.0, 0.0], // 3D vector representing the instance's position
	], // A list of instances of the object (including itself)
	"culling": 0, // Culling mode (0: none, 1: front, 2: back)
	"fill": 0, // Fill mode (0: fill, 1: line, 2: point)
}
```

## WorldMaterial

### FogEffect

#### Layout
	
```javascript
"fog": {
	// Required
	"enabled": true,
	"start": 0.0, // The fog's start position (distance from camera)
	"stop": 10.0, // The fog's end position (distance from camera)
	"color": [0.0, 0.0, 0.0, 0.0], // 4D vector representing the fog's color, or a hex string representing the color value
	"strength": 0.0 // The fog's strength
}
```

### Layout

```javascript
"material": {
	// Optional
	"nearFog": {/* See WorldMaterial:FogEffect:Layout */},
	"farFog": {/* See WorldMaterial:FogEffect:Layout */},
	"ambient": {
		// Required
		"color": [1.0, 1.0, 1.0], // 3D vector representing the ambient light's color
		"strength": 1.0 // How bright the ambient light is
	}
}
```

## Renderable Object

### Armature

#### Layout

```javascript
"armature": {
	// Required
	"bones": [/* See Transform3D:Layout */], // Array containing all bone transforms (NOT its offset matrix) (MAX. 64 bones)
	// Optional
	"relations": {"" : []}, // List map containing relations between bones, mapping: [Parent bone index (as string)] → [Children bone indices...]
	"names": {} //  Map containing bone name associations, mapping [Bone name]: → [Bone index] 
}
```

### Layout

This can be saved as a `JSON` file of extension `.mrod` (Makai Renderable Object Definition).

If data is to be integrated in the file:

```javascript
"object": {
	// Required
	"mesh": {
		"components": "x,y,z,u,v,r,g,b,a,nx,ny,nz", // Vector components in order they appear, separated by commas
		"encoding": "base64", // Vertex data's encoding
		"data": "", // Vertex data as encoded string
	},
	// Optional
	"version": 0 // The renderable definition version
	"trans": {/* See Components:Transform3D:Layout */},
	"material": {/* See ObjectMaterial:Layout */},
	// See gl_renderable(.hpp) for further details
	"blend": {
		// function - Opton 1
		"function": {
			// src - Option 1
			"srcColor": 0,
			"srcAlpha": 0,
			// src - Option 2
			"src": 0,
			// dst - Option 1
			"dstColor": 0,
			"dstAlpha": 0,
			// dst - Option 2
			"dst": 0
		},
		// function - Option 2
		"function": 0,
		// equation - Option 1
		"equation": {
			"color": 0,
			"alpha": 0
		},
		// equation - Option 2
		"equation": 0
	},
	"armature": {/* See Renderable:Armature:Layout */}
	"active": true,
}
```

If data gets saved in a different file:

```javascript
"object": {
	// Required
	"mesh": {
		"components": "x,y,z,u,v,r,g,b,a,nx,ny,nz", // Vector components in order they appear, separated by commas
		"data": {
			"path": "" // Path to ".mesh" file containing binary data, relative to object definition file
		},
	},
	// Optional
	"version": 0 // The renderable definition version
	"trans": {/* See Components:Transform3D:Layout */},
	"material": {/* See ObjectMaterial:Layout */},
	"blend": {
		// function - Opton 1
		"function": {
			// src - Option 1
			"srcColor": 0,
			"srcAlpha": 0,
			// src - Option 2
			"src": 0,
			// dst - Option 1
			"dstColor": 0,
			"dstAlpha": 0,
			// dst - Option 2
			"dst": 0
		},
		// function - Option 2
		"function": 0,
		// equation - Option 1
		"equation": {
			"color": 0,
			"alpha": 0
		},
		// equation - Option 2
		"equation": 0
	},
	"armature": {/* See Renderable:Armature:Layout */},
	"active": true,
}
```

## 3D Scene

### Layout

This can be saved as a `JSON` file of extension `.msd` (Makai Scene Definition).

If data gets saved in a separate file:

```javascript
"scene": {
	// Required
	"camera": {/* See Components:Camera3D:Layout */}, // Scene's camera
	"path": [{
		"source": "", // Object file source relative to file
		"type": "MROD" // Object file type
	}] // Paths to objects
	// Optional
	"space": {/* See Components:Transform3D:Layout */}, // Scene's 3D transform
	"world": {/* See WorldMaterial:Layout */}, // Scene's WorldMaterial
	"version": 0 // The scene definition version
}
```
In the previous case, the object's name gets deduced from the filename, minus the extension.

If data is to be integrated in the file:

```javascript
"scene": {
	// Required
	"camera": {/*See Components:Camera3D:Layout */}, // Scene's camera
	// Option 1:
	"data": [{/* See [Renderable Object]:Layout */}...] // List of object definitions
	// Option 2:
	"data": {"name": {/* [See Renderable Object]:Layout */}...} // Named objects
	// Optional
	"space": {/* See Components:Transform3D:Layout */}, // Scene's 3D transform
	"world": {/* See WorldMaterial:Layout */}, // Scene's WorldMaterial
	"version": 0 // The scene definition version
}
```
In the previous case, if using a list of definitions, the names default to `unnamed` + the object's index.

For a name to be valid, it must **not** contain spaces, newlines, tabs, and/or any of the following characters: \\, /, ?, \*, \<, \>, :, " and/or |.

## Dialogue

### ActorData

#### Layout

```javascript
"actor": {
	// Required
	"name": "",
	"frame": [0.0, 0.0],
	// Optional
	"tint": [1.0, 1.0, 1.0, 1.0], // Or a hex string representing the color
	"leaving": false
}
```

### Layout

This can be saved as a `JSON` file of extension `.mdd` (Makai Dialogue Definition). This does not include actor sprite data, purely only message text data.

```javascript
"dialogue": {
	// Required
	"transitionTime": 100,
	"messages": [
		{
			// Required
			"title": "",
			// Required if title is not an action tag, else optional
			"text": "",
			// Optional
			"titleColor": [1.0, 1.0, 1.0, 1.0], // Or a hex string representing the color
			"textColor": [1.0, 1.0, 1.0, 1.0], // Or a hex string representing the color
			"actors": [{/* See Dialogue:ActorData:Layout */}],
			"easing": "out.cubic",
			"duration": 600,
			"autoplay": false
		}
	]
	// Optional, will be used if corresponding message parameter is not set
	"easing": "out.cubic",
	"duration": 600,
	"autoplay": false
}
``````

## Font Faces

### Layout

This can be saved as a `JSON` file of extension `.mffd` (Makai Font Face Definition).

```javascript
"face": {
	// Required
	"image": {/* See Components:ImageData2D:Layout */},
	// Optional
	"size": [16.0, 16.0] // the caracter row & column count.
	"spacing": [1.0, 1.0] // The X & Y spacing between characters.
}
```
