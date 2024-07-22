#pragma once
#include <concepts>
#include <variant>
#include <string>
#include <vector>

#define STRINGIFY(s) #s
#define DEVTOOLS_ADDVAR(var, ...) DevTools::get()->dragVar(STRINGIFY(var), var, __VA_ARGS__)

template<typename T>
concept IntOrFloat = std::same_as<T, int> || std::same_as<T, float>;

class DevTools
{
public:
    inline static DevTools* get()
    {
        static DevTools instance;
        return &instance;
    }

    void drawImgui();
    void clear();

    template<IntOrFloat T>
    void dragVar(const std::string& uniqueName, T& val, float step = 1.f, T min = 0, T max = 0);

private:
    template<IntOrFloat T>
    struct Var
    {
        T* var;
        T def;
        float step;
        T min;
        T max;
    };

    struct VarContainer
    {
        std::string name;
        std::variant<Var<int>, Var<float>> var;
    };

    std::vector<VarContainer> m_varmap;

};
