#include <windows.h>
#include <commdlg.h> 
#include <iostream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"

#include "sceneManager.hpp"
#include "gltfImporter.hpp"
#include "appConfig.hpp"
#include "uiManager.hpp"


UIManager::UIManager(GLFWwindow* window, float deltaTime, Camera& camera) : 
deltaTime(deltaTime),
camera(camera),
window(window)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    io.DisplaySize = ImVec2((float)width, (float)height);

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

UIManager::~UIManager() 
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

};

void UIManager::draw()
{
	ImGuiIO& io = ImGui::GetIO();
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	io.DisplaySize = ImVec2((float)width, (float)height);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	showFps();
	showCameraTransforms();
	showObjectInspector();
	showLights();
	showTools();
	showMaterialBrowser();

	ImGui::Render();
	auto draw_data = ImGui::GetDrawData();
	if (draw_data != nullptr)
	{
		ImGui_ImplOpenGL3_RenderDrawData(draw_data);
	}
}

void UIManager::showFps()
{
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

void UIManager::showCameraTransforms()
{
	ImGui::SetNextWindowPos(ImVec2(100, 10), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.3f);
	ImGui::Begin("Camera", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Pos: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z); ImGui::SameLine();
	ImGui::Text("Rot: %.1f, %.1f", camera.pitch, camera.yaw);
	ImGui::Text("Front: %.1f, %.1f, %.1f", camera.front.x, camera.front.y, camera.front.z); ImGui::SameLine();
	ImGui::Text("Up: %.1f, %.1f, %.1f", camera.up.x, camera.up.y, camera.up.z);
	ImGui::Text("Right: %.1f, %.1f, %.1f", camera.right.x, camera.right.y, camera.right.z);
	ImGui::Text("Default Pos: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[0].x, camera.defaultCameraMatrix[0].y, camera.defaultCameraMatrix[0].z);  ImGui::SameLine();
	ImGui::Text("Default Rot: %.1f, %.1f", camera.defaultCameraRotation[0], camera.defaultCameraRotation[1]);
	ImGui::Text("Default Front: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[1].x, camera.defaultCameraMatrix[1].y, camera.defaultCameraMatrix[1].z);ImGui::SameLine();
	ImGui::Text("Default Up: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[2].x, camera.defaultCameraMatrix[2].y, camera.defaultCameraMatrix[2].z);
	ImGui::End();
}

void UIManager::showLights()
{
	ImGui::Begin("Lights");
		for (int i = 0; i < SceneManager::getLights().size(); i++)
		{
			ImGui::PushID(i);
			ImGui::Text("Light: %d %s ", i, (SceneManager::getLights()[i].type == 1) ? "directional" : (SceneManager::getLights()[i].type == 2) ? "spot" : "point");
			ImGui::DragFloat3("Position", glm::value_ptr(SceneManager::getLights()[i].position));
			ImGui::SliderFloat("Intensity", &SceneManager::getLights()[i].intensity, 0.0f, 10.0f);
			
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
				SceneManager::getLights()[i].direction = glm::vec3(rotationMatrix * glm::vec4(baseDirection, 0.0f));
			}
			if (SceneManager::getLights()[i].type == 2)
			{
				float cutoffAngle = glm::degrees(glm::acos(SceneManager::getLights()[i].cutOff));
				float outerCutoffAngle = glm::degrees(glm::acos(SceneManager::getLights()[i].outerCutOff));
				if (ImGui::SliderFloat("CutOff Angle", &cutoffAngle, 0.0f, 180.0f))
				{
					if (outerCutoffAngle<=cutoffAngle)
					{ 
						outerCutoffAngle = cutoffAngle+0.001f;
						SceneManager::getLights()[i].outerCutOff = glm::cos(glm::radians(outerCutoffAngle));
					}
					SceneManager::getLights()[i].cutOff = glm::cos(glm::radians(cutoffAngle));
				}

				
				if (ImGui::SliderFloat("Outer CutOff Angle", &outerCutoffAngle, cutoffAngle, 180))
				{
					if (outerCutoffAngle<=cutoffAngle) outerCutoffAngle = cutoffAngle+0.001f;
					SceneManager::getLights()[i].outerCutOff = glm::cos(glm::radians(outerCutoffAngle));
				}
			}
			ImGui::ColorEdit3("Color", glm::value_ptr(SceneManager::getLights()[i].diffuse));
			ImGui::PopID();
			SceneManager::updateLights();
		}
	ImGui::End();
}

void UIManager::showObjectInspector()
{
	if(SceneManager::getSelectedPrimitive() != nullptr)
	{
		ImGui::Begin("Object Inspector");
		ImGui::DragFloat3("Position", glm::value_ptr(SceneManager::getSelectedPrimitive()->transform[3]));
		if (SceneManager::getSelectedPrimitive()->material->diffuse != nullptr)
		ImGui::Image(SceneManager::getSelectedPrimitive()->material->diffuse->id, ImVec2(64, 64));
		ImGui::SameLine();
		if (SceneManager::getSelectedPrimitive()->material->diffuse != nullptr)
		ImGui::Image(SceneManager::getSelectedPrimitive()->material->specular->id, ImVec2(64, 64));
		if (ImGui::Button("Diffuse"))
		{
			std::string filePath = OpenFileDialog();
			if (!filePath.empty())
			{
				// Update the object's texture path
				SceneManager::getSelectedPrimitive()->material->diffuse->SetPath(filePath);
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
				SceneManager::getSelectedPrimitive()->material->specular->SetPath(filePath);
				glActiveTexture(GL_TEXTURE0);

			}
		}
		ImGui::End();
	}
}

void UIManager::showTools()
{
	ImGui::Begin("Tools");
		ImGui::ColorEdit4("BG Color", AppConfig::clearColor);
		
		ImGui::SliderFloat("FOV",&camera.zoom,1.f,100.f,"%.3f");
		ImGui::SliderFloat("Gamma", &AppConfig::gamma,0.01f,5);
		ImGui::SliderFloat("Near plane", &AppConfig::near_plane,-10.0f,10.f, "%.6f");
		ImGui::SliderFloat("Far plane", &AppConfig::far_plane,-10.0f,10.f);
		ImGui::Checkbox("Wireframe Mode", &AppConfig::isWireframe);
		ImGui::Checkbox("ObjectID Debug", &AppConfig::showObjectPicking);
		ImGui::Checkbox("ShadowMap Debug", &AppConfig::showShadowMap);
		if (ImGui::Button("Import Model"))
		{	
			std::string filePath = OpenFileDialog();
			if(!filePath.empty())
			{
				GLTFModel model(filePath, AppConfig::baseShader);
				model.setTransform(glm::translate(glm::mat4(1),glm::vec3(0,1,0)));
				SceneManager::addPrimitives(std::move(model.primitives));
			}
		}
		AppConfig::polygonMode = AppConfig::isWireframe ? GL_LINE : GL_FILL;
		if (ImGui::Button("Reload Shaders")) 
		{
			SceneManager::reloadShaders();
			AppConfig::screenShader.reload();
			AppConfig::skyboxShader.reload();
			std::cout << "Shaders reloaded successfully!" << std::endl;
		}
	ImGui::End();
}

std::string UIManager::OpenFileDialog()
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


void UIManager::showMaterialBrowser()
{
	ImGui::Begin("Material Browser");
		static int columns = 2;
		ImGui::SliderInt("Columns", &columns, 1, 6);
		
		// Calculate item width based on window width and columns
		ImGui::GetWindowContentRegionMax();
		float windowWidth = ImGui::GetWindowWidth();
		float cellWidth = (windowWidth / columns) - ImGui::GetStyle().ItemSpacing.x;
		float imageSize = std::min(64.0f, cellWidth * 0.4f); // Adjust image size to fit cell
		if (ImGui::BeginTable("MaterialGrid", columns, 
			ImGuiTableFlags_ScrollY | 
			ImGuiTableFlags_Borders | 
			ImGuiTableFlags_SizingStretchProp))
		{
			auto materials = SceneManager::getMaterials();
			int itemIdx = 0;
	
			for (const auto& uidAndMat : materials)
			{
				uint32_t uid = uidAndMat.first;
				std::shared_ptr<Mat> mat = uidAndMat.second;
				
				if (itemIdx % columns == 0)
					ImGui::TableNextRow();
				
				ImGui::TableNextColumn();
				
				// Create a group for each material
				ImGui::BeginGroup();
				{
					ImGui::PushID(uid);
					ImGui::TextWrapped("Material: %s", mat->name.c_str());
					ImVec2 imagePos = ImGui::GetCursorPos();

					if (mat->diffuse)
					{
						ImGui::Image(mat->diffuse->id, ImVec2(imageSize, imageSize));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Diffuse");
					}
					ImGui::SameLine();
					if (mat->specular)
					{
						ImGui::Image(mat->specular->id, ImVec2(imageSize, imageSize));
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Specular");
					}
					if (ImGui::Button("Diffuse", ImVec2(imageSize, 0)))
					{
						std::string filePath = OpenFileDialog();
						if (!filePath.empty())
						{
							mat->diffuse->SetPath(filePath);
							glActiveTexture(GL_TEXTURE0);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Specular", ImVec2(imageSize, 0)))
					{
						std::string filePath = OpenFileDialog();
						if (!filePath.empty())
						{
							mat->specular->SetPath(filePath);
							glActiveTexture(GL_TEXTURE0);
						}
					}
					ImGui::PopID();
				}
				ImGui::EndGroup();
				
				itemIdx++;
			}
			ImGui::EndTable();
		}
		
		ImGui::End();
}

bool UIManager::wantCaptureInput() const
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse || io.WantCaptureKeyboard;
}
