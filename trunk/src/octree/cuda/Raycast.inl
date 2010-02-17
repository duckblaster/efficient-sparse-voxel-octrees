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
 
//------------------------------------------------------------------------

#define CAST_STACK_DEPTH        23
#define MAX_RAYCAST_ITERATIONS  10000

//------------------------------------------------------------------------

struct Ray
{
    float3      orig;
    float       orig_sz;
    float3      dir;
    float       dir_sz;
};

//------------------------------------------------------------------------

struct CastResult
{
    float       t;
    float3      pos;
    int         iter;

    S32*        node;
    int         childIdx;
    int         stackPtr;
};

//------------------------------------------------------------------------

class CastStack
{
public:
    __device__      CastStack   (void)                          {}

    __device__ S32* read        (int idx, F32& tmax) const      { U64 e = stack[idx]; tmax = __int_as_float((U32)(e >> 32)); return (S32*)(U32)e; }
    __device__ void write       (int idx, S32* node, F32 tmax)  { stack[idx] = (U32)node | ((U64)__float_as_int(tmax) << 32); }

private:
                    CastStack   (CastStack& other); // forbidden
    CastStack&      operator=   (CastStack& other); // forbidden

private:
    U64             stack[CAST_STACK_DEPTH + 1];
};

//------------------------------------------------------------------------
// Raycaster.
//------------------------------------------------------------------------

__device__ void castRay(CastResult& res, CastStack& stack, volatile Ray& ray)
{
    const float epsilon = exp2f(-CAST_STACK_DEPTH);
    float ray_orig_sz = ray.orig_sz;
    int iter = 0;

    // Get rid of small ray direction components to avoid division by zero.

    if (fabsf(ray.dir.x) < epsilon) ray.dir.x = copysignf(epsilon, ray.dir.x);
    if (fabsf(ray.dir.y) < epsilon) ray.dir.y = copysignf(epsilon, ray.dir.y);
    if (fabsf(ray.dir.z) < epsilon) ray.dir.z = copysignf(epsilon, ray.dir.z);

    // Precompute the coefficients of tx(x), ty(y), and tz(z).
    // The octree is assumed to reside at coordinates [1, 2].

    float tx_coef = 1.0f / -fabs(ray.dir.x);
    float ty_coef = 1.0f / -fabs(ray.dir.y);
    float tz_coef = 1.0f / -fabs(ray.dir.z);

    float tx_bias = tx_coef * ray.orig.x;
    float ty_bias = ty_coef * ray.orig.y;
    float tz_bias = tz_coef * ray.orig.z;

    // Select octant mask to mirror the coordinate system so
    // that ray direction is negative along each axis.

    int octant_mask = 7;
    if (ray.dir.x > 0.0f) octant_mask ^= 1, tx_bias = 3.0f * tx_coef - tx_bias;
    if (ray.dir.y > 0.0f) octant_mask ^= 2, ty_bias = 3.0f * ty_coef - ty_bias;
    if (ray.dir.z > 0.0f) octant_mask ^= 4, tz_bias = 3.0f * tz_coef - tz_bias;

    // Initialize the active span of t-values.

    float t_min = fmaxf(fmaxf(2.0f * tx_coef - tx_bias, 2.0f * ty_coef - ty_bias), 2.0f * tz_coef - tz_bias);
    float t_max = fminf(fminf(tx_coef - tx_bias, ty_coef - ty_bias), tz_coef - tz_bias);
    float h = t_max;
    t_min = fmaxf(t_min, 0.0f);
    t_max = fminf(t_max, 1.0f);

    // Initialize the current voxel to the first child of the root.

    int*   parent           = (int*)getInput().rootNode;
    int2   child_descriptor = make_int2(0, 0); // invalid until fetched
    int    idx              = 0;
    float3 pos              = make_float3(1.0f, 1.0f, 1.0f);
    int    scale            = CAST_STACK_DEPTH - 1;
    float  scale_exp2       = 0.5f; // exp2f(scale - s_max)

    if (1.5f * tx_coef - tx_bias > t_min) idx ^= 1, pos.x = 1.5f;
    if (1.5f * ty_coef - ty_bias > t_min) idx ^= 2, pos.y = 1.5f;
    if (1.5f * tz_coef - tz_bias > t_min) idx ^= 4, pos.z = 1.5f;

    // Traverse voxels along the ray as long as the current voxel
    // stays within the octree.

    while (scale < CAST_STACK_DEPTH)
    {
        updateCounter(PerfCounter_Iterations);
        updateCounter(PerfCounter_Instructions, 15 - 1);
#ifndef KERNEL_RAYCAST_PERF
        iter++;
#if (MAX_RAYCAST_ITERATIONS > 0)
        if (iter > MAX_RAYCAST_ITERATIONS)
            break;
#endif
#endif

        // Fetch child descriptor unless it is already valid.

        if (child_descriptor.x == 0)
        {
            child_descriptor = *(int2*)parent;
            updateCountersForGlobalAccess(3, parent);
        }

        // Determine maximum t-value of the cube by evaluating
        // tx(), ty(), and tz() at its corner.

        float tx_corner = pos.x * tx_coef - tx_bias;
        float ty_corner = pos.y * ty_coef - ty_bias;
        float tz_corner = pos.z * tz_coef - tz_bias;
        float tc_max = fminf(fminf(tx_corner, ty_corner), tz_corner);

        // Process voxel if the corresponding bit in valid mask is set
        // and the active t-span is non-empty.

        int child_shift = idx ^ octant_mask; // permute child slots based on the mirroring
        int child_masks = child_descriptor.x << child_shift;
        if ((child_masks & 0x8000) != 0 && t_min <= t_max)
        {
            // Terminate if the voxel is small enough.

            updateCounter(PerfCounter_Instructions, 3);
            if (tc_max * ray.dir_sz + ray_orig_sz >= scale_exp2)
                break; // at t_min

            // INTERSECT
            // Intersect active t-span with the cube and evaluate
            // tx(), ty(), and tz() at the center of the voxel.

            updateCounter(PerfCounter_Intersect);
            updateCounter(PerfCounter_Instructions, 9 - 1);
            float tv_max = fminf(t_max, tc_max);
            float half = scale_exp2 * 0.5f;
            float tx_center = half * tx_coef + tx_corner;
            float ty_center = half * ty_coef + ty_corner;
            float tz_center = half * tz_coef + tz_corner;

            // Intersect with contour if the corresponding bit in contour mask is set.

            int contour_mask = child_descriptor.y << child_shift;
#ifndef ENABLE_CONTOURS
            contour_mask = 0;
#endif

            if ((contour_mask & 0x80) != 0)
            {
                updateCounter(PerfCounter_Instructions, 35 - 9);
                int   ofs    = (unsigned int)child_descriptor.y >> 8;       // contour pointer
                int   value  = parent[ofs + popc8(contour_mask & 0x7F)];    // contour value
                updateCountersForGlobalAccess(2, &parent[ofs + popc8(contour_mask & 0x7F)]);
                float cthick = (float)(unsigned int)value * scale_exp2 * 0.75f; // thickness
                float cpos   = (float)(value << 7) * scale_exp2 * 1.5f;     // position
                float cdirx  = (float)(value << 14) * ray.dir.x;            // nx
                float cdiry  = (float)(value << 20) * ray.dir.y;            // ny
                float cdirz  = (float)(value << 26) * ray.dir.z;            // nz
                float tcoef  = 1.0f / (cdirx + cdiry + cdirz);
                float tavg   = tx_center * cdirx + ty_center * cdiry + tz_center * cdirz + cpos;
                float tdiff  = cthick * tcoef;

                t_min  = fmaxf(t_min,  tcoef * tavg - fabsf(tdiff)); // Override t_min with tv_min.
                tv_max = fminf(tv_max, tcoef * tavg + fabsf(tdiff));
            }

            // Descend to the first child if the resulting t-span is non-empty.

            updateCounter(PerfCounter_Instructions, 2);
            if (t_min <= tv_max)
            {
                // Terminate if the corresponding bit in the non-leaf mask is not set.

                updateCounter(PerfCounter_Instructions, 2);
                if ((child_masks & 0x0080) == 0)
                    break; // at t_min (overridden with tv_min).

                // PUSH
                // Write current parent to the stack.

                updateCounter(PerfCounter_Push);
                updateCounter(PerfCounter_Instructions, 31 - 6);
#ifndef DISABLE_PUSH_OPTIMIZATION
                if (tc_max < h)
#endif
                {
                    updateCounter(PerfCounter_PushStore);
                    stack.write(scale, parent, t_max);
                    updateCountersForLocalAccess(3, scale);
                }
                h = tc_max;

                // Find child descriptor corresponding to the current voxel.

                int ofs = (unsigned int)child_descriptor.x >> 17; // child pointer
                if ((child_descriptor.x & 0x10000) != 0) // far
                {
                    ofs = parent[ofs * 2]; // far pointer
                    updateCountersForGlobalAccess(2, &parent[ofs * 2]);
                }
                ofs += popc8(child_masks & 0x7F);
                parent += ofs * 2;

                // Select child voxel that the ray enters first.

                idx = 0;
                scale--;
                scale_exp2 = half;

                if (tx_center > t_min) idx ^= 1, pos.x += scale_exp2;
                if (ty_center > t_min) idx ^= 2, pos.y += scale_exp2;
                if (tz_center > t_min) idx ^= 4, pos.z += scale_exp2;

                // Update active t-span and invalidate cached child descriptor.

                t_max = tv_max;
                child_descriptor.x = 0;
                continue;
            }
        }

        // ADVANCE
        // Step along the ray.

        updateCounter(PerfCounter_Advance);
        updateCounter(PerfCounter_Instructions, 14 - 2);
        int step_mask = 0;
        if (tx_corner <= tc_max) step_mask ^= 1, pos.x -= scale_exp2;
        if (ty_corner <= tc_max) step_mask ^= 2, pos.y -= scale_exp2;
        if (tz_corner <= tc_max) step_mask ^= 4, pos.z -= scale_exp2;

        // Update active t-span and flip bits of the child slot index.

        t_min = tc_max;
        idx ^= step_mask;

        // Proceed with pop if the bit flips disagree with the ray direction.

        if ((idx & step_mask) != 0)
        {
            // POP
            // Find the highest differing bit between the two positions.

            updateCounter(PerfCounter_Pop);
            updateCounter(PerfCounter_Instructions, 38 - 4);
            unsigned int differing_bits = 0;
            if ((step_mask & 1) != 0) differing_bits |= __float_as_int(pos.x) ^ __float_as_int(pos.x + scale_exp2);
            if ((step_mask & 2) != 0) differing_bits |= __float_as_int(pos.y) ^ __float_as_int(pos.y + scale_exp2);
            if ((step_mask & 4) != 0) differing_bits |= __float_as_int(pos.z) ^ __float_as_int(pos.z + scale_exp2);
            scale = (__float_as_int((float)differing_bits) >> 23) - 127; // position of the highest bit
            scale_exp2 = __int_as_float((scale - CAST_STACK_DEPTH + 127) << 23); // exp2f(scale - s_max)

            // Restore parent voxel from the stack.

            parent = stack.read(scale, t_max);
            updateCountersForLocalAccess(3, scale);

            // Round cube position and extract child slot index.

            int shx = __float_as_int(pos.x) >> scale;
            int shy = __float_as_int(pos.y) >> scale;
            int shz = __float_as_int(pos.z) >> scale;
            pos.x = __int_as_float(shx << scale);
            pos.y = __int_as_float(shy << scale);
            pos.z = __int_as_float(shz << scale);
            idx  = (shx & 1) | ((shy & 1) << 1) | ((shz & 1) << 2);

            // Prevent same parent from being stored again and invalidate cached child descriptor.

            h = 0.0f;
            child_descriptor.x = 0;
        }
        updateCounter(PerfCounter_Instructions, 2);
    }

    // Indicate miss if we are outside the octree.

#if (MAX_RAYCAST_ITERATIONS > 0)
    if (scale >= CAST_STACK_DEPTH || iter > MAX_RAYCAST_ITERATIONS)
#else
    if (scale >= CAST_STACK_DEPTH)
#endif
    {
        t_min = 2.0f;
    }

    // Undo mirroring of the coordinate system.

    if ((octant_mask & 1) == 0) pos.x = 3.0f - scale_exp2 - pos.x;
    if ((octant_mask & 2) == 0) pos.y = 3.0f - scale_exp2 - pos.y;
    if ((octant_mask & 4) == 0) pos.z = 3.0f - scale_exp2 - pos.z;

    // Output results.

    res.t = t_min;
    res.iter = iter;
    res.pos.x = fminf(fmaxf(ray.orig.x + t_min * ray.dir.x, pos.x + epsilon), pos.x + scale_exp2 - epsilon);
    res.pos.y = fminf(fmaxf(ray.orig.y + t_min * ray.dir.y, pos.y + epsilon), pos.y + scale_exp2 - epsilon);
    res.pos.z = fminf(fmaxf(ray.orig.z + t_min * ray.dir.z, pos.z + epsilon), pos.z + scale_exp2 - epsilon);
    res.node = parent;
    res.childIdx = idx ^ octant_mask ^ 7;
    res.stackPtr = scale;
}

//------------------------------------------------------------------------
