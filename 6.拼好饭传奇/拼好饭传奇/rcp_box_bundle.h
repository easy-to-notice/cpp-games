#pragma once
#include "region.h"

class RcpBoxBundle :public Region {
public:
	RcpBoxBundle(int x, int y) :Region({ x,y,120,124 }) {}
	~RcpBoxBundle() = default;

	void on_cursor_up()override;
	void on_cursor_down()override;
	void on_render(SDL_Renderer* renderer)override;
};
