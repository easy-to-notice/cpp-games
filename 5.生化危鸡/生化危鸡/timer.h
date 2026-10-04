#pragma once
#include <functional>


class Timer {
public:
	Timer() = default;
	~Timer() = default;

	void restart() {
		pass_time = 0;
		shotted = 0;
	}

	void set_wait_time(float val) {
		wait_time = val;
	}

	void set_one_shot(bool flag) {
		one_shot = flag;
	}

	void set_on_timeout(std::function<void()> on_timeout) {
		this->on_timeout = on_timeout;
	}

	void pause() {
		paused = 1;
	}

	void resume() {
		paused = 0;
	}

	void on_update(float delta) {
		if (paused)return;

		pass_time += delta;
		if (pass_time >= wait_time) {
			bool can_shot = (!one_shot || (one_shot && !shotted));
			shotted = 1;
			if (can_shot && on_timeout)
				on_timeout();
			pass_time -= wait_time;
		}
	}

private:
	float pass_time = 0;
	float wait_time = 0;
	bool paused = 0;
	bool shotted = 0;
	bool one_shot = 0;
	std::function<void()> on_timeout;
};