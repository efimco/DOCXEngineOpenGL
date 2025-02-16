
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

	#include <ImGui/imgui.h>
	#include <ImGui/imgui_impl_glfw.h>
	#include <ImGui/imgui_impl_opengl3.h>
	#include <ImGui/imgui_internal.h>
	#include <filesystem> 


	int32_t WINDOW_WIDTH = 1024;
	int32_t WINDOW_HEIGHT = 1024;
	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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
			if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse) wasMousePressed = true;
			else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && wasMousePressed)
			{
				wasMousePressed = false;
				projection = glm::perspective(glm::radians(camera.zoom), float(WINDOW_WIDTH)/float(WINDOW_HEIGHT),0.1f, 100.0f);    
				view = camera.getViewMatrix();
				Primitive* primitive = PickObject(mousePosx, mousePosy, WINDOW_WIDTH, WINDOW_HEIGHT,projection,view,camera.position, SceneManager::primitives);
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
	uint32_t fbo,textureColorbuffer,rbo;
	void initFrameBufferAndRenderTarget()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);

		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	}

	void framebufferSizeCallback(GLFWwindow* window, int32_t newWINDOW_WIDTH, int32_t newWINDOW_HEIGHT)
	{
		WINDOW_WIDTH = newWINDOW_WIDTH;
		WINDOW_HEIGHT = newWINDOW_HEIGHT;
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\multipleLightsSurface.frag").string();
	std::string vShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\vertex.glsl").string();

	std::string fScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.frag").string();
	std::string fRadialBackground = std::filesystem::absolute("..\\..\\src\\shaders\\frameRadialBackground.frag").string();
	std::string vScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.vert").string();

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

	void draw(GLFWwindow* window)
	{
		Shader baseShader (vShaderPath, fShaderPath);
		Shader screenShader(vScreenShader,fScreenShader);
		Shader radialGradientShader(vScreenShader,fRadialBackground);

		initScreenQuad();
		initFrameBufferAndRenderTarget();

		//import
		GLTFModel gltfTent(std::filesystem::absolute("..\\..\\res\\GltfModels\\SceneForRednerDemo.gltf").string(), baseShader);
		gltfTent.setTransform(glm::translate(glm::mat4(1),glm::vec3(0,0,3)));

		GLTFModel gltfTent1(std::filesystem::absolute("..\\..\\res\\GltfModels\\SceneForRednerDemo.gltf").string(), baseShader);
		gltfTent1.setTransform(glm::translate(glm::mat4(1),glm::vec3(0,0,4)));

		SceneManager::addPrimitives(gltfTent.primitives);
		SceneManager::addPrimitives(gltfTent1.primitives);

		//imgui
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");

		CreateLightSSBO();

		{ // lights
			Light pointLight;
			pointLight.type = 0;
			pointLight.intensity = 1;
			pointLight.position = glm::vec3(1.0f, 0.5f, 2.0f);
			pointLight.ambient   = glm::vec3(0.05f);
			pointLight.diffuse   = glm::vec3(0.8f);
			pointLight.specular  = glm::vec3(1.0f);
			pointLight.constant  = 1.0f;
			pointLight.linear    = 0.09f;
			pointLight.quadratic = 0.032f;
			pointLight.cutOff    = glm::cos(glm::radians(12.5f));
			pointLight.outerCutOff = glm::cos(glm::radians(17.5f));

			Light pointLight2;
			pointLight2.type = 0;
			pointLight2.intensity = 0.1f;
			pointLight2.position = glm::vec3(1.0f, 0.5f, 5.0f);
			pointLight2.ambient   = glm::vec3(0.05f);
			pointLight2.diffuse   = glm::vec3(0.8f);
			pointLight2.specular  = glm::vec3(1.0f);
			pointLight2.constant  = 1.0f;
			pointLight2.linear    = 0.09f;
			pointLight2.quadratic = 0.032f;
			pointLight2.cutOff    = glm::cos(glm::radians(12.5f));
			pointLight.outerCutOff = glm::cos(glm::radians(17.5f));

			SceneManager::addLight(pointLight);
			SceneManager::addLight(pointLight2);
			UpdateLights(SceneManager::lights);
		}

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
					ImGui::Image(SceneManager::selectedPrimitive->material.diffuse.id, ImVec2(64, 64));
					ImGui::SameLine();
					ImGui::Image(SceneManager::selectedPrimitive->material.specular.id, ImVec2(64, 64));
					if (ImGui::Button("Diffuse"))
					{
						std::string filePath = OpenFileDialog();
						if (!filePath.empty())
						{
							// Update the object's texture path
							SceneManager::selectedPrimitive->material.diffuse.type = "tDiffuse";
							SceneManager::selectedPrimitive->material.diffuse.SetPath(filePath);
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
							SceneManager::selectedPrimitive->material.specular.type = "tSpecular";
							SceneManager::selectedPrimitive->material.specular.SetPath(filePath);
							glActiveTexture(GL_TEXTURE0);

						}
					}
					ImGui::End();
				}
				ImGui::SliderFloat("FOV",&camera.zoom,1.f,100.f,"%.3f");
				ImGui::SliderFloat("Gamma", &gamma,0.01f,5);
				ImGui::Checkbox("Wireframe Mode", &isWireframe);
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
			}

			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]); 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_DEPTH_TEST);
			radialGradientShader.use();
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glPolygonMode(GL_FRONT_AND_BACK,polygonMode);

			glEnable(GL_DEPTH_TEST);
			glEnable(GL_MULTISAMPLE);
			glDepthFunc(GL_LESS);
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 
			glStencilMask(0x00);


			glfwGetWindowSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
			if( WINDOW_WIDTH != 0 && WINDOW_HEIGHT != 0) 
			{
				projection = glm::perspective(glm::radians(camera.zoom), float(WINDOW_WIDTH)/float(WINDOW_HEIGHT),0.1f, 100.0f);	
				view = camera.getViewMatrix();
			}

			SceneManager::draw(camera,WINDOW_WIDTH,WINDOW_HEIGHT);
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]); 
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
			screenShader.use();  
			glBindVertexArray(quadVAO);
			glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
			glDrawArrays(GL_TRIANGLES, 0, 6);  
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
