#include "Block.hpp"
#include <cstdint>
#include "2d/ActionInterval.h"
#include "2d/ActionManager.h"
#include "2d/ActionTween.h"
#include <2d/SpriteFrameCache.h>

USING_NS_AX;

bool Block::isSheetHD(const Data& d)
{
    constexpr const char* no_hd[] =
    {
        "ForestSheet_MG", "lightSheet", "FrostLevel_MGSheet", "FlameVillageMG"
    };

    for (const auto& nohdsheet : no_hd)
        if (d.spriteSheet == nohdsheet)
            return false;

    return true;
}
bool Block::init(const Block::Data& d)
{
    if(!ax::Sprite::initWithSpriteFrameName(d.texture))
    {
        AXLOGI("Could not create block {}", d.texture);
        return false;
    }

    //pos
    setPosition(d.position);

    //scale
    ax::Vec2 newscale = d.scale;
    if(isSheetHD(d))
    {
        newscale.scale(0.5f);
    }
    setScale(newscale.x, newscale.y);


    setFlippedX(d.flipped[0]);
    setFlippedY(d.flipped[1]);

    if(d.opacity) setOpacity(*d.opacity * 255);
    if(d.rotation) setRotation(*d.rotation);

    if(d.red && d.green && d.blue)
    {
        auto r = static_cast<uint8_t>(*d.red * 255);
        auto g = static_cast<uint8_t>(*d.green * 255);
        auto b = static_cast<uint8_t>(*d.blue * 255);

        setColor(ax::Color3B{r, g, b});
    }


    if(d.customAnim == "pulsing")
    {
        runPulsingAnimationForever();
    }
    else if (d.animated)
    {
        runAnimationLooped(d);
    }


    return true;
}

void Block::runPulsingAnimationForever()
{
    AXLOGI("Running actions: {}", _actionManager->getNumberOfRunningActionsInTarget(this));
    
    auto opacity = static_cast<float>(getOpacity());
    auto a1 = ActionTween::create(2.0f, "pulsing", opacity, opacity * 0.4f);
    auto a2 = a1->reverse();
    auto rep = RepeatForever::create(Sequence::createWithTwoActions(a1, a2));
    runAction(rep);
}

void Block::runAnimationLooped(const Data& d)
{
    //texture: Fire_r_02.png
    //texture: Fire_r_02_looped_002.png

    ax::Vector<ax::SpriteFrame*> frames;

    //convert to view first because otherwise substr returns unnecessary std::string
    std::string_view rawName = std::string_view{d.texture}.substr(0, d.texture.size() - 4);

    auto sfc = SpriteFrameCache::getInstance();
    int i = 1;

    auto getNewFrame = [&](std::string_view type)
    {
        auto ret = sfc->getSpriteFrameByName(fmt::format("{}_looped_{:03}.png", rawName, i));
        i++;
        return ret;
    };

    if (!d.skipStartAnim)
    {
        while (auto frame = getNewFrame("start"))
        {
            AXLOGI("Add start frame");
            frames.pushBack(frame);
        }
    }

    i = 1;

    while (auto frame = getNewFrame("looped"))
    {
        frames.pushBack(frame);
    }

    if (!d.skipEndAnim)
    {
        i = 1;
        while (auto frame = getNewFrame("fade"))
        {
            AXLOGI("Add end frame");
            frames.pushBack(frame);
        }
    }

    runAction(Animate::create(Animation::createWithSpriteFrames(frames, 0.1f, UINT_MAX)));
}

void Block::updateTweenAction(float value, std::string_view key)
{
    if(key == "pulsing")
    {
        setOpacity(static_cast<uint8_t>(value));
    }
}
