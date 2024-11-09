#pragma once

#include <string>
#include "display_buffer.h"

class IDisplayComponent
{
public:
    virtual ~IDisplayComponent() = default;
    virtual void render() const = 0;
};

class DisplayComponent : public IDisplayComponent
{
protected:
    DisplayBuffer *buffer;
    int x;
    int y;
    int width;
    int height;

public:
    DisplayComponent(DisplayBuffer *buffer, int offsetX, int offsetY, int width, int height)
        : buffer(buffer), x(offsetX), y(offsetY), width(width), height(height)
    {
    }

    virtual void render() const override;
};
