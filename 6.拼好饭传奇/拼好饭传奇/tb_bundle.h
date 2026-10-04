#pragma once
#include "region.h"

class TbBundle :public Region {
public:
	TbBundle(int x, int y) :Region({ x,y,120,96 }) {}
	~TbBundle() = default;
	
	void on_cursor_up()override;
	void on_cursor_down()override;
	void on_render(SDL_Renderer* renderer)override;
};