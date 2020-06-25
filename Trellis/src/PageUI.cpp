#include <memory>
#include <utility>
#include "page_ui.h"

PageUI::~PageUI() {
}

PageUI::PageUI() {
}

void PageUI::DrawPieceClickMenu() {
    if (ClickMenuActive) {
        ImGui::OpenPopup("right_click_menu");
        ClickMenuActive = false;
    }
    if (ImGui::BeginPopup("right_click_menu")) {
        ImGui::Text("Piece Options");
        ImGui::Separator();
        MoveToFront = ImGui::Selectable("Move To Front");
        MoveToBack = ImGui::Selectable("Move To Back");
        ImGui::EndPopup();
    }
}

void PageUI::ClearFlags() {
    MoveToBack = false;
    MoveToFront = false;
}
