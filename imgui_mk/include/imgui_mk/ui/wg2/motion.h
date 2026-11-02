#pragma once
// Mk::wg2 动效：缓动函数 + 帧率无关的趋近 + 定时过渡
#include <algorithm>
#include <cmath>

namespace Mk::wg2 {

inline float clamp01(float t) { return std::clamp(t, 0.0f, 1.0f); }
inline float lerp(float a, float b, float t) { return a + (b - a) * clamp01(t); }

inline float easeOutCubic(float t) {
	t = clamp01(t);
	const float u = 1.0f - t;
	return 1.0f - u * u * u;
}

inline float easeInOutCubic(float t) {
	t = clamp01(t);
	return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
}

// 指数趋近：与帧率无关，适合 hover/展开等"跟随目标"的动画
struct Motion {
	float value = 0.0f;

	void toward(float target, float speed, float dt) {
		if (speed <= 0.0f || dt <= 0.0f) return;
		const float k = 1.0f - std::exp(-speed * dt);
		value += (target - value) * k;
	}

	void snap(float v) { value = v; }
};

// 定时过渡：from → to，duration 秒；value() 用 easeOutCubic
struct Transition {
	float from = 0.0f;
	float to = 1.0f;
	float duration = 0.3f;
	float elapsed = 0.0f;
	bool running = false;

	void start(float a, float b, float dur) {
		from = a;
		to = b;
		duration = dur > 0.0f ? dur : 0.0001f;
		elapsed = 0.0f;
		running = true;
	}

	void update(float dt) {
		if (!running) return;
		elapsed += dt;
		if (elapsed >= duration) running = false;
	}

	float t() const { return running ? clamp01(elapsed / duration) : 1.0f; }
	float value() const { return lerp(from, to, easeOutCubic(t())); }
};

} // namespace Mk::wg2
