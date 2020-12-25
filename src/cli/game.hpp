#pragma once

struct Game
{
    float m_width, m_height;

    void init();

    void update(float dt);
    void render();

    void shutdown();

    // The functions down here may be implemented by the RGE user.
    // They permit to control the Game's lifecycle independently from the platform.

    virtual void on_init() {}

    virtual void on_update(float dt) {}
    virtual void on_render() {}

    virtual void on_shutdown() {}
};

