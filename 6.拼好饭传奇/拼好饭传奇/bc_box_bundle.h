#pragma once
#include "region.h"

class BcBoxBundle :public Region {
public:
	BcBoxBundle(int x, int y) :Region({ x,y,120,124 }) {}
	~BcBoxBundle() = default;

	void on_cursor_up()override;
	void on_cursor_down()override;
	void on_render(SDL_Renderer* renderer)override;
};