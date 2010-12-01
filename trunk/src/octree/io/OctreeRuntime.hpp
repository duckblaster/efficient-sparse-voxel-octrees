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
#include "base/Defs.hpp"
#if !FW_CUDA
#   include "OctreeFile.hpp"
#   include "MemoryManager.hpp"
#   include "gpu/Buffer.hpp"
#endif

namespace FW
{
//------------------------------------------------------------------------

class OctreeRuntime // all offsets and sizes are dword-based
{
public:
    enum
    {
        PageBytesLog2   = 13,
        PageBytes       = 1 << PageBytesLog2,
        PageSizeLog2    = PageBytesLog2 - 2,
        PageSize        = 1 << PageSizeLog2,
        TrunksPerBlock  = 512,
    };

    enum FindMode
    {
        FindMode_Load = 0,
        FindMode_Build,
        FindMode_LoadOrBuild,
        FindMode_Unload,
        FindMode_UnloadDeepest,

        FindMode_Max
    };

    enum BlockInfo
    {
        BlockInfo_SliceID = 0,
        BlockInfo_IndexInSlice,
        BlockInfo_BlockPtr,
        BlockInfo_NumAttach,

        BlockInfo_End
    };

    enum AttachInfo
    {
        AttachInfo_Type = 0,
        AttachInfo_Ptr,

        AttachInfo_End
    };

#if !FW_CUDA
    struct FindResult
    {
        S32                 sliceID;
        F32                 score;
    };

private:
    struct Block
    {
        S64                 ofs;                    // -1 if none
        S64                 size;
    };

    struct Trunk // pooled collection of 8 adjacent Nodes
    {
        S32                 nextFreeTrunk;          // -1 if none

        S64                 blockOfs;
        S32                 pageInBlock;
        S32                 trunkInPage;
        S64                 nodeOfs;                // 8 adjacent Nodes followed by 8 FarPtrs

        U8                  validMask[8];
        U8                  nonLeafMask[8];
        S64                 childOfs[8];
    };

    struct Object
    {
        S32                 rootSliceID;
        AttachIO*           attachIO;
        S32                 nodeAlign;

        S32                 trunksPerPage;
        S32                 pagesPerTrunkBlock;
        S32                 trunkBlockInfoOfs;
        S32                 trunkBlockSize;

        Array<Block>        trunkBlocks;
        Array<Trunk>        trunks;
        S32                 firstFreeTrunk;         // -1 if none
    };

    struct SliceBlock
    {
        S32                 childEntry;
        S32                 splitTrunkID;
        Block               block;
    };

    struct Slice
    {
        bool                isReached;
        bool                isUnbuilt;
        bool                isLoaded;
        S32                 numChildrenLoaded;

        S32                 objectID;
        S32                 parentSliceID;          // -1 if none
        S32                 indexInParent;
        S32                 trunkID;                // -1 if none
        S32                 subtrunk;

        Vec3i               cubePos;
        S32                 cubeScale;
        S32                 nodeScale;

        S32                 numNodes;
        S32                 numNodeChildren;

        Array<SliceBlock>   blocks;
    };

    struct LoadBlock
    {
        S32                 childEntry;             // OctreeSlice::ChildEntry
        Array<S32>          data;
        const S32*          rootNodeBlock;
        const S32*          rootNode;

        S32                 splitTrunkID;
        Block               block;
    };

    struct Relocation
    {
        S64                 oldOfs;
        S64                 delta;
    };

public:
                            OctreeRuntime           (MemoryManager::Mode mode);
                            ~OctreeRuntime          (void);

    void                    clear                   (void);

    bool                    addObject               (int objectID, int rootSliceID, const Array<AttachIO::AttachType>& runtimeAttachTypes);
    void                    removeObject            (int objectID);
    bool                    hasObject               (int objectID) const            { return (objectID >= 0 && objectID < m_objects.getSize() && m_objects[objectID]); }
    const Array<AttachIO::AttachType>& getAttachTypes(int objectID) const           { FW_ASSERT(m_objects[objectID]); return m_objects[objectID]->attachIO->getRuntimeTypes(); }

    bool                    setSliceState           (int sliceID, OctreeFile::SliceState state); // true if loadable
    S32                     setSliceToLoad          (const OctreeSlice& sliceData); // returns additional memory consumption after load
    bool                    loadSlice               (void); // false if out of memory
    bool                    loadSlice               (const OctreeSlice& sliceData)  { setSliceToLoad(sliceData); return loadSlice(); }
    void                    unloadSlice             (int sliceID);
    bool                    isSliceLoaded           (int sliceID) const             { return (sliceID < m_slices.getSize() && m_slices[sliceID]->isLoaded); }

    MemoryManager::Mode     getMode                 (void) const                    { return m_mem.getMode(); }
    const S32*              getRootNodeCPU          (int objectID); // NULL if none
    CUdeviceptr             getRootNodeCuda         (int objectID); // NULL if none
    Buffer&                 getBuffer               (void)                          { return m_mem.getBuffer(); }

    void                    findSlices              (Array<FindResult>& results, FindMode mode, int objectID, const Vec3f& areaLo, const Vec3f& areaHi, int maxLevels, int maxResults);
    FindResult              findSlice               (FindMode mode, int objectID, const Vec3f& areaLo, const Vec3f& areaHi, int maxLevels);

    String                  getStats                (void) const;
    S64                     getFreeBytes            (void) const                    { return m_mem.getFreeBytes(); }
    S64                     getUsedBytes            (void) const                    { return m_mem.getTotalBytes() - m_mem.getFreeBytes(); }

    static U8               getNodeValidMask        (const S32* node)               { return (U8)(node[0] >> 8); }
    static bool             hasNodeChild            (const S32* node, int childIdx) { FW_ASSERT(childIdx >= 0 && childIdx < 8); return ((node[0] & (0x0100 << childIdx)) != 0); }
    static int              numNodeChildrenBefore   (const S32* node, int childIdx) { FW_ASSERT(childIdx >= 0 && childIdx < 8); return popc8((node[0] >> 8) & ((1 << childIdx) - 1)); }
    static U8               getNodeNonLeafMask      (const S32* node)               { return (U8)node[0]; }
    static bool             isNodeChildNode         (const S32* node, int childIdx) { FW_ASSERT(childIdx >= 0 && childIdx < 8); return ((node[0] & (0x0001 << childIdx)) != 0); }
    static int              numNodeChildNodesBefore (const S32* node, int childIdx) { FW_ASSERT(childIdx >= 0 && childIdx < 8); return popc8(node[0] & ((1 << childIdx) - 1)); }
    static const S32*       getNodeChildren         (const S32* node)               { int ofs = ((U32)node[0] >> 17) * 2; return node + (((node[0] & 0x10000) == 0) ? ofs : node[ofs] * 2); }
    static const S32*       getNodeChild            (const S32* node, int childIdx) { FW_ASSERT(isNodeChildNode(node, childIdx)); return getNodeChildren(node) + numNodeChildNodesBefore(node, childIdx) * 2; }

    static const S32*       getBlockInfo            (const S32* node)               { const S32* pageHeader = (const S32*)((UPTR)node & -PageBytes); return pageHeader + pageHeader[0]; }
    static const S32*       getBlockStart           (const S32* blockInfo)          { return blockInfo + blockInfo[BlockInfo_BlockPtr]; }

    static int              getNumAttach            (const S32* blockInfo)          { return blockInfo[BlockInfo_NumAttach]; }
    static const S32*       getAttachInfo           (const S32* blockInfo, int attachIdx) { FW_ASSERT(attachIdx >= 0 && attachIdx < getNumAttach(blockInfo)); return blockInfo + BlockInfo_End + attachIdx * AttachInfo_End; }
    static AttachIO::AttachType getAttachType       (const S32* attachInfo)         { return (AttachIO::AttachType)attachInfo[AttachInfo_Type]; }
    static const S32*       getAttachData           (const S32* blockInfo, const S32* attachInfo) { return blockInfo + attachInfo[AttachInfo_Ptr]; }

private:
    void                    upload                  (S64 ofs, const S32* ptr, S64 size) { m_mem.getBuffer().setRange(ofs * sizeof(S32), ptr, size * sizeof(S32)); }
    void                    upload                  (S64 ofs, S32 value)            { upload(ofs, &value, 1); }

    Block                   allocateBlock           (S64 size); // -1 if out of memory; may move blocks around
    void                    freeBlock               (Block& block);

    int                     allocateTrunk           (int objectID); // -1 if out of memory; may move blocks around
    void                    freeTrunk               (int objectID, int trunkID);
    void                    initTrunkBlock          (S32* blockData, int objectID);
    void                    updateTrunkNode         (const Trunk& trunk, int sub);

    void                    unloadSliceInternal     (int sliceID);
    Slice*                  getOrCreateSlice        (int sliceID);
    S64                     getRootNodeOfs          (int objectID); // -1 if none

    void                    gatherImportNodes       (const OctreeSlice* sliceData,
                                                     int*               sliceNodeIdx,
                                                     int*               sliceSplitNodeIdx,
                                                     const S32*         oldRootNode,
                                                     int                numLevels);

    int                     layoutImportNodes       (int objectID);

    void                    buildBlock              (Array<S32>&        blockData,
                                                     int                blockInfoOfs,
                                                     int                sliceID,
                                                     int                indexInSlice,
                                                     int                objectID);

private:
                            OctreeRuntime           (OctreeRuntime&); // forbidden
    OctreeRuntime&          operator=               (OctreeRuntime&); // forbidden

private:
    MemoryManager           m_mem;
    MemoryManager::Relocation m_reloc;

    Array<Object*>          m_objects;
    Array<Slice*>           m_slices;

    S32                     m_numSlicesLoaded;
    S64                     m_numNodesLoaded;
    S64                     m_numNodeChildrenLoaded;

    // setSliceToLoad(), loadSlice(), unloadSliceInternal()

    S32                     m_loadSliceID;
    Buffer                  m_loadBuffer;
    Array<LoadBlock>        m_loadBlocks;
    Array<S32>              m_unloadBlock;

    // gatherImportNodes(), layoutImportNodes(), fillInBlockData()

    Array<AttachIO::ImportNode> m_importNodes;
#endif // !FW_CUDA
};

//------------------------------------------------------------------------
/*

OctreeRuntime data format
-------------------------

- the basic unit of data is 32-bit little-endian int
- Node pointers are qword-based, other pointers are dword-based

Block (absolute pointer must be divisible by 8 kilobytes)
    0       n*2     struct  interleaved array of PageHeader, Node, FarPtr, and PagePadding
    ?       4       struct  BlockInfo
    ?       n*2     struct  array of BlockAttachInfo (BlockInfo.numAttach)
    ?       n*?     struct  pool of BlockXxxAttach
    ?

PageHeader (whenever absolute pointer is divisible by 8 kilobytes)
    0       1       int     infoPtr: pointer to BlockInfo, relative to this PageHeader
    1       1       int     dummy
    2

Node
    0.17    .15     bits    childPtr: if far=0, unsigned qword pointer to first child Node, relative to this Node
    0.17    .15     bits    childPtr: if far=1, unsigned qword pointer to FarPtr, relative to the Node
    0.16    .1      bits    far
    0.8     .8      bits    validMask: whether each child exists
    0.0     .8      bits    nonLeafMask: whether each child is represented by a Node (i.e. is not a leaf)
    1.8     .24     bits    contourPtr: dword pointer to Contour, relative to the Node
    1.0     .8      bits    contourMask: whether each child has a contour
    2

FarPtr
    0       1       int     farPtr: child Node qword pointer, relative to the referencing Node
    1       1       int     dummy
    2

PagePadding (variable number before a PageHeader if there is no space for enough consecutive Nodes)
    0       1       int     dummy
    1       1       int     dummy
    2

BlockInfo
    0       1       int     sliceID
    1       1       int     indexInSlice
    2       1       int     blockPtr: pointer to Block, relative to BlockInfo
    3       1       int     numAttach: number of attachments
    4

BlockAttachInfo
    0       1       int     type (see OctreeSlice::AttachType)
    1       1       int     ptr: pointer to XxxAttach, relative to BlockInfo
    2

Attachments
    See AttachIO.h

*/
//------------------------------------------------------------------------
}
