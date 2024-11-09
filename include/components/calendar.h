#pragma once

#include <vector>
#include "components.h"

class Calendar : public DisplayComponent
{
public:
    Calendar(DisplayBuffer *buffer);

    virtual void render() const override;
};