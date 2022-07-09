#include <cstring>
#include <iostream>
#include "Timeline.h"

Timeline::Timeline() noexcept {
	entities.push_back(Entity{0, {0, 0, 151600000000.}, glm::mat3(1), 695700000.});
	entities.push_back(Entity{1, {0, -(408000 + 12756270 / 2), 0}, {1, 0, 0, 0, 0, -1, 0, 1, 0}, 6371009});
	entities.push_back(Entity{2, {397000000, -(408000 + 12756270 / 2), 0}, glm::mat3(1), 1737400});
	entities.push_back(Entity{4, {}, glm::mat3(1), 157});
}

void Timeline::update(std::chrono::duration<float> time) noexcept {
	// time.count() time as float in milliseconds
	// do updating logic here
	updateSsbo();
}

Buffer const& Timeline::getEntitiesSsbo() const {
	return entitiesSsbo;
}
GLsizei Timeline::getEntitiesSsboSize() const {
	return entities.size() * ENTITY_STRIDE;
}

glm::vec3 const& Timeline::getCameraPosition() const {
	return cameraPosition;
}
glm::mat3 const& Timeline::getCameraRotation() const {
	return cameraRotation;
}
glm::vec3 const& Timeline::getJuliaC() const {
	return juliaC;
}

void Timeline::updateSsbo() {
	uint8_t* data = static_cast<uint8_t*>(glMapNamedBuffer(entitiesSsbo.getId(), GL_WRITE_ONLY));
	for (size_t i = 0; i < entities.size(); ++i) {
		std::memcpy(&data[i * ENTITY_STRIDE + ENTITY_OBJECT_OFFSET], &entities[i].objectId, sizeof(unsigned int));
		std::memcpy(&data[i * ENTITY_STRIDE + ENTITY_POSITION_OFFSET], &entities[i].position, sizeof(glm::vec3));
		std::memcpy(&data[i * ENTITY_STRIDE + ENTITY_ROTATION_OFFSET + 16 * 0], &entities[i].rotation[0], sizeof(glm::mat3));
		std::memcpy(&data[i * ENTITY_STRIDE + ENTITY_ROTATION_OFFSET + 16 * 1], &entities[i].rotation[1], sizeof(glm::mat3));
		std::memcpy(&data[i * ENTITY_STRIDE + ENTITY_ROTATION_OFFSET + 16 * 2], &entities[i].rotation[2], sizeof(glm::mat3));
		std::memcpy(&data[i * ENTITY_STRIDE + ENTITY_SCALE_OFFSET], &entities[i].scale, sizeof(float));
	}
	glUnmapNamedBuffer(entitiesSsbo.getId());
}
