/*
 *  Copyright 2009-2010 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
#pragma once
#include "BuilderMesh.hpp"
#include "3d/ConvexPolyhedron.hpp"

namespace FW
{
//------------------------------------------------------------------------

class ContourShaper
{
public:
                        ContourShaper       (void);
    virtual             ~ContourShaper      (void);

    virtual void        init                (const BuilderMeshAccessor* mesh, F32 voxelSize, F32 maxDeviation) = 0;
    virtual bool        needsNeighbors      (void) const = 0;

    virtual void        setVoxel            (const Vec3f&   voxelPos,
                                             int            numVoxelTris,
                                             const S32*     voxelTris,
                                             const S8*      numBary,
                                             const Vec2f*   bary,
                                             const S32*     dispIsect,
                                             int            numAuxContours,
                                             S32*           auxContours) = 0; // room for one more aux contour

    virtual void        addNeighbor         (int numVoxelTris, const S32* voxelTris) = 0;

    virtual S32         getContour          (void) = 0;
    virtual int         getNumAuxContours   (void) = 0;
    virtual bool        needToRefine        (void) = 0;

private:
                        ContourShaper       (ContourShaper&); // forbidden
    ContourShaper&      operator=           (ContourShaper&); // forbidden
};

//------------------------------------------------------------------------

class HullShaper : public ContourShaper
{
private:
    struct Plane
    {
        F32             weight;
        S32             encoded;
        Vec3f           normal;
        F32             length;
        Vec2f           bounds;
    };

public:
                        HullShaper          (void);
    virtual             ~HullShaper         (void);

    virtual void        init                (const BuilderMeshAccessor* mesh, F32 voxelSize, F32 maxDeviation);
    virtual bool        needsNeighbors      (void) const    { return false; }

    virtual void        setVoxel            (const Vec3f&   voxelPos,
                                             int            numVoxelTris,
                                             const S32*     voxelTris,
                                             const S8*      numBary,
                                             const Vec2f*   bary,
                                             const S32*     dispIsect,
                                             int            numAuxContours,
                                             S32*           auxContours);

    virtual void        addNeighbor         (int numVoxelTris, const S32* voxelTris) { FW_UNREF(numVoxelTris); FW_UNREF(voxelTris); }

    virtual S32         getContour          (void)          { return m_contour; }
    virtual int         getNumAuxContours   (void)          { return m_numAuxContours; }
    virtual bool        needToRefine        (void);

private:
    void                addPlane            (F32 weight, const Vec3f& normal);

private:
                        HullShaper          (HullShaper&); // forbidden
    HullShaper&         operator=           (HullShaper&); // forbidden

private:
    const BuilderMeshAccessor* m_mesh;
    F32                 m_voxelSize;
    F32                 m_voxelSizeRcp;
    F32                 m_devSqr;

    S32                 m_numVoxelTris;
    const S32*          m_voxelTris;
    const S32*          m_dispIsect;

    Vec3f               m_mid;
    bool                m_easyCase;
    bool                m_easyRefine;
    S32                 m_contour;
    S32                 m_numAuxContours;

    Array<Plane>        m_planes;
    Array<Vec3f>        m_dispNormals;
    ConvexPolyhedron    m_polyhedron;
    Array<Vec3f>        m_probes;
};

//------------------------------------------------------------------------
}
