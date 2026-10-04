#pragma once
#include "vector2.h"

class Camera {
public:
	Camera() = default;
	~Camera() = default;

	void set_size(const Vector2& vec) {
		size = vec;
	}

	const Vector2& get_size()const {
		return size;
	}

	const Vector2& get_position()const {
		return position;
	}

	void set_position(const Vector2& vec) {
		position = vec;
	}

	void look_at(const Vector2& target) {
		position = target - size / 2.0f;
	}

private:
	Vector2 size;
	Vector2 position;
};