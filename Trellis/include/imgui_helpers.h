#ifndef IMGUI_HELPERS_H
#define IMGUI_HELPERS_H

#include "imgui.h"

class ImStyleResource {
public:
    ImStyleResource(ImGuiStyleVar idx, const ImVec2 &val) {
        clr = false;
        ImGui::PushStyleVar(idx, val);
    }
    ImStyleResource(ImGuiStyleVar idx, const float &val) {
        clr = false;
        ImGui::PushStyleVar(idx, val);
    }
    ImStyleResource(ImGuiCol idx, const ImU32 &col) {
        clr = true;
        ImGui::PushStyleColor(idx, col);
    }
    ~ImStyleResource() {
        if (clr) {
            ImGui::PopStyleColor();
        } else {
            ImGui::PopStyleVar();
        }
    }

private:
    bool clr;
};

class ImFontResource {
public:
    explicit ImFontResource(ImFont *f) {
        ImGui::PushFont(f);
    }
    ~ImFontResource() {
        ImGui::PopFont();
    }
};

#endif
