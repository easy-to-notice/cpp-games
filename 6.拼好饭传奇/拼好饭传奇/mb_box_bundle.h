#pragma once
#include "region.h"


class MbBoxBundle :public Region {
public:
	MbBoxBundle(int x, int y) :Region({ x,y,120,124 }) {}
	~MbBoxBundle() = default;

	void on_cursor_up()override;
	void on_cursor_down()override;
	void on_render(SDL_Renderer* renderer)override;
};