#include <iostream>
#include "Profiler.h"

void Profiler::beginTextures() {
	textureStart = std::chrono::system_clock::now();
}
void Profiler::endTextures() {
	auto now = std::chrono::system_clock::now();
	textureTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - textureStart).count();
}

void Profiler::beginPathmarcher(){
	pathmarcherStart = std::chrono::system_clock::now();
}
void Profiler::endPathmarcher() {
	auto now = std::chrono::system_clock::now();
	pathmarcherAccumulatedTime += std::chrono::duration_cast<std::chrono::milliseconds>(now - pathmarcherStart).count();
}

void Profiler::beginBloom() {
	bloomStart = std::chrono::system_clock::now();
}
void Profiler::endBloom() {
	auto now = std::chrono::system_clock::now();
	bloomTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - bloomStart).count();
}

void Profiler::beginTonemap() {
	tonemapStart = std::chrono::system_clock::now();
}
void Profiler::endTonemap() {
	auto now = std::chrono::system_clock::now();
	tonemapTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - tonemapStart).count();
}

void Profiler::passFinished() {
	passTime = pathmarcherAccumulatedTime;
	pathmarcherAccumulatedTime = 0;
}

unsigned int Profiler::getTextureTime() const { return textureTime; }
unsigned int Profiler::getPassTime() const { return passTime; }
unsigned int Profiler::getBloomTime() const { return bloomTime; }
unsigned int Profiler::getTonemapTime() const { return tonemapTime; }
