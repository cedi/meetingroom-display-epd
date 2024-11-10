#pragma once

#include <vector>
#include "components.h"

class Status : public DisplayComponent
{
private:
    bool isImportant;
    const uint8_t *icon;

public:
    Status(DisplayBuffer *buffer);
    virtual void render() const override;

    void setImportant(bool b = true) { isImportant = b; }

    // Status icons should be 196x196
    void setIcon(const uint8_t *i) { icon = i; }
    void clearIcon() { setIcon(NULL); }
};