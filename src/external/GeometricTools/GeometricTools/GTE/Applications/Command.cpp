// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Applications/GTApplicationsPCH.h>
#include <Applications/Command.h>
#include <Mathematics/Logger.h>
using namespace gte;

std::string const Command::msOptNotFound("Option not found.");
std::string const Command::msArgRequired("Option requires an argument.");
std::string const Command::msArgOutOfRange("Argument out of range.");
std::string const Command::msFilenameNotFound("Filename not found.");
std::string const Command::msDash("-");

Command::Command(int32_t numArguments, char const* arguments[])
    :
    mSmall(0.0),
    mLarge(0.0),
    mMinSet(false),
    mMaxSet(false),
    mInfSet(false),
    mSupSet(false),
    mLastError("")
{
    // The first argument is always the executable name.
    LogAssert(numArguments > 0, "Invalid number of arguments.");

    mArguments.resize(static_cast<size_t>(numArguments));
    mProcessed.resize(mArguments.size());
    for (size_t i = 0; i < mArguments.size(); ++i)
    {
        mArguments[i] = std::string(arguments[i]);
        mProcessed[i] = false;
    }

    // The executable name, which is considered to have been processed.
    mProcessed[0] = true;
}

Command::Command(int32_t numArguments, char* arguments[])
    :
    mSmall(0.0),
    mLarge(0.0),
    mMinSet(false),
    mMaxSet(false),
    mInfSet(false),
    mSupSet(false),
    mLastError("")
{
    // The first argument is always the executable name.
    LogAssert(numArguments > 0, "Invalid number of arguments.");

    mArguments.resize(static_cast<size_t>(numArguments));
    mProcessed.resize(mArguments.size());
    for (size_t i = 0; i < mArguments.size(); ++i)
    {
        mArguments[i] = std::string(arguments[i]);
        mProcessed[i] = false;
    }

    // The executable name, which is considered to have been processed.
    mProcessed[0] = true;
}

size_t Command::ExcessArguments() const
{
    // Check to see whether any command line arguments were not processed.
    for (size_t i = 1; i < mArguments.size(); ++i)
    {
        if (!mProcessed[i])
        {
            return i;
        }
    }
    return 0;
}

Command& Command::Min(double value)
{
    mSmall = value;
    mMinSet = true;
    return *this;
}

Command& Command::Max(double value)
{
    mLarge = value;
    mMaxSet = true;
    return *this;
}

Command& Command::Inf(double value)
{
    mSmall = value;
    mInfSet = true;
    return *this;
}

Command& Command::Sup(double value)
{
    mLarge = value;
    mSupSet = true;
    return *this;
}

size_t Command::GetBoolean(std::string const& name)
{
    bool value = false;
    return GetBoolean(name, value);
}

size_t Command::GetBoolean(std::string const& name, bool& value)
{
    size_t matchFound = 0;
    value = false;

    for (size_t i = 1; i < mArguments.size(); ++i)
    {
        std::string tmp = mArguments[i];
        if (!mProcessed[i] && mArguments[i] == (msDash + name))
        {
            mProcessed[i] = true;
            matchFound = i;
            value = true;
            break;
        }
    }

    if (matchFound == 0)
    {
        mLastError = msOptNotFound;
    }

    return matchFound;
}

size_t Command::GetInteger(std::string const& name, int32_t& value)
{
    size_t matchFound = 0;

    for (size_t i = 1; i < mArguments.size(); ++i)
    {
        if (!mProcessed[i] && mArguments[i] == (msDash + name))
        {
            std::string argument = mArguments[i + 1];
            if (mProcessed[i + 1] ||
                (argument[0] == '-' && !isdigit(static_cast<int32_t>(argument[1]))))
            {
                mLastError = msArgRequired;
                return 0;
            }

            value = atoi(argument.c_str());
            if ((mMinSet && value < mSmall) ||
                (mMaxSet && value > mLarge) ||
                (mInfSet && value <= mSmall) ||
                (mSupSet && value >= mLarge))
            {
                mLastError = msArgOutOfRange;
                return 0;
            }

            mProcessed[i] = true;
            mProcessed[i + 1] = true;
            matchFound = i;
            break;
        }
    }

    mMinSet = false;
    mMaxSet = false;
    mInfSet = false;
    mSupSet = false;

    if (matchFound == 0)
    {
        mLastError = msOptNotFound;
    }

    return matchFound;
}

size_t Command::GetFloat(std::string const& name, float& value)
{
    size_t matchFound = 0;

    for (size_t i = 1; i < mArguments.size(); ++i)
    {
        if (!mProcessed[i] && mArguments[i] == (msDash + name))
        {
            std::string argument = mArguments[i + 1];
            if (mProcessed[i + 1] ||
                (argument[0] == '-' && !isdigit(static_cast<int32_t>(argument[1]))))
            {
                mLastError = msArgRequired;
                return 0;
            }

            value = static_cast<float>(atof(argument.c_str()));
            if ((mMinSet && value < mSmall) ||
                (mMaxSet && value > mLarge) ||
                (mInfSet && value <= mSmall) ||
                (mSupSet && value >= mLarge))
            {
                mLastError = msArgOutOfRange;
                return 0;
            }

            mProcessed[i] = true;
            mProcessed[i + 1] = true;
            matchFound = i;
            break;
        }
    }

    mMinSet = false;
    mMaxSet = false;
    mInfSet = false;
    mSupSet = false;

    if (matchFound == 0)
    {
        mLastError = msOptNotFound;
    }

    return matchFound;
}

size_t Command::GetDouble(std::string const& name, double& value)
{
    size_t matchFound = 0;

    for (size_t i = 1; i < mArguments.size(); ++i)
    {
        if (!mProcessed[i] && mArguments[i] == (msDash + name))
        {
            std::string argument = mArguments[i + 1];
            if (mProcessed[i + 1] ||
                (argument[0] == '-' && !isdigit(static_cast<int32_t>(argument[1]))))
            {
                mLastError = msArgRequired;
                return 0;
            }

            value = atof(argument.c_str());
            if ((mMinSet && value < mSmall) ||
                (mMaxSet && value > mLarge) ||
                (mInfSet && value <= mSmall) ||
                (mSupSet && value >= mLarge))
            {
                mLastError = msArgOutOfRange;
                return 0;
            }

            mProcessed[i] = true;
            mProcessed[i + 1] = true;
            matchFound = i;
            break;
        }
    }

    mMinSet = false;
    mMaxSet = false;
    mInfSet = false;
    mSupSet = false;

    if (matchFound == 0)
    {
        mLastError = msOptNotFound;
    }

    return matchFound;
}

size_t Command::GetString(std::string const& name, std::string& value)
{
    size_t matchFound = 0;

    for (size_t i = 1; i < mArguments.size(); ++i)
    {
        if (!mProcessed[i] && mArguments[i] == (msDash + name))
        {
            std::string argument = mArguments[i + 1];
            if (mProcessed[i + 1] || argument[0] == '-')
            {
                mLastError = msArgRequired;
                return 0;
            }

            value = argument;
            mProcessed[i] = true;
            mProcessed[i + 1] = true;
            matchFound = i;
            break;
        }
    }

    if (matchFound == 0)
    {
        mLastError = msOptNotFound;
    }

    return matchFound;
}

size_t Command::GetFilename(std::string& value, size_t startArgIndex)
{
    size_t matchFound = 0;

    for (size_t i = startArgIndex; i < mArguments.size(); ++i)
    {
        std::string argument = mArguments[i];
        if (!mProcessed[i] && argument[0] != '-')
        {
            value = argument;
            mProcessed[i] = true;
            matchFound = i;
            break;
        }
    }

    if (matchFound == 0)
    {
        mLastError = msFilenameNotFound;
    }

    return matchFound;
}
