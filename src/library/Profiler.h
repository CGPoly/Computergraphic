#pragma once

#include <chrono>

class Profiler {
public:
	void beginTextures();
	void endTextures();
	void beginPathmarcher();
	void endPathmarcher();
	void beginBloom();
	void endBloom();
	void beginTonemap();
	void endTonemap();
	void passFinished();

	[[nodiscard]] unsigned int getTextureTime() const;
	[[nodiscard]] unsigned int getPassTime() const;
	[[nodiscard]] unsigned int getBloomTime() const;
	[[nodiscard]] unsigned int getTonemapTime() const;

private:
	std::chrono::time_point<std::chrono::system_clock> textureStart{};
	std::chrono::time_point<std::chrono::system_clock> pathmarcherStart{};
	std::chrono::time_point<std::chrono::system_clock> bloomStart{};
	std::chrono::time_point<std::chrono::system_clock> tonemapStart{};

	unsigned int textureTime = 0;
	unsigned int passTime = 0;
	unsigned int bloomTime = 0;
	unsigned int tonemapTime = 0;

	unsigned int pathmarcherAccumulatedTime = 0;
};
