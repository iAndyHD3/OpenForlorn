#include "PlayLayer.hpp"
#include "2d/SpriteBatchNode.h"
#include "Block.hpp"
#include "FileUtils.h"
#include "ImGui/ImGuiPresenter.h"
#include "axmol_glaze.hpp"
#include "DevTools.hpp"
#include "imgui.h"
#include "HashUtil.hpp"

#include <unordered_map>

#include <glaze/glaze.hpp>
#include <axmol.h>


#include <fmt/format.h>


USING_NS_AX;


struct Level
{
    struct SheetData
    {
        std::string texture;
        std::string type;
        bool operator ==(const SheetData& b) const { return texture == b.texture && type == b.type; }
    };

    struct Settings
    {
        std::string bgImage;
    };

    std::unordered_map<std::string, Block::Data> blockContainer;
    std::unordered_map<std::string, SheetData> sheetContainer;
    Settings settings;
    ax::Vec2 playerSpawn;
};

MAKE_HASHABLE(Level::SheetData, self.texture, self.type)

struct PlayLayer::members
{
    ax::Vec2 playerPos = {};
    ax::Vec2 spawnPos = {};
    float camSpeed = 7;
    float zoom = 1;

    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;

    std::unordered_map<Level::SheetData, ax::SpriteBatchNode*> batchNodes;
};

//PlayLayer::getZValueForType
constexpr int getZValueForType(std::string_view type)
{
    int ret = 0;

    if(type == "F1")      ret = 160;
    else if(type == "F2") ret = 150;
    else if(type == "F3") ret = 140;
    else if(type == "F4") ret = 130;

    else if(type == "B1") ret = 50;
    else if(type == "B2") ret = 40;
    else if(type == "B3") ret = 30;
    else if(type == "B4") ret = 20;

    else if(type == "NPC") ret = 90;
    else if(type == "particle") ret = 0;
    else if(type == "P1") ret = 120;
    
    if(type.find('+') != type.npos) ret++;
    else if(type.find('-') != type.npos) ret--;

    return ret;
}

bool PlayLayer::init(const Args& args)
{
    if(!Scene::init()) return false;

    //deserialize the level using glaze

    auto levelstr = FileUtils::getInstance()->getStringFromFile("Level001.json");

    Level lvl;
    constexpr auto opts = glz::opts{
        .error_on_unknown_keys = false,
        .bools_as_numbers = true
    };
    auto errorCode = glz::read<opts, Level>(lvl, levelstr);

    AX_ASSERT(errorCode == 0);


    //LOADING OF EVERYTHING!!!!

    m.spawnPos = lvl.playerSpawn;
    m.playerPos = lvl.playerSpawn;

    //auto bgstr = fmt::format("{}.png", lvl.settings.bgImage);
    //m->bgSprite = Sprite::create(bgstr);
    //m->bgSprite->setPosition(m->playerPos);
    //m->bgSprite->setScale(2.f);
    //addChild(m->bgSprite, -1);


    auto sprch = SpriteFrameCache::getInstance();

    for(const auto& pair : lvl.sheetContainer) 
    {
        const auto& sheetData = pair.second;
        auto plist = fmt::format("{}.plist", sheetData.texture);
        sprch->addSpriteFramesWithFile(plist);

        std::string png = fmt::format("{}.png", sheetData.texture);
        if(!ax::FileUtils::getInstance()->isFileExist(png))
        {
            AXLOGE("Image does not exist", png);
            continue;
        }
        auto inserted = m.batchNodes.insert({sheetData, ax::SpriteBatchNode::create(png)});
        if(!inserted.second)
        {
            AXLOGE("Duplicated sheet? {} {}", sheetData.texture, sheetData.type);
            continue;
        }

        AXLOGI("Inserted sheet {} {}", inserted.first->first.texture, inserted.first->first.type);

        addChild(inserted.first->second, getZValueForType(sheetData.type));
    }

    for(const auto& pair : lvl.blockContainer)
    {
        const auto& bd = pair.second;
        if(bd.texture.find(".plist") != std::string::npos)
        {
            continue;
        }

        auto it = m.batchNodes.find({bd.spriteSheet, bd.sheetType});
        if(it == m.batchNodes.end())
        {
            AXLOGE("Could not find batch node for {} {}", bd.spriteSheet, bd.sheetType);
            continue;
        }

        auto block = utils::createInstance<Block>(&Block::init, bd);
        if(!block)
        {
            AXLOGI("Could not create block {} {}", pair.first, bd.texture.c_str());
            continue;
        }
        SpriteBatchNode* bn = it->second;
        bn->addChild(block, bd.zValue);
    }




    // Some templates (uncomment what you  need)
    auto touchListener = EventListenerTouchAllAtOnce::create();
    touchListener->onTouchesBegan = AX_CALLBACK_2(PlayLayer::onTouchesBegan, this);
    touchListener->onTouchesMoved = AX_CALLBACK_2(PlayLayer::onTouchesMoved, this);
    touchListener->onTouchesEnded = AX_CALLBACK_2(PlayLayer::onTouchesEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    //auto mouseListener           = EventListenerMouse::create();
    //mouseListener->onMouseMove   = AX_CALLBACK_1(PLayLayer::onMouseMove, this);
    //mouseListener->onMouseUp     = AX_CALLBACK_1(PLayLayer::onMouseUp, this);
    //mouseListener->onMouseDown   = AX_CALLBACK_1(PLayLayer::onMouseDown, this);
    //mouseListener->onMouseScroll = AX_CALLBACK_1(PLayLayer::onMouseScroll, this);
    //_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    auto keyboardListener           = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed  = AX_CALLBACK_2(PlayLayer::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(PlayLayer::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 11);

    scheduleUpdate();

    m.zoom = Camera::getDefaultCamera()->getZoom();
    DEVTOOLS_ADDVAR_0(m.camSpeed);
    DEVTOOLS_ADDVAR_0(m.playerPos.x);
    DEVTOOLS_ADDVAR_0(m.playerPos.y);
    DevTools::get()->dragVar("zoom", m.zoom, 0.05, 0.1f, 2.0f);

    //so that we dont spawn in the dark...
    
    return true;
}

void PlayLayer::onEnter()
{
    Scene::onEnter();
    auto imgui = extension::ImGuiPresenter::getInstance(); 
    imgui->addRenderLoop("#playlayer", std::bind(&PlayLayer::onDrawImgui, this), this);
}

void PlayLayer::onDrawImgui()
{
    if (!ImGui::Begin("Hello world")) return ImGui::End();
    
    ImGui::SetWindowSize({600, 350}, ImGuiCond_Once);
    DevTools::get()->drawImgui();

    if (ImGui::Button("Player spawn"))
    {
        m.playerPos = m.spawnPos;
    }
    
    ImGui::End();
    
}

void PlayLayer::update(float dt)
{
    if(m.up) onUp();
    if(m.down) onDown();
    if(m.left) onLeft();
    if(m.right) onRight();

    this->getDefaultCamera()->setPosition(m.playerPos);
    this->getDefaultCamera()->setZoom(m.zoom);
}


void PlayLayer::onLeft()
{
    m.playerPos.x -= m.camSpeed;
}

void PlayLayer::onRight()
{
    m.playerPos.x += m.camSpeed;
}
void PlayLayer::onUp()
{
    m.playerPos.y += m.camSpeed;
}
void PlayLayer::onDown()
{
    m.playerPos.y -= m.camSpeed;
}

void PlayLayer::onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches)
    {
        AXLOG("onTouchesBegan detected, X:%f  Y:%f", t->getLocation().x, t->getLocation().y);
    }
}

void PlayLayer::onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches)
    {
        AXLOG("onTouchesMoved detected, X:%f  Y:%f", t->getLocation().x, t->getLocation().y);
    }
}

void PlayLayer::onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event)
{
    for (auto&& t : touches)
    {
        AXLOG("onTouchesEnded detected, X:%f  Y:%f", t->getLocation().x, t->getLocation().y);
    }
}
/*
void PLayLayer::onMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    AXLOG("onMouseDown detected, Key: %d", static_cast<int>(e->getMouseButton()));
}

void PLayLayer::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    AXLOG("onMouseUp detected, Key: %d", static_cast<int>(e->getMouseButton()));
}

void PLayLayer::onMouseMove(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    AXLOG("onMouseMove detected, X:%f  Y:%f", e->getCursorX(), e->getCursorY());
}

void PLayLayer::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    AXLOG("onMouseScroll detected, X:%f  Y:%f", e->getScrollX(), e->getScrollY());
}
*/

void PlayLayer::onKeyPressed(EventKeyboard::KeyCode code, Event* event)
{
    using enum EventKeyboard::KeyCode;
    switch(code)
    {
        case KEY_A: m.left = true; break;
        case KEY_S: m.down = true; break;
        case KEY_D: m.right = true; break;
        case KEY_W: m.up = true; break;
        default: break;
    }
}

void PlayLayer::onKeyReleased(EventKeyboard::KeyCode code, Event* event)
{
    using enum EventKeyboard::KeyCode;
    switch(code)
    {
        case KEY_A: m.left = false; break;
        case KEY_S: m.down = false; break;
        case KEY_D: m.right = false; break;
        case KEY_W: m.up = false; break;
        default: break;
    }
}

void PlayLayer::onExit()
{
    auto imgui = extension::ImGuiPresenter::getInstance();
    imgui->removeRenderLoop("#playlayer");

    Scene::onExit();
}

PlayLayer::~PlayLayer() = default;
PlayLayer::PlayLayer() : mptr(new members), m(*mptr.get()) {};
