#pragma once
#include <ctime>

//TODO: change this with a proper random library

static void randInit()
{
    std::srand(std::time(nullptr));
}

static GLfloat randZeroOne()
{
    return static_cast<GLfloat>(std::rand()) / static_cast<GLfloat>(RAND_MAX);
}

static GLfloat randMinusOneOne()
{
    return randZeroOne() * 2 - 1;
}

