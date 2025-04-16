#define GLM_ENABLE_EXPERIMENTAL
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <commdlg.h> 
#include <iostream>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_internal.h>
#include <filesystem> 

#include "sceneManager.hpp"
#include "pickingBuffer.hpp"
#include "gltfImporter.hpp"
#include "cubemap.hpp"
#include "depthBuffer.hpp"
#include "uiManager.hpp"
#include "appConfig.hpp"
	

	Camera camera(glm::vec3(-10.0f, 3.0f, 13.0f), glm::vec3(0.0f,1.0f,0.0f), -45.0f, 0.0f);

	bool wireframeKeyPressed = false;
	bool rightKeyPressed = false;
	float increasedSpeed = camera.speed *3;
	float defaultSpeed = camera.speed;

	std::vector<glm::vec3> defaultCameraMatrix = {camera.position, camera.front, camera.up};
	float defaultCameraRotation[] = {camera.pitch, camera.yaw};
	bool cameraReseted = true;

	double mousePosx, mousePosy;
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);


	Primitive* getIdFromPickColor(const glm::vec3 &color) 
		{
			const float golden_ratio_conjugate = 0.618033988749895f;
			glm::vec3 hsv = rgb2hsv(color);
			float h = hsv.x;
			Primitive* closestObject  = nullptr;
			const unsigned int MAX_PICKABLE_OBJECTS = 1000;  // adjust as needed
			for (Primitive& primitive: SceneManager::getPrimitives()) 
				{
					float computedH = glm::fract(primitive.vao * golden_ratio_conjugate);
					std::cout << computedH << " " << h << std::endl;
					// Allow a small tolerance since floating-point imprecision can occur
					if (glm::abs(computedH - h) < 0.01f) 
					{
						closestObject = &primitive;
						break;
					}
				}
			return closestObject;
		}

	void processMovement(GLFWwindow* window, float deltaTime)
		{
			if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
			{
				if (!rightKeyPressed)
				{
					rightKeyPressed = true;
					glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
				}
				if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(FORWARD,deltaTime) , cameraReseted = true;
				if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(BACKWARD,deltaTime) , cameraReseted = true;
				if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(LEFT,deltaTime) , cameraReseted = true;
				if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(RIGHT,deltaTime) , cameraReseted = true;
				if(glfwGetKey(window,GLFW_KEY_Q) == GLFW_PRESS) camera.processKeyboard(DOWN,deltaTime) , cameraReseted = true;
				if(glfwGetKey(window,GLFW_KEY_E) == GLFW_PRESS) camera.processKeyboard(UP,deltaTime) , cameraReseted = true;
			}
			if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
			{
				if (rightKeyPressed) 
					glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
				rightKeyPressed = false;
			} 
		
			if (glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				camera.speed = increasedSpeed;
			else 
				camera.speed = defaultSpeed;
		
			if (glfwGetKey(window,GLFW_KEY_F) == GLFW_PRESS) cameraReseted = false;
		}

	void processInput(GLFWwindow* window,bool& isWireframe, float deltaTime, uint32_t& pickingFBO)
	{
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window,true);

		if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !wireframeKeyPressed)
			{
				wireframeKeyPressed = true;
				isWireframe = !isWireframe;
		}

		if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
			wireframeKeyPressed = false;

		static bool wasMousePressed = false;
		if( AppConfig::WINDOW_WIDTH != 0 && AppConfig::WINDOW_HEIGHT != 0) 
		{
			if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
				wasMousePressed = true;
			else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && wasMousePressed)
			{
				wasMousePressed = false;
				projection = glm::perspective(glm::radians(camera.zoom), float(AppConfig::WINDOW_WIDTH)/float(AppConfig::WINDOW_HEIGHT),0.1f, 100.0f);    
				view = camera.getViewMatrix();
				glm::vec3 pickedColor = pickObjectAt(mousePosx, mousePosy, AppConfig::WINDOW_HEIGHT, pickingFBO);
				Primitive* primitive = getIdFromPickColor(pickedColor);
				if (primitive != nullptr)
				{
					if (SceneManager::getSelectedPrimitive() != primitive)
					{
						if (SceneManager::getSelectedPrimitive() != nullptr) SceneManager::getSelectedPrimitive()->selected = false;
						primitive->selected = true;
						SceneManager::setSelectedPrimitive(primitive);
					}
					std::cout << "VAO: " << primitive->vao << std::endl;
				}
				else
				{
					if (SceneManager::getSelectedPrimitive() != nullptr)
					{
						SceneManager::getSelectedPrimitive()->selected = false;
						SceneManager::setSelectedPrimitive(nullptr);
					}
				}
			}
		}
	}

	float lastX = (float)(AppConfig::WINDOW_WIDTH / 2), lastY = (float) (AppConfig::WINDOW_HEIGHT / 2);
	bool firstMouse = true;

	void mouseCallback(GLFWwindow* window, double xPos, double yPos)
	{
		UIManager* ui = static_cast<UIManager*>(glfwGetWindowUserPointer(window));
		if (!ui->wantCaptureInput())
		{
			if (firstMouse)
			{	lastX = (float)xPos;
				lastY = (float)yPos;
				firstMouse = false;
			}
			mousePosx = xPos;
			mousePosy = yPos;
			glfwGetWindowSize(window,&AppConfig::WINDOW_WIDTH, &AppConfig::WINDOW_HEIGHT);
			
			float xOffset = (float)xPos - (float)lastX;
			float yOffset = lastY - (float)yPos ;
			lastX = (float)xPos;
			lastY = (float)yPos;
			if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) 
				camera.processMouseMovement(xOffset,yOffset);
		}
	}

	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		UIManager* ui = static_cast<UIManager*>(glfwGetWindowUserPointer(window));
		if (!ui->wantCaptureInput())
			camera.processMouseScroll((float)yOffset);
	}

	uint32_t lightSSBO;

	void CreateLightSSBO() 
	{
		glCreateBuffers(1, &lightSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightSSBO);
	};

	void UpdateLights(std::vector<Light>& lights)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(Light), lights.data(), GL_DYNAMIC_DRAW);
	}


	float quadVertices[] = { 
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	float debugQuadVertices[] = { 
		// positions   // texCoords
		0.75f,  1.0f,  0.0f, 1.0f,	// right top
		0.75f, 0.75f,  0.0f, 0.0f,	//left bottom
		1.0f, 0.75f,  1.0f, 0.0f,	//right bottom

		0.75f,  1.0f,  0.0f, 1.0f,	// right top
		1.0f, 0.75f,  1.0f, 0.0f,	//right bottom
		1.0f,  1.0f,  1.0f, 1.0f	//left top
	};

	uint32_t screenQuadVAO, screenQuadVBO;
	void initScreenQuad()
	{
		glCreateVertexArrays(1, &screenQuadVAO);
		glCreateBuffers(1, &screenQuadVBO);
		glNamedBufferData(screenQuadVBO, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(screenQuadVAO, 0, screenQuadVBO, 0, sizeof(float) * 4);

		glEnableVertexArrayAttrib(screenQuadVAO, 0);
		glVertexArrayAttribFormat(screenQuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(screenQuadVAO, 0, 0);

		glEnableVertexArrayAttrib(screenQuadVAO, 1);
		glVertexArrayAttribFormat(screenQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
		glVertexArrayAttribBinding(screenQuadVAO, 1, 0);

	}

	uint32_t debugQuadVAO, debugQuadVBO;
	void initDebugQuad()
	{
		glCreateVertexArrays(1, &debugQuadVAO);
		glCreateBuffers(1, &debugQuadVBO);
		glNamedBufferData(debugQuadVBO, sizeof(quadVertices), &debugQuadVertices, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(debugQuadVAO, 0, debugQuadVBO, 0, sizeof(float) * 4);

		glEnableVertexArrayAttrib(debugQuadVAO, 0);
		glVertexArrayAttribFormat(debugQuadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(debugQuadVAO, 0, 0);

		glEnableVertexArrayAttrib(debugQuadVAO, 1);
		glVertexArrayAttribFormat(debugQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
		glVertexArrayAttribBinding(debugQuadVAO, 1, 0);

	}
	uint32_t fbo, textureColorBufferMultiSampled, rbo, intermediateFBO, screenTexture;
	void initFrameBufferAndRenderTarget()
	{
		glCreateFramebuffers(1, &fbo);

		glCreateRenderbuffers(1, &rbo);
		glNamedRenderbufferStorage(rbo, GL_DEPTH24_STENCIL8, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
		glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


		// create a color attachment texture
		glCreateTextures(GL_TEXTURE_2D, 1, &screenTexture);
		glTextureStorage2D(screenTexture, 1, GL_RGB8, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
		glTextureParameteri(screenTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(screenTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, screenTexture, 0);

	}

	void framebufferSizeCallback(GLFWwindow* window, int32_t newWINDOW_WIDTH, int32_t newWINDOW_HEIGHT)
	{
		AppConfig::WINDOW_WIDTH = newWINDOW_WIDTH;
		AppConfig::WINDOW_HEIGHT = newWINDOW_HEIGHT;
		AppConfig::isFramebufferSizeSetted = false;
	}

	glm::vec3 lightColor = glm::vec3(1.0f);
	glm::mat4 lightmodel = glm::mat4(1.0f);

	float lightIntensity = 1.0f;
	glm::mat4 model = glm::mat4(1.0f);

	float lastFrame = 0;
	float deltaTime = 0;



	void draw(GLFWwindow* window)
	{
		Cubemap cubemap{};
		initScreenQuad();
		initDebugQuad();
		initFrameBufferAndRenderTarget();
		PickingBuffer pickingbuffer(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_WIDTH);
		DepthBuffer depthBuffer(2048, 2048);
		UIManager uiManager(window, deltaTime, camera);

		AppConfig::initShaders();
		//import
		GLTFModel gltfTent1(std::filesystem::absolute("..\\..\\res\\GltfModels\\BarDiorama.glb").string(), AppConfig::baseShader);
		SceneManager::addPrimitives(std::move(gltfTent1.primitives));

		//imgui

		CreateLightSSBO();

		
		Light pointLight;
		pointLight.type = 2;
		pointLight.intensity = 1;
		pointLight.position = glm::vec3(1.58f, 1.95f, 1.94f);
		pointLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
		pointLight.ambient   = glm::vec3(0.05f);
		pointLight.diffuse   = glm::vec3(0.8f);
		pointLight.specular  = glm::vec3(1.0f);
		pointLight.constant  = 1.0f;
		pointLight.linear    = 0.09f;
		pointLight.quadratic = 0.032f;
		pointLight.cutOff    = glm::cos(glm::radians(20.5f));
		pointLight.outerCutOff = glm::cos(glm::radians(72.5f));

		Light spotLight;
		spotLight.type = 2;
		spotLight.intensity = 1;
		spotLight.position = glm::vec3(1.58f, 1.95f, 3.94f);
		spotLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
		spotLight.ambient   = glm::vec3(0.05f);
		spotLight.diffuse   = glm::vec3(0.8f);
		spotLight.specular  = glm::vec3(1.0f);
		spotLight.constant  = 1.0f;
		spotLight.linear    = 0.09f;
		spotLight.quadratic = 0.032f;
		spotLight.cutOff    = glm::cos(glm::radians(20.5f));
		spotLight.outerCutOff = glm::cos(glm::radians(72.5f));

		Light directionalLight;
		directionalLight.type = 1;
		directionalLight.intensity = 0.1f;
		directionalLight.position = glm::vec3(0, 300.0f, 0);
		directionalLight.direction = glm::vec3( 0.0f, 0.0f,  0.0f);
		directionalLight.ambient   = glm::vec3(0.05f);
		directionalLight.diffuse   = glm::vec3(0.8f);
		directionalLight.specular  = glm::vec3(1.0f);
		directionalLight.constant  = 1.0f;
		directionalLight.linear    = 0.09f;
		directionalLight.quadratic = 0.032f;

		SceneManager::addLight(pointLight);
		SceneManager::addLight(directionalLight);
		SceneManager::addLight(spotLight);
		UpdateLights(SceneManager::getLights());


		while(!glfwWindowShouldClose(window))
		{   
			glBindTexture(GL_TEXTURE_2D, 0);
			float time = (float)glfwGetTime();
			
			deltaTime = time - lastFrame;
			lastFrame = time;
			if (!uiManager.wantCaptureInput())
			{
				processInput(window, AppConfig::isWireframe, deltaTime,pickingbuffer.pickingFBO);
				processMovement(window, deltaTime);
			}

			//framebuffer size change callback processing
			if (!AppConfig::isFramebufferSizeSetted)
			{
				glDeleteTextures(1, &screenTexture); 
				glDeleteRenderbuffers(1, &rbo);
				glDeleteFramebuffers(1, &fbo);

				glCreateFramebuffers(1, &fbo);

				glCreateRenderbuffers(1, &rbo);
				glNamedRenderbufferStorage(rbo, GL_DEPTH24_STENCIL8, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
				glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		
		
				// create a color attachment texture
				glCreateTextures(GL_TEXTURE_2D, 1, &screenTexture);
				glTextureStorage2D(screenTexture, 1, GL_RGB8, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
				glTextureParameteri(screenTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTextureParameteri(screenTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, screenTexture, 0);

				pickingbuffer.resize(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

				glViewport(0, 0, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

				AppConfig::isFramebufferSizeSetted = true;
			}
			
			//camera interpolated reset
			if(!cameraReseted && glm::length(camera.position - defaultCameraMatrix[0]) > .01 )
			{	
				float speed = 10 * deltaTime;
				camera.position = glm::mix(camera.position, defaultCameraMatrix[0], speed);
				camera.front = glm::mix(camera.front, defaultCameraMatrix[1], speed);
				camera.up = glm::mix(camera.up, defaultCameraMatrix[2], speed);
				camera.pitch = glm::mix(camera.pitch, defaultCameraRotation[0], speed);
				camera.yaw = glm::mix(camera.yaw, defaultCameraRotation[1], speed);
			}
			else cameraReseted = true; 

			if( AppConfig::WINDOW_WIDTH != 0 && AppConfig::WINDOW_HEIGHT != 0) 
			{
				projection = glm::perspective(glm::radians(camera.zoom), float(AppConfig::WINDOW_WIDTH)/float(AppConfig::WINDOW_HEIGHT),0.1f, 100.0f);	
				view = camera.getViewMatrix();
			}

			glEnable(GL_MULTISAMPLE);
			glEnable(GL_BLEND);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glDepthFunc(GL_LESS);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 
			glStencilMask(0x00);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//DIRECTIONAL LIGHT SHADOW MAP PASS
			glViewport(0, 0, depthBuffer.width, depthBuffer.height);
			depthBuffer.bind();
			glClear(GL_DEPTH_BUFFER_BIT);

			glm::vec3 sceneCenter = glm::vec3(0.0f); 
			float distance = 10000.0f;

			glm::mat4 lightProjection = glm::ortho(-7.0f, 7.0f, -7.0f, 7.0f, distance + AppConfig::near_plane,distance + AppConfig::far_plane); 
			
			glm::vec3 lightDirection = glm::normalize(SceneManager::getLights()[1].direction);

			glm::vec3 lightPos = camera.position + lightDirection * distance;

			glm::mat4 lightView = glm::lookAt(lightPos, lightDirection, glm::vec3(0.0, 1.0, 0.0));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;
			SceneManager::setShader(AppConfig::depthShader);
			SceneManager::draw(camera, lightSpaceMatrix, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			//OBJECT ID PASS
			glViewport(0, 0, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);
			pickingbuffer.bind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			SceneManager::setShader(AppConfig::pickingShader);
			SceneManager::draw(camera, lightSpaceMatrix, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT);

			//MAIN RENDER PASS
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glClearColor(AppConfig::clearColor[0], AppConfig::clearColor[1], AppConfig::clearColor[2], AppConfig::clearColor[3]); 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glPolygonMode(GL_FRONT_AND_BACK,AppConfig::polygonMode);
			SceneManager::setShader(AppConfig::baseShader);
			SceneManager::draw(camera, lightSpaceMatrix, AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT, depthBuffer.depthMap, AppConfig::gamma);
			
			//CUBEMAP RENDER PASS
			glDepthFunc(GL_LEQUAL);
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			glm::mat4 skyView = glm::mat4(glm::mat3(camera.getViewMatrix()));  
			cubemap.draw(AppConfig::skyboxShader, projection, skyView);
			glDepthFunc(GL_LESS);

			//SCREEN QUAD RENDER PASS
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			glClearColor(AppConfig::clearColor[0], AppConfig::clearColor[1], AppConfig::clearColor[2], AppConfig::clearColor[3]); 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			AppConfig::screenShader.use();  
			AppConfig::screenShader.setFloat("near_plane", distance + AppConfig::near_plane);
			AppConfig::screenShader.setFloat("far_plane", distance + AppConfig::far_plane);
			glBindVertexArray(screenQuadVAO);
			glBindTextureUnit(0, screenTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);  

			//DEBUG QUAD RENDER PASS
			if(AppConfig::showObjectPicking)
			{
				glBindVertexArray(debugQuadVAO);
				glBindTexture(GL_TEXTURE_2D, pickingbuffer.pickingTexture);
				glDrawArrays(GL_TRIANGLES, 0, 6);  
			} 
			if(AppConfig::showShadowMap)
			{
				glBindVertexArray(debugQuadVAO);
				glBindTexture(GL_TEXTURE_2D, depthBuffer.depthMap);
				glDrawArrays(GL_TRIANGLES, 0, 6);  
			} 

			//IMGUI RENDER PASS
			uiManager.draw();

			UpdateLights(SceneManager::getLights());

			glfwSwapBuffers(window);
			glfwPollEvents();    
		}
	}


	int main()	
	{
		GLFWwindow* window;
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		// glfwWindowHint(GLFW_FLOATING, GLFW_TRUE); // Uncomment this line to make the window floating
		glfwWindowHint(GLFW_SAMPLES, 16);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); 
		
		window = glfwCreateWindow(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT, "Main Window", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		}

		glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
		glfwSetCursorPosCallback(window, mouseCallback);  
		
		glfwSetScrollCallback(window,scrollCallback);  
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		draw(window);

		glfwTerminate();
		return 0;
	}
