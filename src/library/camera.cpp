#include "Camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "GLFW/glfw3.h"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

Camera::Camera() {
	this->lastX = 0;
	this->lastY = 0;
	this->dragStartX = 0;
	this->dragStartY = 0;
	this->leftDown = false;
	this->middleDown = false;
	this->rightDown = false;
	this->dragging = false;

	this->look_at = glm::vec3(0.f);
	this->phi = 0.f;
	this->theta = 0.f;
	this->radius = 5.f;

	this->view_mat = glm::identity<glm::mat4>();
	update();
}

glm::mat4 Camera::view_matrix() {
	return this->view_mat;
}

glm::vec3 Camera::position(){
	glm::mat3 R(this->view_mat);
	glm::vec3 t(this->view_mat[3]);
	return -glm::transpose(R) * t;
}

void Camera::mouse(int button, int action, int) {
	int crt_x = this->lastX;
	int crt_y = this->lastY;
	if (action == GLFW_PRESS) {
		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:    this->dragStartX = crt_x;
				this->dragStartY = crt_y;
				this->leftDown = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:   this->dragStartX = crt_x;
				this->dragStartY = crt_y;
				this->rightDown = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:  this->dragStartX = crt_x;
				this->dragStartY = crt_y;
				this->middleDown = true;
				break;
		}
	} else {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			this->leftDown = false;
			if (this->dragging) {
				this->dragging = false;
			} else {
				click_left(this->dragStartX, this->dragStartY);
			}
		} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			this->rightDown = false;
			if (this->dragging) {
				this->dragging = false;
			} else {
				click_right(this->dragStartX, this->dragStartY);
			}
		} else {
			this->middleDown = false;
			if (this->dragging) {
				this->dragging = false;
			} else {
				click_middle(this->dragStartX, this->dragStartY);
			}
		}
	}
}

void Camera::motion(int x, int y) {
	if ((this->leftDown || this->rightDown || this->middleDown) && (abs(x - this->dragStartX) + abs(y - this->dragStartY) > 2)) {
		this->dragging = true;
	}

	if (this->dragging) {
		if (this->leftDown) drag_left(x, y, x - this->lastX, y - this->lastY);
		if (this->rightDown) drag_right(x, y, x - this->lastX, y - this->lastY);
		if (this->middleDown) drag_middle(x, y, x - this->lastX, y - this->lastY);
	} else {
		move(x, y, x - this->lastX, y - this->lastY);
	}

	this->lastX = x;
	this->lastY = y;
}

void Camera::scroll(int delta) {
	this->radius += 0.5f * delta;
	if (this->radius < 0.001f)
		this->radius = 0.001f;

	update();
}

void Camera::click_left(int x, int y) {
}

void Camera::click_right(int x, int y) {
}

void Camera::click_middle(int x, int y) {
}

void Camera::move(int x, int y, int dx, int dy) {
}

void Camera::drag_left(int x, int y, int dx, int dy) {
}

void Camera::drag_right(int x, int y, int dx, int dy) {
	this->theta -= 0.01f * dy;
	if (this->theta < -0.5*M_PI) this->theta = -0.5*M_PI;
	if (this->theta >  0.5*M_PI) this->theta =  0.5*M_PI;
	// technically not necessary, but might avoid numeric instability
	while (this->phi < 0.0)       this->phi += 2.0*M_PI;
	while (this->phi >= 2.0*M_PI) this->phi -= 2.0*M_PI;


	this->phi -= 0.01f * dx;
	update();
}

void Camera::drag_middle(int x, int y, int dx, int dy) {
	glm::vec3 u(this->view_mat[0][0],
	            this->view_mat[1][0],
	            this->view_mat[2][0]);
	glm::vec3 v(this->view_mat[0][1],
	            this->view_mat[1][1],
	            this->view_mat[2][1]);
	this->look_at -= 0.01f * dx * u;
	this->look_at += 0.01f * dy * v;
	update();
}

void Camera::update() {
	glm::mat4 trans_radius = glm::identity<glm::mat4>();
	glm::mat4 trans_center = glm::identity<glm::mat4>();
	glm::mat4 rot_theta = glm::identity<glm::mat4>();
	glm::mat4 rot_phi = glm::identity<glm::mat4>();

	trans_radius[3][2] = -this->radius;
	trans_center[3] = glm::vec4(-this->look_at, 1.f);

	rot_theta[1][1] = cosf(this->theta);
	rot_theta[2][1] = sinf(this->theta);
	rot_theta[1][2] = -rot_theta[2][1];
	rot_theta[2][2] = rot_theta[1][1];

	rot_phi[0][0] = cosf(this->phi);
	rot_phi[0][2] = sinf(this->phi);
	rot_phi[2][0] = -rot_phi[0][2];
	rot_phi[2][2] = rot_phi[0][0];

	this->view_mat = trans_radius * rot_theta * rot_phi * trans_center;
	this->hasChanged = true;
}

bool Camera::pollChanged() {
	bool changed = this->hasChanged;
	this->hasChanged = false;
	return changed;
}
