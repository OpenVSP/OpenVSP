// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.12.04

#pragma once

// The ETManifoldMesh class represents an edge-triangle manifold mesh. The 'E'
// stands for edge and the 'T' stands for triangle. It is general purpose,
// allowing insertion and removal of triangles at any time. However, the
// performance is limited because of the use of C++ container classes
// (unordered sets and maps). If your application requires an edge-triangle
// manifold mesh for which no triangles will be removed, a better choice is
// StaticVETManifoldMesh.

#include <Mathematics/Logger.h>
#include <Mathematics/EdgeKey.h>
#include <Mathematics/HashCombine.h>
#include <Mathematics/TriangleKey.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gte
{
    class ETManifoldMesh
    {
    public:
        // Edge data types.
        class Edge;
        using ECreator = std::unique_ptr<Edge>(*)(int32_t, int32_t);
        using EMap = std::unordered_map<EdgeKey<false>, std::unique_ptr<Edge>,
            EdgeKey<false>, EdgeKey<false>>;

        // Triangle data types.
        class Triangle;
        using TCreator = std::unique_ptr<Triangle>(*)(int32_t, int32_t, int32_t);
        using TMap = std::unordered_map<TriangleKey<true>, std::unique_ptr<Triangle>,
            TriangleKey<true>, TriangleKey<true>>;

        // Edge object.
        class Edge
        {
        public:
            virtual ~Edge() = default;

            Edge(int32_t v0, int32_t v1)
                :
                V{ v0, v1 },
                T{ nullptr, nullptr }
            {
            }

            // Vertices of the edge.
            std::array<int32_t, 2> V;

            // Triangles sharing the edge.
            std::array<Triangle*, 2> T;
        };

        // Triangle object.
        class Triangle
        {
        public:
            virtual ~Triangle() = default;

            Triangle(int32_t v0, int32_t v1, int32_t v2)
                :
                V{ v0, v1, v2 },
                E{ nullptr, nullptr, nullptr },
                T{ nullptr, nullptr, nullptr }
            {
            }

            // The edge <u0,u1> is directed. Determine whether the triangle
            // has an edge <V[i],V[(i+1)%3]> = <u0,u1> (return +1) or an edge
            // <V[i],V[(i+1)%3]> = <u1,u0> (return -1) or does not have an
            // edge meeting either condition (return 0).
            int32_t WhichSideOfEdge(int32_t u0, int32_t u1) const
            {
                for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
                {
                    if (V[i0] == u0 && V[i1] == u1)
                    {
                        return +1;
                    }
                    if (V[i0] == u1 && V[i1] == u0)
                    {
                        return -1;
                    }
                }
                return 0;
            }

            Triangle* GetAdjacentOfEdge(int32_t u0, int32_t u1)
            {
                for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
                {
                    if ((V[i0] == u0 && V[i1] == u1) || (V[i0] == u1 && V[i1] == u0))
                    {
                        return T[i0];
                    }
                }
                return nullptr;
            }

            bool GetOppositeVertexOfEdge(int32_t u0, int32_t u1, int32_t& uOpposite)
            {
                for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
                {
                    if ((V[i0] == u0 && V[i1] == u1) || (V[i0] == u1 && V[i1] == u0))
                    {
                        uOpposite = V[(i1 + 1) % 3];
                        return true;
                    }
                }
                return false;
            }

            // Vertices, listed in counterclockwise order (V[0],V[1],V[2]).
            std::array<int32_t, 3> V;

            // Adjacent edges. E[i] points to edge (V[i],V[(i+1)%3]).
            std::array<Edge*, 3> E;

            // Adjacent triangles. T[i] points to the adjacent triangle
            // sharing edge E[i].
            std::array<Triangle*, 3> T;
        };


        // Construction and destruction.
        virtual ~ETManifoldMesh() = default;

        ETManifoldMesh(ECreator eCreator = nullptr, TCreator tCreator = nullptr)
            :
            mECreator(eCreator ? eCreator : CreateEdge),
            mEMap{},
            mTCreator(tCreator ? tCreator : CreateTriangle),
            mTMap{},
            mThrowOnNonmanifoldInsertion(true)
        {
        }

        // Support for a deep copy of the mesh. The mEMap and mTMap objects
        // have dynamically allocated memory for edges and triangles. A
        // shallow copy of the pointers isn't possible with unique_ptr.
        ETManifoldMesh(ETManifoldMesh const& mesh)
            :
            ETManifoldMesh()
        {
            *this = mesh;
        }

        ETManifoldMesh& operator=(ETManifoldMesh const& mesh)
        {
            Clear();

            mECreator = mesh.mECreator;
            mTCreator = mesh.mTCreator;
            mThrowOnNonmanifoldInsertion = mesh.mThrowOnNonmanifoldInsertion;
            for (auto const& element : mesh.mTMap)
            {
                // The typecast avoids warnings about not storing the return
                // value in a named variable. The return value is discarded.
                (void)Insert(element.first.V[0], element.first.V[1], element.first.V[2]);
            }

            return *this;
        }

        // Member access.
        inline EMap const& GetEdges() const
        {
            return mEMap;
        }

        inline TMap const& GetTriangles() const
        {
            return mTMap;
        }

        // If the insertion of a triangle fails because the mesh would become
        // nonmanifold, the default behavior is to throw an exception. You
        // can disable this behavior and continue gracefully without an
        // exception. The return value is the previous value of the internal
        // state mAssertOnNonmanifoldInsertion.
        bool ThrowOnNonmanifoldInsertion(bool doException)
        {
            std::swap(doException, mThrowOnNonmanifoldInsertion);
            return doException;  // return the previous state
        }

        // If <v0,v1,v2> is not in the mesh, a Triangle object is created and
        // returned; otherwise, <v0,v1,v2> is in the mesh and nullptr is
        // returned. If the insertion leads to a nonmanifold mesh, the call
        // fails with a nullptr returned.
        virtual Triangle* Insert(int32_t v0, int32_t v1, int32_t v2)
        {
            TriangleKey<true> tkey(v0, v1, v2);
            if (mTMap.find(tkey) != mTMap.end())
            {
                // The triangle already exists. Return a null pointer as a
                // signal to the caller that the insertion failed.
                return nullptr;
            }

            // Create the new triangle. It will be added to mTMap at the end
            // of the function so that if an assertion is triggered and the
            // function returns early, the (bad) triangle will not be part of
            // the mesh.
            std::unique_ptr<Triangle> newTri = mTCreator(v0, v1, v2);
            Triangle* tri = newTri.get();

            // Add the edges to the mesh if they do not already exist.
            for (int32_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                EdgeKey<false> ekey(tri->V[i0], tri->V[i1]);
                Edge* edge = nullptr;
                auto eiter = mEMap.find(ekey);
                if (eiter == mEMap.end())
                {
                    // This is the first time the edge is encountered.
                    std::unique_ptr<Edge> newEdge = mECreator(tri->V[i0], tri->V[i1]);
                    edge = newEdge.get();
                    mEMap[ekey] = std::move(newEdge);

                    // Update the edge and triangle.
                    edge->T[0] = tri;
                    tri->E[i0] = edge;
                }
                else
                {
                    // This is the second time the edge is encountered.
                    edge = eiter->second.get();
                    LogAssert(edge != nullptr, "Unexpected condition.");

                    if (mThrowOnNonmanifoldInsertion)
                    {
                        // tri and edge->T[0] must have a shared edge
                        // (tri->V[i0],tri->V[i1]). For tri, the directed
                        // edge is <tri->V[i0],tri->V[i1]>. For edge->T[0],
                        // the directed edge must be <tri->V[i1],tri->V[i0]>.
                        for (size_t j = 0; j < 3; ++j)
                        {
                            if (edge->T[0]->V[j] == tri->V[i0])
                            {
                                LogAssert(
                                    edge->T[0]->V[(j + 2) % 3] == tri->V[i1],
                                    "Attempt to create nonmanifold mesh.");
                            }
                        }
                    }

                    // Update the edge.
                    if (edge->T[1])
                    {
                        if (mThrowOnNonmanifoldInsertion)
                        {
                            LogError("Attempt to create nonmanifold mesh.");
                        }
                        else
                        {
                            return nullptr;
                        }
                    }
                    edge->T[1] = tri;

                    // Update the adjacent triangles.
                    auto adjacent = edge->T[0];
                    LogAssert(adjacent != nullptr, "Unexpected condition.");
                    for (int32_t j = 0; j < 3; ++j)
                    {
                        if (adjacent->E[j] == edge)
                        {
                            adjacent->T[j] = tri;
                            break;
                        }
                    }

                    // Update the triangle.
                    tri->E[i0] = edge;
                    tri->T[i0] = adjacent;
                }
            }

            mTMap[tkey] = std::move(newTri);
            return tri;
        }

        // If <v0,v1,v2> is in the mesh, it is removed and 'true' is
        // returned; otherwise, <v0,v1,v2> is not in the mesh and 'false' is
        // returned.
        virtual bool Remove(int32_t v0, int32_t v1, int32_t v2)
        {
            TriangleKey<true> tkey(v0, v1, v2);
            auto titer = mTMap.find(tkey);
            if (titer == mTMap.end())
            {
                // The triangle does not exist.
                return false;
            }

            // Get the triangle.
            Triangle* tri = titer->second.get();

            // Remove the edges and update adjacent triangles if necessary.
            for (int32_t i = 0; i < 3; ++i)
            {
                // Inform the edges the triangle is being deleted.
                auto edge = tri->E[i];
                LogAssert(edge != nullptr, "Unexpected condition.");

                if (edge->T[0] == tri)
                {
                    // One-triangle edges always have pointer at index zero.
                    edge->T[0] = edge->T[1];
                    edge->T[1] = nullptr;
                }
                else if (edge->T[1] == tri)
                {
                    edge->T[1] = nullptr;
                }
                else
                {
                    LogError("Unexpected condition.");
                }

                // Remove the edge if you have the last reference to it.
                if (!edge->T[0] && !edge->T[1])
                {
                    EdgeKey<false> ekey(edge->V[0], edge->V[1]);
                    mEMap.erase(ekey);
                }

                // Inform adjacent triangles the triangle is being deleted.
                auto adjacent = tri->T[i];
                if (adjacent)
                {
                    for (int32_t j = 0; j < 3; ++j)
                    {
                        if (adjacent->T[j] == tri)
                        {
                            adjacent->T[j] = nullptr;
                            break;
                        }
                    }
                }
            }

            mTMap.erase(tkey);
            return true;
        }

        // Destroy the edges and triangles to obtain an empty mesh.
        virtual void Clear()
        {
            mEMap.clear();
            mTMap.clear();
        }

        // A manifold mesh is closed if each edge is shared twice. A closed
        // mesh is not necessarily oriented. For example, you could have a
        // mesh with spherical topology. The upper hemisphere has outer
        // facing normals and the lower hemisphere has inner-facing normals.
        // The discontinuity in orientation occurs on the circle shared by the
        // hemispheres.
        bool IsClosed() const
        {
            for (auto const& element : mEMap)
            {
                Edge* edge = element.second.get();
                if (!edge->T[0] || !edge->T[1])
                {
                    return false;
                }
            }
            return true;
        }

        // Test whether all triangles in the mesh are oriented consistently
        // and that no two triangles are coincident. The latter means that
        // you cannot have both triangles <v0,v1,v2> and <v0,v2,v1> in the
        // mesh to be considered oriented.
        bool IsOriented() const
        {
            for (auto const& element : mEMap)
            {
                Edge* edge = element.second.get();
                if (edge->T[0] && edge->T[1])
                {
                    // In each triangle, find the ordered edge that
                    // corresponds to the unordered edge element.first. Also
                    // find the vertex opposite that edge.
                    bool edgePositive[2] = { false, false };
                    int32_t vOpposite[2] = { -1, -1 };
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        auto tri = edge->T[j];
                        for (int32_t i = 0; i < 3; ++i)
                        {
                            size_t szI = static_cast<size_t>(i);
                            if (tri->V[i] == element.first.V[0])
                            {
                                int32_t vNext = tri->V[(szI + 1) % 3];
                                if (vNext == element.first.V[1])
                                {
                                    edgePositive[j] = true;
                                    vOpposite[j] = tri->V[(szI + 2) % 3];
                                }
                                else
                                {
                                    edgePositive[j] = false;
                                    vOpposite[j] = vNext;
                                }
                                break;
                            }
                        }
                    }

                    // To be oriented consistently, the edges must have
                    // reversed ordering and the oppositive vertices cannot
                    // match.
                    if (edgePositive[0] == edgePositive[1] || vOpposite[0] == vOpposite[1])
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        // Compute the connected components of the edge-triangle graph that
        // the mesh represents. The first function returns pointers into
        // 'this' object's containers, so you must consume the components
        // before clearing or destroying 'this'. The second function returns
        // triangle keys, which requires three times as much storage as the
        // pointers but allows you to clear or destroy 'this' before consuming
        // the components.
        void GetComponents(std::vector<std::vector<Triangle*>>& components) const
        {
            components.clear();

            // visited: 0 (unvisited), 1 (discovered), 2 (finished)
            TrianglePtrIntMap visited{};
            for (auto const& element : mTMap)
            {
                visited.insert(std::make_pair(element.second.get(), 0));
            }

            for (auto& element : mTMap)
            {
                Triangle* tri = element.second.get();
                if (visited[tri] == 0)
                {
                    std::vector<Triangle*> component{};
                    DepthFirstSearch(tri, visited, component);
                    components.push_back(std::move(component));
                }
            }
        }

        void GetComponents(std::vector<std::vector<TriangleKey<true>>>& components) const
        {
            components.clear();

            // visited: 0 (unvisited), 1 (discovered), 2 (finished)
            TrianglePtrIntMap visited{};
            for (auto const& element : mTMap)
            {
                visited.insert(std::make_pair(element.second.get(), 0));
            }

            for (auto& element : mTMap)
            {
                Triangle* tri = element.second.get();
                if (visited[tri] == 0)
                {
                    std::vector<Triangle*> component{};
                    DepthFirstSearch(tri, visited, component);

                    std::vector<TriangleKey<true>> keyComponent{};
                    keyComponent.reserve(component.size());
                    for (auto const& t : component)
                    {
                        keyComponent.push_back(TriangleKey<true>(t->V[0], t->V[1], t->V[2]));
                    }
                    components.push_back(std::move(keyComponent));
                }
            }
        }

        // Create a compact edge-triangle graph. The vertex indices are those
        // integers passed to an Insert(...) call. These have no meaning to
        // the semantics of maintaining an edge-triangle manifold mesh, so
        // this class makes no assumption about them. The vertex indices do
        // not necessarily start at 0 and they are not necessarily contiguous
        // numbers. The triangles are represented by triples of vertex
        // indices. The compact graph stores these in an array of N triples,
        // say,
        //   T[0] = (v0,v1,v2), T[1] = (v3,v4,v5), ...
        // Each triangle has up to 3 adjacent triangles. The compact graph
        // stores the adjacency information in an array of N triples, say,
        // and the indices of adjacent triangle are stored in an array of Nt
        //   A[0] = (t0,t1,t2), A[1] = (t3,t4,t5), ...
        // where the ti are indices into the array of triangles. For example,
        // the triangle T[0] has edges E[0] = (v0,v1), E[1] = (v1,v2) and
        // E[2] = (v2,v0). The edge E[0] has adjacent triangle T[0]. If E[0]
        // has another adjacent triangle, it is T[A[0][0]. If it does not have
        // another adjacent triangle, then A[0][0] = -1 (represented by
        // std::numeric_limits<size_t>::max()). Similar assignments are made
        // for the other two edges which produces A[0][1] for E[1] and
        // A[0][2] for E[2].
        void CreateCompactGraph(
            std::vector<std::array<size_t, 3>>& triangles,
            std::vector<std::array<size_t, 3>>& adjacents) const
        {
            size_t const numTriangles = mTMap.size();
            LogAssert(numTriangles > 0, "Invalid input.");

            triangles.resize(numTriangles);
            adjacents.resize(numTriangles);

            TrianglePtrSizeTMap triIndexMap;
            triIndexMap.insert(std::make_pair(nullptr, std::numeric_limits<size_t>::max()));
            size_t index = 0;
            for (auto const& element : mTMap)
            {
                triIndexMap.insert(std::make_pair(element.second.get(), index++));
            }

            index = 0;
            for (auto const& element : mTMap)
            {
                auto const& triPtr = element.second;
                auto& tri = triangles[index];
                auto& adj = adjacents[index];
                for (size_t j = 0; j < 3; ++j)
                {
                    tri[j] = triPtr->V[j];
                    adj[j] = triIndexMap[triPtr->T[j]];
                }
                ++index;
            }
        }

        // The output of CreateCompactGraph can be used to compute the
        // connected components of the graph, each component having
        // triangles with the same chirality (winding order). Using only
        // the mesh topology, it is not possible to ensure that the
        // chirality is the same for all the components. Additional
        // application-specific geometric information is required.
        //
        // The 'components' contain indices into the 'triangles' array
        // and is partitioned into C subarrays, each representing a
        // connected component. The lengths of the subarrays are
        // stored in 'numComponentTriangles[]'. The number of elements
        // of this array is C. It is the case that the number of triangles
        // in the mesh is sum_{i=0}^{C-1} numComponentTriangles[i].
        //
        // On return, the 'triangles' and 'adjacents' have been modified
        // and have the correct chirality.
        static void GetComponentsConsistentChirality(
            std::vector<std::array<size_t, 3>>& triangles,
            std::vector<std::array<size_t, 3>>& adjacents,
            std::vector<size_t>& components,
            std::vector<size_t>& numComponentTriangles)
        {
            LogAssert(
                triangles.size() > 0 && triangles.size() == adjacents.size(),
                "Invalid inputs.");

            // Use a breadth-first search to process the chirality of the
            // triangles. Keep track of the connected components.
            size_t const numTriangles = triangles.size();
            std::vector<bool> visited(numTriangles, false);
            components.reserve(numTriangles);
            components.clear();

            // The 'firstUnvisited' index is the that of the first triangle to
            // process in a connected component of the mesh.
            for (;;)
            {
                // Let n[i] be the number of elements of the i-th connected
                // component. Let C be the number of components. During the
                // execution of this loop, the array numComponentTriangles
                // stores
                //   {0, n[0], n[0]+n[1], ..., n[0]+...+n[C-1]=numTriangles}
                // At the end of this function, the array is modified to
                //   {n[0], n[1], ..., n[C-1]}
                numComponentTriangles.push_back(components.size());

                // Find the starting index of a connected component.
                size_t firstUnvisited = numTriangles;
                for (size_t i = 0; i < numTriangles; ++i)
                {
                    if (!visited[i])
                    {
                        firstUnvisited = i;
                        break;
                    }
                }
                if (firstUnvisited == numTriangles)
                {
                    // All connected components have been found.
                    break;
                }

                // Initialize the queue to start at the first unvisited
                // triangle of a connected component.
                std::queue<size_t> triQueue;
                triQueue.push(firstUnvisited);
                visited[firstUnvisited] = true;
                components.push_back(firstUnvisited);

                // Perform the breadth-first search.
                while (!triQueue.empty())
                {
                    size_t curIndex = triQueue.front();
                    triQueue.pop();

                    auto const& curTriangle = triangles[curIndex];
                    for (size_t i0 = 0; i0 < 3; ++i0)
                    {
                        size_t adjIndex = adjacents[curIndex][i0];
                        if (adjIndex != std::numeric_limits<size_t>::max() && !visited[adjIndex])
                        {
                            // The current-triangle has a directed edge
                            // <curTriangle[i0],curTriangle[i1]> and there is
                            // a triangle adjacent to it.
                            size_t i1 = (i0 + 1) % 3;
                            auto& adjTriangle = triangles[adjIndex];
                            size_t tv0 = curTriangle[i0];
                            size_t tv1 = curTriangle[i1];

                            // To have the same chirality, it is required
                            // that the adjacent triangle have the directed
                            // edge <curTriangle[i1],curTriangle[i0]>
                            bool sameChirality = true;
                            size_t j0{}, j1{};
                            for (j0 = 0; j0 < 3; ++j0)
                            {
                                j1 = (j0 + 1) % 3;
                                if (adjTriangle[j0] == tv0)
                                {
                                    if (adjTriangle[j1] == tv1)
                                    {
                                        // The adjacent triangle has the same
                                        // directed edge as the current
                                        // triangle, so the chiralities do
                                        // not match.
                                        sameChirality = false;
                                    }
                                    break;
                                }
                            }
                            LogAssert(j0 < 3, "Unexpected condition.");

                            if (!sameChirality)
                            {
                                // Swap the vertices of the adjacent triangle
                                // that form the shared directed edge of the
                                // current triangle. This requires that the
                                // adjacency information for the other two
                                // edges of the adjacent triangle be swapped.
                                auto& adjAdjacent = adjacents[adjIndex];
                                size_t j2 = (j1 + 1) % 3;
                                std::swap(adjTriangle[j0], adjTriangle[j1]);
                                std::swap(adjAdjacent[j1], adjAdjacent[j2]);
                            }

                            // The adjacent triangle has been processed, but
                            // it might have neighbors that need to be
                            // processed. Push the adjacent triangle into the
                            // queue to ensure this happens. Insert the
                            // adjacent triangle into the active connected
                            // component.
                            triQueue.push(adjIndex);
                            visited[adjIndex] = true;
                            components.push_back(adjIndex);
                        }
                    }
                }
            }

            // Read the comments at the beginning of this function.
            size_t const numSizes = numComponentTriangles.size();
            LogAssert(numSizes > 1, "Unexpected condition.");
            for (size_t i0 = 0, i1 = 1; i1 < numComponentTriangles.size(); i0 = i1++)
            {
                numComponentTriangles[i0] = numComponentTriangles[i1] - numComponentTriangles[i0];
            }
            numComponentTriangles.resize(numSizes - 1);
        }

        // This is a simple wrapper around CreateCompactGraph(...) and
        // GetComponentsConsistentChirality(...), in particular when you
        // do not need to work directly with the connected components.
        // The mesh is reconstructed, because the bookkeeping details of
        // trying to modify the mesh in-place are horrendous. NOTE: If
        // your mesh has more than 1 connected component, you should read
        // the comments for GetComponentsConsistentChirality(...) about
        // the potential for different chiralities between components.
        virtual void MakeConsistentChirality()
        {
            std::vector<std::array<size_t, 3>> triangles{};
            std::vector<std::array<size_t, 3>> adjacents{};
            CreateCompactGraph(triangles, adjacents);

            std::vector<size_t> components{}, numComponentTriangles{};
            GetComponentsConsistentChirality(triangles, adjacents,
                components, numComponentTriangles);

            // Only the 'triangles' array is needed to reconstruct the
            // mesh. The other arrays are discarded.
            Clear();

            for (auto const& triangle : triangles)
            {
                int32_t v0 = static_cast<int32_t>(triangle[0]);
                int32_t v1 = static_cast<int32_t>(triangle[1]);
                int32_t v2 = static_cast<int32_t>(triangle[2]);

                // The typecast avoids warnings about not storing the return
                // value in a named variable. The return value is discarded.
                (void)Insert(v0, v1, v2);
            }
        }

        // Compute the boundary-edge components of the mesh. These are
        // polygons that are simple for the strict definition of manifold
        // mesh that disallows bow-tie configurations. The GTE mesh
        // implementations do allow bow-tie configurations, in which case
        // some polygons might not be simple. If you select duplicateEndpoints
        // to be false, a component has consecutive vertices
        // (v[0], v[1], ..., v[n-1]) and the polygon has edges
        // (v[0],v[1]), (v[1],v[2]), ..., (v[n-2],v[n-1]), (v[n-1],v[0]).
        // If duplicateEndpoints is set to true, a component has consecutive
        // vertices (v[0], v[1], ..., v[n-1], v[0]), emphasizing that the
        // component is closed.
        void GetBoundaryPolygons(std::vector<std::vector<int32_t>>& polygons,
            bool duplicateEndpoints) const
        {
            polygons.clear();

            // Get the boundary edges. The index into the Triangle::T[]
            // adjacency array is also stored to help with the traversal of
            // polygons.
            BoundaryEdgeMap boundaryEdges{};
            for (auto const& triangleElement : mTMap)
            {
                auto const* tri = triangleElement.second.get();
                for (size_t i = 0; i < 3; ++i)
                {
                    if (tri->T[i] == nullptr)
                    {
                        std::array<int32_t, 2> directed{ tri->V[i], tri->V[(i + 1) % 3] };
                        BoundaryEdge edge(tri, i, false);
                        boundaryEdges.insert(std::make_pair(directed, edge));
                    }
                }
            }

            for (auto const& initialEdge : boundaryEdges)
            {
                if (!initialEdge.second.visited)
                {
                    std::vector<int32_t> polygon{};
                    GetBoundaryPolygon(initialEdge.second.triangle, initialEdge.second.index,
                        boundaryEdges, polygon);
                    polygons.push_back(std::move(polygon));
                }
            }

            if (!duplicateEndpoints)
            {
                for (auto& polygon : polygons)
                {
                    polygon.resize(polygon.size() - 1);
                }
            }
        }

    protected:
        using TrianglePtrIntMap = std::unordered_map<Triangle*, int32_t>;
        using TrianglePtrSizeTMap = std::unordered_map<Triangle*, size_t>;

        struct BoundaryEdge
        {
            BoundaryEdge()
                :
                triangle(nullptr),
                index(std::numeric_limits<size_t>::max()),
                visited(false)
            {
            }

            BoundaryEdge(Triangle const* inTriangle, size_t inIndex, bool inVisited)
                :
                triangle(inTriangle),
                index(inIndex),
                visited(inVisited)
            {
            }

            Triangle const* triangle;
            size_t index;
            bool visited;
        };

        using BoundaryEdgeMap = std::map<std::array<int32_t, 2>, BoundaryEdge>;

        // The edge data and default edge creation.
        static std::unique_ptr<Edge> CreateEdge(int32_t v0, int32_t v1)
        {
            return std::make_unique<Edge>(v0, v1);
        }

        // The triangle data and default triangle creation.
        static std::unique_ptr<Triangle> CreateTriangle(int32_t v0, int32_t v1, int32_t v2)
        {
            return std::make_unique<Triangle>(v0, v1, v2);
        }

        // Support for computing connected components. This is a
        // straightforward depth-first search of the graph but uses a
        // preallocated stack rather than a recursive function that could
        // possibly overflow the call stack.
        void DepthFirstSearch(
            Triangle* tInitial,
            TrianglePtrIntMap& visited,
            std::vector<Triangle*>& component) const
        {
            // Allocate the maximum-size stack that can occur in the
            // depth-first search. The stack is empty when the index top
            // is -1.
            std::vector<Triangle*> tStack(mTMap.size());
            int32_t top = -1;
            tStack[++top] = tInitial;
            while (top >= 0)
            {
                Triangle* tri = tStack[top];
                visited[tri] = 1;
                int32_t i;
                for (i = 0; i < 3; ++i)
                {
                    Triangle* adj = tri->T[i];
                    if (adj && visited[adj] == 0)
                    {
                        tStack[++top] = adj;
                        break;
                    }
                }
                if (i == 3)
                {
                    visited[tri] = 2;
                    component.push_back(tri);
                    --top;
                }
            }
        }

        void GetBoundaryPolygon(
            Triangle const* initialTriangle,
            size_t initialIndex,
            BoundaryEdgeMap& boundaryEdges,
            std::vector<int32_t>& polygon) const
        {
            auto const* tri = initialTriangle;
            size_t i0 = initialIndex;
            size_t i1 = (i0 + 1) % 3;
            std::array<int32_t, 2> vEdge = { tri->V[i0], tri->V[i1] };
            polygon.push_back(vEdge[0]);
            while (!boundaryEdges[vEdge].visited)
            {
                polygon.push_back(vEdge[1]);
                boundaryEdges[vEdge].visited = true;

                // Traverse the triangle strip with vertex at vEdge[1] until
                // the last triangle is encountered. The final edge of the
                // last triangle is the next boundary edge and starts at
                // vEdge[1].
                std::set<Triangle const*> visited{};
                visited.insert(tri);
                while (tri->T[i1] != nullptr)
                {
                    tri = tri->T[i1];
                    auto result = visited.insert(tri);

                    // It this assertion is trigerred, try calling
                    // <mesh>.IsOriented() before calling GetBoundaryPolygons.
                    // If <mesh>.IsOriented() returns 'false', the call to
                    // GetBoundaryPolygons() will fail.
                    LogAssert(result.second, "Triangle already visited. Is the mesh orientable?");

                    size_t j{};
                    for (j = 0; j < 3; ++j)
                    {
                        if (vEdge[1] == tri->V[j])
                        {
                            i1 = j;
                            break;
                        }
                    }
                    LogAssert(j < 3, "Unexpected condition.");
                }

                size_t i2 = (i1 + 1) % 3;
                vEdge[0] = vEdge[1];
                // NOTE: Microsoft Visual Studio 2022 (17.4.4) generates
                // warning C28020 for the next line of code. The code analyzer
                // believes that i2 does not satisfy 0 <= i2 <= 2. This is
                // incorrect because i2 is an unsigned integer computed
                // modulo 3.
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(disable : 28020)
#endif
                vEdge[1] = tri->V[i2];
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
                i0 = i1;
                i1 = i2;
            }
        }

        ECreator mECreator;
        EMap mEMap;
        TCreator mTCreator;
        TMap mTMap;
        bool mThrowOnNonmanifoldInsertion;
    };
}
