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

#include "texture.hpp"
#include "appConfig.hpp"
#include "gltfImporter.hpp"
#include "sceneManager.hpp"
#include "uiManager.hpp"


UIManager::UIManager(GLFWwindow* window, Camera& camera, SceneNode* scene) : camera(camera), window(window), scene(scene), m_appConfig(AppConfig::get())
{
	m_noTexture = new Tex(std::filesystem::absolute("..\\..\\res\\icons\\No Image.png").string().c_str());
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
	int readY = static_cast<int>(m_appConfig.renderHeight - (mousePos.y - viewportPos.y));
	ImVec2 cursorPos(static_cast<float>(readX), static_cast<float>(readY));
	m_viewportState.cursorPos = cursorPos;
}

void UIManager::getViewportPos()
{
	m_viewportState.position = m_viewportPos;
}

ViewportState UIManager::getViewportState()
{
	return m_viewportState;
}

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
	m_viewportState.size = m_vpSize;
	ImVec2 origin = ImGui::GetCursorScreenPos();

	if ((m_appConfig.renderWidth != (int)m_vpSize.x) || (m_appConfig.renderHeight != (int)m_vpSize.y))
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

	const ImVec2 debugQuadSize(winSize.x / 4, winSize.y / 4);
	ImVec2 thumbPos(m_viewportPos.x + winSize.x - debugQuadSize.x,
		m_viewportPos.y);

	ImGui::SetCursorScreenPos(thumbPos);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs; // block inputs so clicks pass through
	if (m_appConfig.showObjectPicking)
	{
		ImGui::BeginChild("PassPreview", debugQuadSize, false, flags);
		ImTextureID passTex = (ImTextureID)m_pickingTexture;
		ImGui::Image(passTex, debugQuadSize, uv0, uv1, ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::EndChild();
	}
	else if (m_appConfig.showShadowMap)
	{
		ImGui::BeginChild("PassPreview", debugQuadSize, false, flags);
		ImTextureID passTex = (ImTextureID)m_shadowMapTexture;
		ImGui::Image(passTex, debugQuadSize, uv0, uv1);
		ImGui::EndChild();
	}

	// G-Buffer debug view
	if (m_gBuffer != nullptr) {

		static int currentBuffer = 0;
		if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
			currentBuffer = (currentBuffer + 1) % 7; // Cycle through 7 buffers
		}

		ImGui::BeginChild("GBufferPreview", debugQuadSize, false, flags);

		GLuint texToShow;
		switch (currentBuffer) {
		case 0: texToShow = m_gBuffer->tPosition; break;
		case 1: texToShow = m_gBuffer->tNormal; break;
		case 2: texToShow = m_gBuffer->tAlbedo; break;
		case 3: texToShow = m_gBuffer->tRoughness; break;
		case 4: texToShow = m_gBuffer->tMetallic; break;
		case 5: texToShow = m_gBuffer->tDepth; break;
		case 6: texToShow = m_gBuffer->tVelocity; break;
		}

		ImGui::Image((ImTextureID)texToShow, debugQuadSize, uv0, uv1);
		ImGui::EndChild();
	}

	ImGui::End();
}

glm::vec2 UIManager::getViewportSize() { return glm::vec2(m_vpSize.x, m_vpSize.y); }

void UIManager::setScreenTexture(uint32_t texId) { m_screenTexture = texId; }

void UIManager::setPickingTexture(uint32_t texId) { m_pickingTexture = texId; }

void UIManager::setShadowMapTexture(uint32_t texId) { m_shadowMapTexture = texId; }

void UIManager::setGBuffer(GBuffer* gBuffer)
{
	m_gBuffer = gBuffer;
}

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
	// ImGui::Text("Default Pos: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[0].x, camera.defaultCameraMatrix[0].y,
	// 	camera.defaultCameraMatrix[0].z);
	// ImGui::SameLine();
	// ImGui::Text("Default Rot: %.1f, %.1f", camera.defaultCameraRotation[0], camera.defaultCameraRotation[1]);
	// ImGui::Text("Default Front: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[1].x, camera.defaultCameraMatrix[1].y,
	// 	camera.defaultCameraMatrix[1].z);
	// ImGui::SameLine();
	// ImGui::Text("Default Up: %.1f, %.1f, %.1f", camera.defaultCameraMatrix[2].x, camera.defaultCameraMatrix[2].y,
	// 	camera.defaultCameraMatrix[2].z);
	ImGui::End();
}

void UIManager::showLights()
{
	ImGui::Begin("Lights");

	ImGui::End();
}

void UIManager::showObjectInspector()
{
	if (SceneManager::getSelectedPrimitive() != nullptr)
	{
		ImGui::Begin("Object Inspector");
		ImGui::Text("Object: %s", SceneManager::getSelectedPrimitive()->name.c_str());
		ImGui::Text("Material: %s", SceneManager::getSelectedPrimitive()->material->name.c_str());
		ImGui::DragFloat3("Position", glm::value_ptr(SceneManager::getSelectedPrimitive()->transform.matrix[3]), 0.01f, -100.0f, 100.0f);

		bool hasDiffuse = SceneManager::getSelectedPrimitive()->material->tDiffuse != nullptr && SceneManager::getSelectedPrimitive()->material->tDiffuse->id > 0;
		bool hasSpecular = SceneManager::getSelectedPrimitive()->material->tSpecular != nullptr && SceneManager::getSelectedPrimitive()->material->tSpecular->id > 0;
		bool hasNormal = SceneManager::getSelectedPrimitive()->material->tNormal != nullptr && SceneManager::getSelectedPrimitive()->material->tNormal->id > 0;
		auto& material = SceneManager::getSelectedPrimitive()->material;

		{
			uint32_t id = (hasDiffuse == true) ? material->tDiffuse->id : m_noTexture->id;
			ImGui::Image(id, ImVec2(64, 64));
			ImGui::SameLine();
		}

		{
			uint32_t id = (hasSpecular == true) ? material->tSpecular->id : m_noTexture->id;
			ImGui::Image(id, ImVec2(64, 64));
			ImGui::SameLine();

		}

		{
			uint32_t id = (hasNormal == true) ? material->tNormal->id : m_noTexture->id;
			ImGui::Image(id, ImVec2(64, 64));
		}

		if (ImGui::Button("Diffuse", ImVec2(64, 20)))
		{
			std::string filePath = OpenFileDialog(FileType::IMAGE);
			if (!filePath.empty())
			{
				material->tDiffuse->setPath(filePath);
				glActiveTexture(GL_TEXTURE0);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Specular", ImVec2(64, 20)))
		{
			std::string filePath = OpenFileDialog(FileType::IMAGE);
			if (!filePath.empty())
			{
				material->tSpecular->setPath(filePath);
				glActiveTexture(GL_TEXTURE0);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Normal", ImVec2(64, 20)))
		{
			std::string filePath = OpenFileDialog(FileType::IMAGE);
			if (!filePath.empty())
			{
				material->tNormal->setPath(filePath);
				glActiveTexture(GL_TEXTURE0);
			}
		}


		// Tiling options for Diffuse, Specular, and Normal textures

		// Diffuse texture tiling
		{
			bool* isTiled = &material->tDiffuse->tiled;
			ImGui::PushID(material->tDiffuse->id);
			if (!hasDiffuse) ImGui::BeginDisabled();
			ImGui::Checkbox("Diffuse Tiled", isTiled);
			if (!hasDiffuse) ImGui::EndDisabled();
			ImGui::PopID();
			material->tDiffuse->setTiled(*isTiled);
		}

		// Specular texture tiling
		{
			ImGui::SameLine();
			bool* isTiled = &material->tSpecular->tiled;
			ImGui::PushID(material->tSpecular->id);
			if (!hasSpecular) ImGui::BeginDisabled();
			ImGui::Checkbox("Specular Tiled", isTiled);
			if (!hasSpecular) ImGui::EndDisabled();
			ImGui::PopID();
			material->tSpecular->setTiled(*isTiled);
		}

		// Normal texture tiling
		{
			ImGui::SameLine();
			bool* isTiled = &material->tNormal->tiled;
			ImGui::PushID(material->tNormal->id);
			if (!hasNormal) ImGui::BeginDisabled();
			ImGui::Checkbox("Normal Tiled", isTiled);
			if (!hasNormal) ImGui::EndDisabled();
			ImGui::PopID();
			material->tNormal->setTiled(*isTiled);
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
		ImGui::ColorEdit4("Albedo", glm::value_ptr(SceneManager::getSelectedPrimitive()->material->albedo));
		ImGui::SliderFloat("Roughness", &SceneManager::getSelectedPrimitive()->material->roughness, 0.04f, 1.0f);
		ImGui::SliderFloat("Metallic", &SceneManager::getSelectedPrimitive()->material->metallic, 0.0f, 1.0f);

		ImGui::End();
	}
};

void UIManager::showTools()
{
	ImGui::Begin("Tools");
	ImGui::ColorEdit4("BG Color", m_appConfig.clearColor);

	ImGui::SliderFloat("FOV", &camera.zoom, 1.f, 100.f, "%.3f");
	ImGui::SliderFloat("Gamma", &m_appConfig.gamma, 0.01f, 5);
	ImGui::SliderFloat("Near plane", &m_appConfig.nearPlane, -10.0f, 10.f, "%.6f");
	ImGui::SliderFloat("Far plane", &m_appConfig.farPlane, -10.0f, 10.f);
	ImGui::Checkbox("Wireframe Mode", &m_appConfig.isWireframe);
	ImGui::SliderFloat("CubeMap intensity", &m_appConfig.irradianceMapIntensity, 0.0f, 3.0f);
	ImGui::SliderFloat("CubeMap Rotatation Y", &m_appConfig.irradianceMapRotationY, -180.0f, 180.0f);
	ImGui::SliderFloat("Backgorund Blur", &m_appConfig.backgroundBlur, 0.0f, 1.0f);
	ImGui::Checkbox("ObjectID Debug", &m_appConfig.showObjectPicking);
	ImGui::Checkbox("ShadowMap Debug", &m_appConfig.showShadowMap);
	ImGui::Checkbox("FXAA", &m_appConfig.isFXAA);
	if (ImGui::Button("Load CubeMap"))
	{
		std::string filePath = OpenFileDialog(FileType::IMAGE);
		if (!filePath.empty())
		{
			// Signal that a new cubemap needs to be loaded
			m_appConfig.cubeMapPath = filePath;
			m_appConfig.reloadCubeMap = true;
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
	m_appConfig.polygonMode = m_appConfig.isWireframe ? GL_LINE : GL_FILL;
	if (ImGui::Button("Reload Shaders"))
	{
		SceneManager::reloadShaders();
		// m_appConfig.screenShader->reload();
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
	size_t objectsCount = SceneManager::getModels().size() + SceneManager::getPrimitives().size();
	ImGui::Text("Objects in Scene: %zu", objectsCount);
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

				if (mat->tDiffuse)
				{
					ImGui::Image(mat->tDiffuse->id, ImVec2(imageSize, imageSize));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Diffuse");
				}
				ImGui::SameLine();
				if (mat->tSpecular)
				{
					ImGui::Image(mat->tSpecular->id, ImVec2(imageSize, imageSize));
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Specular");
				}

				if (ImGui::Button("Diffuse", ImVec2(imageSize, 0)))
				{
					std::string filePath = OpenFileDialog(FileType::IMAGE);
					if (!filePath.empty())
					{
						mat->tDiffuse->setPath(filePath);
						glActiveTexture(GL_TEXTURE0);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Specular", ImVec2(imageSize, 0)))
				{
					std::string filePath = OpenFileDialog(FileType::IMAGE);
					if (!filePath.empty())
					{
						mat->tSpecular->setPath(filePath);
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
