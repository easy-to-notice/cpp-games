#pragma once
#include <functional>

//继承/回调 实现
//通用计时器 - 只需扩展方法逻辑无需扩展数据成员 -> 回调

class Timer {
public:
	Timer() = default;
	~Timer() = default;

	void restart() {
		pass_time = 0;
		shotted = 0;
	}

	void set_wait_time(int val) {
		wait_time = val;
	}

	void set_one_shot(bool flag) {
		one_shot = flag;
	}

	void set_callback(std::function<void()> callback) {
		this->callback = callback;
	}

	void pause() {
		paused = 1;
	}

	void resume() {
		paused = 0;
	}

	void on_update(int delta) {
		if (paused)return;

		pass_time += delta;
		if (pass_time >= wait_time) {
			if ((!one_shot || (one_shot && !shotted)) && callback)
				callback();
			shotted = 1;
			pass_time = 0;
		}
	}

private:
	int pass_time = 0;	// 已过时间
	int wait_time = 0;	// 等待时间
	bool paused = 0;	// 是否暂停
	bool shotted = 0;	// 是否触发
	bool one_shot = 0;	// 单次触发
	std::function<void()> callback;
};