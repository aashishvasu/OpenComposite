#pragma once
#include <string>
#include <map>
#include <vector>

class SudoFontMeta {
public:
	SudoFontMeta(std::vector<char> data, std::vector<char> image);
	~SudoFontMeta();

	struct CharInfo {
		// Which character this Character references.
		wchar_t CharacterCode;

		// Location of this character in the packed image.
		short PackedX;
		short PackedY;
		short PackedWidth;
		short PackedHeight;

		// Where to draw this character on the target.
		short XOffset;
		short YOffset;

		// How much to advance your X position after drawing this character.
		// The total amount to advance for each char is ( XAdvance + GetKerning( nextChar ) ).
		short XAdvance;
	};

	struct pix_t {
		uint8_t r, g, b, a;
	};

	/**
	 * Copy a character onto pixel data.
	 *
	 * Does not copy transparent regions, so this respects the background.
	 */
	void Blit(wchar_t ch, int x, int y, int img_width, pix_t targetColour, pix_t *rawPixels, bool hpad = true);

	int Width(wchar_t ch);
	int Width(std::wstring str);

	unsigned int GetLineHeight() { return lineHeight; }

private:
	std::map<wchar_t, CharInfo> chars;
	std::vector<uint8_t> pixel_data;

	unsigned int imgWidth;
	unsigned int imgHeight;

	unsigned int lineHeight;
};

