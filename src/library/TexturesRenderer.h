#pragma once

#include "glad/glad.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Profiler.h"

class TexturesRenderer {
public:
	explicit TexturesRenderer(
			unsigned int earthResolution,
			unsigned int moonResolution,
			unsigned int gasgiantResolution
	) noexcept;
	TexturesRenderer(TexturesRenderer const&) = delete;
	TexturesRenderer(TexturesRenderer&&) = delete;

	TexturesRenderer& operator=(TexturesRenderer const&) = delete;
	TexturesRenderer& operator=(TexturesRenderer&&) = delete;

	template<class T>
	void render(float time, std::optional<std::tuple<Profiler<T>&, T>> profiling = {});

	[[nodiscard]] Texture const& getEarthAlbedoPlusHeight() const;
	[[nodiscard]] Texture const& getMoonAlbedoPlusHeight() const;
	[[nodiscard]] Texture const& getGasgiantAlbedo() const;
private:
	const glm::uvec2 workGroupSize{32};
	const glm::uvec2 workGroupCount{4};

	unsigned int earthResolution;
	unsigned int moonResolution;
	unsigned int gasgiantResolution;
	float lastTime = -1;

	Texture earthAlbedoPlusHeight;
	Texture moonAlbedoPlusHeight;
	Texture gasgiantAlbedo;

	ShaderProgram earthTextureProgram{{
		{ "textures/earth.comp", GL_COMPUTE_SHADER }
	}};
	ShaderProgram moonTextureProgram{{
		{ "textures/moon.comp", GL_COMPUTE_SHADER }
	}};
	ShaderProgram gasgiantTextureProgram{{
		{ "textures/gasgiant.comp", GL_COMPUTE_SHADER }
	}};

	void renderImpl(float time);
	void dispatchOverTexture(ShaderProgram& program, unsigned int width, unsigned int height) const;
};

template<class T>
void TexturesRenderer::render(float time, std::optional<std::tuple<Profiler<T>&, T>> profiling) {
	bool earthCompile = earthTextureProgram.compile();
	bool moonCompile = moonTextureProgram.compile();
	bool gasgiantCompile = gasgiantTextureProgram.compile();

	if (earthCompile || moonCompile || gasgiantCompile)
		lastTime = -1; // reset last time, because earthTextureProgram source changed
	if (!earthTextureProgram.isValid() || !moonTextureProgram.isValid() || !gasgiantTextureProgram.isValid() || lastTime == time)
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
