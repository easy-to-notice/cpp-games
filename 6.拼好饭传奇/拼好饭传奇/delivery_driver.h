#pragma once
#include "timer.h"
#include "meal.h"
#include "region.h"

class DeliveryDriver :public Region {
public:
	DeliveryDriver(int x, int y);
	~DeliveryDriver() = default;

	void on_cursor_up() override;
	void on_update(float delta) override;
	void on_render(SDL_Renderer* renderer) override;

private:
	enum class Status {
		Waiting,Completed
	};

private:
	Timer timer_refresh;// 外卖员刷新计时器
	float time_waited = 0;// 已经等待的时间
	bool is_waiting = false;// 当前是否正在等待
	bool is_meituan = false;// 是否是美团外卖员
	std::vector<Meal> meal_list;//餐品列表
	std::vector<Status> status_list;//餐品状态列表
	int num_drink = 0, num_dish = 0;// 饮料和菜品数量
	const float time_waited_limit = 40; // 等待时间上限

private:
	void refresh();
	bool check_and_place(Meal target);
};