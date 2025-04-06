#pragma once
#include "glm/glm.hpp"
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
	UP,
	DOWN
};

const float YAW 			= -90.0f;
const float PITCH 			= 0.0f;
const float SENSITIVITY		= 0.5f;
const float ZOOM  			= 45.f;
const float SPEED 			= 1.5f;

class Camera
{
	public:
		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 up;
		glm::vec3 right;
		glm::vec3 worldUp;

		float yaw;
		float pitch;

		float zoom;
		float speed;
		float sensitivity;

		Camera(glm::vec3 pos, glm::vec3 upv, float yaw , float pitch);

		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

		glm::mat4 getViewMatrix();

		void processKeyboard(CameraMovement direction,float deltaTime);

		void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

		void processMouseScroll(float yOffset);

	private:

		void updateCameraVecotrs();
};