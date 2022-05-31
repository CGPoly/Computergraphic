#pragma once

#include "glm/fwd.hpp"
#include "glm/detail/type_vec3.hpp"
#include "glm/detail/type_mat4x4.hpp"

class Camera
{
private:
	// mouse state
	int lastX;
	int lastY;
	int dragStartX;
	int dragStartY;
	bool leftDown;
	bool middleDown;
	bool rightDown;
	bool dragging;

	// transform state
	glm::vec3 look_at{};
	float phi;
	float theta;
	float radius;
	glm::mat4 view_mat{};

	void click_left(int x, int y);
	void click_right(int x, int y);
	void click_middle(int x, int y);
	void move(int x, int y, int dx, int dy);
	void drag_left(int x, int y, int dx, int sy);
	void drag_right(int x, int y, int dx, int dy);
	void drag_middle(int x, int y, int dx, int dy);
	void update();

public:
	Camera();

	glm::mat4 view_matrix();
	glm::vec3 position();

	void mouse(int button, int action, int);
	void motion(int x, int y);
	void scroll(int delta);
};
