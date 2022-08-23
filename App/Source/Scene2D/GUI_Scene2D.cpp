/**
 CGUI_Scene2D
 @brief A class which manages the GUI for Scene2D
 By: Toh Da Jun
 Date: May 2021
 */
#include "GUI_Scene2D.h"

#include <iostream>
using namespace std;

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::CGUI_Scene2D(void)
	: cSettings(NULL)
	, m_fProgressBar(0.0f)
	, cInventoryManager(NULL)
	, cInventoryItem(NULL)
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CGUI_Scene2D::~CGUI_Scene2D(void)
{
	if (cInventoryManager)
	{
		cInventoryManager->Destroy();
		cInventoryManager = NULL;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// We won't delete this since it was created elsewhere
	cSettings = NULL;
}

/**
  @brief Initialise this instance
  */
bool CGUI_Scene2D::Init(void)
{
	// Get the handler to the CSettings instance
	cSettings = CSettings::GetInstance();

	// Store the CFPSCounter singleton instance here
	cFPSCounter = CFPSCounter::GetInstance();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CSettings::GetInstance()->pWindow, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Define the window flags
	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	//// Show the mouse pointer
	//glfwSetInputMode(CSettings::GetInstance()->pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Initialise the cInventoryManager
	cInventoryManager = CInventoryManager::GetInstance();
	cInventoryItem = cInventoryManager->Add("Item", "Image/empty.tga", 1, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	// dummy flare 
	cInventoryItem = cInventoryManager->Add("Item2", "Image/Flare.tga", 1, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);
	// dummy cereal
	cInventoryItem = cInventoryManager->Add("Item3", "Image/Cornflakes.tga", 1, 0);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	// hi hz whistle
	cInventoryItem = cInventoryManager->Add("Whistle", "Image/frequency.tga", 2, 2);
	cInventoryItem->vec2Size = glm::vec2(25, 25);

	// These variables are for IMGUI demo only
	show_demo_window = true;
	show_another_window = true;
	clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	m_fProgressBar = 0.0f;

	return true;
}

/**
 @brief Update this instance
 */
void CGUI_Scene2D::Update(const double dElapsedTime)
{
	// Calculate the relative scale to our default windows width
	const float relativeScale_x = cSettings->iWindowWidth / 800.0f;
	const float relativeScale_y = cSettings->iWindowHeight / 600.0f;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create an invisible window which covers the entire OpenGL window
	ImGui::Begin("Invisible window", NULL, window_flags);
	ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetWindowSize(ImVec2((float)cSettings->iWindowWidth, (float)cSettings->iWindowHeight));
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);

	// Display the FPS
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", cFPSCounter->GetFrameRate());

	// Render the Health
	//ImGuiWindowFlags healthWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
	//	ImGuiWindowFlags_NoBackground |
	//	ImGuiWindowFlags_NoTitleBar |
	//	ImGuiWindowFlags_NoMove |
	//	ImGuiWindowFlags_NoResize |
	//	ImGuiWindowFlags_NoCollapse |
	//	ImGuiWindowFlags_NoScrollbar;
	//ImGui::Begin("Health", NULL, healthWindowFlags);
	//ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.03f));
	//ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	//ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	//cInventoryItem = cInventoryManager->GetItem("Health");
	//ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
	//	ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
	//		cInventoryItem->vec2Size.y * relativeScale_y),
	//	ImVec2(0,1), ImVec2(1,0));
	//ImGui::SameLine();
	//ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
	//ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	//ImGui::ProgressBar(cInventoryItem->GetCount() /
	//	(float)cInventoryItem->GetMaxCount(), ImVec2(100.0f * relativeScale_x, 20.0f * relativeScale_y));
	//ImGui::PopStyleColor();
	//ImGui::PopStyleColor();
	//
	//ImGui::End();

	// Render the Lives 
	/*ImGuiWindowFlags livesWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar; 
	ImGui::Begin("Lives", NULL, livesWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.85f, cSettings->iWindowHeight * 0.03f));
	ImGui::SetWindowSize(ImVec2(100.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItem = cInventoryManager->GetItem("Lives");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
			cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1,1,0,1), "%d / %d",
		cInventoryItem->GetCount(), cInventoryItem->GetMaxCount());

	ImGui::End();*/

	// Render the inventory items
	//cInventoryItem = cInventoryManager->GetItem("Item");
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));  // Set a background color
	ImGuiWindowFlags inventoryWindowFlags = ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Image", NULL, inventoryWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.8f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	if (itemType == 1)
		cInventoryItem = cInventoryManager->GetItem("Item2");
	else if (itemType == 2)
		cInventoryItem = cInventoryManager->GetItem("Item3");
	else
		cInventoryItem = cInventoryManager->GetItem("Item");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
			cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();
	ImGui::PopStyleColor();

	// Render whistle
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));  // Set a background color
	ImGui::Begin("Image2", NULL, inventoryWindowFlags);
	ImGui::SetWindowPos(ImVec2(cSettings->iWindowWidth * 0.03f, cSettings->iWindowHeight * 0.9f));
	ImGui::SetWindowSize(ImVec2(200.0f * relativeScale_x, 25.0f * relativeScale_y));
	cInventoryItem = cInventoryManager->GetItem("Whistle");
	ImGui::Image((void*)(intptr_t)cInventoryItem->GetTextureID(),
		ImVec2(cInventoryItem->vec2Size.x * relativeScale_x,
			cInventoryItem->vec2Size.y * relativeScale_y),
		ImVec2(0, 1), ImVec2(1, 0));
	ImGui::SameLine();
	ImGui::SetWindowFontScale(1.5f * relativeScale_y);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d / %d",
		cInventoryItem->GetCount(), cInventoryItem->GetMaxCount());
	ImGui::End();
	ImGui::PopStyleColor();

	ImGui::End();
}


/**
 @brief Set up the OpenGL display environment before rendering
 */
void CGUI_Scene2D::PreRender(void)
{
}

/**
 @brief Render this instance
 */
void CGUI_Scene2D::Render(void)
{
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 @brief PostRender Set up the OpenGL display environment after rendering.
 */
void CGUI_Scene2D::PostRender(void)
{
}

void CGUI_Scene2D::SetItemType(int type)
{
	itemType = type;
}
