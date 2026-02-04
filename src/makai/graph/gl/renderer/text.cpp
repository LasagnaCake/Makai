#include "../../../file/json.hpp"

#include "text.hpp"

using namespace Makai; using namespace Makai::Graph;

namespace JSON = Makai::JSON;

inline Vector2 fromJSONArrayV2(JSON::Value const& json, Vector2 const& defaultValue = 0) {
	try {
		if (json.isArray())
			return Vector2(
				json[0].get<float>(),
				json[1].get<float>()
			);
		else if (json.isNumber())
			return json.get<float>();
		else return defaultValue;
	} catch (std::exception const& e) {
		return defaultValue;
	}
}

inline Vector3 fromJSONArrayV3(JSON::Value const& json, Vector3 const& defaultValue = 0) {
	try {
		if (json.isArray())
			return Vector3(
				json[0].get<float>(),
				json[1].get<float>(),
				json[2].get<float>()
			);
		else if (json.isNumber())
			return json.get<float>();
		else return defaultValue;
	} catch (std::exception const& e) {
		return defaultValue;
	}
}

inline Vector4 fromJSONArrayV4(JSON::Value const& json, Vector4 const& defaultValue = 0) {
	try {
		if (json.isArray())
			return Vector4(
				json[0].get<float>(),
				json[1].get<float>(),
				json[2].get<float>(),
				json[3].get<float>()
			);
		else if (json.isNumber())
			return json.get<float>();
		else return defaultValue;
	} catch (std::exception const& e) {
		return defaultValue;
	}
}

FontFace::FontFace(): instance(new FontData()) {}

FontFace::FontFace(FontData const& font): instance(new FontData{font}) {}

FontFace::FontFace(String const& path): FontFace() {
	using namespace Makai::Literals::Text;
	JSON::Value tx				= File::getJSON(path);
	if (tx.contains("normal"s))
		instance->faces.normal		= Texture2D::fromJSON(tx["normal"s], OS::FS::directoryFromPath(path));
	else instance->faces.normal		= Texture2D::fromJSON(tx["image"s], OS::FS::directoryFromPath(path));
	if (tx.contains("emphasis"s))
		instance->faces.emphasis	= Texture2D::fromJSON(tx["emphasis"s], OS::FS::directoryFromPath(path));
	instance->size					= fromJSONArrayV2(tx["size"s], 16);
	instance->spacing				= fromJSONArrayV2(tx["spacing"s], 1);
	instance->start					= tx["start"s].template get<usize>(0x20ull);
}

FontFace& FontFace::operator=(FontFace const& other) {
	instance = other.instance;
	return *this;
}

FontFace& FontFace::operator=(FontData const& font) {
	instance = new FontData{font};
	return *this;
}

FontData& FontFace::operator*() const	{return data();		}
FontData* FontFace::operator->() const	{return &data();	}

FontData& FontFace::data() const {return *instance; }

bool FontFace::exists() const	{return instance.exists() && (instance->faces.normal.exists() || instance->faces.emphasis.exists());	}
FontFace::operator bool() const	{return exists();																						}

template<Makai::Type::OneOf<CharTextData, UTF8TextData> T>
List<float> getTextLineStarts(T& text, FontData const& font, List<usize> const& breaks) {
	using Char = typename decltype(text.content)::DataType;
	List<float> result;
	switch (text.lineWrap) {
	case LineWrap::LW_CHARACTER: {
			// Separate text by newline characters
			auto const lines = text.content.split({Char{'\n'}});
			// Calculate starting points
			for (auto& l : lines) {
				usize lineSize		= l.size();
				usize lastLineSize	= lineSize % (text.rect.h+1);
				if (lineSize > text.rect.h) {
					for (usize i = 0; i < (lineSize - lastLineSize) / text.rect.h; i++)
						result.pushBack(0);
				}
				result.pushBack(
					((float)text.rect.h - (float)lastLineSize)
				*	(text.spacing.x + font.spacing.x)
				*	text.textAlign.x
				+	(
						(text.spacing.x + font.spacing.x)
					*	text.textAlign.x
					*	text.textAlign.x
					*	text.textAlign.x
					)
				);
			}
		}
	case LineWrap::LW_FULL_WORD:
	case LineWrap::LW_HYPHEN_WORD: {
			for (usize const& lb: breaks) {
				result.pushBack(
					((float)text.rect.h - (float)lb)
				*	(text.spacing.x + font.spacing.x)
				*	text.textAlign.x
				+	(
						(text.spacing.x + font.spacing.x)
					*	text.textAlign.x
					*	text.textAlign.x
					*	text.textAlign.x
					)
				);
			}
		}
	}
	// Return result
	return result;
}

template<Makai::Type::OneOf<CharTextData, UTF8TextData> T>
Vector2 getTextRectStart(T const& text, FontData const& font) {
	Vector2 rectPos = Vector2(text.rect.h, text.rect.v) * text.rectAlign;
	rectPos += (text.spacing + font.spacing) * text.rectAlign * Vector2(1, -1);
	rectPos *= (text.spacing + font.spacing);
	return rectPos;
}

template<class T>
constexpr List<usize> calculateIndices(T const& words, TextRect const& rect) {
	List<usize> indices;
	/*usize
		ls = 0,
		sc = 0
	;
	for (String const& w: words) {
		if ((ls + sc + w.size()) > (rect.h-1)) {
			indices.pushBack(ls + sc - 1);
			ls = w.size();
			sc = 1;
		} else {
			ls += w.size();
			sc++;
		}
	}
	indices.pushBack(ls + sc - 1);*/
	usize
		lastBreak	= 0,
		curWord		= 0
	;
	for (auto const& word: words) {
		for (auto const& c: word) {
			if (c == decltype(c){'\n'}) {
				indices.pushBack(curWord+lastBreak-1);
				lastBreak = 0;
			} else if ((lastBreak + (++curWord)) > (rect.h-1)) {
				indices.pushBack(lastBreak-1);
				lastBreak = 0;
			}
		}
		lastBreak += ++curWord;
		curWord = 0;
	}
	indices.pushBack(lastBreak-1);
	return indices;
}

template<class T>
List<usize> getTextLineWrapIndices(T& text) {
	List<usize>	indices;
	switch (text.lineWrap) {
	case LineWrap::LW_CHARACTER:
		break;
	case LineWrap::LW_HYPHEN_WORD: {
			indices = calculateIndices(
				text.content.split({' ', '~', '\t', '-'}),
				text.rect
			);
		}
		break;
	case LineWrap::LW_FULL_WORD: {
			indices = calculateIndices(
				text.content.split({' ', '~', '\t'}),
				text.rect
			);
		}
		break;
	}
	return indices;
}

void CharLabel::generate() {
	// Clear previous characters
	clearAllVertices();
	CharLabel::VertexList vertices;
	// If no text is present, return
	if (!text)					return;
	if (text->content.empty())	return;
	DEBUGLN("Generating text for '", text->content, "'...");
	// The current character's position
	Vector2		cursor;
	TextRect	chrRect = {0,0};
	Vector2		rectStart = getTextRectStart(*text, *font);
	// The current character's top left UV index
	Vector2 uv;
	int64 index;
	// The lines' starting positions (if applicable)
	List<usize>	lineEnd = getTextLineWrapIndices(*text);
	List<float>	lineStart = getTextLineStarts(*text, *font, lineEnd);
	cursor.x	= lineStart[0];
	cursor.y 	= text->rect.v - Math::min<usize>(lineEnd.size(), text->rect.v);
	cursor.y	*= (text->spacing.y + font->spacing.y) * -text->textAlign.y;
	cursor -= rectStart * Vector2(1,-1);
	// The current line and current character
	usize curLine = 0;
	usize curChar = 0;
	// Start and end of character set
	auto const charStart	= static_cast<int64>(font->start);
	auto const charEnd		= static_cast<int64>(font->size.x * font->size.y);
	// Loop through each character and...
	for (char pc: text->content) {
		usize c = CTL::bitcast<uint8>(pc);
		// Check if max characters hasn't been reached
		if (text->maxChars == 0 || ((ssize(curChar) > ssize(text->maxChars-1)) && (text->maxChars > -1))) break;
		else curChar++;
		// Check if character is newline
		bool
			newline = (c == '\n'),
			endOfWordLine = false
		;
		// Check if should break line
		if (text->lineWrap != LineWrap::LW_CHARACTER && text->content.size() >= text->rect.h)
			endOfWordLine = curLine < lineEnd.size() ? (chrRect.h > lineEnd[curLine]) : false;
		// If cursor has reached the rect's horizontal limit or newline, move to new line
		if((chrRect.h >= text->rect.h) || newline || endOfWordLine) {
			// If cursor has reach the rect's vertical limit, break
			if(chrRect.v >= text->rect.v) break;
			if (++curLine < lineStart.size()) cursor.x = (lineStart[curLine]) - rectStart.x;
			else cursor.x = -rectStart.x;
			cursor.y -= (text->spacing.y + font->spacing.y) * 1.0;
			chrRect.h = 0;
			chrRect.v++;
			// If newline, move on to next character
			if (newline) {
				if (endOfWordLine) chrRect.v--;
				continue;
			}
		}
		// If cursor has reach the rect's vertical limit, break
		if(chrRect.v >= text->rect.v) break;
		// If character below min range (or a control character), skip
		if (c < 0x20) continue;
		if (c < font->start) continue;
		// Get character index
		index = Math::max<int64>(c - charStart, 0);
		// Get character's top left UV index in the font texture
		bool const inFontRange = index < charEnd;
		uv = inFontRange
		?	Vector2(
			static_cast<int64>(index % static_cast<int64>(font->size.x)),
			static_cast<int64>(index / font->size.x)
		):	Vector2(
			static_cast<int64>((bitcast<uint8>('?') - charStart) % static_cast<int64>(font->size.x)),
			static_cast<int64>((bitcast<uint8>('?') - charStart) / font->size.x)
		);
		// Get vertex positions
		Vector2 pos[4] = {
			cursor,
			cursor + Vector2(1,0),
			cursor + Vector2(0,-1),
			cursor + Vector2(1,-1),
		};
		// Get UV positions
		Vector2 uvs[4] = {
			uv / font->size,
			(uv + Vector2(1,0)) / font->size,
			(uv + Vector2(0,1)) / font->size,
			(uv + Vector2(1,1)) / font->size,
		};
		// Color indicator (for character errors)
		Vector4 const charColor = inFontRange ? Color::WHITE : Color::RED;
		// Nightmare
		vertices.pushBack(Vertex(pos[0], uvs[0], charColor));
		vertices.pushBack(Vertex(pos[1], uvs[1], charColor));
		vertices.pushBack(Vertex(pos[2], uvs[2], charColor));
		vertices.pushBack(Vertex(pos[1], uvs[1], charColor));
		vertices.pushBack(Vertex(pos[2], uvs[2], charColor));
		vertices.pushBack(Vertex(pos[3], uvs[3], charColor));
		// Increment cursor
		cursor.x += text->spacing.x + font->spacing.x;
		chrRect.h++;
	}
	setVertices(vertices, TextType::TT_NORMAL);
}

void UTF8Label::generate() {
	// Clear previous characters
	clearAllVertices();
	CharLabel::VertexList vertices;
	// If no text is present, return
	if (!text)					return;
	if (text->content.empty())	return;
	// The current character's position
	Vector2		cursor;
	TextRect	chrRect = {0,0};
	Vector2		rectStart = getTextRectStart(*text, *font);
	// The current character's top left UV index
	Vector2 uv;
	int64 index;
	// The lines' starting positions (if applicable)
	List<usize>	lineEnd = getTextLineWrapIndices(*text);
	List<float>	lineStart = getTextLineStarts(*text, *font, lineEnd);
	cursor.x	= lineStart[0];
	cursor.y 	= text->rect.v - Math::min<usize>(lineEnd.size(), text->rect.v);
	cursor.y	*= (text->spacing.y + font->spacing.y) * -text->textAlign.y;
	cursor -= rectStart * Vector2(1,-1);
	// The current line and current character
	usize curLine = 0;
	usize curChar = 0;
	// Start and end of character set
	auto const charStart	= static_cast<int64>(font->start);
	auto const charEnd		= static_cast<int64>(font->size.x * font->size.y);
	// Loop through each character and...
	for (UTF::Character<8> const& pc: text->content) {
		usize c = pc.value();
		// Check if max characters hasn't been reached
		if (text->maxChars == 0 || ((ssize(curChar) > ssize(text->maxChars-1)) && (text->maxChars > -1))) break;
		else curChar++;
		// Check if character is newline
		bool
			newline = (c == decltype(pc){'\n'}.value()),
			endOfWordLine = false
		;
		// Check if should break line
		if (text->lineWrap != LineWrap::LW_CHARACTER && text->content.size() >= text->rect.h)
			endOfWordLine = curLine < lineEnd.size() ? (chrRect.h > lineEnd[curLine]) : false;
		// If cursor has reached the rect's horizontal limit or newline, move to new line
		if((chrRect.h >= text->rect.h) || newline || endOfWordLine) {
			// If cursor has reach the rect's vertical limit, break
			if(chrRect.v >= text->rect.v) break;
			if (++curLine < lineStart.size()) cursor.x = (lineStart[curLine]) - rectStart.x;
			else cursor.x = -rectStart.x;
			cursor.y -= (text->spacing.y + font->spacing.y) * 1.0;
			chrRect.h = 0;
			chrRect.v++;
			// If newline, move on to next character
			if (newline) {
				if (endOfWordLine) chrRect.v--;
				continue;
			}
		}
		// If cursor has reach the rect's vertical limit, break
		if(chrRect.v >= text->rect.v) break;
		// If character below min range (or a control character), skip
		if (c < 0x20) continue;
		if (c < font->start) continue;
		// Get character index
		index = Math::max<int64>(c - charStart, 0);
		// Get character's top left UV index in the font texture
		bool const inFontRange = index < charEnd;
		uv = inFontRange
		?	Vector2(
			static_cast<int64>(index % static_cast<int64>(font->size.x)),
			static_cast<int64>(index / font->size.x)
		):	Vector2(
			static_cast<int64>((bitcast<uint8>('?') - charStart) % static_cast<int64>(font->size.x)),
			static_cast<int64>((bitcast<uint8>('?') - charStart) / font->size.x)
		);
		// Get vertex positions
		Vector2 pos[4] = {
			cursor,
			cursor + Vector2(1,0),
			cursor + Vector2(0,-1),
			cursor + Vector2(1,-1),
		};
		// Get UV positions
		Vector2 uvs[4] = {
			uv / font->size,
			(uv + Vector2(1,0)) / font->size,
			(uv + Vector2(0,1)) / font->size,
			(uv + Vector2(1,1)) / font->size,
		};
		// Color indicator (for character errors)
		Vector4 const charColor = inFontRange ? Color::WHITE : Color::RED;
		// Nightmare
		vertices.pushBack(Vertex(pos[0], uvs[0], charColor));
		vertices.pushBack(Vertex(pos[1], uvs[1], charColor));
		vertices.pushBack(Vertex(pos[2], uvs[2], charColor));
		vertices.pushBack(Vertex(pos[1], uvs[1], charColor));
		vertices.pushBack(Vertex(pos[2], uvs[2], charColor));
		vertices.pushBack(Vertex(pos[3], uvs[3], charColor));
		// Increment cursor
		cursor.x += text->spacing.x + font->spacing.x;
		chrRect.h++;
	}
	setVertices(vertices, TextType::TT_NORMAL);
}
