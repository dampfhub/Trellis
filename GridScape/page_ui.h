#ifndef PAGE_UI_H
#define PAGE_UI_H

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class PageUI {
public:
	unsigned int WindowWidth;
	unsigned int WindowHeight;

	bool ClickMenuActive = false;
	bool MoveToFront = false;
	bool MoveToBack = false;

	int ActivePage = 0;
	int PlayerPageView = 0;

	~PageUI();
	PageUI(unsigned int width, unsigned int height);
	void DrawPieceClickMenu();
	void ClearFlags();
};

#endif