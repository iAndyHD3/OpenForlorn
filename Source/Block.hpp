#pragma once

#include <2d/Sprite.h>
#include <math/Vec2.h>
#include <optional>

struct Block : public ax::Sprite
{
    struct Data
    {
        std::optional<float> rotation;
        std::optional<float> opacity;

        std::optional<float> red;
        std::optional<float> blue;
        std::optional<float> green;

        ax::Vec2 scale;
        ax::Vec2 position;
        int p_uID;
        int zValue;

        std::string spriteSheet;
        std::string texture;
        std::string sheetType;

        bool darken;
        std::array<bool, 2> flipped;
    };
    bool isSheetHD(const Data& data);
    bool init(const Data& data);
};