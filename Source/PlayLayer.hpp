#pragma once

#include <2d/Scene.h>
#include <memory>

struct PlayLayer : public ax::Scene
{
    struct members;
    std::unique_ptr<members> mptr;
    members& m;

    struct Args
    {
        int levelID = 1;
    };

    bool init(const Args& args);

    virtual ~PlayLayer();
    PlayLayer();


    // touch
    void onTouchesBegan(const std::vector<ax::Touch*>& touches, ax::Event* event);
    void onTouchesMoved(const std::vector<ax::Touch*>& touches, ax::Event* event);
    void onTouchesEnded(const std::vector<ax::Touch*>& touches, ax::Event* event);

    // mouse
    void onMouseDown(ax::Event* event);
    void onMouseUp(ax::Event* event);
    void onMouseMove(ax::Event* event);
    void onMouseScroll(ax::Event* event);

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event* event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event* event);


    
    void onLeft();
    void onRight();
    void onUp();
    void onDown();

    void onDrawImgui();

    void onEnter() override;
    void onExit() override;

    void update(float) override;
};