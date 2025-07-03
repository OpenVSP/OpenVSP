// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/ParticleController.h>
#include <random>

namespace gte
{
    class BloodCellController : public ParticleController
    {
    public:
        BloodCellController(std::shared_ptr<Camera> const&, BufferUpdater const& postUpdate);

    protected:
        virtual void UpdatePointMotion(float ctrlTime) override;

        std::default_random_engine mDRE;
        std::uniform_real_distribution<float> mURD;
    };
}
