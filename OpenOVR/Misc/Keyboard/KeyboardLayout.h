#pragma once
#include <string>
#include <vector>
#include <map>

class KeyboardLayout {
public:
	struct Key {
		int id;

		wchar_t ch, shift;
		float x, y;

		std::wstring label, labelShift;
		float w, h;

		// Is the key touching the right-most side of the keyboard?
		bool spansToRight;

		// The IDs of the keys on each side, and -1 indicates there is no key on that side
		int toLeft, toRight, toUp, toDown;
	};

	using keymap_t = std::vector<Key>;

	KeyboardLayout(std::vector<char>);
	~KeyboardLayout();

	const keymap_t & GetKeymap() const { return keys; }
	int GetWidth() const { return width; }

private:
	keymap_t keys;
	int width = 0;
};

