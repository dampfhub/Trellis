#include "camera.h"
#include "resource_manager.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Camera2D::Camera2D()
	: Bounds(0.0f),
	BoardDims(glm::vec2(0.0f)),
	ZoomBounds(glm::vec2(0.0f, 0.0f)),
	Position(glm::vec2(0.0f, 0.0f)),
	Velocity(glm::vec2(0.0f, 0.0f)),
	Zoom(1.0f),
	Speed(1.0f),
	ZoomSpeed(0.0f) {}

Camera2D::Camera2D(float bounds, glm::vec2 board_dimensions, glm::vec2 zoom_bounds)
	: Bounds(bounds),
	BoardDims(board_dimensions),
	ZoomBounds(zoom_bounds),
	Position(glm::vec2(0.0f, 0.0f)),
	Velocity(glm::vec2(0.0f, 0.0f)),
	Zoom(1.0f),
	Speed(1.0f),
	ZoomSpeed(0.0f) {}

void Camera2D::Update(float dt, glm::vec2 board_dims) {
	this->BoardDims = board_dims;
	glm::mat4 view = glm::mat4(1.0f);
	this->Position -= this->Velocity;
	this->Zoom += this->ZoomSpeed * dt;

	// Check if would zoom out of zoom bounds
	if (this->Zoom < this->ZoomBounds.x)
		this->Zoom = this->ZoomBounds.x;
	if (this->Zoom > this->ZoomBounds.y)
		this->Zoom = this->ZoomBounds.y;

	// Scuffed right boundary equation (<3 Taylor)
	// Comes from:
	// Solve[m == (d - p)*z, p] // FullSimplify
	// { { p->d - m / z } }
	if ((this->BoardDims.x - this->Position.x) * this->Zoom < (this->ScreenDims.x - this->Bounds))
		this->Position.x = this->BoardDims.x - (this->ScreenDims.x - this->Bounds) / this->Zoom;
	if ((this->BoardDims.y - this->Position.y) * this->Zoom < (this->ScreenDims.y - this->Bounds))
		this->Position.y = this->BoardDims.y - (this->ScreenDims.y - this->Bounds) / this->Zoom;
	// Check if camera would move out of bounds
	if (this->Position.x < -this->Bounds / this->Zoom)
		this->Position.x = -this->Bounds / this->Zoom;
	if (this->Position.y < -this->Bounds / this->Zoom)
		this->Position.y = -this->Bounds / this->Zoom;

	// Setup view matrix using camera position and zoom
	view = glm::scale(view, glm::vec3(this->Zoom, this->Zoom, 1.0f));
	view = glm::translate(view, glm::vec3(-this->Position, 0.0f));
	this->View = view;

	this->Velocity = glm::vec2(0.0f, 0.0f);
	this->ZoomSpeed = 0.0f;
}

void Camera2D::Move(glm::vec2 mv) {
	this->Velocity = mv * this->Speed;
}

void Camera2D::ZoomIn() {
	this->ZoomSpeed = 5.0f;
}

void Camera2D::ZoomOut() {
	this->ZoomSpeed = -5.0f;
}