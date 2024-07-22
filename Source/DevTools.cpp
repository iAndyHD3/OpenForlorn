#include "DevTools.hpp"
#include <imgui.h>
#include <glaze/util/utility.hpp>

template<IntOrFloat T>
void DevTools::dragVar(const std::string& uniqueName, T& var, float step, T min, T max)
{
    m_varmap.emplace_back(uniqueName, Var<T>{&var, var, step, min, max});
}


void DevTools::drawImgui()
{
    for(auto& [stringid, variant] : m_varmap)
    {

        if(Var<int>* var = std::get_if<Var<int>>(&variant))
        {
            ImGui::DragInt(stringid.c_str(), var->var, var->step, var->min, var->max);
            continue;
        }
        Var<float>* var = std::get_if<Var<float>>(&variant);
        ImGui::DragFloat(stringid.c_str(), var->var, var->step, var->min, var->max);
    }
}

void DevTools::clear()
{
    m_varmap.clear();
}

template void DevTools::dragVar(const std::string& uniqueName, float& val, float step, float min, float max);
template void DevTools::dragVar(const std::string& uniqueName, int& var, float step, int min, int max);
