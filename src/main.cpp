#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "stb_image.h"
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_internal.h>
#include <iomanip>
#include "camera.h"
#include "objReader.h"


int32_t width = 800;
int32_t height = 800;
Camera camera(glm::vec3(0.0f,0.0f,3.0f));
bool wireframeKeyPressed = false;
bool rightKeyPressed = false;
float increasedSpeed = camera.speed *3;
float defaultSpeed = camera.speed;
glm::mat4 defaultMatrix = camera.getViewMatrix();
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
		if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(FORWARD,deltaTime);
		if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(BACKWARD,deltaTime);
		if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(LEFT,deltaTime);
		if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(RIGHT,deltaTime);
		if(glfwGetKey(window,GLFW_KEY_Q) == GLFW_PRESS) camera.processKeyboard(DOWN,deltaTime);
		if(glfwGetKey(window,GLFW_KEY_E) == GLFW_PRESS) camera.processKeyboard(UP,deltaTime);
	}else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
	{
		if (rightKeyPressed) glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
		rightKeyPressed = false;

	} 

	if (glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.speed = increasedSpeed;
	else camera.speed = defaultSpeed;
	//if (glfwGetKey(window,GLFW_KEY_F) == GLFW_PRESS) defaultMatrix


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

void loadTexture(uint32_t* texture, const char* path)
{
		int32_t iWidth, iHeight, nrChannels;
	glGenTextures(1,texture);
	glBindTexture(GL_TEXTURE_2D,*texture);
	//wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data  = stbi_load(path, &iWidth, &iHeight, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Faild to Load Texture: "<< path << std::endl;
	}
	stbi_image_free(data);

}

void framebufferSizeCallback(GLFWwindow* window, int32_t newWidth, int32_t newHeight)
{
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
}


void draw(GLFWwindow* window)
{
	ObjReader objReader;
	//std::string path =  "C:\\Users\\Efim\\Desktop\\123.obj"; 
	std::string path =  "C:\\Users\\Efim\\Desktop\\junkyard\\OpenGLLearn\\res\\Ufo_C.obj";
	objReader.readObj(path);

	glEnable(GL_DEPTH_TEST);
	std::string fShaderPath = "C:\\Users\\Efim\\Desktop\\junkyard\\OpenGLLearn\\src\\shaders\\frag.glsl";
	std::string vShaderPath = "C:\\Users\\Efim\\Desktop\\junkyard\\OpenGLLearn\\src\\shaders\\vertex.glsl";
	Shader myShader (vShaderPath,fShaderPath);
	Shader myShader1 (vShaderPath,fShaderPath);


	uint32_t VAO, VBO, UFOVAO, UFOVBO;
	glGenVertexArrays(1, &VAO); //generates vertex array objects
	glGenBuffers(1, &VBO); //generates vertex buffer objects

	glBindVertexArray(VAO); //bind
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*4, vertices.data(), GL_STATIC_DRAW);	

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	//texCoord attribute
	// glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	// glEnableVertexAttribArray(1);
	
	
	glGenVertexArrays(1,&UFOVAO);
	glGenBuffers(1,&UFOVBO);
	glBindVertexArray(UFOVAO);
	glBindBuffer(GL_ARRAY_BUFFER,UFOVBO);
	glBufferData(GL_ARRAY_BUFFER,objReader.finalData.size()*4,objReader.finalData.data(),GL_STATIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);



	uint32_t texture1, texture2;
	loadTexture(&texture1,"../res/textures/container.jpg");
	loadTexture(&texture2,"../res/textures/deloSdelano.png");


	
	//imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	uint32_t* activeTexture = &texture1;
	GLenum polygonMode = GL_FILL;
	static bool isWireframe = false;
	static int selectedTexture = 0;


	float lastFrame = 0;
	float deltaTime = 0;
	glm::mat4 view = 		glm::mat4(1.0f);
	glm::mat4 projection = 	glm::mat4(1.0f);
	glm::mat4 model =  		glm::mat4(1.0f);
	while(!glfwWindowShouldClose(window))
	{   
		float time = (float)glfwGetTime();
		deltaTime = time - lastFrame;
		lastFrame = time;
		processInput(window,&isWireframe,deltaTime);
		//imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Tools");
		//ImGui::ColorEdit4("BG Color", clearColor);
		ImGui::SliderFloat("FOV",&camera.zoom,1.f,100.f,"%.3f");
		if(ImGui::Combo("Chose Active texture",&selectedTexture,"Texture 1\0Texture 2\0"))
		{		

			if (selectedTexture == 0) activeTexture = &texture1;
			if (selectedTexture == 1) activeTexture = &texture2;
		}
		ImGui::Checkbox("Wireframe Mode",&isWireframe);
		polygonMode = isWireframe ? GL_LINE : GL_FILL;
		glPolygonMode(GL_FRONT_AND_BACK,polygonMode);
		ImGui::End();

		glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *activeTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);



		glfwGetWindowSize(window,&width, &height);

		projection = glm::perspective(glm::radians(camera.zoom), float(width)/float(height),0.1f, 100.0f);	
		view = camera.getViewMatrix();

		myShader.use();
		glBindVertexArray(VAO);
		for(uint32_t i = 0; i < 10; i++)
		{			
			model =  		glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i; 
			model = glm::rotate(model, glm::radians(angle)*time, glm::vec3(1.0f, 0.3f, 0.5f));
			myShader.setMat4("projection",projection);
			myShader.setMat4("view",view);			
			myShader.setMat4("model",model);
			glDrawArrays(GL_TRIANGLES,0,36);
		}


		glBindVertexArray(UFOVAO);
		myShader1.use();
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.3,0.3,0.3));
		myShader1.setMat4("projection",projection);
		myShader1.setMat4("view",view);
		myShader1.setMat4("model",model);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(objReader.finalData.size() / 8));

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();    
	}


	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

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
