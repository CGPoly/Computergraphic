#pragma once

#include "motion_control.h"

class Timeline {
public:
	motion_control motionControl{};

	Timeline() = default;
	Timeline(Timeline const&) = delete;
	Timeline(Timeline&&) = delete;

	Timeline& operator=(Timeline const&) = delete;
	Timeline& operator=(Timeline&&) = delete;

	void update(std::chrono::duration<float> time);

	[[nodiscard]] unsigned int getEarthResolution() const;
	[[nodiscard]] unsigned int getMoonResolution() const;
	[[nodiscard]] unsigned int getGasgiantResolution() const;

private:
	unsigned int earthResolution = 1024 * 4;
	unsigned int moonResolution = 1024 * 4;
	unsigned int gasgiantResolution = 1025 * 4;
};
