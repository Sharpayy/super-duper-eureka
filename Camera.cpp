#include "Camera.h"
#include <iostream>

float smoothstep(float actualPos, float destination, float dt) {
	dt = glm::clamp(dt, 0.0f, 1.0f);
	float smoothT = glm::smoothstep(0.0f, 1.0f, dt);
	return glm::mix(actualPos, destination, smoothT);
}
Camera::Camera(RenderableMapSettings* settings, uint32_t screenWidth, uint32_t screenHeight) {
	this->settings = settings;
	this->screenHalfWidth = screenWidth / 2.0f;
	this->screenHalfHeight = screenHeight / 2.0f;
}

glm::vec2 Camera::getMousePosition() {
	int x, y;
	SDL_GetMouseState(&x, &y);

	float scaleX = settings->ScaleX / (float)screenHalfWidth;
	float scaleY = settings->ScaleY / (float)screenHalfHeight;

	return glm::fvec2{ (x - (float)screenHalfWidth) * scaleX + settings->MoveX, (-1 * (y - (float)screenHalfHeight)) * scaleY + settings->MoveY };
}