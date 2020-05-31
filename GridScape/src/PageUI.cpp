#include <memory>
#include <utility>
#include "page_ui.h"

PageUI::~PageUI() {
}

PageUI::PageUI(std::shared_ptr<std::pair<int, int>> screenDims)
	: ScreenDims(screenDims) {
}

void PageUI::DrawPieceClickMenu() {
	if (this->ClickMenuActive) {
		ImGui::OpenPopup("right_click_menu");
		this->ClickMenuActive = false;
	}
	if (ImGui::BeginPopup("right_click_menu")) {
		ImGui::Text("Piece Options");
		ImGui::Separator();
		this->MoveToFront = ImGui::Selectable("Move To Front");
		this->MoveToBack = ImGui::Selectable("Move To Back");
		ImGui::EndPopup();
	}
}

void PageUI::ClearFlags() {
	this->MoveToBack = false;
	this->MoveToFront = false;
}