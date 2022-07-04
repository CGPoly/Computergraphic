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

	template<class T>
	void render(float time, std::optional<std::tuple<Profiler<T>&, T>> profiling = {});

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
		{ "planets_textures.comp", GL_COMPUTE_SHADER }
	}};

	void renderImpl(float time);
};

template<class T>
void TexturesRenderer::render(float time, std::optional<std::tuple<Profiler<T>&, T>> profiling) {
	if (textureProgram.compile())
		lastTime = -1; // reset last time, because textureProgram source changed
	if (!textureProgram.isValid() || lastTime == time)
		return;

	if (profiling) {
		auto [profiler, type] = *profiling;
		profiler.begin(type);
	}

	renderImpl(time);
	lastTime = time;

	if (profiling) {
		auto [profiler, type] = *profiling;
		profiler.end(type);
		profiler.commit(type);
	}
}
