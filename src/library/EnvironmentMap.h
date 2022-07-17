#pragma once

#include "Texture.h"

class EnvironmentMap {
private:
	Texture environmentMap;

	static Texture loadTexture();

public:
	EnvironmentMap();

	[[nodiscard]] Texture const& getTexture();
};
