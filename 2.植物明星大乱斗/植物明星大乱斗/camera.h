#pragma once
#include "vector2.h"
#include "timer.h"

class Camera {
public:
	Camera() {
		timer_shaker.set_one_shot(1);
		timer_shaker.set_callback([&]() {
			is_shaking = 0;
			reset();
			});
	}
	~Camera() = default;

	const Vector2& get_position()const {
		return position;
	}

	void reset() {
		position.x = 0, position.y = 0;
	}

	void on_update(int delta) {
		timer_shaker.on_update(delta);
		
		if (is_shaking) {
			position.x = (-50 + rand() % 100) / 50.0f * shaking_strength;
			position.y = (-50 + rand() % 100) / 50.0f * shaking_strength;
		}
	}

	void shake(float strength, int duration) {
		is_shaking = true;
		shaking_strength = strength;

		timer_shaker.set_wait_time(duration);
		timer_shaker.restart();
	}

private:
	Vector2 position;			// 摄像机位置
	Timer timer_shaker;			// 摄像机抖动计时器
	bool is_shaking = 0;		// 是否正在抖动
	float shaking_strength = 0;	// 抖动幅度
};