#pragma once

class NoCopy
{
public:
    NoCopy(const NoCopy& other) = delete;
    NoCopy& operator=(const NoCopy& other) = delete;
};
