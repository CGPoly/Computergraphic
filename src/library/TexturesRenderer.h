#pragma once

#include "glad/glad.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Profiler.h"

class TexturesRenderer {
public:
	explicit TexturesRenderer(unsigned int resolution) noexcept;
	TexturesRenderer(TexturesRenderer const&) = delete;
	TexturesRenderer(TexturesRenderer&&) = delete;

	TexturesRenderer& operator=(TexturesRenderer const&) = delete;
	TexturesRenderer& operator=(TexturesRenderer&&) = delete;

	void render(float time, Profiler* profiler = nullptr);

	[[nodiscard]] Texture const& getAlbedo() const;
	[[nodiscard]] Texture const& getDisplacement() const;
	[[nodiscard]] Texture const& getRoughness() const;
private:
	const glm::uvec2 workGroupSize{32};
	const glm::uvec2 workGroupCount{4};

	unsigned int resolution;
	float lastTime = -1;

	Texture albedoTexture;
	Texture displacementTexture;
	Texture roughnessTexture;

	ShaderProgram textureProgram{{
		{ "textures.comp", GL_COMPUTE_SHADER }
	}};

};
