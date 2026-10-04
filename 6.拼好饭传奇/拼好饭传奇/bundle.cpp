#include "bundle.h"
#include "res_mgr.h"
#include "cursor_mgr.h"

void Bundle::on_cursor_up() {
	if (CursorMgr::instance()->get_picked() == meal_type)
		CursorMgr::instance()->set_picked(Meal::None);
}

void Bundle::on_cursor_down() {
	if (CursorMgr::instance()->get_picked() == Meal::None)
		CursorMgr::instance()->set_picked(meal_type);
}

void Bundle::on_render(SDL_Renderer* renderer) {
	static SDL_Texture* texture = ResMgr::instance()->find_texture("cola_bundle");
	SDL_RenderCopy(renderer, texture, nullptr, &rect);
}