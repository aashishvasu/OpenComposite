#include "stdafx.h"
#include "SudoFontMeta.h"

#include "Misc/lodepng.h"

#include <assert.h>

using namespace std;

vector<char> readBytes(vector<char> &data, size_t len) {
	assert(len <= data.size());
	vector<char> result(data.begin(), data.begin() + len);
	data.erase(data.begin(), data.begin() + len);
	return result;
}

template<typename T>
T readData(vector<char> &data) {
	T val;
	vector<char> sVal = readBytes(data, sizeof(val));
	memcpy(&val, sVal.data(), sizeof(val));
	return val;
}

enum SectionID_t : uint16_t {
	ID_FONT_INFO = 0,
	ID_CHARACTERS,
	ID_KERNING,
	ID_CONFIG,

	ID_END = 999,
};

SudoFontMeta::SudoFontMeta(vector<char> data, vector<char> image) {
	string headerReq = "\x0bSudoFont1.1";
	vector<char> headerVec = readBytes(data, headerReq.length());
	string header(headerVec.data(), headerVec.size());
	assert(header == headerReq);

	unsigned int origTexW, origTexH;

	while (true) {
		auto sectionID = readData<SectionID_t>(data);

		if (sectionID == ID_END)
			break;

		auto sectionSize = readData<uint32_t>(data);

		vector<char> sec = readBytes(data, sectionSize);

		if (sectionID == ID_FONT_INFO) {
			lineHeight = readData<uint16_t>(sec);
			origTexW = readData<uint16_t>(sec);
			origTexH = readData<uint16_t>(sec);
		} else if (sectionID == ID_CHARACTERS) {
			auto count = readData<uint16_t>(sec);

			for (size_t i = 0; i < count; i++) {
				CharInfo info = { 0 };

				info.CharacterCode = readData<uint16_t>(sec);

				info.PackedX = readData<uint16_t>(sec);
				info.PackedY = readData<uint16_t>(sec);
				info.PackedWidth = readData<uint16_t>(sec);
				info.PackedHeight = readData<uint16_t>(sec);

				info.XOffset = readData<uint16_t>(sec);
				info.YOffset = readData<uint16_t>(sec);

				info.XAdvance = readData<uint16_t>(sec);

				assert(chars.count(info.CharacterCode) == 0);
				chars[info.CharacterCode] = info;
			}
		}
	}

	// Load the image itself
	lodepng::decode(
		pixel_data,
		imgWidth, imgHeight,
		(const uint8_t*)image.data(), image.size(),
		LCT_RGBA, 8
	);

	assert(origTexW == imgWidth);
	assert(origTexH == imgHeight);
}

SudoFontMeta::~SudoFontMeta() {
}

void SudoFontMeta::Blit(wchar_t ch, int x, int y, int img_width, pix_t targetColour, pix_t *rawPixels, bool hpad) {
	pix_t *pixels = (pix_t*)rawPixels;
	const pix_t *font = (const pix_t*)pixel_data.data();

	const CharInfo &info = chars.at(ch);

	for (int ix = 0; ix < info.PackedWidth; ix++) {
		for (int iy = 0; iy < info.PackedHeight; iy++) {
			int px = ix + info.PackedX;
			int py = iy + info.PackedY;
			pix_t p = font[px + py * imgHeight];

			if (p.a == 0)
				continue;

			int tx = ix + x + (hpad ? info.XOffset : 0);
			int ty = iy + y + info.YOffset;

			assert(tx > 0);
			assert(ty > 0);

			size_t idx = tx + ty * img_width;
			assert(idx >= 0);
			pix_t &out = pixels[idx];
			out = targetColour;
		}
	}
}

int SudoFontMeta::Width(wchar_t ch) {
	const CharInfo &info = chars.at(ch);
	return info.XAdvance;
}

int SudoFontMeta::Width(wstring str) {
	int width = 0;
	for (wchar_t ch : str)
		width += Width(ch);
	return width;
}
