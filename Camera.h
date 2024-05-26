#pragma once
#include <glm.hpp>

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

class Camera {
public:
	Camera() = default;
	Camera(RenderableMapSettings* settings, uint32_t screenWidth, uint32_t screenHeight) {
		this->settings = settings;
		this->screenHalfWidth = screenWidth / 2.0f;
		this->screenHalfHeight = screenHeight / 2.0f;
	}

	glm::vec2 getMousePosition() {
		int x, y;
		SDL_GetMouseState(&x, &y);
		
		float scaleX = settings->ScaleX / (float)screenHalfWidth;
		float scaleY = settings->ScaleY / (float)screenHalfHeight;

		return glm::fvec2{ (x - (float)screenHalfWidth) * scaleX + settings->MoveX, (- 1 * (y - (float)screenHalfHeight)) * scaleY + settings->MoveY};
	}
private:
	RenderableMapSettings* settings;
	uint32_t screenHalfWidth;
	uint32_t screenHalfHeight;
};