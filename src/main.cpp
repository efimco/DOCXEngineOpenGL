#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "objectManager.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_internal.h>

#include <filesystem>


int32_t width = 800;
int32_t height = 800;
Camera camera(glm::vec3(0.0f,0.0f,3.0f));
bool wireframeKeyPressed = false;
bool rightKeyPressed = false;
float increasedSpeed = camera.speed *3;
float defaultSpeed = camera.speed;
std::vector<glm::vec3> defaultCameraMatrix = {camera.position,camera.front,camera.up};
float defaultCameraRotation[] = {camera.pitch, camera.yaw};
bool cameraReseted = true;

void processInput(GLFWwindow* window,bool* isWireframe, float deltaTime)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window,true);

	if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		if (!wireframeKeyPressed)
		{
			wireframeKeyPressed = true;
			if(*isWireframe) *isWireframe = false;
			else *isWireframe =  true;
		}
	}
	else if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) wireframeKeyPressed = false;


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
	}else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
	{
		if (rightKeyPressed) glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
		rightKeyPressed = false;

	} 

	if (glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.speed = increasedSpeed;
	else camera.speed = defaultSpeed;

	if (glfwGetKey(window,GLFW_KEY_F) == GLFW_PRESS) cameraReseted = false;

}

float lastX = (float)(width / 2), lastY = (float) (height / 2);
bool firstMouse = true;
float clearColor[4] = { 0.133f, 0.192f, 0.265f, 1.0f };

void mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{	lastX = (float)xPos;
		lastY = (float)yPos;
		firstMouse = false;
	}	

	glm::mat4 view = 		glm::mat4(1.0f);
	glm::mat4 projection = 	glm::mat4(1.0f);

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

std::vector<float> vertices = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f), 
    glm::vec3( 2.0f,  5.0f, -15.0f), 
    glm::vec3(-1.5f, -2.2f, -2.5f),  
    glm::vec3(-3.8f, -2.0f, -12.3f),  
    glm::vec3( 2.4f, -0.4f, -3.5f),  
    glm::vec3(-1.7f,  3.0f, -7.5f),  
    glm::vec3( 1.3f, -2.0f, -2.5f),  
    glm::vec3( 1.5f,  2.0f, -2.5f), 
    glm::vec3( 1.5f,  0.2f, -1.5f), 
    glm::vec3(-1.3f,  1.0f, -1.5f), 
	
};


void framebufferSizeCallback(GLFWwindow* window, int32_t newWidth, int32_t newHeight)
{
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
}


void draw(GLFWwindow* window)
{
    std::string path = "..\\..\\res\\123_smooth.obj";
	std::string tentPath = "..\\..\\res\\textures\\SciFiTent\\Tent.obj";
	std::string boxPath = "..\\..\\res\\box.obj";

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS );
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00);
	std::string fShaderPath = "..\\..\\src\\shaders\\frag.glsl";
	std::string vShaderPath = "..\\..\\src\\shaders\\vertex.glsl";

	std::string fLightShaderPath = "..\\..\\src\\shaders\\fragLight.glsl";
	std::string vLightShaderPath = "..\\..\\src\\shaders\\vertLight.glsl";

	Shader cubeShader (vShaderPath, fShaderPath);
	Shader tentShader (vShaderPath, fShaderPath);
	Shader lightShader (vLightShaderPath, fLightShaderPath);

	Model cube(path.c_str(),cubeShader);
	Model tent(tentPath.c_str(),cubeShader);
	Model lightCube(boxPath.c_str(),lightShader);
	
	GLTFModel gltfTent("..\\..\\res\\GltfModels\\SceneForRednerDemo.gltf",cubeShader);
	gltfTent.setTransform(glm::translate(glm::mat4(1),glm::vec3(0,0,3)));
	ObjectManager::addPrimitives(gltfTent.primitives);

	//imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	GLenum polygonMode = GL_FILL;
	static bool isWireframe = false;

	float lastFrame = 0;
	float deltaTime = 0;

	glm::mat4 view = 		glm::mat4(1.0f);
	glm::mat4 projection = 	glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);

	float lightIntensity = 1.0f;
	glm::vec3 lightColor = glm::vec3(1.0f);
	glm::mat4 lightmodel = glm::mat4(1.0f);

	lightmodel = glm::translate(lightmodel, glm::vec3(0.0f, 5.0f, 0.0f));
	lightmodel = glm::scale(lightmodel, glm::vec3(1.0f, 1.0f, 1.0f));
	cube.model = glm::scale(cube.model, glm::vec3(.1f));
	cube.model = glm::translate(cube.model, glm::vec3(3.0f,.0f,10));
	tent.model = glm::translate(tent.model, glm::vec3(0.0f,.0f,2));
	float gamma = 1;

	while(!glfwWindowShouldClose(window))
	{   
		float time = (float)glfwGetTime();
		deltaTime = time - lastFrame;
		lastFrame = time;
		processInput(window,&isWireframe,deltaTime);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Tools");
		ImGui::ColorEdit4("BG Color", clearColor);
		ImGui::ColorEdit3("LightColor", glm::value_ptr(lightColor));
		ImGui::DragFloat("Light intensity", &lightIntensity);
		glm::vec3 lightPos = glm::vec3(lightmodel[3]);
		ImGui::DragFloat3("LightPos", glm::value_ptr(lightPos));
		lightmodel[3] = glm::vec4(lightPos, 1.0f);
		ImGui::SliderFloat("FOV",&camera.zoom,1.f,100.f,"%.3f");
		ImGui::SliderFloat("Gamma", &gamma,0.01f,5);
		ImGui::Checkbox("Wireframe Mode", &isWireframe);
		polygonMode = isWireframe ? GL_LINE : GL_FILL;
		glPolygonMode(GL_FRONT_AND_BACK,polygonMode);

		if (ImGui::Button("Reload Shaders")) 
		{
            cube.shader.reload(cube.shader.vPath.c_str(), cube.shader.fPath.c_str());
            lightCube.shader.reload(lightCube.shader.vPath.c_str(), lightCube.shader.fPath.c_str());
			tent.shader.reload(vShaderPath.c_str(), fShaderPath.c_str());
            std::cout << "Shaders reloaded successfully!" << std::endl;
        }

		ImGui::End();

		glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

		glfwGetWindowSize(window,&width, &height);
		if( width != 0 && height != 0) 
		{
			projection = glm::perspective(glm::radians(camera.zoom), float(width)/float(height),0.1f, 100.0f);	
			view = camera.getViewMatrix();
		}


		lightCube.shader.use();
		lightCube.shader.setMat4("projection",projection);
		lightCube.shader.setMat4("view",view);
		lightCube.shader.setFloat("lightIntensity",lightIntensity);
		lightCube.shader.setVec3("lightColor",lightColor);
		lightCube.shader.setMat4("model",lightmodel);
		lightCube.draw();

		
		ObjectManager::draw(camera,width,height);


		cube.shader.use();
		cube.shader.setMat4("projection",projection);
		cube.shader.setMat4("view",view);
		cube.shader.setVec3("viewPos", camera.position);
		cube.shader.setVec3("light.position", lightmodel[3]);
		cube.shader.setVec3("light.diffuse", lightColor);
		cube.shader.setVec3("light.ambient", glm::vec3(0.05,0.1,0.2));
		cube.shader.setVec3("light.specular", glm::vec3(1));
		cube.shader.setFloat("light.intensity", lightIntensity);
		cube.shader.setFloat("material.shininess", 32);
		cube.shader.setFloat("gamma", gamma);
		cube.draw();

		tent.shader.use();
		tent.shader.setMat4("projection",projection);
		tent.shader.setMat4("view",view);
		tent.shader.setVec3("viewPos", camera.position);
		tent.shader.setVec3("light.position", lightmodel[3]);
		tent.shader.setVec3("light.diffuse", lightColor);
		tent.shader.setVec3("light.ambient", glm::vec3(0.05,0.1,0.2));
		tent.shader.setVec3("light.specular", glm::vec3(1));
		tent.shader.setFloat("light.intensity", lightIntensity);
		tent.shader.setFloat("material.shininess", 32);
		tent.shader.setFloat("gamma", gamma);
		tent.draw();



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
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); 
	
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
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
