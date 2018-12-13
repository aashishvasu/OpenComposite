#include "stdafx.h"
#include "KeyboardLayout.h"
#include "VRKeyboard.h"

#include <algorithm>
#include <cwctype>

using namespace std;

static void wstrim(wstring &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](wchar_t ch) {
		return !std::iswspace(ch);
	}));

	s.erase(std::find_if(s.rbegin(), s.rend(), [](wchar_t ch) {
		return !std::iswspace(ch);
	}).base(), s.end());
}

static wstring pullstring(wstring &in) {
	size_t wordlen = 0;

	while (wordlen < in.length() && !std::iswspace(in[wordlen])) {
		wordlen++;
	}

	wstring word = in.substr(0, wordlen);

	// Chop the spaces off the remander
	size_t spacelen = wordlen;
	while (spacelen < in.length() && std::iswspace(in[spacelen])) {
		spacelen++;
	}

	in.erase(0, spacelen);

	return word;
}

static wchar_t escapeChar(const wstring &str) {
	if (str[0] != '\\')
		return str[0];

	wchar_t esc = str[1];

	switch (esc) {
	case 't':
		return '\t';
	case '\\':
		return '\\';
	case 'n':
		return '\n';
	case 'b':
		return '\b'; // Backspace
	case 's':
		return ' '; // For whitespace-deliminated stuff
	case 'z':
		return '\x01'; // Shift
	case 'c':
		return '\x02'; // Caps lock
	case 'q':
		return '\x03'; // Done
	}

	string utf = VRKeyboard::CHAR_CONV.to_bytes(str);

	string msg = "Unknown escape sequence '" + utf + "'";
	OOVR_ABORT(msg.c_str());
}

KeyboardLayout::KeyboardLayout(std::vector<char> data) {
	wstring contents = VRKeyboard::CHAR_CONV.from_bytes(string(data.data(), data.size()));

	Key *last = nullptr;

	auto addKey = [&](wchar_t ch, wchar_t shift, float x, float y) -> Key& {
		int id = (int)keys.size();

		keys.push_back(Key());
		Key &key = keys.back();
		last = &key;

		key = { 0 };

		key.id = id;

		key.ch = ch;
		key.shift = shift;

		key.x = x;
		key.y = y;

		key.label = ch;
		key.labelShift = shift;

		key.w = 1;
		key.h = 1;

		return key;
	};

	auto addNextKey = [&](wchar_t ch, wchar_t shift) {
		if (!last)
			OOVR_ABORT("Cannot add bank or unpositioned key as first key!");

		addKey(ch, shift, last->x + last->w, last->y);
	};

	while (!contents.empty()) {
		size_t newlinePos = contents.find_first_of('\n');
		wstring line = contents.substr(0, newlinePos);
		contents.erase(0, newlinePos + 1);

		wstrim(line);
		if (line.empty())
			continue;

		// Comments
		if (line[0] == '#')
			continue;

		wstring op = pullstring(line);

		if (op == L"bank") {
			wstring chars = pullstring(line);
			wstring shift = pullstring(line);

			if (chars.length() != shift.length())
				OOVR_ABORTF("Keyboard layout: bank lower/upper length mismatch: '%ls' vs '%ls'", chars.c_str(), shift.c_str());

			for (int i = 0; i < chars.length(); i++) {
				addNextKey(chars[i], shift[i]);
			}
			continue;
		}

		if (op == L"width") {
			width = std::stoi(pullstring(line));
			continue;
		}

		if (op[0] == '.') {
			wstring prop = op.substr(1);
			string utfProp = VRKeyboard::CHAR_CONV.to_bytes(prop);

			if (!last) {
				string msg = "Cannot set property '" + utfProp + "' without a prior key!";
				OOVR_ABORT(msg.c_str());
			}

			if (prop == L"spans_to_right") {
				last->spansToRight = pullstring(line) != L"false";
				continue;
			} else if (prop == L"label") {
				last->label = pullstring(line);
				last->labelShift = last->label;
				continue;
			}

			string msg = "Unknown property property '" + utfProp + "' for key '" + VRKeyboard::CHAR_CONV.to_bytes(last->label) + "'";
			OOVR_ABORT(msg.c_str());

			continue;
		}

		wchar_t ch = escapeChar(pullstring(line));
		wchar_t shift = escapeChar(pullstring(line));

		wstring wx, wy;

		wx = pullstring(line);
		wy = pullstring(line);

		if (wx.empty() != wy.empty()) {
			string msg = string("x/y empty mismatch for keyboard char '") + ((char)ch) + "'";
			OOVR_ABORT(msg.c_str());
		}

		if (wx.empty()) {
			addNextKey(ch, shift);
		} else {
			Key &key = addKey(ch, shift, std::stof(wx), std::stof(wy));

			wstring ww = pullstring(line);
			wstring wh = pullstring(line);

			if (!ww.empty()) {
				if (wh.empty()) {
					string msg = string("word width/height mismatch for keyboard char '") + ((char)ch) + "'";
					OOVR_ABORT(msg.c_str());
				}

				key.w = std::stof(ww);
				key.h = std::stof(wh);
			}
		}
	}

	if (width == 0)
		OOVR_ABORT("Missing keyboard layout width specifier");

	// Calculate the adjacent keys
	for (Key &key : keys) {
		int ids[4] = { -1, -1, -1, -1 };
		float distances[4] = { 1000, 1000, 1000, 1000 };
		// Find the nearest keys in each direction
		for (Key &to : keys) {
			if (&key == &to)
				continue;

			float dx = to.x - key.x;
			float dy = to.y - key.y;

			float lenSq = dx * dx + dy * dy;

			int angle = (int)(atan2(-dy, dx) * 180 / math_pi);

			// Spin everything back 45deg, so the borders between regions are diagonal
			angle -= 45;

			if (angle < 0)
				angle += 360;

			int index = angle / 90;

			int deviation = abs(angle - (index * 90 + 45));
			lenSq += deviation / 45.0f;

			if (lenSq < distances[index]) {
				distances[index] = lenSq;
				ids[index] = to.id;
			}
		}

		key.toRight = ids[0];
		key.toDown = ids[1];
		key.toLeft = ids[2];
		key.toUp = ids[3];
	}
}

KeyboardLayout::~KeyboardLayout() {
}
