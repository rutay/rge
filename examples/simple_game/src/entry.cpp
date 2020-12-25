#include "entry.hpp"

#include <stdio.h>

#include "cli/boot.hpp"

void Simple_Game::on_init()
{
    printf("Init\n");
}

void Simple_Game::on_render()
{
    printf("Render\n");
}

RGE_DEFINE_MAIN(Simple_Game);
