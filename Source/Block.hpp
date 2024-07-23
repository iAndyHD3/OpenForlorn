#pragma once

#include <2d/Sprite.h>
#include <math/Vec2.h>
#include <optional>
#include "2d/ActionTween.h"

struct Block : public ax::Sprite, ax::ActionTweenDelegate
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
        std::string customAnim;

        bool darken = false;
        bool animated = false;
        bool skipStartAnim = false;
        bool skipEndAnim = false;
        std::array<bool, 2> flipped = {false, false};

    };

    bool isSheetHD(const Data& data);
    bool init(const Data& data);

    void runPulsingAnimationForever();
    void runAnimationLooped(const Data& d);

    virtual void updateTweenAction(float value, std::string_view key) override;
};
