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

		/// @brief Comparison operator (defaulted).
		constexpr bool operator==(TextRect const& other) const			= default;
		/// @brief Threeway comparison operator (defaulted).
		constexpr ValueOrder operator<=>(TextRect const& other) const	= default;
	};

	/// @brief Font face data.
	struct FontData {
		struct Faces {
			/// @brief Normal font character sheet.
			Texture2D	normal;
			/// @brief Emphasis font character sheet.
			Texture2D	emphasis;
		} faces;
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
		/// @brief Default constructor.
		FontFace();

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

	namespace Base {
		/// @brief Base text display data.
		/// @tparam TString String type.
		template<class TString>
		struct TextData {
			/// @brief Text to display.
			TString		content		= "Hello\nWorld!";
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

			/// @brief Converts the text data to different encoding.
			template <class T>
			constexpr operator TextData<T>() const 
			requires (Type::Different<T, TString>) {
				return {toString(content), *this};
			}

			/// @brief Comparison operator (defaulted).
			constexpr bool operator==(TextData const& other) const			= default;
			/// @brief Threeway comparison operator (defaulted).
			constexpr ValueOrder operator<=>(TextData const& other) const	= default;
		};

		/// @brief Base text display abstract class.
		/// @tparam TString String type.
		template<class TString>
		class ALabel: public AGraphic {
		public:
			using ContentType	= TextData<TString>;
			using VertexList	= List<Vertex>;

			/// @brief Constructs the label.
			/// @param layer Layer to register the object to. By default, it is layer zero.
			/// @param manual Whether the object is manually rendered. By default, it is `false`.
			ALabel(usize const& layer = 0, bool const manual = false): AGraphic(layer, manual) {}

			/// @brief Destructor.
			virtual ~ALabel() {}

			/// @brief Font face to use.
			FontFace					font;
			/// @brief Text to display.
			Instance<ContentType>		text		= new ContentType();
			/// @brief Material to use. Texture effect gets ignored.
			Material::ObjectMaterial	material;

		protected:
			enum class TextType {
				TT_NORMAL,
				TT_EMPHASIS,
				TT_END
			};

			virtual void generate() = 0;

			void setVertices(VertexList const& verts, TextType const type) {
				switch (type) {
					case TextType::TT_NORMAL:	normalText = verts;		break;
					case TextType::TT_EMPHASIS:	emphasisText = verts;	break;
				}
			}

			void clearVertices(TextType const type) {
				switch (type) {
					case TextType::TT_NORMAL:	normalText = {};	break;
					case TextType::TT_EMPHASIS:	emphasisText = {};	break;
				}
			}

			void clearAllVertices() {
				for (TextType type = TextType{0}; type < TextType::TT_MAX_TYPES; ++type)
					clearVertices(type);
			}

		private:
			/// @brief Underlying normal text to render.
			VertexList normalText;
			/// @brief Underlying emphasis text to render.
			VertexList emphasisText;

			/// @brief Last text displayed.
			Instance<ContentType> last = new ContentType{"",{0,0}};

			void draw() override {
				if (!font) return;
				// If text changed, update label
				if (text && (*text != *last)) {
					*last = *text;
					generate();
				}
				prepare();
				for (TextType type = TextType{0}; type < TextType::TT_MAX_TYPES; ++type)
					showText(type);
			}

			void setFont(TextType const type) {
				// Set font
				material.texture.enabled = true;
				switch (type) {
					case TextType::TT_NORMAL:	material.texture.image = font->faces.normal;	break;
					case TextType::TT_EMPHASIS:	material.texture.image = font->faces.emphasis;	break;
				}
			}

			VertexList& getVertices(TextType const type) {
				switch (type) {
					return	normalText;		break;
					return	emphasisText;	break;
				}
			}

			void showText(TextType const type) {
				// If no normal text, return
				if (!normalText.size()) return;
				// Set shader data
				setFont(type);
				material.use(shader);
				auto& verts = getVertices(type);
				// Display to screen
				display(
					verts.data(), verts.size(),
					material.culling,
					material.fill,
					DisplayMode::ODM_TRIS,
					material.instances.size()
				);
			}
		};
	}

	/// @brief `char` string text display.
	class CharLabel: public Base::ALabel<String> {void generate() override;};
	/// @brief `char` string text display data.
	using CharTextData = typename Base::ALabel<String>::ContentType;

	/// @brief UTF-8 string text display.
	class UTF8Label: public Base::ALabel<UTF8String> {void generate() override;};
	/// @brief UTF-8 string Text display data.
	using UTF8TextData = typename Base::ALabel<UTF8String>::ContentType;

	/// @brief Implementation details.
	namespace Impl {
		/// @brief Text display type wrapper.
		/// @tparam T Text string type.
		template<class T>
		struct LabelType;

		template<> struct LabelType<String>:		TypeContainer<CharLabel> {};
		template<> struct LabelType<UTF8String>:	TypeContainer<UTF8Label> {};
	}

	/// @brief Text display.
	/// @tparam T Text string type.
	template<class T> using Label		= typename Impl::LabelType<T>::Type;
	/// @brief Text display data.
	/// @tparam T Text string type.
	template<class T> using TextData	= typename Impl::LabelType<T>::Type::ContentType;
}

#endif // MAKAILIB_GRAPH_RENDERER_TEXT_H
