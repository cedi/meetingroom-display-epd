#pragma once

#include <vector>
#include "components.h"

class Status : public DisplayComponent
{
public:
    Status(DisplayBuffer *buffer);
    virtual void render() const override;
};