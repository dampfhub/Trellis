#ifndef UI_H
#define UI_H
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imfilebrowser.h"

#include "page.h"

#include <vector>
#include <string>

class UI {
public:
	unsigned int WindowWidth;
	unsigned int WindowHeight;
	ImGui::FileBrowser * FileDialog;

	// Flags
	bool PageSelectOpen = false;
	bool PageSettingsOpen = false;
	bool PageAddOpen = false;
	bool AddFromPreset = false;
	bool AddPage = false;
	
	// Data
	std::string PageName = "";
	int PageSize;

	int ActivePage = 0;
	int PlayerPageView = 0;

	~UI();
	UI(unsigned int width, unsigned int height);
	void Draw(std::vector <Page *> pages, Page * active_page);
	void DrawMenu(std::vector <Page *> pages, Page * active_page);
	void DrawPageSelect(std::vector<Page *> pages, Page * active_page);
	void DrawPageSettings(Page * active_page);

	Page * GetActivePage(std::vector<Page *> pages);
	void ClearFlags();

private:
	char PageNameBuf[128] = "";
};
#endif