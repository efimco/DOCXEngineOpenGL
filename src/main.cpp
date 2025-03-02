
	#define GLM_ENABLE_EXPERIMENTAL
	#include <glad/glad.h>
	#include <windows.h>	
	#include <GLFW/glfw3.h>


	#include <glm/gtc/matrix_transform.hpp>
	#include <glm/gtc/type_ptr.hpp>
	#include <commdlg.h> 

	#include "sceneManager.h"
	#include "objectPicking.h"
	#include "gltfImIporter.h"
	#include "cubemap.h"
	#include "depthbuffer.h"

	#include <ImGui/imgui.h>
	#include <ImGui/imgui_impl_glfw.h>
	#include <ImGui/imgui_impl_opengl3.h>
	#include <ImGui/imgui_internal.h>
	#include <filesystem> 


	int32_t WINDOW_WIDTH = 1024;
	int32_t WINDOW_HEIGHT = 1024;
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

	bool showObjectPicking = false;
	bool showShadowMap = false;

	
	void processInput(GLFWwindow* window,bool& isWireframe, float deltaTime)
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
		if( WINDOW_WIDTH != 0 && WINDOW_HEIGHT != 0) 
		{
			if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
				wasMousePressed = true;
			else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && wasMousePressed)
			{
				wasMousePressed = false;
				projection = glm::perspective(glm::radians(camera.zoom), float(WINDOW_WIDTH)/float(WINDOW_HEIGHT),0.1f, 100.0f);    
				view = camera.getViewMatrix();
				glm::vec3 pickedColor = pickObjectAt(mousePosx, mousePosy, WINDOW_HEIGHT);
				Primitive* primitive = getIdFromPickColor(pickedColor);
				if (primitive != nullptr)
				{
					if (SceneManager::selectedPrimitive != primitive)
					{
						if (SceneManager::selectedPrimitive != nullptr) SceneManager::selectedPrimitive->selected = false;
						primitive->selected = true;
						SceneManager::selectedPrimitive = primitive;
					}
					std::cout << "VAO: " << primitive->vao << std::endl;
				}
				else
				{
					if (SceneManager::selectedPrimitive != nullptr)
					{
						SceneManager::selectedPrimitive->selected = false;
						SceneManager::selectedPrimitive = nullptr;
					}
				}
			}
		}


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

	float lastX = (float)(WINDOW_WIDTH / 2), lastY = (float) (WINDOW_HEIGHT / 2);
	bool firstMouse = true;
	float clearColor[4] = { 0.133f, 0.192f, 0.265f, 1.0f };

	void mouseCallback(GLFWwindow* window, double xPos, double yPos)
	{
		if (firstMouse)
		{	lastX = (float)xPos;
			lastY = (float)yPos;
			firstMouse = false;
		}
		mousePosx = xPos;
		mousePosy = yPos;
		glfwGetWindowSize(window,&WINDOW_WIDTH, &WINDOW_HEIGHT);
		
		float xOffset = (float)xPos - (float)lastX;
		float yOffset = lastY - (float)yPos ;
		lastX = (float)xPos;
		lastY = (float)yPos;
		if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) camera.processMouseMovement(xOffset,yOffset);
	}

	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		camera.processMouseScroll((float)yOffset);
	}

	std::string OpenFileDialog()
	{
		OPENFILENAMEA ofn;
		char fileName[260] = { 0 };    // buffer for file name

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = fileName;
		ofn.nMaxFile = sizeof(fileName);

		// Filter: display image files by default (you can adjust as needed)
		ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp\0All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrTitle = "Select a Texture";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		// Open the dialog box
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return std::string(ofn.lpstrFile);
		}

		return std::string();
	}

	uint32_t lightSSBO;

	void CreateLightSSBO() 
	{
		glGenBuffers(1, &lightSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
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

	float objectIdQuadVertices[] = { 
		// positions   // texCoords
		0.75f,  1.0f,  0.0f, 1.0f,	// right top
		0.75f, 0.75f,  0.0f, 0.0f,	//left bottom
		1.0f, 0.75f,  1.0f, 0.0f,	//right bottom

		0.75f,  1.0f,  0.0f, 1.0f,	// right top
		1.0f, 0.75f,  1.0f, 0.0f,	//right bottom
		1.0f,  1.0f,  1.0f, 1.0f	//left top
	};

	uint32_t quadVAO, quadVBO;
	void initScreenQuad()
	{
		glCreateVertexArrays(1, &quadVAO);
		glCreateBuffers(1, &quadVBO);
		glNamedBufferData(quadVBO, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, sizeof(float) * 4);

		glEnableVertexArrayAttrib(quadVAO, 0);
		glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(quadVAO, 0, 0);

		glEnableVertexArrayAttrib(quadVAO, 1);
		glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
		glVertexArrayAttribBinding(quadVAO, 1, 0);

	}

	uint32_t objectIdVAO, objectIdVBO;
	void initObjectIdQuad()
	{
		glCreateVertexArrays(1, &objectIdVAO);
		glCreateBuffers(1, &objectIdVBO);
		glNamedBufferData(objectIdVBO, sizeof(quadVertices), &objectIdQuadVertices, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(objectIdVAO, 0, objectIdVBO, 0, sizeof(float) * 4);

		glEnableVertexArrayAttrib(objectIdVAO, 0);
		glVertexArrayAttribFormat(objectIdVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(objectIdVAO, 0, 0);

		glEnableVertexArrayAttrib(objectIdVAO, 1);
		glVertexArrayAttribFormat(objectIdVAO, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2);
		glVertexArrayAttribBinding(objectIdVAO, 1, 0);

	}
	uint32_t fbo, textureColorBufferMultiSampled, rbo, intermediateFBO, screenTexture;
	void initFrameBufferAndRenderTarget()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &textureColorBufferMultiSampled);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		// configure second post-processing framebuffer
		glGenFramebuffers(1, &intermediateFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
		// create a color attachment texture
		glGenTextures(1, &screenTexture);
		glBindTexture(GL_TEXTURE_2D, screenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
		

	}

	void framebufferSizeCallback(GLFWwindow* window, int32_t newWINDOW_WIDTH, int32_t newWINDOW_HEIGHT)
	{
		WINDOW_WIDTH = newWINDOW_WIDTH;
		WINDOW_HEIGHT = newWINDOW_HEIGHT;
		
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);

		glBindTexture(GL_TEXTURE_2D, screenTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

		glBindTexture(GL_TEXTURE_2D, pickingTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);
		
	}

	std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\multipleLightsSurface.frag").string();
	std::string vShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\vertex.vert").string();

	std::string fScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.frag").string();
	std::string fRadialBackground = std::filesystem::absolute("..\\..\\src\\shaders\\frameRadialBackground.frag").string();

	std::string vScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.vert").string();

	std::string vPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.vert").string();
	std::string fPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.frag").string();

	std::string vSkyboxShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\cubemap.vert").string();
	std::string fSkyboxShader = std::filesystem::absolute("..\\..\\src\\shaders\\\\cubemap\\cubemap.frag").string();

	std::string simpleDepthShader = std::filesystem::absolute("..\\..\\src\\shaders\\simpleDepthShader.vert").string();
	std::string fEmptyShader = std::filesystem::absolute("..\\..\\src\\shaders\\empty.frag").string();

	glm::vec3 lightColor = glm::vec3(1.0f);
	glm::mat4 lightmodel = glm::mat4(1.0f);

	float lightIntensity = 1.0f;
	glm::mat4 model = glm::mat4(1.0f);
	GLenum polygonMode = GL_FILL;
	bool isWireframe = false;

	float lastFrame = 0;
	float deltaTime = 0;

	float gamma = 1;
	bool openFileDialog = false;

	float near_plane = 4.0f, far_plane = 0.0001f;

	void draw(GLFWwindow* window)
	{
		Shader baseShader (vShaderPath, fShaderPath);
		Shader screenShader(vScreenShader,fScreenShader);
		Shader radialGradientShader(vScreenShader,fRadialBackground);
		Shader pickingShader(vPickingShader,fPickingShader);
		Shader skyboxShader(vSkyboxShader,fSkyboxShader);
		Shader depthShader(simpleDepthShader,fEmptyShader);

		uint32_t cubemapTexture = loadCubemap();
		initScreenQuad();
		initObjectIdQuad();
		initFrameBufferAndRenderTarget();
		initPickingFBO(WINDOW_WIDTH, WINDOW_HEIGHT);
		initCubemap();
		initDepthMap(SHADOW_WIDTH, SHADOW_HEIGHT);

		//import
		GLTFModel gltfTent1(std::filesystem::absolute("..\\..\\res\\GltfModels\\BarDiorama.glb").string(), baseShader);
		SceneManager::addPrimitives(gltfTent1.primitives);

		//imgui
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");

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
		directionalLight.position = glm::vec3(-2.0f, 4.0f, -1.0f);
		directionalLight.direction = glm::vec3( 0.0f, -1.0f,  0.0f);
		directionalLight.ambient   = glm::vec3(0.05f);
		directionalLight.diffuse   = glm::vec3(0.8f);
		directionalLight.specular  = glm::vec3(1.0f);
		directionalLight.constant  = 1.0f;
		directionalLight.linear    = 0.09f;
		directionalLight.quadratic = 0.032f;

		SceneManager::addLight(pointLight);
		SceneManager::addLight(directionalLight);
		SceneManager::addLight(spotLight);
		UpdateLights(SceneManager::lights);
		

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
		screenShader.use();
		screenShader.setInt("screenTexture", 0);

		while(!glfwWindowShouldClose(window))
		{   
			glBindTexture(GL_TEXTURE_2D, 0);
			float time = (float)glfwGetTime();
			
			deltaTime = time - lastFrame;
			lastFrame = time;
			processInput(window, isWireframe, deltaTime);
			
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

			//imgui
			{
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::Begin("Tools");
				ImGui::ColorEdit4("BG Color", clearColor);
				glm::vec3 lightPos = glm::vec3(lightmodel[3]);
				if(SceneManager::selectedPrimitive != nullptr)
				{
					ImGui::Begin("Object Inspector");
					ImGui::DragFloat3("Position", glm::value_ptr(SceneManager::selectedPrimitive->transform[3]));
					ImGui::Image(SceneManager::selectedPrimitive->material.diffuse -> id, ImVec2(64, 64));
					ImGui::SameLine();
					ImGui::Image(SceneManager::selectedPrimitive->material.specular -> id, ImVec2(64, 64));
					if (ImGui::Button("Diffuse"))
					{
						std::string filePath = OpenFileDialog();
						if (!filePath.empty())
						{
							// Update the object's texture path
							SceneManager::selectedPrimitive->material.diffuse -> type = "tDiffuse";
							SceneManager::selectedPrimitive->material.diffuse -> SetPath(filePath);
							glActiveTexture(GL_TEXTURE0);

						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Specular"))
					{
						std::string filePath = OpenFileDialog();
						if (!filePath.empty())
						{
							// Update the object's texture path
							SceneManager::selectedPrimitive->material.specular -> type = "tSpecular";
							SceneManager::selectedPrimitive->material.specular -> SetPath(filePath);
							glActiveTexture(GL_TEXTURE0);

						}
					}
					ImGui::End();
				}
				ImGui::SliderFloat("FOV",&camera.zoom,1.f,100.f,"%.3f");
				ImGui::SliderFloat("Gamma", &gamma,0.01f,5);
				ImGui::SliderFloat("Near plane", &near_plane,0.00001f,30.f, "%.6f");
				ImGui::SliderFloat("Far plane", &far_plane,0.00001f,100.f);
				ImGui::Checkbox("Wireframe Mode", &isWireframe);
				ImGui::Checkbox("ObjectID Debug", &showObjectPicking);
				ImGui::Checkbox("ShadowMap Debug", &showShadowMap);
				if (ImGui::Button("Import Model"))
				{	
					std::string filePath = OpenFileDialog();
					if(!filePath.empty())
					{
						GLTFModel model(filePath, baseShader);
						model.setTransform(glm::translate(glm::mat4(1),glm::vec3(0,1,0)));
						SceneManager::addPrimitives(model.primitives);
					}
				}
				polygonMode = isWireframe ? GL_LINE : GL_FILL;
				if (ImGui::Button("Reload Shaders")) 
				{
					UpdateLights(SceneManager::lights);
					SceneManager::reloadShaders();
					screenShader.reload();
					skyboxShader.reload();
					radialGradientShader.reload();
					std::cout << "Shaders reloaded successfully!" << std::endl;
				}
				ImGui::End();
				
				//fps counter
				ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
				ImGui::SetNextWindowBgAlpha(0.3f);
				ImGui::Begin("FPS", nullptr,
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_NoSavedSettings);
				ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
				ImGui::Text("ms: %.4f", deltaTime);
				ImGui::End();

				ImGui::SetNextWindowPos(ImVec2(100, 10), ImGuiCond_Always);
				ImGui::SetNextWindowBgAlpha(0.3f);
				ImGui::Begin("Camera", nullptr,
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_NoSavedSettings);
				ImGui::Text("Pos: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z); ImGui::SameLine();
				ImGui::Text("Rot: %.1f, %.1f", camera.yaw, camera.pitch);
				ImGui::End();

				ImGui::Begin("Lights");
				for (int i = 0; i < SceneManager::lights.size(); i++)
				{
					ImGui::PushID(i);
					ImGui::Text("Light: %d %s ", i, (SceneManager::lights[i].type == 1) ? "directional" : (SceneManager::lights[i].type == 2) ? "spot" : "point");
					ImGui::DragFloat3("Position", glm::value_ptr(SceneManager::lights[i].position));
					ImGui::SliderFloat("Intensity", &SceneManager::lights[i].intensity, 0.0f, 10.0f);
					if (SceneManager::lights[i].type == 1 || SceneManager::lights[i].type == 2)
					{
						static glm::vec3 lightRotation = glm::vec3(0.0f); 
						if (ImGui::DragFloat3("Light Rotation", glm::value_ptr(lightRotation), 0.1f)) 
						{
							float pitch = glm::radians(lightRotation.x);
							float yaw   = glm::radians(lightRotation.y);
							float roll  = glm::radians(lightRotation.z);
							glm::vec3 baseDirection(0.0f, -1.0f, 0.0f);
							glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
							glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
							glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), roll,  glm::vec3(0.0f, 0.0f, 1.0f));
							glm::mat4 rotationMatrix = rotZ * rotY * rotX;
							SceneManager::lights[i].direction = glm::vec3(rotationMatrix * glm::vec4(baseDirection, 0.0f));
						}
					}
					if (SceneManager::lights[i].type == 2)
					{
						float cutoffAngle = glm::degrees(glm::acos(SceneManager::lights[i].cutOff));
						float outerCutoffAngle = glm::degrees(glm::acos(SceneManager::lights[i].outerCutOff));
						if (ImGui::SliderFloat("CutOff Angle", &cutoffAngle, 0.0f, 180.0f))
						{
							if (outerCutoffAngle<=cutoffAngle)
							{ 
								outerCutoffAngle = cutoffAngle+0.001f;
								SceneManager::lights[i].outerCutOff = glm::cos(glm::radians(outerCutoffAngle));
							}
							SceneManager::lights[i].cutOff = glm::cos(glm::radians(cutoffAngle));
						}

						
						if (ImGui::SliderFloat("Outer CutOff Angle", &outerCutoffAngle, cutoffAngle, 180))
						{
							if (outerCutoffAngle<=cutoffAngle) outerCutoffAngle = cutoffAngle+0.001f;
							SceneManager::lights[i].outerCutOff = glm::cos(glm::radians(outerCutoffAngle));
						}
					}
					ImGui::ColorEdit3("Color", glm::value_ptr(SceneManager::lights[i].diffuse));
					UpdateLights(SceneManager::lights);
					ImGui::PopID();
				}
				ImGui::End();
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


			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			

			glm::vec3 sceneCenter = glm::vec3(0.0f); 
			float distance = 3.0f;

			glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); 
			
			glm::vec3 lightDirection = glm::normalize(SceneManager::lights[1].direction);

			glm::vec3 lightPos = sceneCenter - lightDirection * distance;

			glm::mat4 lightView = glm::lookAt(lightPos, lightDirection, glm::vec3(0.0, 1.0, 0.0));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;
			SceneManager::setShader(depthShader);
			SceneManager::draw(camera, lightSpaceMatrix, WINDOW_WIDTH, WINDOW_HEIGHT);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
			glClearColor(0.3f,0.3f,0.3f,1.0f); 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glfwGetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
			if( WINDOW_WIDTH != 0 && WINDOW_HEIGHT != 0) 
			{
				projection = glm::perspective(glm::radians(camera.zoom), float(WINDOW_WIDTH)/float(WINDOW_HEIGHT),0.1f, 100.0f);	
				view = camera.getViewMatrix();
			}

			SceneManager::setShader(pickingShader);
			SceneManager::draw(camera, lightSpaceMatrix, WINDOW_WIDTH, WINDOW_HEIGHT);

			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]); 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glPolygonMode(GL_FRONT_AND_BACK,polygonMode);

			SceneManager::setShader(baseShader);
			SceneManager::draw(camera, lightSpaceMatrix, WINDOW_WIDTH, WINDOW_HEIGHT, depthMap, gamma);
			glDepthFunc(GL_LEQUAL);
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			glm::mat4 skyView = glm::mat4(glm::mat3(camera.getViewMatrix()));  
			drawCubemap(cubemapTexture, skyboxShader, projection, skyView);
			glBindTextureUnit(5, 0);

			
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
			glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]); 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			
			
			screenShader.use();  
			screenShader.setFloat("near_plane", near_plane);
			screenShader.setFloat("far_plane", far_plane);
			glBindVertexArray(quadVAO);
			glBindTexture(GL_TEXTURE_2D, screenTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);  


			if(showObjectPicking)
			{
				glBindVertexArray(objectIdVAO);
				glBindTexture(GL_TEXTURE_2D, pickingTexture);
				glDrawArrays(GL_TRIANGLES, 0, 6);  
			} 
			if(showShadowMap)
			{
				glBindVertexArray(objectIdVAO);
				glBindTexture(GL_TEXTURE_2D, depthMap);
				glDrawArrays(GL_TRIANGLES, 0, 6);  
			} 
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(window);
			glfwPollEvents();    
		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	int main()	
	{
		GLFWwindow* window;
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		// glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
		glfwWindowHint(GLFW_SAMPLES, 16);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); 
		
		window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Main Window", NULL, NULL);
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
