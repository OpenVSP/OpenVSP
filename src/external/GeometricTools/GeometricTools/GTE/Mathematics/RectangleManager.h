// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/IntrAlignedBox2AlignedBox2.h>
#include <Mathematics/EdgeKey.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <set>
#include <vector>

namespace gte
{
    template <typename Real>
    class RectangleManager
    {
    public:
        // Construction.
        RectangleManager(std::vector<AlignedBox2<Real>>& rectangles)
            :
            mRectangles(rectangles)
        {
            Initialize();
        }

        // No default construction, copy construction, or assignment are
        // allowed.
        RectangleManager() = delete;
        RectangleManager(RectangleManager const&) = delete;
        RectangleManager& operator=(RectangleManager const&) = delete;

        // This function is called by the constructor and does the
        // sort-and-sweep to initialize the update system.  However, if you
        // add or remove items from the array of rectangles after the
        // constructor call, you will need to call this function once before
        // you start the multiple calls of the update function.
        void Initialize()
        {
            // Get the rectangle endpoints.
            int32_t intrSize = static_cast<int32_t>(mRectangles.size()), endpSize = 2 * intrSize;
            mXEndpoints.resize(endpSize);
            mYEndpoints.resize(endpSize);
            for (int32_t i = 0, j = 0; i < intrSize; ++i)
            {
                mXEndpoints[j].type = 0;
                mXEndpoints[j].value = mRectangles[i].min[0];
                mXEndpoints[j].index = i;
                mYEndpoints[j].type = 0;
                mYEndpoints[j].value = mRectangles[i].min[1];
                mYEndpoints[j].index = i;
                ++j;

                mXEndpoints[j].type = 1;
                mXEndpoints[j].value = mRectangles[i].max[0];
                mXEndpoints[j].index = i;
                mYEndpoints[j].type = 1;
                mYEndpoints[j].value = mRectangles[i].max[1];
                mYEndpoints[j].index = i;
                ++j;
            }

            // Sort the rectangle endpoints.
            std::sort(mXEndpoints.begin(), mXEndpoints.end());
            std::sort(mYEndpoints.begin(), mYEndpoints.end());

            // Create the interval-to-endpoint lookup tables.
            mXLookup.resize(endpSize);
            mYLookup.resize(endpSize);
            for (int32_t j = 0; j < endpSize; ++j)
            {
                mXLookup[2 * static_cast<size_t>(mXEndpoints[j].index) + static_cast<size_t>(mXEndpoints[j].type)] = j;
                mYLookup[2 * static_cast<size_t>(mYEndpoints[j].index) + static_cast<size_t>(mYEndpoints[j].type)] = j;
            }

            // Active set of rectangles (stored by index in array).
            std::set<int32_t> active;

            // Set of overlapping rectangles (stored by pairs of indices in
            // array).
            mOverlap.clear();

            // Sweep through the endpoints to determine overlapping
            // x-intervals.
            for (int32_t i = 0; i < endpSize; ++i)
            {
                Endpoint const& endpoint = mXEndpoints[i];
                int32_t index = endpoint.index;
                if (endpoint.type == 0)  // an interval 'begin' value
                {
                    // In the 1D problem, the current interval overlaps with
                    // all the active intervals.  In 2D we also need to check
                    // for y-overlap.
                    for (auto activeIndex : active)
                    {
                        // Rectangles activeIndex and index overlap in the
                        // x-dimension.  Test for overlap in the y-dimension.
                        AlignedBox2<Real> const& r0 = mRectangles[activeIndex];
                        AlignedBox2<Real> const& r1 = mRectangles[index];
                        if (r0.max[1] >= r1.min[1] && r0.min[1] <= r1.max[1])
                        {
                            if (activeIndex < index)
                            {
                                mOverlap.insert(EdgeKey<false>(activeIndex, index));
                            }
                            else
                            {
                                mOverlap.insert(EdgeKey<false>(index, activeIndex));
                            }
                        }
                    }
                    active.insert(index);
                }
                else  // an interval 'end' value
                {
                    active.erase(index);
                }
            }
        }

        // After the system is initialized, you can move the rectangles using
        // this function.  It is not enough to modify the input array of
        // rectangles because the endpoint values stored internally by this
        // class must also change.  You can also retrieve the current
        // rectangles information.
        void SetRectangle(int32_t i, AlignedBox2<Real> const& rectangle)
        {
            size_t szI = static_cast<size_t>(i);
            mRectangles[szI] = rectangle;
            mXEndpoints[mXLookup[2 * szI]].value = rectangle.min[0];
            mXEndpoints[mXLookup[2 * szI + 1]].value = rectangle.max[0];
            mYEndpoints[mYLookup[2 * szI]].value = rectangle.min[1];
            mYEndpoints[mYLookup[2 * szI + 1]].value = rectangle.max[1];
        }

        inline void GetRectangle(int32_t i, AlignedBox2<Real>& rectangle) const
        {
            rectangle = mRectangles[i];
        }

        // When you are finished moving rectangles, call this function to
        // determine the overlapping rectangles.  An incremental update is
        // applied to determine the new set of overlapping rectangles.
        void Update()
        {
            InsertionSort(mXEndpoints, mXLookup);
            InsertionSort(mYEndpoints, mYLookup);
        }

        // If (i,j) is in the overlap set, then rectangle i and rectangle j
        // are overlapping.  The indices are those for the the input array.
        // The set elements (i,j) are stored so that i < j.
        inline std::set<EdgeKey<false>> const& GetOverlap() const
        {
            return mOverlap;
        }

    private:
        class Endpoint
        {
        public:
            Real value; // endpoint value
            int32_t type;   // '0' if interval min, '1' if interval max.
            int32_t index;  // index of interval containing this endpoint

            // Support for sorting of endpoints.
            bool operator<(Endpoint const& endpoint) const
            {
                if (value < endpoint.value)
                {
                    return true;
                }
                if (value > endpoint.value)
                {
                    return false;
                }
                return type < endpoint.type;
            }
        };

        void InsertionSort(std::vector<Endpoint>& endpoint, std::vector<int32_t>& lookup)
        {
            // Apply an insertion sort.  Under the assumption that the
            // rectangles have not changed much since the last call, the
            // endpoints are nearly sorted.  The insertion sort should be very
            // fast in this case.

            TIQuery<Real, AlignedBox2<Real>, AlignedBox2<Real>> query;
            int32_t endpSize = static_cast<int32_t>(endpoint.size());
            for (int32_t j = 1; j < endpSize; ++j)
            {
                Endpoint key = endpoint[j];
                int32_t i = j - 1;
                while (i >= 0 && key < endpoint[i])
                {
                    Endpoint e0 = endpoint[i];
                    Endpoint e1 = endpoint[static_cast<size_t>(i) + 1];

                    // Update the overlap status.
                    if (e0.type == 0)
                    {
                        if (e1.type == 1)
                        {
                            // The 'b' of interval E0.index was smaller than
                            // the 'e' of interval E1.index, and the intervals
                            // *might have been* overlapping.  Now 'b' and 'e'
                            // are swapped, and the intervals cannot overlap.
                            // Remove the pair from the overlap set.  The
                            // removal operation needs to find the pair and
                            // erase it if it exists.  Finding the pair is the
                            // expensive part of the operation, so there is no
                            // real time savings in testing for existence
                            // first, then deleting if it does.
                            mOverlap.erase(EdgeKey<false>(e0.index, e1.index));
                        }
                    }
                    else
                    {
                        if (e1.type == 0)
                        {
                            // The 'b' of interval E1.index was larger than
                            // the 'e' of interval E0.index, and the intervals
                            // were not overlapping.  Now 'b' and 'e' are
                            // swapped, and the intervals *might be*
                            // overlapping.  Determine if they are overlapping
                            // and then insert.
                            if (query(mRectangles[e0.index], mRectangles[e1.index]).intersect)
                            {
                                mOverlap.insert(EdgeKey<false>(e0.index, e1.index));
                            }
                        }
                    }

                    // Reorder the items to maintain the sorted list.
                    endpoint[i] = e1;
                    endpoint[static_cast<size_t>(i) + 1] = e0;
                    lookup[2 * static_cast<size_t>(e1.index) + static_cast<size_t>(e1.type)] = i;
                    lookup[2 * static_cast<size_t>(e0.index) + static_cast<size_t>(e0.type)] = i + 1;
                    --i;
                }
                endpoint[static_cast<size_t>(i) + 1] = key;
                lookup[2 * static_cast<size_t>(key.index) + static_cast<size_t>(key.type)] = i + 1;
            }
        }

        std::vector<AlignedBox2<Real>>& mRectangles;
        std::vector<Endpoint> mXEndpoints, mYEndpoints;
        std::set<EdgeKey<false>> mOverlap;

        // The intervals are indexed 0 <= i < n.  The endpoint array has 2*n
        // entries.  The original 2*n interval values are ordered as
        //   b[0], e[0], b[1], e[1], ..., b[n-1], e[n-1]
        // When the endpoint array is sorted, the mapping between interval
        // values and endpoints is lost.  In order to modify interval values
        // that are stored in the endpoint array, we need to maintain the
        // mapping.  This is done by the following lookup table of 2*n
        // entries.  The value mLookup[2*i] is the index of b[i] in the
        // endpoint array.  The value mLookup[2*i+1] is the index of e[i]
        // in the endpoint array.
        std::vector<int32_t> mXLookup, mYLookup;
    };
}
