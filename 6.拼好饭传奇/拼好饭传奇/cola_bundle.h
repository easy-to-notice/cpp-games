#pragma once
#include "region.h"

class ColaBundle :public Region {
public:
	ColaBundle(int x, int y) : Region({ x, y, 120, 124 }) {}
	ColaBundle() = default;

	void on_cursor_up() override;
	void on_cursor_down() override;
	void on_render(SDL_Renderer* renderer) override;
};