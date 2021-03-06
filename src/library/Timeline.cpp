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

float Timeline::spline_time(float time) {
    // zoom from orbit to fractal
    return constant_time(time, 0, 2, 0)+ease(time, 2, 3, 0, 1) + constant_time(time, 3, 12, 1)
    // dramatic laser
    + ease(time,12, 13, 1, 0.015) + constant_time(time, 13, 14, 0.015) + ease(time, 14, 15, 0.015, 0.65)
    // docking
    + constant_time(time, 15, 17.5, .65) + ease(time, 17.5, 18, .65, 1) + constant_time(time, 18, 24.2, 1)
    //near fractal
    + ease(time, 24.2, 25, 1, 0.2) + constant_time(time, 25, 100, 0.2);
}

float Timeline::constant_time(float t, float start_t, float end_t, float speed) {
    if (t < start_t) return 0;
    if (end_t < t) return speed*end_t-speed*start_t;
    return speed*t-speed*start_t;
}

//integral of a quadratic ease function
float Timeline::ease(float time, float start_t, float end_t, float start_speed, float end_speed) {
    float a = start_speed;
    float b = end_speed;
    float c1 = end_t;
    float d = start_t;
    float c = 1/(c1-d);
    float t = c*(time-d);
    if (t < 0) return 0;
    auto F1 = [&a,&b,&c](float t){return 1.f/c*(2.f/3.f*(b-a)*t*t*t+a*t);};
    auto F2 = [&a,&b,&c,&F1](float t){
        return 1.f/c*((b-a)*(t-1.f/2.f-(2.f/3.f*t*t*t-2.f*t*t+2.f*t)+1.f/12.f-1.f/2.f+1.f)+a*t-a/2.f)+F1(.5f);
    };
    if (0 <= t && t <= 0.5) return F1(t);
    if (0.5 < t && t <= 1) return F2(t);
    return F2(1.f);
}
