#include "Timeline.h"

void Timeline::update(std::chrono::duration<float> time) {
	/*
	 * time.count() returns time as float in seconds
	 *
	 * example:
	 * if (time.count() > 10 && time.count() < 20)
	 *  moonAlbedoResolution = 1024 * 16;
	 * else
	 *  moonAlbedoResolution = 1024 * 4;
	 */
}

unsigned int Timeline::getEarthResolution() const {
	return earthResolution;
}
unsigned int Timeline::getMoonResolution() const {
	return moonResolution;
}
unsigned int Timeline::getGasgiantResolution() const {
	return gasgiantResolution;
}
