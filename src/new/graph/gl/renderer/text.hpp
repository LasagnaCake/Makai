#ifndef MAKAILIB_GRAPH_RENDERER_TEXT_H
#define MAKAILIB_GRAPH_RENDERER_TEXT_H

#include "../../../compat/ctl.hpp"
#include "../texture.hpp"
#include "drawable.hpp"

/// @brief Graphical facilities.
namespace Makai::Graph {
	using Makai::Graph::Texture2D;

	/// @brief Text rectangle.
	struct TextRect {
		/// @brief Character count per line.
		usize h = 0;
		/// @brief Line count.
		usize v = 0;
	};

	/// @brief Font face data.
	struct FontData {
		/// @brief Font character sheet.
		Texture2D	image;
		/// @brief Font sheet character count.
		Vector2		size	= Vector2(16);
		/// @brief Font spacing.
		Vector2		spacing	= Vector2(1);
	};

	/// @brief Font face.
	struct FontFace {
	public:
		using FontInstance = Instance<FontData>;
		
		// This needs to be initialized this specific way, else it breaks somehow???
		/// @brief Empty constructor.
		FontFace(): instance(new FontData()) {}

		/// @brief Constructs the font face from font data.
		/// @param font Font to use.
		FontFace(FontData const& font);

		/// @brief Constructs the font face from a font file.
		/// @param path Path to font file.
		FontFace(String const& path);

		/// @brief Assignment operator (`FontFace`).
		/// @param other `FontFace` to bind.
		/// @return Reference to self.
		FontFace& operator=(FontFace const& other);

		/// @brief Assignment operator (`FontData`).
		/// @param other `FontData` to form a font from.
		/// @return Reference to self.
		FontFace& operator=(FontData const& font);

		/// @brief Dereference operator.
		/// @return Reference to underlying font data.
		FontData& operator*() const;
		/// @brief Pointer access operator.
		/// @return Pointer to underlying font data.
		FontData* operator->() const;

		/// @brief Returns a reference to the underlying font data.
		/// @return Reference to underlying font data.
		FontData& data() const;

		/// @brief Returns whether the font face has a font.
		/// @return Whether the font exists.
		bool exists() const;
		/// @brief Returns whether the font face has a font.
		/// @return Whether the font exists.
		operator bool() const;

	private:
		/// @brief Instance of underlying font data.
		FontInstance instance;
	};

	/// @brief Text line wrap mode.
	enum class LineWrap {
		LW_CHARACTER,
		LW_FULL_WORD,
		LW_HYPHEN_WORD
	};

	/// @brief Label display data.
	struct TextData {
		/// @brief Text to display.
		String		content		= "Hello\nWorld!";
		/// @brief Text display rectangle.
		TextRect	rect		= {40, 100};
		/// @brief Text alignment (justification).
		Vector2		textAlign	= 0;
		/// @brief Text rectangle alignment.
		Vector2		rectAlign	= 0;
		/// @brief Character spacing modifier.
		Vector2		spacing		= 0;
		/// @brief Maximum displayed characters.
		long		maxChars	= -1;
		/// @brief Line wrapping mode.
		LineWrap	lineWrap	= LineWrap::LW_CHARACTER;
	};
	
	class Label: public IGLDrawable {
	public:
		Label(usize const& layer = 0, bool const manual = false): IGLDrawable(layer, manual) {}

		virtual ~Label() {}

		FontFace					font;
		TextData					text;
		Material::ObjectMaterial	material;

	private:
		List<Vertex> vertices;

		TextData last = {"",{0,0}};

		void draw() override;

		void update();
	};
}

#endif // MAKAILIB_GRAPH_RENDERER_TEXT_H
