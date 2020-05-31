#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <utility>

class Camera2D {
public:
	glm::vec2 Position, Velocity;
	float     Zoom, Speed, ZoomSpeed;
	
	float Bounds;
	glm::vec2 BoardDims;
	glm::vec2 ZoomBounds;

	glm::mat4 View = glm::mat4(1.0f);

	Camera2D(float bounds,
	        glm::vec2 board_dimensions,
	        glm::vec2 zoom_bounds,
	        std::shared_ptr<std::pair<int, int>> screenDims);

	void Update(float dt, glm::vec2 board_dims);
	void Move(glm::vec2 mv);

	void ZoomIn();
	void ZoomOut();
private:
	std::shared_ptr<std::pair<int, int>> ScreenDims;
};

#endif