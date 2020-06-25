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
	UI();
	void Draw(Page::page_vector_t &pages, Page::page_vector_it_t active_page);
	void DrawMenu(Page::page_vector_t &pages, Page::page_vector_it_t active_page);
	void DrawPageSelect(Page::page_vector_t &pages, Page::page_vector_it_t active_page);
	void DrawPageSettings(Page::page_vector_it_t active_page);

    std::vector<std::unique_ptr<Page>>::iterator GetActivePage(std::vector<std::unique_ptr<Page>> &pages);
	void ClearFlags();

private:
	char PageNameBuf[128] = "";
};
#endif