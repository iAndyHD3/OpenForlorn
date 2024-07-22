#include "Block.hpp"


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


    return true;
}

