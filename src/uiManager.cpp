#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <iostream>
#include <set>


#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"
#include <ImGui/imgui_internal.h>

#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"


#include "appConfig.hpp"
#include "gltfImporter.hpp"
#include "sceneManager.hpp"
#include "uiManager.hpp"


UIManager::UIManager(GLFWwindow* window, Camera& camera, SceneNode* scene) : camera(camera), window(window), scene(scene)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
	io.DisplaySize = ImVec2((float)width, (float)height);

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	m_viewportState.mouseWheel = 0;

	if (!std::filesystem::exists("imgui.ini"))
	{
		ImGui::LoadIniSettingsFromDisk("resources/default_imgui.ini");
	}
}

UIManager::~UIManager()
{
	// Only shutdown if context exists
	if (ImGui::GetCurrentContext() != nullptr)
	{
		// Make sure we have the correct OpenGL context
		if (window)
		{
			glfwMakeContextCurrent(window);
		}
		// Shutdown in reverse order of initialization
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext();
	}
};

void UIManager::draw(float deltaTime)
{
	ImGuiIO& io = ImGui::GetIO();
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	io.DisplaySize = ImVec2((float)width, (float)height);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("InvisibleDockSpaceWindow", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// 3) DockSpace call — this is where other windows will dock
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	showViewport(deltaTime);
	showObjectInspector();
	showLights();
	showTools();
	showMaterialBrowser();
	showOutliner();
	// showCameraTransforms();

	getScroll();
	getViewportPos();
	getCursorPos();

	ImGui::Render();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	auto draw_data = ImGui::GetDrawData();
	if (draw_data != nullptr)
	{
		ImGui_ImplOpenGL3_RenderDrawData(draw_data);
	}
}

void UIManager::getScroll()
{
	float yOffset = ImGui::GetIO().MouseWheel;
	m_viewportState.mouseWheel = yOffset;
}

void UIManager::getCursorPos()
{
	ImVec2 mousePos = ImGui::GetIO().MousePos;
	ImVec2 viewportPos = m_viewportState.position;
	int readX = static_cast<int>(mousePos.x - viewportPos.x);
	// need to flip y axis cuz imgui makes it top left, but opengl uses bottom left
	int readY = static_cast<int>(AppConfig::RENDER_HEIGHT - (mousePos.y - viewportPos.y));
	ImVec2 cursorPos(static_cast<float>(readX), static_cast<float>(readY));
	m_viewportState.cursorPos = cursorPos;
}

void UIManager::getViewportPos() { m_viewportState.position = m_viewportPos; }

ViewportState UIManager::getViewportState() { return m_viewportState; }

void UIManager::showViewport(float deltaTime)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
	ImGui::PopStyleVar();

	ImTextureID texID = (ImTextureID)m_screenTexture;

	// flip UVs if your texture appears upside-down
	ImVec2 uv0 = ImVec2(0, 1);
	ImVec2 uv1 = ImVec2(1, 0);

	m_vpSize = ImGui::GetContentRegionAvail();
	ImVec2 origin = ImGui::GetCursorScreenPos();

	if ((AppConfig::RENDER_WIDTH != (int)m_vpSize.x) || (AppConfig::RENDER_HEIGHT != (int)m_vpSize.y))
	{
		viewportSizeSetteled = false;
	}
	else
	{
		viewportSizeSetteled = true;
	}

	viewportHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
	m_viewportState.isHovered = viewportHovered;

	ImGui::Image(texID, m_vpSize, uv0, uv1);

	// show FPS
	ImGui::SetCursorScreenPos(ImVec2(origin.x + 10, origin.y + 10));
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::SetCursorScreenPos(ImVec2(origin.x + 10, origin.y + 20));
	ImGui::Text("ms: %.4f", deltaTime);

	// DEBUG QUAD PASS
	//  fetch parent window pos & size
	m_viewportPos = ImGui::GetWindowPos();
	ImVec2 winSize = ImGui::GetWindowSize();
	ImVec2 padding = ImGui::GetStyle().WindowPadding;

	const ImVec2 debugQuadSize(winSize.x / 10, winSize.y / 10);
	ImVec2 thumbPos(m_viewportPos.x + winSize.x - debugQuadSize.x - padding.x,
		m_viewportPos.y + padding.y); // compute top-right corner inside the window

	ImGui::SetCursorScreenPos(thumbPos);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs; // block inputs so clicks pass through
	if (AppConfig::showObjectPicking)
	{
		ImGui::BeginChild("PassPreview", debugQuadSize, false, flags);
		ImTextureID passTex = (ImTextureID)m_pickingTexture;
		ImGui::Image(passTex, debugQuadSize, uv0, uv1);
		ImGui::EndChild();
	}
	else if (AppConfig::showShadowMap)
	{
		ImGui::BeginChild("PassPreview", debugQuadSize, false, flags);
		ImTextureID passTex = (ImTextureID)m_shadowMapTexture;
		ImGui::Image(passTex, debugQuadSize, uv0, uv1);
		ImGui::EndChild();
	}

	ImGui::End();
}

glm::vec2 UIManager::getViewportSize() { return glm::vec2(m_vpSize.x, m_vpSize.y); }

void UIManager::setScreenTexture(uint32_t texId) { m_screenTexture = texId; }

void UIManager::setPickingTexture(uint32_t texId) { m_pickingTexture = texId; }

void UIManager::setShadowMapTexture(uint32_t texId) { m_shadowMapTexture = texId; }

void UIManager::showCameraTransforms()
{
	ImGui::SetNextWindowPos(ImVec2(100, 10), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.3f);
	ImGui::Begin("Camera", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Pos: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z);
	ImGui::SameLine();
	ImGui::Text("Rot: %.1f, %.1f", camera.pitch, camera.yaw);
	ImGui::Text("Front: %.1f, %.1f, %.1f", camera.front.x, camera.front.y, camera.front.z);
	ImGui::SameLine();
	ImGui::Text("Up: %.1f, %.1f, %.1f", camera.up.x, camera.up.y, camera.up.z);
	ImGui::Text("Right: %.1f, %.1f, %.1f", camera.right.x, camera.right.y, camera.right.z);
	ImGui::Text("Default Pos: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[0].x, camera.defaultCameraMatrix[0].y,
		camera.defaultCameraMatrix[0].z);
	ImGui::SameLine();
	ImGui::Text("Default Rot: %.1f, %.1f", camera.defaultCameraRotation[0], camera.defaultCameraRotation[1]);
	ImGui::Text("Default Front: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[1].x, camera.defaultCameraMatrix[1].y,
		camera.defaultCameraMatrix[1].z);
	ImGui::SameLine();
	ImGui::Text("Default Up: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[2].x, camera.defaultCameraMatrix[2].y,
		camera.defaultCameraMatrix[2].z);
	ImGui::End();
}

void UIManager::showLights()
{
	ImGui::Begin("Lights");
	for (int i = 0; i < SceneManager::getLights().size(); i++)
	{
		ImGui::PushID(i);
		ImGui::Text("Light: %d %s ", i,
			(SceneManager::getLights()[i].type == 1) ? "directional"
			: (SceneManager::getLights()[i].type == 2) ? "spot"
			: "point");
		ImGui::DragFloat3("Position", glm::value_ptr(SceneManager::getLights()[i].position));
		ImGui::SliderFloat("Intensity", &SceneManager::getLights()[i].intensity, 0.0f, 10.0f);

		static glm::vec3 lightRotation = glm::vec3(0.0f);
		if (ImGui::DragFloat3("Light Rotation", glm::value_ptr(lightRotation), 0.1f))
		{
			float pitch = glm::radians(lightRotation.x);
			float yaw = glm::radians(lightRotation.y);
			float roll = glm::radians(lightRotation.z);
			glm::vec3 baseDirection(0.0f, -1.0f, 0.0f);
			glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), roll, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 rotationMatrix = rotZ * rotY * rotX;
			SceneManager::getLights()[i].direction = glm::vec3(rotationMatrix * glm::vec4(baseDirection, 0.0f));
		}
		if (SceneManager::getLights()[i].type == 2)
		{
			float cutoffAngle = glm::degrees(glm::acos(SceneManager::getLights()[i].cutOff));
			float outerCutoffAngle = glm::degrees(glm::acos(SceneManager::getLights()[i].outerCutOff));
			if (ImGui::SliderFloat("CutOff Angle", &cutoffAngle, 0.0f, 180.0f))
			{
				if (outerCutoffAngle <= cutoffAngle)
				{
					outerCutoffAngle = cutoffAngle + 0.001f;
					SceneManager::getLights()[i].outerCutOff = glm::cos(glm::radians(outerCutoffAngle));
				}
				SceneManager::getLights()[i].cutOff = glm::cos(glm::radians(cutoffAngle));
			}

			if (ImGui::SliderFloat("Outer CutOff Angle", &outerCutoffAngle, cutoffAngle, 180))
			{
				if (outerCutoffAngle <= cutoffAngle)
					outerCutoffAngle = cutoffAngle + 0.001f;
				SceneManager::getLights()[i].outerCutOff = glm::cos(glm::radians(outerCutoffAngle));
			}
		}
		ImGui::ColorEdit3("Color", glm::value_ptr(SceneManager::getLights()[i].diffuse));
		ImGui::PopID();
	}
	ImGui::End();
}

void UIManager::showObjectInspector()
{
	if (SceneManager::getSelectedPrimitive() != nullptr)
	{
		ImGui::Begin("Object Inspector");
		ImGui::Text("Object: %s", SceneManager::getSelectedPrimitive()->name.c_str());
		ImGui::DragFloat3("Position", glm::value_ptr(SceneManager::getSelectedPrimitive()->transform.matrix[3]),
			0.01f, -100.0f, 100.0f);
		if (SceneManager::getSelectedPrimitive()->material->diffuse != nullptr)
			ImGui::Image(SceneManager::getSelectedPrimitive()->material->diffuse->id, ImVec2(64, 64));
		ImGui::SameLine();
		if (SceneManager::getSelectedPrimitive()->material->diffuse != nullptr)
			ImGui::Image(SceneManager::getSelectedPrimitive()->material->specular->id, ImVec2(64, 64));
		if (ImGui::Button("Diffuse"))
		{
			std::string filePath = OpenFileDialog(FileType::IMAGE);
			if (!filePath.empty())
			{
				// Update the object's texture path
				SceneManager::getSelectedPrimitive()->material->diffuse->setPath(filePath);
				glActiveTexture(GL_TEXTURE0);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Specular"))
		{
			std::string filePath = OpenFileDialog(FileType::IMAGE);
			if (!filePath.empty())
			{
				// Update the object's texture path
				SceneManager::getSelectedPrimitive()->material->specular->setPath(filePath);
				glActiveTexture(GL_TEXTURE0);
			}
		}
		if (ImGui::Button("DELETE"))
		{
			if (auto selectedPrimitive = SceneManager::getSelectedPrimitive())
			{
				if (selectedPrimitive->parent)
				{
					selectedPrimitive->parent->removeChild(selectedPrimitive);
				}
			}
			ImGui::End();
			return;
		}

		ImGui::SliderFloat("Roughness", &SceneManager::getSelectedPrimitive()->material->roughness, 0.04f, 1.0f);
		ImGui::SliderFloat("Metallic", &SceneManager::getSelectedPrimitive()->material->metallic, 0.0f, 1.0f);

		ImGui::End();
	}
}

void UIManager::showTools()
{
	ImGui::Begin("Tools");
	ImGui::ColorEdit4("BG Color", AppConfig::clearColor);

	ImGui::SliderFloat("FOV", &camera.zoom, 1.f, 100.f, "%.3f");
	ImGui::SliderFloat("Gamma", &AppConfig::gamma, 0.01f, 5);
	ImGui::SliderFloat("Near plane", &AppConfig::near_plane, -10.0f, 10.f, "%.6f");
	ImGui::SliderFloat("Far plane", &AppConfig::far_plane, -10.0f, 10.f);
	ImGui::Checkbox("Wireframe Mode", &AppConfig::isWireframe);
	ImGui::SliderFloat("CubeMap intensity", &AppConfig::irradianceMapIntensity, 0.0f, 3.0f);
	ImGui::SliderFloat("CubeMap Rotatation Y", &AppConfig::irradianceMapRotationY, -180.0f, 180.0f);
	ImGui::SliderFloat("Backgorund Blur", &AppConfig::backgroundBlur, 0.0f, 1.0f);
	ImGui::Checkbox("ObjectID Debug", &AppConfig::showObjectPicking);
	ImGui::Checkbox("ShadowMap Debug", &AppConfig::showShadowMap);
	if (ImGui::Button("Load CubeMap"))
	{
		std::string filePath = OpenFileDialog(FileType::IMAGE);
		if (!filePath.empty())
		{
			// Signal that a new cubemap needs to be loaded
			AppConfig::cubeMapPath = filePath;
			AppConfig::reloadCubeMap = true;
		}
	}
	if (ImGui::Button("Import Model"))
	{
		std::string filePath = OpenFileDialog(FileType::MODEL);
		if (!filePath.empty())
		{
			GLTFModel model(filePath);
			scene->addChild(std::move(model.getModel()));
		}
	}
	AppConfig::polygonMode = AppConfig::isWireframe ? GL_LINE : GL_FILL;
	if (ImGui::Button("Reload Shaders"))
	{
		SceneManager::reloadShaders();
		// AppConfig::screenShader->reload();
		std::cout << "Shaders reloaded successfully!" << std::endl;
	}
	ImGui::End();
}

static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;
void UIManager::displaySceneNode(SceneNode* node) const
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool is_folder = node->children.size() > 0;
	if (is_folder)
	{
		bool open = ImGui::TreeNodeEx(node->name.c_str(), tree_node_flags);
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(typeid(*node).name() + 6); // skip "struct" or "class"
		if (open)
		{
			for (auto& child : node->children)
			{
				displaySceneNode(child.get());
			}
			ImGui::TreePop();
		}
	}
	else
	{
		bool selected = false;
		ImGui::Bullet();
		static std::set<std::string> selectedNodes;
		bool isSelected = selectedNodes.count(node->name) > 0;
		if (ImGui::Selectable(node->name.c_str(), isSelected,
			ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
		{
			if (ImGui::GetIO().KeyCtrl)
			{
				if (isSelected)
					selectedNodes.erase(node->name);
				else
					selectedNodes.insert(node->name);
			}
			else
			{
				selectedNodes.clear();
				selectedNodes.insert(node->name);
			}
			std::cout << "Node clicked: " << node->name << std::endl;
			Primitive* prim = dynamic_cast<Primitive*>(node);
			if (prim != nullptr)
			{
				if (ImGui::GetIO().KeyCtrl)
				{
					SceneManager::selectPrimitive(prim->vao, true);
				}
				else
				{
					SceneManager::selectPrimitive(prim->vao, false);
				}
			}
		}
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(typeid(*node).name() + 6); // skip "struct" or "class"
	}
}

void UIManager::showOutliner()
{
	ImGui::Begin("Outliner");
	int objectsCount =
		SceneManager::getModels().size() + SceneManager::getPrimitives().size() + SceneManager::getLights().size();
	ImGui::Text("Objects in Scene: %d", objectsCount);
	if (ImGui::BeginTable("OutlinerTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.8f);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.4f);
		ImGui::TableHeadersRow();
		displaySceneNode(scene);


		ImGui::EndTable();
	}
	ImGui::End();
}


std::string UIManager::OpenFileDialog(FileType type)
{
	OPENFILENAMEA ofn;
	char fileName[260] = { 0 }; // buffer for file name

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);

	// Filter: display image files by default (you can adjust as needed)
	if (type == FileType::IMAGE)
	{
		ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.hdr;*.exr\0All Files\0*.*\0";
	}
	else if (type == FileType::MODEL)
	{
		ofn.lpstrFilter = "Model Files\0*.gltf;*.glb;*.obj\0All Files\0*.*\0";
	}
	else
	{
		ofn.lpstrFilter = "All Files\0*.*\0";
	}
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
		ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
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
					std::string filePath = OpenFileDialog(FileType::IMAGE);
					if (!filePath.empty())
					{
						mat->diffuse->setPath(filePath);
						glActiveTexture(GL_TEXTURE0);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Specular", ImVec2(imageSize, 0)))
				{
					std::string filePath = OpenFileDialog(FileType::IMAGE);
					if (!filePath.empty())
					{
						mat->specular->setPath(filePath);
						glActiveTexture(GL_TEXTURE0);
					}
				}

				ImGui::SliderFloat("Roughness", &mat->roughness, 0.04f, 1.0f);
				ImGui::SliderFloat("Metallic", &mat->metallic, 0.0f, 1.0f);

				ImGui::PopID();
			}
			ImGui::EndGroup();

			itemIdx++;
		}
		ImGui::EndTable();
	}

	ImGui::End();
}
