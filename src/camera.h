#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


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

		Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 upv = glm::vec3(0.0f,1.0f,0.0f), float yaw = YAW, float pitch = PITCH) :
		front(glm::vec3(0.0f,0.0f,-1.0f)),
		speed(SPEED),
		zoom(ZOOM),
		sensitivity(SENSITIVITY)
		{
			position = pos;
			worldUp = upv; 
			this -> yaw = yaw;
			this -> pitch = pitch;
			updateCameraVecotrs();
		}

		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw = YAW, float pitch = PITCH) :
		front(glm::vec3(0.0f,0.0f,-1.0f)),
		speed(SPEED),
		zoom(ZOOM),
		sensitivity(SENSITIVITY)
		{
			position = glm::vec3(posX,posY,posZ);
			worldUp = glm::vec3(upX,upY,upZ);
			this -> yaw = yaw;
			this -> pitch = pitch;
			updateCameraVecotrs();
		}

		glm::mat4 getViewMatrix()
		{
			return glm::lookAt(position, position + front, up);
		}

		void processKeyboard(CameraMovement direction,float deltaTime)
		{
			float velocity = speed * deltaTime;
			if (direction == FORWARD) position += front * velocity;
			if (direction == BACKWARD) position -= front * velocity;
			if (direction == LEFT) position -= right * velocity;
			if (direction == RIGHT) position += right * velocity;
			if (direction == UP) position += up * velocity;
			if (direction == DOWN) position -= up * velocity;
		}

		void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
		{	
				xOffset *= sensitivity;
				yOffset *= sensitivity;

				yaw += xOffset;
				pitch += yOffset;

				if(constrainPitch)
				{
					if (pitch > 89.0f) pitch = 89.0f;
					if (pitch < -89.0f) pitch = -89.0f;
				}
				updateCameraVecotrs();
		}

		void processMouseScroll(float yOffset)
		{
			zoom -= yOffset;

			if (zoom < 1.0f) zoom = 1.0f;
			if (zoom > 100.0f) zoom = 100.0f;
		}

	private:

		void updateCameraVecotrs()
		{
			glm::vec3 direction;
			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(pitch));
			direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			front = glm::normalize(direction);
			right = glm::normalize(glm::cross(front,worldUp));
			up = glm::normalize(glm::cross(right,front));
		}
};