#pragma once
#include "region.h"
#include "meal.h"

class Bundle :public Region {
public:
	Bundle() = default;
	~Bundle() = default;

	void on_cursor_up() override;
	void on_cursor_down() override;
	void on_render(SDL_Renderer* renderer) override;

private:
	Meal meal_type;
};