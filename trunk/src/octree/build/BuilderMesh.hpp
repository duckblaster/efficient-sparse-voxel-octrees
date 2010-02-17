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
#include "base/Math.hpp"
#include "base/Thread.hpp"
#include "3d/Mesh.hpp"
#include "AttribFilter.hpp"
#include "TextureSampler.hpp"
#include "DisplacementMap.hpp"

namespace FW
{
//------------------------------------------------------------------------

#define FW_MIN_ATTRIB_WEIGHT    1.0e-8f

//------------------------------------------------------------------------

class BuilderMesh
{
public:
    enum
    {
        MaxTriangleBatchSize        = 1024,
        MaxThreads                  = 32,       // cannot be more than 32, used in a bit mask
        MaxLockedBatchesPerThread   = 128,
        MaxExpandedBatches          = 1024
    };

    struct Triangle
    {
        Vec3f               p;          // v[0].pos
        Vec3f               pu;         // (v[1].pos - v[0].pos)
        Vec3f               pv;         // (v[2].pos - v[0].pos)
        Vec3f               plo;        // min(v[0].pos, v[1].pos, v[2].pos)
        Vec3f               phi;        // max(v[0].pos, v[1].pos, v[2].pos)

        Vec3f               n;          // v[0].normal
        Vec3f               nu;         // v[1].normal - v[0].normal
        Vec3f               nv;         // v[2].normal - v[0].normal
        Vec3f               nlo;        // min(v[0].normal, v[1].normal, v[2].normal)
        Vec3f               nhi;        // max(v[0].normal, v[1].normal, v[2].normal)

        Vec2f               t;
        Vec2f               tu;
        Vec2f               tv;

        Vec4f               color;      // material diffuse
        Vec3f               geomNormal; // cross(nu, nv)
        Vec3f               avgNormal;
        F32                 area;
        AttribFilter::Value average;
        U8                  boundaryMask; // bitmask of 3 edges

        TextureSampler*     colorTex;
        TextureSampler*     alphaTex;
        DisplacedTriangle*  dispTri;
    };

private:
    struct Batch
    {
        bool                        expanded;
        int                         firstTri;
        int                         numTris;
        int                         numDispTris;
        Array<Triangle>             tris;
        Array<DisplacedTriangle>    dispTris;
        Batch*                      nextLRU;
        Batch*                      prevLRU;
        U32                         lockMask;
    };

    void                    lockBatch           (int tid, Batch* batch);
    void                    unlockBatch         (int tid);
    const Triangle&         getTriProper        (Batch* batch, int i, int tid);

public:
                            BuilderMesh         (const MeshBase* foreignMesh); // acquires ownership of foreignMesh
                            ~BuilderMesh        (void);

    const Mat4f&            getOctreeToObject   (void) const    { return m_octreeToObject; }
    int                     getBitsPerTri       (void) const    { return m_bitsPerTri; }

    int                     getNumTris          (void) const    { return m_numTris; }
    void                    freeThreadSlot      (int tid);
    const Triangle&         getTri              (int i, int tid)
    {
        const TriangleEntry& te = m_triMap[i];
        Batch* batch = te.batch;
        i -= batch->firstTri; // triangle index within batch

        // fast path if we have locked this
        if (batch->lockMask & (1<<tid))
            return batch->tris[i];

        // hoax HOAX! not safe when running out of cache!
//      if (batch->expanded)
//          return batch->tris[i];

        return getTriProper(batch, i, tid);
    }

private:
    static int              hashTexture         (Hash<const Image*, S32>& hash, Texture tex);
    void                    subdivideTriangles  (int firstTri, int numTris, Array<Vec3f>& tCenter);
    void                    constructBatch      (int firstTri, int numTris);
    void                    expandBatch         (Batch* batch);
    void                    collapseBatch       (Batch* batch);
    void                    validateLRUList     (void) const;
    void                    validateTextures    (void) const;

private:
                            BuilderMesh         (BuilderMesh&); // forbidden
    BuilderMesh&            operator=           (BuilderMesh&); // forbidden

private:
    struct TriangleEntry
    {
        int     submesh;
        int     indexInSubmesh;
        Batch*  batch;
        U32     combo;      // index in batch and boundary mask combined

        int     getIndexInBatch(void) const { return (int)(combo >> 3); }
        U8      getBoundaryMask(void) const { return (U8)(combo & 7); }

        void    setIndexInBatch(int idx)   { combo &= 7; combo |= (idx << 3); }
        void    setBoundaryMask(U32 bmask) { combo &= ~7; combo |= (bmask & 7); }
    };

    struct LockedBatches
    {
        Array<Batch*>       list;
        int                 count;
        int                 head;
        int                 tail;
    };

    Mesh<VertexPNT>*        m_mesh;
    Mat4f                   m_xform;            // mesh to octree space
    Mat4f                   m_octreeToObject;
    S32                     m_bitsPerTri;
    Array<TriangleEntry>    m_triMap;
    Array<TextureSampler*>  m_textures;
    Array<DisplacementMap*> m_dispMaps;
    Hash<const Image*, S32> m_texHash;
    Hash<const Image*, S32> m_dispHash;
    int                     m_numTris;          // total number of triangles
    Array<Batch*>           m_batches;
    Batch*                  m_batchLRUHead;     // most recently used
    Batch*                  m_batchLRUTail;     // least recently used
    int                     m_numExpandedBatches;
    mutable Spinlock        m_lock;
    Array<LockedBatches>    m_lockedBatches;    // batches locked by each accessing thread
    int                     m_dummy;
};

class BuilderMeshAccessor
{
public:
                            BuilderMeshAccessor (const BuilderMesh* mesh, int threadIdx) : m_mesh(const_cast<BuilderMesh*>(mesh)), m_threadIdx(threadIdx) {}
                            ~BuilderMeshAccessor(void) {}

    const Mat4f&            getOctreeToObject   (void) const    { return m_mesh->getOctreeToObject(); }
    int                     getBitsPerTri       (void) const    { return m_mesh->getBitsPerTri(); }

    int                             getNumTris  (void) const    { return m_mesh->getNumTris(); }
    const BuilderMesh::Triangle&    getTri      (int i) const   { return m_mesh->getTri(i, m_threadIdx); }

private:
                            BuilderMeshAccessor (BuilderMeshAccessor&); // forbidden
    BuilderMeshAccessor&    operator=           (BuilderMeshAccessor&); // forbidden

private:
    BuilderMesh*            m_mesh;
    int                     m_threadIdx;
};

//------------------------------------------------------------------------
}
