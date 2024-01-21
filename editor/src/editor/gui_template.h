#pragma once
#include "imgui.h"
#include <cstdio>
#include <functional>
#include <sstream>
#include <type_traits>

namespace editor {

template <typename T, typename FN_getConvertTable>
void __addPropertyEditable(FN_getConvertTable convertTable, const char* propName, T& value, std::function<void()> value_changed = nullptr, std::function<bool(T)> cb = nullptr, const char* errorPopup = nullptr)
{
    std::pair<const char**, size_t> p = std::make_pair(nullptr, 0);
    if (convertTable) {
        p = convertTable();
    }
    ImGui::TableNextColumn();
    ImGui::Text("%s", propName);
    ImGui::TableNextColumn();
    char buf[128];
    sprintf(buf, "##%s", propName);
    bool ret = false;
    T val;
    if (p.first && p.second) {
        // create a selectable table
        const char* preview_value = nullptr;
        if (value >= 0 && value < p.second) {
            preview_value = p.first[value];
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(buf, preview_value)) {
            for (int i = 0; i < p.second; i++) {
                if (ImGui::Selectable(p.first[i], i == value)) {
                    // update value
                    value = i;
                    if (value_changed)
                        value_changed();
                }
            }
            ImGui::EndCombo();
        }
    } else {
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (std::is_integral<T>::value) {
            int tmp_val = value;
            ret = ImGui::InputInt(buf, &tmp_val, 1, 100, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
            val = tmp_val;
        } else if (std::is_floating_point<T>::value) {
            double tmp_val = value;
            ret = ImGui::InputDouble(buf, &tmp_val, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
            val = tmp_val;
        } else if (std::is_pointer<T>::value) {
        }
    }
    if (ret) {
        if (cb && !cb(val)) {
            if (errorPopup) {
                ImGui::OpenPopup(errorPopup);
            }
            return;
        }
        value = val;
        if (value_changed)
            value_changed();
    }
    ImGui::TableNextRow();
}

template <typename T>
void addPropertyReadonly(const char* propName, T& value, std::function<void(T)> appendCb = nullptr)
{
    ImGui::TableNextColumn();
    ImGui::Text("%s", propName);
    ImGui::TableNextColumn();
    ImGui::Text("%s", (std::stringstream() << value).str().c_str());
    if (appendCb)
        appendCb(value);
    ImGui::TableNextRow();
}

template <typename T>
void addPropertyEditable(const char* propName, T& value, std::function<void()> value_changed = nullptr, std::function<bool(T)> cb = nullptr, const char* errorPopup = nullptr)
{
    __addPropertyEditable<T, std::pair<const char**, size_t>()>(nullptr, propName, value, value_changed, cb, errorPopup);
}

template <typename T>
void addPropertyEditableNoCheck(const char* propName, T& value, std::function<void()> value_changed = nullptr)
{
    __addPropertyEditable<T, std::pair<const char**, size_t>()>(nullptr, propName, value, value_changed, std::function<bool(T)>(), nullptr);
}

template <typename T, typename FN_getConvertTable>
void addPropertySelectable(FN_getConvertTable convertTable, const char* propName, T& value, std::function<void()> value_changed = nullptr)
{
    __addPropertyEditable<T, FN_getConvertTable>(convertTable, propName, value, value_changed, nullptr, nullptr);
}

}