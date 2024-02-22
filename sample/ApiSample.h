#pragma once
#include"Application.h"
#include"core\CoreUtils.h"


class ApiSample : public Application
{
public:
    ApiSample(const AppDesc& appDesc);
    ~ApiSample() = default;
    NONE_COPYABLE_NONE_MOVEABLE(ApiSample)

private:
    void Draw();

public:
    void Step() override { Draw(); };

    bool Setup() override;

    void Release() override;
};