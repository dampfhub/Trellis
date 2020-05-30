#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "texture.h"
#include "sprite_renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject {
public:
	glm::vec2 Position, Size;
	glm::vec3 Color;
	float	  Rotation;

	Texture2D Sprite;

	bool Clickable;

	bool FollowMouse = false;

	GameObject();
	GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, 
		bool clickable = true, glm::vec3 color = glm::vec3(1.0f));

	void Draw(SpriteRenderer *renderer, bool draw_border);
	bool CheckContainment(glm::vec2 pos);
	//bool CheckBorderContainment(glm::vec2 pos);
	glm::vec2 DistanceFromTopLeft(glm::vec2 pos);
};


#endif