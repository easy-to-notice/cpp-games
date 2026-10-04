#pragma once
#include "region.h"
#include "meal.h"

class TakeoutBox :public Region {
public:
	TakeoutBox(int x, int y) :Region({ x,y,120,124 }) {}
	~TakeoutBox() = default;

	void on_cursor_up()override;
	void on_cursor_down()override;
	void on_render(SDL_Renderer* renderer)override;

private:
	Meal meal = Meal::None;

private:
	bool can_place(Meal target);
};