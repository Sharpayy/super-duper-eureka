#pragma once
#include <glm.hpp>
#include <SDL.h>

typedef struct _RenderableMapSettings
{
	float MoveX;
	float MoveY;
	float ScaleX;
	float ScaleY;

	glm::mat4 CameraMatrix;
	glm::mat4 ScaleMatrix;
	uint32_t NeedUpdate;

} RenderableMapSettings;

float smoothstep(float actualPos, float destination, float dt);

class Camera {
public:
	Camera() = default;
	Camera(RenderableMapSettings* settings, uint32_t screenWidth, uint32_t screenHeight);

	glm::vec2 getMousePosition();
private:
	RenderableMapSettings* settings;
	uint32_t screenHalfWidth;
	uint32_t screenHalfHeight;
};