#pragma once

#include <chrono>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Buffer.hpp"

class Timeline {
public:
	Timeline() noexcept;
	Timeline(Timeline const&) = delete;
	Timeline(Timeline&&) = delete;

	Timeline& operator=(Timeline const&) = delete;
	Timeline& operator=(Timeline&&) = delete;

	void update(std::chrono::duration<float> time) noexcept;

	[[nodiscard]] Buffer const& getEntitiesSsbo() const;
	[[nodiscard]] GLsizei getEntitiesSsboSize() const;

	[[nodiscard]] glm::vec3 const& getCameraPosition() const;
	[[nodiscard]] glm::mat3 const& getCameraRotation() const;

	[[nodiscard]] glm::vec3 const& getJuliaC() const;

private:
	static unsigned int const MAX_ENTITIES = 20;

	static unsigned int const ENTITY_STRIDE = 96;
	static unsigned int const ENTITY_OBJECT_OFFSET = 0;
	static unsigned int const ENTITY_POSITION_OFFSET = 16;
	static unsigned int const ENTITY_ROTATION_OFFSET = 32;
	static unsigned int const ENTITY_SCALE_OFFSET = 80;

	struct Entity {
		unsigned int objectId;
		glm::vec3 position;
		glm::mat3 rotation;
		float scale;
	};

	Buffer entitiesSsbo = Buffer::immutable(
			GL_SHADER_STORAGE_BUFFER,
			MAX_ENTITIES * sizeof(Entity),
			nullptr,
			GL_MAP_WRITE_BIT
	);

	std::vector<Entity> entities{};

	glm::vec3 cameraPosition{};
	glm::mat3 cameraRotation{1};

	glm::vec3 juliaC{};

	void updateSsbo();
};
