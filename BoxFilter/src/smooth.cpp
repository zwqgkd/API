/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2014-2015, Itseez Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"

#include <vector>

#include "opencv2/core/hal/intrin.hpp"
#include "opencl_kernels_imgproc.hpp"

#include "opencv2/core/openvx/ovx_defs.hpp"

#include "filter.hpp"

#include "fixedpoint.inl.hpp"
/*
 * This file includes the code, contributed by Simon Perreault
 * (the function icvMedianBlur_8u_O1)
 *
 * Constant-time median filtering -- http://nomis80.org/ctmf.html
 * Copyright (C) 2006 Simon Perreault
 *
 * Contact:
 *  Laboratoire de vision et systemes numeriques
 *  Pavillon Adrien-Pouliot
 *  Universite Laval
 *  Sainte-Foy, Quebec, Canada
 *  G1K 7P4
 *
 *  perreaul@gel.ulaval.ca
 */

namespace cv
{

/****************************************************************************************\
                                         Box Filter
\****************************************************************************************/

template<typename T, typename ST>
struct RowSum :
        public BaseRowFilter
{
    RowSum( int _ksize, int _anchor ) :
        BaseRowFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
    }

    virtual void operator()(const uchar* src, uchar* dst, int width, int cn) CV_OVERRIDE
    {
        const T* S = (const T*)src;
        ST* D = (ST*)dst;
        int i = 0, k, ksz_cn = ksize*cn;

        width = (width - 1)*cn;
        if( ksize == 3 )
        {
            for( i = 0; i < width + cn; i++ )
            {
                D[i] = (ST)S[i] + (ST)S[i+cn] + (ST)S[i+cn*2];
            }
        }
        else if( ksize == 5 )
        {
            for( i = 0; i < width + cn; i++ )
            {
                D[i] = (ST)S[i] + (ST)S[i+cn] + (ST)S[i+cn*2] + (ST)S[i + cn*3] + (ST)S[i + cn*4];
            }
        }
        else if( cn == 1 )
        {
            ST s = 0;
            for( i = 0; i < ksz_cn; i++ )
                s += (ST)S[i];
            D[0] = s;
            for( i = 0; i < width; i++ )
            {
                s += (ST)S[i + ksz_cn] - (ST)S[i];
                D[i+1] = s;
            }
        }
        else if( cn == 3 )
        {
            ST s0 = 0, s1 = 0, s2 = 0;
            for( i = 0; i < ksz_cn; i += 3 )
            {
                s0 += (ST)S[i];
                s1 += (ST)S[i+1];
                s2 += (ST)S[i+2];
            }
            D[0] = s0;
            D[1] = s1;
            D[2] = s2;
            for( i = 0; i < width; i += 3 )
            {
                s0 += (ST)S[i + ksz_cn] - (ST)S[i];
                s1 += (ST)S[i + ksz_cn + 1] - (ST)S[i + 1];
                s2 += (ST)S[i + ksz_cn + 2] - (ST)S[i + 2];
                D[i+3] = s0;
                D[i+4] = s1;
                D[i+5] = s2;
            }
        }
        else if( cn == 4 )
        {
            ST s0 = 0, s1 = 0, s2 = 0, s3 = 0;
            for( i = 0; i < ksz_cn; i += 4 )
            {
                s0 += (ST)S[i];
                s1 += (ST)S[i+1];
                s2 += (ST)S[i+2];
                s3 += (ST)S[i+3];
            }
            D[0] = s0;
            D[1] = s1;
            D[2] = s2;
            D[3] = s3;
            for( i = 0; i < width; i += 4 )
            {
                s0 += (ST)S[i + ksz_cn] - (ST)S[i];
                s1 += (ST)S[i + ksz_cn + 1] - (ST)S[i + 1];
                s2 += (ST)S[i + ksz_cn + 2] - (ST)S[i + 2];
                s3 += (ST)S[i + ksz_cn + 3] - (ST)S[i + 3];
                D[i+4] = s0;
                D[i+5] = s1;
                D[i+6] = s2;
                D[i+7] = s3;
            }
        }
        else
            for( k = 0; k < cn; k++, S++, D++ )
            {
                ST s = 0;
                for( i = 0; i < ksz_cn; i += cn )
                    s += (ST)S[i];
                D[0] = s;
                for( i = 0; i < width; i += cn )
                {
                    s += (ST)S[i + ksz_cn] - (ST)S[i];
                    D[i+cn] = s;
                }
            }
    }
};


template<typename ST, typename T>
struct ColumnSum :
        public BaseColumnFilter
{
    ColumnSum( int _ksize, int _anchor, double _scale ) :
        BaseColumnFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
    }

    virtual void reset() CV_OVERRIDE { sumCount = 0; }

    virtual void operator()(const uchar** src, uchar* dst, int dststep, int count, int width) CV_OVERRIDE
    {
        int i;
        ST* SUM;
        bool haveScale = scale != 1;
        double _scale = scale;

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            memset((void*)SUM, 0, width*sizeof(ST));

            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const ST* Sp = (const ST*)src[0];

                for( i = 0; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const ST* Sp = (const ST*)src[0];
            const ST* Sm = (const ST*)src[1-ksize];
            T* D = (T*)dst;
            if( haveScale )
            {
                for( i = 0; i <= width - 2; i += 2 )
                {
                    ST s0 = SUM[i] + Sp[i], s1 = SUM[i+1] + Sp[i+1];
                    D[i] = saturate_cast<T>(s0*_scale);
                    D[i+1] = saturate_cast<T>(s1*_scale);
                    s0 -= Sm[i]; s1 -= Sm[i+1];
                    SUM[i] = s0; SUM[i+1] = s1;
                }

                for( ; i < width; i++ )
                {
                    ST s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<T>(s0*_scale);
                    SUM[i] = s0 - Sm[i];
                }
            }
            else
            {
                for( i = 0; i <= width - 2; i += 2 )
                {
                    ST s0 = SUM[i] + Sp[i], s1 = SUM[i+1] + Sp[i+1];
                    D[i] = saturate_cast<T>(s0);
                    D[i+1] = saturate_cast<T>(s1);
                    s0 -= Sm[i]; s1 -= Sm[i+1];
                    SUM[i] = s0; SUM[i+1] = s1;
                }

                for( ; i < width; i++ )
                {
                    ST s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<T>(s0);
                    SUM[i] = s0 - Sm[i];
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    std::vector<ST> sum;
};


template<>
struct ColumnSum<int, uchar> :
        public BaseColumnFilter
{
    ColumnSum( int _ksize, int _anchor, double _scale ) :
        BaseColumnFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
    }

    virtual void reset() CV_OVERRIDE { sumCount = 0; }

    virtual void operator()(const uchar** src, uchar* dst, int dststep, int count, int width) CV_OVERRIDE
    {
        int* SUM;
        bool haveScale = scale != 1;
        double _scale = scale;

#if CV_SIMD128
        bool haveSIMD128 = hasSIMD128();
#endif

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            memset((void*)SUM, 0, width*sizeof(int));
            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const int* Sp = (const int*)src[0];
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for (; i <= width - 4; i += 4)
                    {
                        v_store(SUM + i, v_load(SUM + i) + v_load(Sp + i));
                    }
                }
#endif
                for( ; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const int* Sp = (const int*)src[0];
            const int* Sm = (const int*)src[1-ksize];
            uchar* D = (uchar*)dst;
            if( haveScale )
            {
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {

                    v_float32x4 v_scale = v_setall_f32((float)_scale);
                    for( ; i <= width-8; i+=8 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_uint32x4 v_s0d = v_reinterpret_as_u32(v_round(v_cvt_f32(v_s0) * v_scale));
                        v_uint32x4 v_s01d = v_reinterpret_as_u32(v_round(v_cvt_f32(v_s01) * v_scale));

                        v_uint16x8 v_dst = v_pack(v_s0d, v_s01d);
                        v_pack_store(D + i, v_dst);

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<uchar>(s0*_scale);
                    SUM[i] = s0 - Sm[i];
                }
            }
            else
            {
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width-8; i+=8 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_uint16x8 v_dst = v_pack(v_reinterpret_as_u32(v_s0), v_reinterpret_as_u32(v_s01));
                        v_pack_store(D + i, v_dst);

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif

                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<uchar>(s0);
                    SUM[i] = s0 - Sm[i];
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    std::vector<int> sum;
};


template<>
struct ColumnSum<ushort, uchar> :
public BaseColumnFilter
{
    enum { SHIFT = 23 };

    ColumnSum( int _ksize, int _anchor, double _scale ) :
    BaseColumnFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
        divDelta = 0;
        divScale = 1;
        if( scale != 1 )
        {
            int d = cvRound(1./scale);
            double scalef = ((double)(1 << SHIFT))/d;
            divScale = cvFloor(scalef);
            scalef -= divScale;
            divDelta = d/2;
            if( scalef < 0.5 )
                divDelta++;
            else
                divScale++;
        }
    }

    virtual void reset() CV_OVERRIDE { sumCount = 0; }

    virtual void operator()(const uchar** src, uchar* dst, int dststep, int count, int width) CV_OVERRIDE
    {
        const int ds = divScale;
        const int dd = divDelta;
        ushort* SUM;
        const bool haveScale = scale != 1;

#if CV_SIMD128
        bool haveSIMD128 = hasSIMD128();
#endif

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            memset((void*)SUM, 0, width*sizeof(SUM[0]));
            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const ushort* Sp = (const ushort*)src[0];
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width - 8; i += 8 )
                    {
                        v_store(SUM + i, v_load(SUM + i) + v_load(Sp + i));
                    }
                }
#endif
                for( ; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const ushort* Sp = (const ushort*)src[0];
            const ushort* Sm = (const ushort*)src[1-ksize];
            uchar* D = (uchar*)dst;
            if( haveScale )
            {
                int i = 0;
            #if CV_SIMD128
                v_uint32x4 ds4 = v_setall_u32((unsigned)ds);
                v_uint16x8 dd8 = v_setall_u16((ushort)dd);

                for( ; i <= width-16; i+=16 )
                {
                    v_uint16x8 _sm0 = v_load(Sm + i);
                    v_uint16x8 _sm1 = v_load(Sm + i + 8);

                    v_uint16x8 _s0 = v_add_wrap(v_load(SUM + i), v_load(Sp + i));
                    v_uint16x8 _s1 = v_add_wrap(v_load(SUM + i + 8), v_load(Sp + i + 8));

                    v_uint32x4 _s00, _s01, _s10, _s11;

                    v_expand(_s0 + dd8, _s00, _s01);
                    v_expand(_s1 + dd8, _s10, _s11);

                    _s00 = v_shr<SHIFT>(_s00*ds4);
                    _s01 = v_shr<SHIFT>(_s01*ds4);
                    _s10 = v_shr<SHIFT>(_s10*ds4);
                    _s11 = v_shr<SHIFT>(_s11*ds4);

                    v_int16x8 r0 = v_pack(v_reinterpret_as_s32(_s00), v_reinterpret_as_s32(_s01));
                    v_int16x8 r1 = v_pack(v_reinterpret_as_s32(_s10), v_reinterpret_as_s32(_s11));

                    _s0 = v_sub_wrap(_s0, _sm0);
                    _s1 = v_sub_wrap(_s1, _sm1);

                    v_store(D + i, v_pack_u(r0, r1));
                    v_store(SUM + i, _s0);
                    v_store(SUM + i + 8, _s1);
                }
            #endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = (uchar)((s0 + dd)*ds >> SHIFT);
                    SUM[i] = (ushort)(s0 - Sm[i]);
                }
            }
            else
            {
                int i = 0;
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<uchar>(s0);
                    SUM[i] = (ushort)(s0 - Sm[i]);
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    int divDelta;
    int divScale;
    std::vector<ushort> sum;
};


template<>
struct ColumnSum<int, short> :
        public BaseColumnFilter
{
    ColumnSum( int _ksize, int _anchor, double _scale ) :
        BaseColumnFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
    }

    virtual void reset() CV_OVERRIDE { sumCount = 0; }

    virtual void operator()(const uchar** src, uchar* dst, int dststep, int count, int width) CV_OVERRIDE
    {
        int i;
        int* SUM;
        bool haveScale = scale != 1;
        double _scale = scale;

#if CV_SIMD128
        bool haveSIMD128 = hasSIMD128();
#endif

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            memset((void*)SUM, 0, width*sizeof(int));
            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const int* Sp = (const int*)src[0];
                i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width - 4; i+=4 )
                    {
                        v_store(SUM + i, v_load(SUM + i) + v_load(Sp + i));
                    }
                }
                #endif
                for( ; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const int* Sp = (const int*)src[0];
            const int* Sm = (const int*)src[1-ksize];
            short* D = (short*)dst;
            if( haveScale )
            {
                i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    v_float32x4 v_scale = v_setall_f32((float)_scale);
                    for( ; i <= width-8; i+=8 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_int32x4 v_s0d =  v_round(v_cvt_f32(v_s0) * v_scale);
                        v_int32x4 v_s01d = v_round(v_cvt_f32(v_s01) * v_scale);
                        v_store(D + i, v_pack(v_s0d, v_s01d));

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<short>(s0*_scale);
                    SUM[i] = s0 - Sm[i];
                }
            }
            else
            {
                i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width-8; i+=8 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_store(D + i, v_pack(v_s0, v_s01));

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif

                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<short>(s0);
                    SUM[i] = s0 - Sm[i];
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    std::vector<int> sum;
};


template<>
struct ColumnSum<int, ushort> :
        public BaseColumnFilter
{
    ColumnSum( int _ksize, int _anchor, double _scale ) :
        BaseColumnFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
    }

    virtual void reset() CV_OVERRIDE { sumCount = 0; }

    virtual void operator()(const uchar** src, uchar* dst, int dststep, int count, int width) CV_OVERRIDE
    {
        int* SUM;
        bool haveScale = scale != 1;
        double _scale = scale;

#if CV_SIMD128
        bool haveSIMD128 = hasSIMD128();
#endif

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            memset((void*)SUM, 0, width*sizeof(int));
            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const int* Sp = (const int*)src[0];
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for (; i <= width - 4; i += 4)
                    {
                        v_store(SUM + i, v_load(SUM + i) + v_load(Sp + i));
                    }
                }
#endif
                for( ; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const int* Sp = (const int*)src[0];
            const int* Sm = (const int*)src[1-ksize];
            ushort* D = (ushort*)dst;
            if( haveScale )
            {
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    v_float32x4 v_scale = v_setall_f32((float)_scale);
                    for( ; i <= width-8; i+=8 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_uint32x4 v_s0d = v_reinterpret_as_u32(v_round(v_cvt_f32(v_s0) * v_scale));
                        v_uint32x4 v_s01d = v_reinterpret_as_u32(v_round(v_cvt_f32(v_s01) * v_scale));
                        v_store(D + i, v_pack(v_s0d, v_s01d));

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<ushort>(s0*_scale);
                    SUM[i] = s0 - Sm[i];
                }
            }
            else
            {
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width-8; i+=8 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_store(D + i, v_pack(v_reinterpret_as_u32(v_s0), v_reinterpret_as_u32(v_s01)));

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<ushort>(s0);
                    SUM[i] = s0 - Sm[i];
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    std::vector<int> sum;
};

template<>
struct ColumnSum<int, int> :
        public BaseColumnFilter
{
    ColumnSum( int _ksize, int _anchor, double _scale ) :
        BaseColumnFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
    }

    virtual void reset() CV_OVERRIDE { sumCount = 0; }

    virtual void operator()(const uchar** src, uchar* dst, int dststep, int count, int width) CV_OVERRIDE
    {
        int* SUM;
        bool haveScale = scale != 1;
        double _scale = scale;

#if CV_SIMD128
        bool haveSIMD128 = hasSIMD128();
#endif

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            memset((void*)SUM, 0, width*sizeof(int));
            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const int* Sp = (const int*)src[0];
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width - 4; i+=4 )
                    {
                        v_store(SUM + i, v_load(SUM + i) + v_load(Sp + i));
                    }
                }
#endif
                for( ; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const int* Sp = (const int*)src[0];
            const int* Sm = (const int*)src[1-ksize];
            int* D = (int*)dst;
            if( haveScale )
            {
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    v_float32x4 v_scale = v_setall_f32((float)_scale);
                    for( ; i <= width-4; i+=4 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s0d = v_round(v_cvt_f32(v_s0) * v_scale);

                        v_store(D + i, v_s0d);
                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = saturate_cast<int>(s0*_scale);
                    SUM[i] = s0 - Sm[i];
                }
            }
            else
            {
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width-4; i+=4 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);

                        v_store(D + i, v_s0);
                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = s0;
                    SUM[i] = s0 - Sm[i];
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    std::vector<int> sum;
};


template<>
struct ColumnSum<int, float> :
        public BaseColumnFilter
{
    ColumnSum( int _ksize, int _anchor, double _scale ) :
        BaseColumnFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
        scale = _scale;
        sumCount = 0;
    }

    virtual void reset() CV_OVERRIDE { sumCount = 0; }

    virtual void operator()(const uchar** src, uchar* dst, int dststep, int count, int width) CV_OVERRIDE
    {
        int* SUM;
        bool haveScale = scale != 1;
        double _scale = scale;

#if CV_SIMD128
        bool haveSIMD128 = hasSIMD128();
#endif

        if( width != (int)sum.size() )
        {
            sum.resize(width);
            sumCount = 0;
        }

        SUM = &sum[0];
        if( sumCount == 0 )
        {
            memset((void*)SUM, 0, width*sizeof(int));
            for( ; sumCount < ksize - 1; sumCount++, src++ )
            {
                const int* Sp = (const int*)src[0];
                int i = 0;
#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width - 4; i+=4 )
                    {
                        v_store(SUM + i, v_load(SUM + i) + v_load(Sp + i));
                    }
                }
#endif

                for( ; i < width; i++ )
                    SUM[i] += Sp[i];
            }
        }
        else
        {
            CV_Assert( sumCount == ksize-1 );
            src += ksize-1;
        }

        for( ; count--; src++ )
        {
            const int * Sp = (const int*)src[0];
            const int * Sm = (const int*)src[1-ksize];
            float* D = (float*)dst;
            if( haveScale )
            {
                int i = 0;

#if CV_SIMD128
                if( haveSIMD128 )
                {
                    v_float32x4 v_scale = v_setall_f32((float)_scale);
                    for (; i <= width - 8; i += 8)
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_store(D + i, v_cvt_f32(v_s0) * v_scale);
                        v_store(D + i + 4, v_cvt_f32(v_s01) * v_scale);

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = (float)(s0*_scale);
                    SUM[i] = s0 - Sm[i];
                }
            }
            else
            {
                int i = 0;

#if CV_SIMD128
                if( haveSIMD128 )
                {
                    for( ; i <= width-8; i+=8 )
                    {
                        v_int32x4 v_s0 = v_load(SUM + i) + v_load(Sp + i);
                        v_int32x4 v_s01 = v_load(SUM + i + 4) + v_load(Sp + i + 4);

                        v_store(D + i, v_cvt_f32(v_s0));
                        v_store(D + i + 4, v_cvt_f32(v_s01));

                        v_store(SUM + i, v_s0 - v_load(Sm + i));
                        v_store(SUM + i + 4, v_s01 - v_load(Sm + i + 4));
                    }
                }
#endif
                for( ; i < width; i++ )
                {
                    int s0 = SUM[i] + Sp[i];
                    D[i] = (float)(s0);
                    SUM[i] = s0 - Sm[i];
                }
            }
            dst += dststep;
        }
    }

    double scale;
    int sumCount;
    std::vector<int> sum;
};

#ifdef HAVE_OPENCL

static bool ocl_boxFilter3x3_8UC1( InputArray _src, OutputArray _dst, int ddepth,
                                   Size ksize, Point anchor, int borderType, bool normalize )
{
    const ocl::Device & dev = ocl::Device::getDefault();
    int type = _src.type(), sdepth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);

    if (ddepth < 0)
        ddepth = sdepth;

    if (anchor.x < 0)
        anchor.x = ksize.width / 2;
    if (anchor.y < 0)
        anchor.y = ksize.height / 2;

    if ( !(dev.isIntel() && (type == CV_8UC1) &&
         (_src.offset() == 0) && (_src.step() % 4 == 0) &&
         (_src.cols() % 16 == 0) && (_src.rows() % 2 == 0) &&
         (anchor.x == 1) && (anchor.y == 1) &&
         (ksize.width == 3) && (ksize.height == 3)) )
        return false;

    float alpha = 1.0f / (ksize.height * ksize.width);
    Size size = _src.size();
    size_t globalsize[2] = { 0, 0 };
    size_t localsize[2] = { 0, 0 };
    const char * const borderMap[] = { "BORDER_CONSTANT", "BORDER_REPLICATE", "BORDER_REFLECT", 0, "BORDER_REFLECT_101" };

    globalsize[0] = size.width / 16;
    globalsize[1] = size.height / 2;

    char build_opts[1024];
    sprintf(build_opts, "-D %s %s", borderMap[borderType], normalize ? "-D NORMALIZE" : "");

    ocl::Kernel kernel("boxFilter3x3_8UC1_cols16_rows2", cv::ocl::imgproc::boxFilter3x3_oclsrc, build_opts);
    if (kernel.empty())
        return false;

    UMat src = _src.getUMat();
    _dst.create(size, CV_MAKETYPE(ddepth, cn));
    if (!(_dst.offset() == 0 && _dst.step() % 4 == 0))
        return false;
    UMat dst = _dst.getUMat();

    int idxArg = kernel.set(0, ocl::KernelArg::PtrReadOnly(src));
    idxArg = kernel.set(idxArg, (int)src.step);
    idxArg = kernel.set(idxArg, ocl::KernelArg::PtrWriteOnly(dst));
    idxArg = kernel.set(idxArg, (int)dst.step);
    idxArg = kernel.set(idxArg, (int)dst.rows);
    idxArg = kernel.set(idxArg, (int)dst.cols);
    if (normalize)
        idxArg = kernel.set(idxArg, (float)alpha);

    return kernel.run(2, globalsize, (localsize[0] == 0) ? NULL : localsize, false);
}

#define DIVUP(total, grain) ((total + grain - 1) / (grain))
#define ROUNDUP(sz, n)      ((sz) + (n) - 1 - (((sz) + (n) - 1) % (n)))

static bool ocl_boxFilter( InputArray _src, OutputArray _dst, int ddepth,
                           Size ksize, Point anchor, int borderType, bool normalize, bool sqr = false )
{
    const ocl::Device & dev = ocl::Device::getDefault();
    int type = _src.type(), sdepth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type), esz = CV_ELEM_SIZE(type);
    bool doubleSupport = dev.doubleFPConfig() > 0;

    if (ddepth < 0)
        ddepth = sdepth;

    if (cn > 4 || (!doubleSupport && (sdepth == CV_64F || ddepth == CV_64F)) ||
        _src.offset() % esz != 0 || _src.step() % esz != 0)
        return false;

    if (anchor.x < 0)
        anchor.x = ksize.width / 2;
    if (anchor.y < 0)
        anchor.y = ksize.height / 2;

    int computeUnits = ocl::Device::getDefault().maxComputeUnits();
    float alpha = 1.0f / (ksize.height * ksize.width);
    Size size = _src.size(), wholeSize;
    bool isolated = (borderType & BORDER_ISOLATED) != 0;
    borderType &= ~BORDER_ISOLATED;
    int wdepth = std::max(CV_32F, std::max(ddepth, sdepth)),
        wtype = CV_MAKE_TYPE(wdepth, cn), dtype = CV_MAKE_TYPE(ddepth, cn);

    const char * const borderMap[] = { "BORDER_CONSTANT", "BORDER_REPLICATE", "BORDER_REFLECT", 0, "BORDER_REFLECT_101" };
    size_t globalsize[2] = { (size_t)size.width, (size_t)size.height };
    size_t localsize_general[2] = { 0, 1 }, * localsize = NULL;

    UMat src = _src.getUMat();
    if (!isolated)
    {
        Point ofs;
        src.locateROI(wholeSize, ofs);
    }

    int h = isolated ? size.height : wholeSize.height;
    int w = isolated ? size.width : wholeSize.width;

    size_t maxWorkItemSizes[32];
    ocl::Device::getDefault().maxWorkItemSizes(maxWorkItemSizes);
    int tryWorkItems = (int)maxWorkItemSizes[0];

    ocl::Kernel kernel;

    if (dev.isIntel() && !(dev.type() & ocl::Device::TYPE_CPU) &&
        ((ksize.width < 5 && ksize.height < 5 && esz <= 4) ||
         (ksize.width == 5 && ksize.height == 5 && cn == 1)))
    {
        if (w < ksize.width || h < ksize.height)
            return false;

        // Figure out what vector size to use for loading the pixels.
        int pxLoadNumPixels = cn != 1 || size.width % 4 ? 1 : 4;
        int pxLoadVecSize = cn * pxLoadNumPixels;

        // Figure out how many pixels per work item to compute in X and Y
        // directions.  Too many and we run out of registers.
        int pxPerWorkItemX = 1, pxPerWorkItemY = 1;
        if (cn <= 2 && ksize.width <= 4 && ksize.height <= 4)
        {
            pxPerWorkItemX = size.width % 8 ? size.width % 4 ? size.width % 2 ? 1 : 2 : 4 : 8;
            pxPerWorkItemY = size.height % 2 ? 1 : 2;
        }
        else if (cn < 4 || (ksize.width <= 4 && ksize.height <= 4))
        {
            pxPerWorkItemX = size.width % 2 ? 1 : 2;
            pxPerWorkItemY = size.height % 2 ? 1 : 2;
        }
        globalsize[0] = size.width / pxPerWorkItemX;
        globalsize[1] = size.height / pxPerWorkItemY;

        // Need some padding in the private array for pixels
        int privDataWidth = ROUNDUP(pxPerWorkItemX + ksize.width - 1, pxLoadNumPixels);

        // Make the global size a nice round number so the runtime can pick
        // from reasonable choices for the workgroup size
        const int wgRound = 256;
        globalsize[0] = ROUNDUP(globalsize[0], wgRound);

        char build_options[1024], cvt[2][40];
        sprintf(build_options, "-D cn=%d "
                "-D ANCHOR_X=%d -D ANCHOR_Y=%d -D KERNEL_SIZE_X=%d -D KERNEL_SIZE_Y=%d "
                "-D PX_LOAD_VEC_SIZE=%d -D PX_LOAD_NUM_PX=%d "
                "-D PX_PER_WI_X=%d -D PX_PER_WI_Y=%d -D PRIV_DATA_WIDTH=%d -D %s -D %s "
                "-D PX_LOAD_X_ITERATIONS=%d -D PX_LOAD_Y_ITERATIONS=%d "
                "-D srcT=%s -D srcT1=%s -D dstT=%s -D dstT1=%s -D WT=%s -D WT1=%s "
                "-D convertToWT=%s -D convertToDstT=%s%s%s -D PX_LOAD_FLOAT_VEC_CONV=convert_%s -D OP_BOX_FILTER",
                cn, anchor.x, anchor.y, ksize.width, ksize.height,
                pxLoadVecSize, pxLoadNumPixels,
                pxPerWorkItemX, pxPerWorkItemY, privDataWidth, borderMap[borderType],
                isolated ? "BORDER_ISOLATED" : "NO_BORDER_ISOLATED",
                privDataWidth / pxLoadNumPixels, pxPerWorkItemY + ksize.height - 1,
                ocl::typeToStr(type), ocl::typeToStr(sdepth), ocl::typeToStr(dtype),
                ocl::typeToStr(ddepth), ocl::typeToStr(wtype), ocl::typeToStr(wdepth),
                ocl::convertTypeStr(sdepth, wdepth, cn, cvt[0]),
                ocl::convertTypeStr(wdepth, ddepth, cn, cvt[1]),
                normalize ? " -D NORMALIZE" : "", sqr ? " -D SQR" : "",
                ocl::typeToStr(CV_MAKE_TYPE(wdepth, pxLoadVecSize)) //PX_LOAD_FLOAT_VEC_CONV
                );


        if (!kernel.create("filterSmall", cv::ocl::imgproc::filterSmall_oclsrc, build_options))
            return false;
    }
    else
    {
        localsize = localsize_general;
        for ( ; ; )
        {
            int BLOCK_SIZE_X = tryWorkItems, BLOCK_SIZE_Y = std::min(ksize.height * 10, size.height);

            while (BLOCK_SIZE_X > 32 && BLOCK_SIZE_X >= ksize.width * 2 && BLOCK_SIZE_X > size.width * 2)
                BLOCK_SIZE_X /= 2;
            while (BLOCK_SIZE_Y < BLOCK_SIZE_X / 8 && BLOCK_SIZE_Y * computeUnits * 32 < size.height)
                BLOCK_SIZE_Y *= 2;

            if (ksize.width > BLOCK_SIZE_X || w < ksize.width || h < ksize.height)
                return false;

            char cvt[2][50];
            String opts = format("-D LOCAL_SIZE_X=%d -D BLOCK_SIZE_Y=%d -D ST=%s -D DT=%s -D WT=%s -D convertToDT=%s -D convertToWT=%s"
                                 " -D ANCHOR_X=%d -D ANCHOR_Y=%d -D KERNEL_SIZE_X=%d -D KERNEL_SIZE_Y=%d -D %s%s%s%s%s"
                                 " -D ST1=%s -D DT1=%s -D cn=%d",
                                 BLOCK_SIZE_X, BLOCK_SIZE_Y, ocl::typeToStr(type), ocl::typeToStr(CV_MAKE_TYPE(ddepth, cn)),
                                 ocl::typeToStr(CV_MAKE_TYPE(wdepth, cn)),
                                 ocl::convertTypeStr(wdepth, ddepth, cn, cvt[0]),
                                 ocl::convertTypeStr(sdepth, wdepth, cn, cvt[1]),
                                 anchor.x, anchor.y, ksize.width, ksize.height, borderMap[borderType],
                                 isolated ? " -D BORDER_ISOLATED" : "", doubleSupport ? " -D DOUBLE_SUPPORT" : "",
                                 normalize ? " -D NORMALIZE" : "", sqr ? " -D SQR" : "",
                                 ocl::typeToStr(sdepth), ocl::typeToStr(ddepth), cn);

            localsize[0] = BLOCK_SIZE_X;
            globalsize[0] = DIVUP(size.width, BLOCK_SIZE_X - (ksize.width - 1)) * BLOCK_SIZE_X;
            globalsize[1] = DIVUP(size.height, BLOCK_SIZE_Y);

            kernel.create("boxFilter", cv::ocl::imgproc::boxFilter_oclsrc, opts);
            if (kernel.empty())
                return false;

            size_t kernelWorkGroupSize = kernel.workGroupSize();
            if (localsize[0] <= kernelWorkGroupSize)
                break;
            if (BLOCK_SIZE_X < (int)kernelWorkGroupSize)
                return false;

            tryWorkItems = (int)kernelWorkGroupSize;
        }
    }

    _dst.create(size, CV_MAKETYPE(ddepth, cn));
    UMat dst = _dst.getUMat();

    int idxArg = kernel.set(0, ocl::KernelArg::PtrReadOnly(src));
    idxArg = kernel.set(idxArg, (int)src.step);
    int srcOffsetX = (int)((src.offset % src.step) / src.elemSize());
    int srcOffsetY = (int)(src.offset / src.step);
    int srcEndX = isolated ? srcOffsetX + size.width : wholeSize.width;
    int srcEndY = isolated ? srcOffsetY + size.height : wholeSize.height;
    idxArg = kernel.set(idxArg, srcOffsetX);
    idxArg = kernel.set(idxArg, srcOffsetY);
    idxArg = kernel.set(idxArg, srcEndX);
    idxArg = kernel.set(idxArg, srcEndY);
    idxArg = kernel.set(idxArg, ocl::KernelArg::WriteOnly(dst));
    if (normalize)
        idxArg = kernel.set(idxArg, (float)alpha);

    return kernel.run(2, globalsize, localsize, false);
}

#undef ROUNDUP

#endif

}


cv::Ptr<cv::BaseRowFilter> cv::getRowSumFilter(int srcType, int sumType, int ksize, int anchor)
{
    int sdepth = CV_MAT_DEPTH(srcType), ddepth = CV_MAT_DEPTH(sumType);
    CV_Assert( CV_MAT_CN(sumType) == CV_MAT_CN(srcType) );

    if( anchor < 0 )
        anchor = ksize/2;

    if( sdepth == CV_8U && ddepth == CV_32S )
        return makePtr<RowSum<uchar, int> >(ksize, anchor);
    if( sdepth == CV_8U && ddepth == CV_16U )
        return makePtr<RowSum<uchar, ushort> >(ksize, anchor);
    if( sdepth == CV_8U && ddepth == CV_64F )
        return makePtr<RowSum<uchar, double> >(ksize, anchor);
    if( sdepth == CV_16U && ddepth == CV_32S )
        return makePtr<RowSum<ushort, int> >(ksize, anchor);
    if( sdepth == CV_16U && ddepth == CV_64F )
        return makePtr<RowSum<ushort, double> >(ksize, anchor);
    if( sdepth == CV_16S && ddepth == CV_32S )
        return makePtr<RowSum<short, int> >(ksize, anchor);
    if( sdepth == CV_32S && ddepth == CV_32S )
        return makePtr<RowSum<int, int> >(ksize, anchor);
    if( sdepth == CV_16S && ddepth == CV_64F )
        return makePtr<RowSum<short, double> >(ksize, anchor);
    if( sdepth == CV_32F && ddepth == CV_64F )
        return makePtr<RowSum<float, double> >(ksize, anchor);
    if( sdepth == CV_64F && ddepth == CV_64F )
        return makePtr<RowSum<double, double> >(ksize, anchor);

    CV_Error_( CV_StsNotImplemented,
        ("Unsupported combination of source format (=%d), and buffer format (=%d)",
        srcType, sumType));
}


cv::Ptr<cv::BaseColumnFilter> cv::getColumnSumFilter(int sumType, int dstType, int ksize,
                                                     int anchor, double scale)
{
    int sdepth = CV_MAT_DEPTH(sumType), ddepth = CV_MAT_DEPTH(dstType);
    CV_Assert( CV_MAT_CN(sumType) == CV_MAT_CN(dstType) );

    if( anchor < 0 )
        anchor = ksize/2;

    if( ddepth == CV_8U && sdepth == CV_32S )
        return makePtr<ColumnSum<int, uchar> >(ksize, anchor, scale);
    if( ddepth == CV_8U && sdepth == CV_16U )
        return makePtr<ColumnSum<ushort, uchar> >(ksize, anchor, scale);
    if( ddepth == CV_8U && sdepth == CV_64F )
        return makePtr<ColumnSum<double, uchar> >(ksize, anchor, scale);
    if( ddepth == CV_16U && sdepth == CV_32S )
        return makePtr<ColumnSum<int, ushort> >(ksize, anchor, scale);
    if( ddepth == CV_16U && sdepth == CV_64F )
        return makePtr<ColumnSum<double, ushort> >(ksize, anchor, scale);
    if( ddepth == CV_16S && sdepth == CV_32S )
        return makePtr<ColumnSum<int, short> >(ksize, anchor, scale);
    if( ddepth == CV_16S && sdepth == CV_64F )
        return makePtr<ColumnSum<double, short> >(ksize, anchor, scale);
    if( ddepth == CV_32S && sdepth == CV_32S )
        return makePtr<ColumnSum<int, int> >(ksize, anchor, scale);
    if( ddepth == CV_32F && sdepth == CV_32S )
        return makePtr<ColumnSum<int, float> >(ksize, anchor, scale);
    if( ddepth == CV_32F && sdepth == CV_64F )
        return makePtr<ColumnSum<double, float> >(ksize, anchor, scale);
    if( ddepth == CV_64F && sdepth == CV_32S )
        return makePtr<ColumnSum<int, double> >(ksize, anchor, scale);
    if( ddepth == CV_64F && sdepth == CV_64F )
        return makePtr<ColumnSum<double, double> >(ksize, anchor, scale);

    CV_Error_( CV_StsNotImplemented,
        ("Unsupported combination of sum format (=%d), and destination format (=%d)",
        sumType, dstType));
}


cv::Ptr<cv::FilterEngine> cv::createBoxFilter( int srcType, int dstType, Size ksize,
                    Point anchor, bool normalize, int borderType )
{
    int sdepth = CV_MAT_DEPTH(srcType);
    int cn = CV_MAT_CN(srcType), sumType = CV_64F;
    if( sdepth == CV_8U && CV_MAT_DEPTH(dstType) == CV_8U &&
        ksize.width*ksize.height <= 256 )
        sumType = CV_16U;
    else if( sdepth <= CV_32S && (!normalize ||
        ksize.width*ksize.height <= (sdepth == CV_8U ? (1<<23) :
            sdepth == CV_16U ? (1 << 15) : (1 << 16))) )
        sumType = CV_32S;
    sumType = CV_MAKETYPE( sumType, cn );

    Ptr<BaseRowFilter> rowFilter = getRowSumFilter(srcType, sumType, ksize.width, anchor.x );
    Ptr<BaseColumnFilter> columnFilter = getColumnSumFilter(sumType,
        dstType, ksize.height, anchor.y, normalize ? 1./(ksize.width*ksize.height) : 1);

    return makePtr<FilterEngine>(Ptr<BaseFilter>(), rowFilter, columnFilter,
           srcType, dstType, sumType, borderType );
}

#ifdef HAVE_OPENVX
namespace cv
{
    namespace ovx {
        template <> inline bool skipSmallImages<VX_KERNEL_BOX_3x3>(int w, int h) { return w*h < 640 * 480; }
    }
    static bool openvx_boxfilter(InputArray _src, OutputArray _dst, int ddepth,
                                 Size ksize, Point anchor,
                                 bool normalize, int borderType)
    {
        if (ddepth < 0)
            ddepth = CV_8UC1;
        if (_src.type() != CV_8UC1 || ddepth != CV_8U || !normalize ||
            _src.cols() < 3 || _src.rows() < 3 ||
            ksize.width != 3 || ksize.height != 3 ||
            (anchor.x >= 0 && anchor.x != 1) ||
            (anchor.y >= 0 && anchor.y != 1) ||
            ovx::skipSmallImages<VX_KERNEL_BOX_3x3>(_src.cols(), _src.rows()))
            return false;

        Mat src = _src.getMat();

        if ((borderType & BORDER_ISOLATED) == 0 && src.isSubmatrix())
            return false; //Process isolated borders only
        vx_enum border;
        switch (borderType & ~BORDER_ISOLATED)
        {
        case BORDER_CONSTANT:
            border = VX_BORDER_CONSTANT;
            break;
        case BORDER_REPLICATE:
            border = VX_BORDER_REPLICATE;
            break;
        default:
            return false;
        }

        _dst.create(src.size(), CV_8UC1);
        Mat dst = _dst.getMat();

        try
        {
            ivx::Context ctx = ovx::getOpenVXContext();

            Mat a;
            if (dst.data != src.data)
                a = src;
            else
                src.copyTo(a);

            ivx::Image
                ia = ivx::Image::createFromHandle(ctx, VX_DF_IMAGE_U8,
                                                  ivx::Image::createAddressing(a.cols, a.rows, 1, (vx_int32)(a.step)), a.data),
                ib = ivx::Image::createFromHandle(ctx, VX_DF_IMAGE_U8,
                                                  ivx::Image::createAddressing(dst.cols, dst.rows, 1, (vx_int32)(dst.step)), dst.data);

            //ATTENTION: VX_CONTEXT_IMMEDIATE_BORDER attribute change could lead to strange issues in multi-threaded environments
            //since OpenVX standard says nothing about thread-safety for now
            ivx::border_t prevBorder = ctx.immediateBorder();
            ctx.setImmediateBorder(border, (vx_uint8)(0));
            ivx::IVX_CHECK_STATUS(vxuBox3x3(ctx, ia, ib));
            ctx.setImmediateBorder(prevBorder);
        }
        catch (ivx::RuntimeError & e)
        {
            VX_DbgThrow(e.what());
        }
        catch (ivx::WrapperError & e)
        {
            VX_DbgThrow(e.what());
        }

        return true;
    }
}
#endif

#if defined(HAVE_IPP)
namespace cv
{
static bool ipp_boxfilter(Mat &src, Mat &dst, Size ksize, Point anchor, bool normalize, int borderType)
{
#ifdef HAVE_IPP_IW
    CV_INSTRUMENT_REGION_IPP()

#if IPP_VERSION_X100 < 201801
    // Problem with SSE42 optimization for 16s and some 8u modes
    if(ipp::getIppTopFeatures() == ippCPUID_SSE42 && (((src.depth() == CV_16S || src.depth() == CV_16U) && (src.channels() == 3 || src.channels() == 4)) || (src.depth() == CV_8U && src.channels() == 3 && (ksize.width > 5 || ksize.height > 5))))
        return false;

    // Other optimizations has some degradations too
    if((((src.depth() == CV_16S || src.depth() == CV_16U) && (src.channels() == 4)) || (src.depth() == CV_8U && src.channels() == 1 && (ksize.width > 5 || ksize.height > 5))))
        return false;
#endif

    if(!normalize)
        return false;

    if(!ippiCheckAnchor(anchor, ksize))
        return false;

    try
    {
        ::ipp::IwiImage       iwSrc      = ippiGetImage(src);
        ::ipp::IwiImage       iwDst      = ippiGetImage(dst);
        ::ipp::IwiSize        iwKSize    = ippiGetSize(ksize);
        ::ipp::IwiBorderSize  borderSize(iwKSize);
        ::ipp::IwiBorderType  ippBorder(ippiGetBorder(iwSrc, borderType, borderSize));
        if(!ippBorder)
            return false;

        CV_INSTRUMENT_FUN_IPP(::ipp::iwiFilterBox, iwSrc, iwDst, iwKSize, ::ipp::IwDefault(), ippBorder);
    }
    catch (::ipp::IwException)
    {
        return false;
    }

    return true;
#else
    CV_UNUSED(src); CV_UNUSED(dst); CV_UNUSED(ksize); CV_UNUSED(anchor); CV_UNUSED(normalize); CV_UNUSED(borderType);
    return false;
#endif
}
}
#endif


void cv::boxFilter( InputArray _src, OutputArray _dst, int ddepth,
                Size ksize, Point anchor,
                bool normalize, int borderType )
{
    CV_INSTRUMENT_REGION()

    CV_OCL_RUN(_dst.isUMat() &&
               (borderType == BORDER_REPLICATE || borderType == BORDER_CONSTANT ||
                borderType == BORDER_REFLECT || borderType == BORDER_REFLECT_101),
               ocl_boxFilter3x3_8UC1(_src, _dst, ddepth, ksize, anchor, borderType, normalize))

    CV_OCL_RUN(_dst.isUMat(), ocl_boxFilter(_src, _dst, ddepth, ksize, anchor, borderType, normalize))

    Mat src = _src.getMat();
    int stype = src.type(), sdepth = CV_MAT_DEPTH(stype), cn = CV_MAT_CN(stype);
    if( ddepth < 0 )
        ddepth = sdepth;
    _dst.create( src.size(), CV_MAKETYPE(ddepth, cn) );
    Mat dst = _dst.getMat();
    if( borderType != BORDER_CONSTANT && normalize && (borderType & BORDER_ISOLATED) != 0 )
    {
        if( src.rows == 1 )
            ksize.height = 1;
        if( src.cols == 1 )
            ksize.width = 1;
    }

    Point ofs;
    Size wsz(src.cols, src.rows);
    if(!(borderType&BORDER_ISOLATED))
        src.locateROI( wsz, ofs );

    CALL_HAL(boxFilter, cv_hal_boxFilter, src.ptr(), src.step, dst.ptr(), dst.step, src.cols, src.rows, sdepth, ddepth, cn,
             ofs.x, ofs.y, wsz.width - src.cols - ofs.x, wsz.height - src.rows - ofs.y, ksize.width, ksize.height,
             anchor.x, anchor.y, normalize, borderType&~BORDER_ISOLATED);

    CV_OVX_RUN(true,
               openvx_boxfilter(src, dst, ddepth, ksize, anchor, normalize, borderType))

    CV_IPP_RUN_FAST(ipp_boxfilter(src, dst, ksize, anchor, normalize, borderType));

    borderType = (borderType&~BORDER_ISOLATED);

    Ptr<FilterEngine> f = createBoxFilter( src.type(), dst.type(),
                        ksize, anchor, normalize, borderType );

    f->apply( src, dst, wsz, ofs );
}


void cv::blur( InputArray src, OutputArray dst,
           Size ksize, Point anchor, int borderType )
{
    CV_INSTRUMENT_REGION()

    boxFilter( src, dst, -1, ksize, anchor, true, borderType );
}


/****************************************************************************************\
                                    Squared Box Filter
\****************************************************************************************/

namespace cv
{

template<typename T, typename ST>
struct SqrRowSum :
        public BaseRowFilter
{
    SqrRowSum( int _ksize, int _anchor ) :
        BaseRowFilter()
    {
        ksize = _ksize;
        anchor = _anchor;
    }

    virtual void operator()(const uchar* src, uchar* dst, int width, int cn) CV_OVERRIDE
    {
        const T* S = (const T*)src;
        ST* D = (ST*)dst;
        int i = 0, k, ksz_cn = ksize*cn;

        width = (width - 1)*cn;
        for( k = 0; k < cn; k++, S++, D++ )
        {
            ST s = 0;
            for( i = 0; i < ksz_cn; i += cn )
            {
                ST val = (ST)S[i];
                s += val*val;
            }
            D[0] = s;
            for( i = 0; i < width; i += cn )
            {
                ST val0 = (ST)S[i], val1 = (ST)S[i + ksz_cn];
                s += val1*val1 - val0*val0;
                D[i+cn] = s;
            }
        }
    }
};

static Ptr<BaseRowFilter> getSqrRowSumFilter(int srcType, int sumType, int ksize, int anchor)
{
    int sdepth = CV_MAT_DEPTH(srcType), ddepth = CV_MAT_DEPTH(sumType);
    CV_Assert( CV_MAT_CN(sumType) == CV_MAT_CN(srcType) );

    if( anchor < 0 )
        anchor = ksize/2;

    if( sdepth == CV_8U && ddepth == CV_32S )
        return makePtr<SqrRowSum<uchar, int> >(ksize, anchor);
    if( sdepth == CV_8U && ddepth == CV_64F )
        return makePtr<SqrRowSum<uchar, double> >(ksize, anchor);
    if( sdepth == CV_16U && ddepth == CV_64F )
        return makePtr<SqrRowSum<ushort, double> >(ksize, anchor);
    if( sdepth == CV_16S && ddepth == CV_64F )
        return makePtr<SqrRowSum<short, double> >(ksize, anchor);
    if( sdepth == CV_32F && ddepth == CV_64F )
        return makePtr<SqrRowSum<float, double> >(ksize, anchor);
    if( sdepth == CV_64F && ddepth == CV_64F )
        return makePtr<SqrRowSum<double, double> >(ksize, anchor);

    CV_Error_( CV_StsNotImplemented,
              ("Unsupported combination of source format (=%d), and buffer format (=%d)",
               srcType, sumType));
}

}

void cv::sqrBoxFilter( InputArray _src, OutputArray _dst, int ddepth,
                       Size ksize, Point anchor,
                       bool normalize, int borderType )
{
    CV_INSTRUMENT_REGION()

    int srcType = _src.type(), sdepth = CV_MAT_DEPTH(srcType), cn = CV_MAT_CN(srcType);
    Size size = _src.size();

    if( ddepth < 0 )
        ddepth = sdepth < CV_32F ? CV_32F : CV_64F;

    if( borderType != BORDER_CONSTANT && normalize )
    {
        if( size.height == 1 )
            ksize.height = 1;
        if( size.width == 1 )
            ksize.width = 1;
    }

    CV_OCL_RUN(_dst.isUMat() && _src.dims() <= 2,
               ocl_boxFilter(_src, _dst, ddepth, ksize, anchor, borderType, normalize, true))

    int sumDepth = CV_64F;
    if( sdepth == CV_8U )
        sumDepth = CV_32S;
    int sumType = CV_MAKETYPE( sumDepth, cn ), dstType = CV_MAKETYPE(ddepth, cn);

    Mat src = _src.getMat();
    _dst.create( size, dstType );
    Mat dst = _dst.getMat();

    Ptr<BaseRowFilter> rowFilter = getSqrRowSumFilter(srcType, sumType, ksize.width, anchor.x );
    Ptr<BaseColumnFilter> columnFilter = getColumnSumFilter(sumType,
                                                            dstType, ksize.height, anchor.y,
                                                            normalize ? 1./(ksize.width*ksize.height) : 1);

    Ptr<FilterEngine> f = makePtr<FilterEngine>(Ptr<BaseFilter>(), rowFilter, columnFilter,
                                                srcType, dstType, sumType, borderType );
    Point ofs;
    Size wsz(src.cols, src.rows);
    src.locateROI( wsz, ofs );

    f->apply( src, dst, wsz, ofs );
}


/****************************************************************************************\
                                     Gaussian Blur
\****************************************************************************************/

cv::Mat cv::getGaussianKernel( int n, double sigma, int ktype )
{
    CV_Assert(n > 0);
    const int SMALL_GAUSSIAN_SIZE = 7;
    static const float small_gaussian_tab[][SMALL_GAUSSIAN_SIZE] =
    {
        {1.f},
        {0.25f, 0.5f, 0.25f},
        {0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f},
        {0.03125f, 0.109375f, 0.21875f, 0.28125f, 0.21875f, 0.109375f, 0.03125f}
    };

    const float* fixed_kernel = n % 2 == 1 && n <= SMALL_GAUSSIAN_SIZE && sigma <= 0 ?
        small_gaussian_tab[n>>1] : 0;

    CV_Assert( ktype == CV_32F || ktype == CV_64F );
    Mat kernel(n, 1, ktype);
    float* cf = kernel.ptr<float>();
    double* cd = kernel.ptr<double>();

    double sigmaX = sigma > 0 ? sigma : ((n-1)*0.5 - 1)*0.3 + 0.8;
    double scale2X = -0.5/(sigmaX*sigmaX);
    double sum = 0;

    int i;
    for( i = 0; i < n; i++ )
    {
        double x = i - (n-1)*0.5;
        double t = fixed_kernel ? (double)fixed_kernel[i] : std::exp(scale2X*x*x);
        if( ktype == CV_32F )
        {
            cf[i] = (float)t;
            sum += cf[i];
        }
        else
        {
            cd[i] = t;
            sum += cd[i];
        }
    }

    CV_DbgAssert(fabs(sum) > 0);
    sum = 1./sum;
    for( i = 0; i < n; i++ )
    {
        if( ktype == CV_32F )
            cf[i] = (float)(cf[i]*sum);
        else
            cd[i] *= sum;
    }

    return kernel;
}

namespace cv {

template <typename T>
static std::vector<T> getFixedpointGaussianKernel( int n, double sigma )
{
    if (sigma <= 0)
    {
        if(n == 1)
            return std::vector<T>(1, softdouble(1.0));
        else if(n == 3)
        {
            T v3[] = { softdouble(0.25), softdouble(0.5), softdouble(0.25) };
            return std::vector<T>(v3, v3 + 3);
        }
        else if(n == 5)
        {
            T v5[] = { softdouble(0.0625), softdouble(0.25), softdouble(0.375), softdouble(0.25), softdouble(0.0625) };
            return std::vector<T>(v5, v5 + 5);
        }
        else if(n == 7)
        {
            T v7[] = { softdouble(0.03125), softdouble(0.109375), softdouble(0.21875), softdouble(0.28125), softdouble(0.21875), softdouble(0.109375), softdouble(0.03125) };
            return std::vector<T>(v7, v7 + 7);
        }
    }


    softdouble sigmaX = sigma > 0 ? softdouble(sigma) : mulAdd(softdouble(n),softdouble(0.15),softdouble(0.35));// softdouble(((n-1)*0.5 - 1)*0.3 + 0.8)
    softdouble scale2X = softdouble(-0.5*0.25)/(sigmaX*sigmaX);
    std::vector<softdouble> values(n);
    softdouble sum(0.);
    for(int i = 0, x = 1 - n; i < n; i++, x+=2 )
    {
        // x = i - (n - 1)*0.5
        // t = std::exp(scale2X*x*x)
        values[i] = exp(softdouble(x*x)*scale2X);
        sum += values[i];
    }
    sum = softdouble::one()/sum;

    std::vector<T> kernel(n);
    for(int i = 0; i < n; i++ )
    {
        kernel[i] = values[i] * sum;
    }

    return kernel;
};

template <typename ET, typename FT>
void hlineSmooth1N(const ET* src, int cn, const FT* m, int, FT* dst, int len, int)
{
    for (int i = 0; i < len*cn; i++, src++, dst++)
        *dst = (*m) * (*src);
}
template <>
void hlineSmooth1N<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16* m, int, ufixedpoint16* dst, int len, int)
{
    int lencn = len*cn;
    v_uint16x8 v_mul = v_setall_u16(*((uint16_t*)m));
    int i = 0;
    for (; i <= lencn - 16; i += 16)
    {
        v_uint8x16 v_src = v_load(src + i);
        v_uint16x8 v_tmp0, v_tmp1;
        v_expand(v_src, v_tmp0, v_tmp1);
        v_store((uint16_t*)dst + i, v_mul*v_tmp0);
        v_store((uint16_t*)dst + i + 8, v_mul*v_tmp1);
    }
    if (i <= lencn - 8)
    {
        v_uint16x8 v_src = v_load_expand(src + i);
        v_store((uint16_t*)dst + i, v_mul*v_src);
        i += 8;
    }
    for (; i < lencn; i++)
        dst[i] = m[0] * src[i];
}
template <typename ET, typename FT>
void hlineSmooth1N1(const ET* src, int cn, const FT*, int, FT* dst, int len, int)
{
    for (int i = 0; i < len*cn; i++, src++, dst++)
        *dst = *src;
}
template <>
void hlineSmooth1N1<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16*, int, ufixedpoint16* dst, int len, int)
{
    int lencn = len*cn;
    int i = 0;
    for (; i <= lencn - 16; i += 16)
    {
        v_uint8x16 v_src = v_load(src + i);
        v_uint16x8 v_tmp0, v_tmp1;
        v_expand(v_src, v_tmp0, v_tmp1);
        v_store((uint16_t*)dst + i, v_shl<8>(v_tmp0));
        v_store((uint16_t*)dst + i + 8, v_shl<8>(v_tmp1));
    }
    if (i <= lencn - 8)
    {
        v_uint16x8 v_src = v_load_expand(src + i);
        v_store((uint16_t*)dst + i, v_shl<8>(v_src));
        i += 8;
    }
    for (; i < lencn; i++)
        dst[i] = src[i];
}
template <typename ET, typename FT>
void hlineSmooth3N(const ET* src, int cn, const FT* m, int, FT* dst, int len, int borderType)
{
    if (len == 1)
    {
        FT msum = borderType != BORDER_CONSTANT ? m[0] + m[1] + m[2] : m[1];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else
    {
        // Point that fall left from border
        for (int k = 0; k < cn; k++)
            dst[k] = m[1] * src[k] + m[2] * src[cn + k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = borderInterpolate(-1, len, borderType);
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[0] * src[src_idx*cn + k];
        }

        src += cn; dst += cn;
        for (int i = cn; i < (len - 1)*cn; i++, src++, dst++)
            *dst = m[0] * src[-cn] + m[1] * src[0] + m[2] * src[cn];

        // Point that fall right from border
        for (int k = 0; k < cn; k++)
            dst[k] = m[0] * src[k - cn] + m[1] * src[k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = (borderInterpolate(len, len, borderType) - (len - 1))*cn;
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[2] * src[src_idx + k];
        }
    }
}
template <>
void hlineSmooth3N<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16* m, int, ufixedpoint16* dst, int len, int borderType)
{
    if (len == 1)
    {
        ufixedpoint16 msum = borderType != BORDER_CONSTANT ? m[0] + m[1] + m[2] : m[1];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else
    {
        // Point that fall left from border
        for (int k = 0; k < cn; k++)
            dst[k] = m[1] * src[k] + m[2] * src[cn + k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = borderInterpolate(-1, len, borderType);
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[0] * src[src_idx*cn + k];
        }

        src += cn; dst += cn;
        int i = cn, lencn = (len - 1)*cn;
        v_uint16x8 v_mul0 = v_setall_u16(*((uint16_t*)m));
        v_uint16x8 v_mul1 = v_setall_u16(*((uint16_t*)(m + 1)));
        v_uint16x8 v_mul2 = v_setall_u16(*((uint16_t*)(m + 2)));
        for (; i <= lencn - 16; i += 16, src += 16, dst += 16)
        {
            v_uint16x8 v_src00, v_src01, v_src10, v_src11, v_src20, v_src21;
            v_expand(v_load(src - cn), v_src00, v_src01);
            v_expand(v_load(src), v_src10, v_src11);
            v_expand(v_load(src + cn), v_src20, v_src21);
            v_store((uint16_t*)dst, v_src00 * v_mul0 + v_src10 * v_mul1 + v_src20 * v_mul2);
            v_store((uint16_t*)dst + 8, v_src01 * v_mul0 + v_src11 * v_mul1 + v_src21 * v_mul2);
        }
        for (; i < lencn; i++, src++, dst++)
            *dst = m[0] * src[-cn] + m[1] * src[0] + m[2] * src[cn];

        // Point that fall right from border
        for (int k = 0; k < cn; k++)
            dst[k] = m[0] * src[k - cn] + m[1] * src[k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = (borderInterpolate(len, len, borderType) - (len - 1))*cn;
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[2] * src[src_idx + k];
        }
    }
}
template <typename ET, typename FT>
void hlineSmooth3N121(const ET* src, int cn, const FT*, int, FT* dst, int len, int borderType)
{
    if (len == 1)
    {
        if(borderType != BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
                dst[k] = FT(src[k]);
        else
            for (int k = 0; k < cn; k++)
                dst[k] = FT(src[k])>>1;
    }
    else
    {
        // Point that fall left from border
        for (int k = 0; k < cn; k++)
            dst[k] = (FT(src[k])>>1) + (FT(src[cn + k])>>2);
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = borderInterpolate(-1, len, borderType);
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + (FT(src[src_idx*cn + k])>>2);
        }

        src += cn; dst += cn;
        for (int i = cn; i < (len - 1)*cn; i++, src++, dst++)
            *dst = (FT(src[-cn])>>2) + (FT(src[cn])>>2) + (FT(src[0])>>1);

        // Point that fall right from border
        for (int k = 0; k < cn; k++)
            dst[k] = (FT(src[k - cn])>>2) + (FT(src[k])>>1);
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = (borderInterpolate(len, len, borderType) - (len - 1))*cn;
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + (FT(src[src_idx + k])>>2);
        }
    }
}
template <>
void hlineSmooth3N121<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16*, int, ufixedpoint16* dst, int len, int borderType)
{
    if (len == 1)
    {
        if (borderType != BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
                dst[k] = ufixedpoint16(src[k]);
        else
            for (int k = 0; k < cn; k++)
                dst[k] = ufixedpoint16(src[k]) >> 1;
    }
    else
    {
        // Point that fall left from border
        for (int k = 0; k < cn; k++)
            dst[k] = (ufixedpoint16(src[k])>>1) + (ufixedpoint16(src[cn + k])>>2);
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = borderInterpolate(-1, len, borderType);
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + (ufixedpoint16(src[src_idx*cn + k])>>2);
        }

        src += cn; dst += cn;
        int i = cn, lencn = (len - 1)*cn;
        for (; i <= lencn - 16; i += 16, src += 16, dst += 16)
        {
            v_uint16x8 v_src00, v_src01, v_src10, v_src11, v_src20, v_src21;
            v_expand(v_load(src - cn), v_src00, v_src01);
            v_expand(v_load(src), v_src10, v_src11);
            v_expand(v_load(src + cn), v_src20, v_src21);
            v_store((uint16_t*)dst, (v_src00 + v_src20 + (v_src10 << 1)) << 6);
            v_store((uint16_t*)dst + 8, (v_src01 + v_src21 + (v_src11 << 1)) << 6);
        }
        for (; i < lencn; i++, src++, dst++)
            *((uint16_t*)dst) = (uint16_t(src[-cn]) + uint16_t(src[cn]) + (uint16_t(src[0]) << 1)) << 6;

        // Point that fall right from border
        for (int k = 0; k < cn; k++)
            dst[k] = (ufixedpoint16(src[k - cn])>>2) + (ufixedpoint16(src[k])>>1);
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = (borderInterpolate(len, len, borderType) - (len - 1))*cn;
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + (ufixedpoint16(src[src_idx + k])>>2);
        }
    }
}
template <typename ET, typename FT>
void hlineSmooth3Naba(const ET* src, int cn, const FT* m, int, FT* dst, int len, int borderType)
{
    if (len == 1)
    {
        FT msum = borderType != BORDER_CONSTANT ? (m[0]<<1) + m[1] : m[1];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else
    {
        // Point that fall left from border
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = borderInterpolate(-1, len, borderType);
            for (int k = 0; k < cn; k++)
                dst[k] = m[1] * src[k] + m[0] * src[cn + k] + m[0] * src[src_idx*cn + k];
        }
        else
        {
            for (int k = 0; k < cn; k++)
                dst[k] = m[1] * src[k] + m[0] * src[cn + k];
        }

        src += cn; dst += cn;
        for (int i = cn; i < (len - 1)*cn; i++, src++, dst++)
            *dst = m[1] * src[0] + m[0] * src[-cn] + m[0] * src[cn];

        // Point that fall right from border
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = (borderInterpolate(len, len, borderType) - (len - 1))*cn;
            for (int k = 0; k < cn; k++)
                dst[k] = m[1] * src[k] + m[0] * src[k - cn] + m[0] * src[src_idx + k];
        }
        else
        {
            for (int k = 0; k < cn; k++)
                dst[k] = m[0] * src[k - cn] + m[1] * src[k];
        }
    }
}
template <>
void hlineSmooth3Naba<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16* m, int, ufixedpoint16* dst, int len, int borderType)
{
    if (len == 1)
    {
        ufixedpoint16 msum = borderType != BORDER_CONSTANT ? (m[0]<<1) + m[1] : m[1];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else
    {
        // Point that fall left from border
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = borderInterpolate(-1, len, borderType);
            for (int k = 0; k < cn; k++)
                ((uint16_t*)dst)[k] = ((uint16_t*)m)[1] * src[k] + ((uint16_t*)m)[0] * ((uint16_t)(src[cn + k]) + (uint16_t)(src[src_idx*cn + k]));
        }
        else
        {
            for (int k = 0; k < cn; k++)
                dst[k] = m[1] * src[k] + m[0] * src[cn + k];
        }

        src += cn; dst += cn;
        int i = cn, lencn = (len - 1)*cn;
        v_uint16x8 v_mul0 = v_setall_u16(*((uint16_t*)m));
        v_uint16x8 v_mul1 = v_setall_u16(*((uint16_t*)m+1));
        for (; i <= lencn - 16; i += 16, src += 16, dst += 16)
        {
            v_uint16x8 v_src00, v_src01, v_src10, v_src11, v_src20, v_src21;
            v_expand(v_load(src - cn), v_src00, v_src01);
            v_expand(v_load(src), v_src10, v_src11);
            v_expand(v_load(src + cn), v_src20, v_src21);
            v_store((uint16_t*)dst, (v_src00 + v_src20) * v_mul0 + v_src10 * v_mul1);
            v_store((uint16_t*)dst + 8, (v_src01 + v_src21) * v_mul0 + v_src11 * v_mul1);
        }
        for (; i < lencn; i++, src++, dst++)
            *((uint16_t*)dst) = ((uint16_t*)m)[1] * src[0] + ((uint16_t*)m)[0] * ((uint16_t)(src[-cn]) + (uint16_t)(src[cn]));

        // Point that fall right from border
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int src_idx = (borderInterpolate(len, len, borderType) - (len - 1))*cn;
            for (int k = 0; k < cn; k++)
                ((uint16_t*)dst)[k] = ((uint16_t*)m)[1] * src[k] + ((uint16_t*)m)[0] * ((uint16_t)(src[k - cn]) + (uint16_t)(src[src_idx + k]));
        }
        else
        {
            for (int k = 0; k < cn; k++)
                dst[k] = m[0] * src[k - cn] + m[1] * src[k];
        }
    }
}
template <typename ET, typename FT>
void hlineSmooth5N(const ET* src, int cn, const FT* m, int, FT* dst, int len, int borderType)
{
    if (len == 1)
    {
        FT msum = borderType != BORDER_CONSTANT ? m[0] + m[1] + m[2] + m[3] + m[4] : m[2];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else if (len == 2)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k   ] = m[2] * src[k] + m[3] * src[k+cn];
                dst[k+cn] = m[1] * src[k] + m[2] * src[k+cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(2, len, borderType)*cn;
            int idxp2 = borderInterpolate(3, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k     ] = m[1] * src[k + idxm1] + m[2] * src[k] + m[3] * src[k + cn] + m[4] * src[k + idxp1] + m[0] * src[k + idxm2];
                dst[k + cn] = m[0] * src[k + idxm1] + m[1] * src[k] + m[2] * src[k + cn] + m[3] * src[k + idxp1] + m[4] * src[k + idxp2];
            }
        }
    }
    else if (len == 3)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k       ] = m[2] * src[k] + m[3] * src[k + cn] + m[4] * src[k + 2*cn];
                dst[k +   cn] = m[1] * src[k] + m[2] * src[k + cn] + m[3] * src[k + 2*cn];
                dst[k + 2*cn] = m[0] * src[k] + m[1] * src[k + cn] + m[2] * src[k + 2*cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(3, len, borderType)*cn;
            int idxp2 = borderInterpolate(4, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k       ] = m[2] * src[k] + m[3] * src[k + cn] + m[4] * src[k + 2*cn] + m[0] * src[k + idxm2] + m[1] * src[k + idxm1];
                dst[k +   cn] = m[1] * src[k] + m[2] * src[k + cn] + m[3] * src[k + 2*cn] + m[0] * src[k + idxm1] + m[4] * src[k + idxp1];
                dst[k + 2*cn] = m[0] * src[k] + m[1] * src[k + cn] + m[2] * src[k + 2*cn] + m[3] * src[k + idxp1] + m[4] * src[k + idxp2];
            }
        }
    }
    else
    {
        // Points that fall left from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = m[2] * src[k] + m[3] * src[cn + k] + m[4] * src[2*cn + k];
            dst[k + cn] = m[1] * src[k] + m[2] * src[cn + k] + m[3] * src[2*cn + k] + m[4] * src[3*cn + k];
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + m[0] * src[idxm2 + k] + m[1] * src[idxm1 + k];
                dst[k + cn] = dst[k + cn] + m[0] * src[idxm1 + k];
            }
        }

        src += 2*cn; dst += 2*cn;
        for (int i = 2*cn; i < (len - 2)*cn; i++, src++, dst++)
            *dst = m[0] * src[-2*cn] + m[1] * src[-cn] + m[2] * src[0] + m[3] * src[cn] + m[4] * src[2*cn];

        // Points that fall right from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = m[0] * src[k - 2*cn] + m[1] * src[k - cn] + m[2] * src[k] + m[3] * src[k + cn];
            dst[k + cn] = m[0] * src[k - cn] + m[1] * src[k] + m[2] * src[k + cn];
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxp1 = (borderInterpolate(len, len, borderType) - (len - 2))*cn;
            int idxp2 = (borderInterpolate(len+1, len, borderType) - (len - 2))*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + m[4] * src[idxp1 + k];
                dst[k + cn] = dst[k + cn] + m[3] * src[idxp1 + k] + m[4] * src[idxp2 + k];
            }
        }
    }
}
template <>
void hlineSmooth5N<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16* m, int, ufixedpoint16* dst, int len, int borderType)
{
    if (len == 1)
    {
        ufixedpoint16 msum = borderType != BORDER_CONSTANT ? m[0] + m[1] + m[2] + m[3] + m[4] : m[2];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else if (len == 2)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[3] * src[k + cn];
                dst[k + cn] = m[1] * src[k] + m[2] * src[k + cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(2, len, borderType)*cn;
            int idxp2 = borderInterpolate(3, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[1] * src[k + idxm1] + m[2] * src[k] + m[3] * src[k + cn] + m[4] * src[k + idxp1] + m[0] * src[k + idxm2];
                dst[k + cn] = m[0] * src[k + idxm1] + m[1] * src[k] + m[2] * src[k + cn] + m[3] * src[k + idxp1] + m[4] * src[k + idxp2];
            }
        }
    }
    else if (len == 3)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[3] * src[k + cn] + m[4] * src[k + 2 * cn];
                dst[k + cn] = m[1] * src[k] + m[2] * src[k + cn] + m[3] * src[k + 2 * cn];
                dst[k + 2 * cn] = m[0] * src[k] + m[1] * src[k + cn] + m[2] * src[k + 2 * cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(3, len, borderType)*cn;
            int idxp2 = borderInterpolate(4, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[3] * src[k + cn] + m[4] * src[k + 2 * cn] + m[0] * src[k + idxm2] + m[1] * src[k + idxm1];
                dst[k + cn] = m[1] * src[k] + m[2] * src[k + cn] + m[3] * src[k + 2 * cn] + m[0] * src[k + idxm1] + m[4] * src[k + idxp1];
                dst[k + 2 * cn] = m[0] * src[k] + m[1] * src[k + cn] + m[2] * src[k + 2 * cn] + m[3] * src[k + idxp1] + m[4] * src[k + idxp2];
            }
        }
    }
    else
    {
        // Points that fall left from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = m[2] * src[k] + m[3] * src[cn + k] + m[4] * src[2 * cn + k];
            dst[k + cn] = m[1] * src[k] + m[2] * src[cn + k] + m[3] * src[2 * cn + k] + m[4] * src[3 * cn + k];
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + m[0] * src[idxm2 + k] + m[1] * src[idxm1 + k];
                dst[k + cn] = dst[k + cn] + m[0] * src[idxm1 + k];
            }
        }

        src += 2 * cn; dst += 2 * cn;
        int i = 2*cn, lencn = (len - 2)*cn;
        v_uint16x8 v_mul0 = v_setall_u16(*((uint16_t*)m));
        v_uint16x8 v_mul1 = v_setall_u16(*((uint16_t*)(m + 1)));
        v_uint16x8 v_mul2 = v_setall_u16(*((uint16_t*)(m + 2)));
        v_uint16x8 v_mul3 = v_setall_u16(*((uint16_t*)(m + 3)));
        v_uint16x8 v_mul4 = v_setall_u16(*((uint16_t*)(m + 4)));
        for (; i <= lencn - 16; i += 16, src += 16, dst += 16)
        {
            v_uint16x8 v_src00, v_src01, v_src10, v_src11, v_src20, v_src21, v_src30, v_src31, v_src40, v_src41;
            v_expand(v_load(src - 2*cn), v_src00, v_src01);
            v_expand(v_load(src - cn), v_src10, v_src11);
            v_expand(v_load(src), v_src20, v_src21);
            v_expand(v_load(src + cn), v_src30, v_src31);
            v_expand(v_load(src + 2*cn), v_src40, v_src41);
            v_store((uint16_t*)dst, v_src00 * v_mul0 + v_src10 * v_mul1 + v_src20 * v_mul2 + v_src30 * v_mul3 + v_src40 * v_mul4);
            v_store((uint16_t*)dst + 8, v_src01 * v_mul0 + v_src11 * v_mul1 + v_src21 * v_mul2 + v_src31 * v_mul3 + v_src41 * v_mul4);
        }
        for (; i < lencn; i++, src++, dst++)
            *dst = m[0] * src[-2*cn] + m[1] * src[-cn] + m[2] * src[0] + m[3] * src[cn] + m[4] * src[2*cn];

        // Points that fall right from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = m[0] * src[k - 2 * cn] + m[1] * src[k - cn] + m[2] * src[k] + m[3] * src[k + cn];
            dst[k + cn] = m[0] * src[k - cn] + m[1] * src[k] + m[2] * src[k + cn];
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxp1 = (borderInterpolate(len, len, borderType) - (len - 2))*cn;
            int idxp2 = (borderInterpolate(len + 1, len, borderType) - (len - 2))*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + m[4] * src[idxp1 + k];
                dst[k + cn] = dst[k + cn] + m[3] * src[idxp1 + k] + m[4] * src[idxp2 + k];
            }
        }
    }
}
template <typename ET, typename FT>
void hlineSmooth5N14641(const ET* src, int cn, const FT*, int, FT* dst, int len, int borderType)
{
    if (len == 1)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
                dst[k] = (FT(src[k])>>3)*(uint8_t)3;
        else
            for (int k = 0; k < cn; k++)
                dst[k] = src[k];
    }
    else if (len == 2)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (FT(src[k])>>4)*(uint8_t)6 + (FT(src[k + cn])>>2);
                dst[k + cn] = (FT(src[k]) >> 2) + (FT(src[k + cn])>>4)*(uint8_t)6;
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(2, len, borderType)*cn;
            int idxp2 = borderInterpolate(3, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (FT(src[k])>>4)*(uint8_t)6 + (FT(src[k + idxm1])>>2) + (FT(src[k + cn])>>2) + (FT(src[k + idxp1])>>4) + (FT(src[k + idxm2])>>4);
                dst[k + cn] = (FT(src[k + cn])>>4)*(uint8_t)6 + (FT(src[k])>>2) + (FT(src[k + idxp1])>>2) + (FT(src[k + idxm1])>>4) + (FT(src[k + idxp2])>>4);
            }
        }
    }
    else if (len == 3)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (FT(src[k])>>4)*(uint8_t)6 + (FT(src[k + cn])>>2) + (FT(src[k + 2 * cn])>>4);
                dst[k + cn] = (FT(src[k + cn])>>4)*(uint8_t)6 + (FT(src[k])>>2) + (FT(src[k + 2 * cn])>>2);
                dst[k + 2 * cn] = (FT(src[k + 2 * cn])>>4)*(uint8_t)6 + (FT(src[k + cn])>>2) + (FT(src[k])>>4);
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(3, len, borderType)*cn;
            int idxp2 = borderInterpolate(4, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (FT(src[k])>>4)*(uint8_t)6 + (FT(src[k + cn])>>2) + (FT(src[k + idxm1])>>2) + (FT(src[k + 2 * cn])>>4) + (FT(src[k + idxm2])>>4);
                dst[k + cn] = (FT(src[k + cn])>>4)*(uint8_t)6 + (FT(src[k])>>2) + (FT(src[k + 2 * cn])>>2) + (FT(src[k + idxm1])>>4) + (FT(src[k + idxp1])>>4);
                dst[k + 2 * cn] = (FT(src[k + 2 * cn])>>4)*(uint8_t)6 + (FT(src[k + cn])>>2) + (FT(src[k + idxp1])>>2) + (FT(src[k])>>4) + (FT(src[k + idxp2])>>4);
            }
        }
    }
    else
    {
        // Points that fall left from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = (FT(src[k])>>4)*(uint8_t)6 + (FT(src[cn + k])>>2) + (FT(src[2 * cn + k])>>4);
            dst[k + cn] = (FT(src[cn + k])>>4)*(uint8_t)6 + (FT(src[k])>>2) + (FT(src[2 * cn + k])>>2) + (FT(src[3 * cn + k])>>4);
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + (FT(src[idxm2 + k])>>4) + (FT(src[idxm1 + k])>>2);
                dst[k + cn] = dst[k + cn] + (FT(src[idxm1 + k])>>4);
            }
        }

        src += 2 * cn; dst += 2 * cn;
        for (int i = 2 * cn; i < (len - 2)*cn; i++, src++, dst++)
            *dst = (FT(src[0])>>4)*(uint8_t)6 + (FT(src[-cn])>>2) + (FT(src[cn])>>2) + (FT(src[-2 * cn])>>4) + (FT(src[2 * cn])>>4);

        // Points that fall right from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = (FT(src[k])>>4)*(uint8_t)6 + (FT(src[k - cn])>>2) + (FT(src[k + cn])>>2) + (FT(src[k - 2 * cn])>>4);
            dst[k + cn] = (FT(src[k + cn])>>4)*(uint8_t)6 + (FT(src[k])>>2) + (FT(src[k - cn])>>4);
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxp1 = (borderInterpolate(len, len, borderType) - (len - 2))*cn;
            int idxp2 = (borderInterpolate(len + 1, len, borderType) - (len - 2))*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + (FT(src[idxp1 + k])>>4);
                dst[k + cn] = dst[k + cn] + (FT(src[idxp1 + k])>>2) + (FT(src[idxp2 + k])>>4);
            }
        }
    }
}
template <>
void hlineSmooth5N14641<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16*, int, ufixedpoint16* dst, int len, int borderType)
{
    if (len == 1)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
                dst[k] = (ufixedpoint16(src[k])>>3) * (uint8_t)3;
        else
        {
            for (int k = 0; k < cn; k++)
                dst[k] = src[k];
        }
    }
    else if (len == 2)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (ufixedpoint16(src[k]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k + cn]) >> 2);
                dst[k + cn] = (ufixedpoint16(src[k]) >> 2) + (ufixedpoint16(src[k + cn]) >> 4) * (uint8_t)6;
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(2, len, borderType)*cn;
            int idxp2 = borderInterpolate(3, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (ufixedpoint16(src[k]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k + idxm1]) >> 2) + (ufixedpoint16(src[k + cn]) >> 2) + (ufixedpoint16(src[k + idxp1]) >> 4) + (ufixedpoint16(src[k + idxm2]) >> 4);
                dst[k + cn] = (ufixedpoint16(src[k + cn]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k]) >> 2) + (ufixedpoint16(src[k + idxp1]) >> 2) + (ufixedpoint16(src[k + idxm1]) >> 4) + (ufixedpoint16(src[k + idxp2]) >> 4);
            }
        }
    }
    else if (len == 3)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (ufixedpoint16(src[k]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k + cn]) >> 2) + (ufixedpoint16(src[k + 2 * cn]) >> 4);
                dst[k + cn] = (ufixedpoint16(src[k + cn]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k]) >> 2) + (ufixedpoint16(src[k + 2 * cn]) >> 2);
                dst[k + 2 * cn] = (ufixedpoint16(src[k + 2 * cn]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k + cn]) >> 2) + (ufixedpoint16(src[k]) >> 4);
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(3, len, borderType)*cn;
            int idxp2 = borderInterpolate(4, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = (ufixedpoint16(src[k]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k + cn]) >> 2) + (ufixedpoint16(src[k + idxm1]) >> 2) + (ufixedpoint16(src[k + 2 * cn]) >> 4) + (ufixedpoint16(src[k + idxm2]) >> 4);
                dst[k + cn] = (ufixedpoint16(src[k + cn]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k]) >> 2) + (ufixedpoint16(src[k + 2 * cn]) >> 2) + (ufixedpoint16(src[k + idxm1]) >> 4) + (ufixedpoint16(src[k + idxp1]) >> 4);
                dst[k + 2 * cn] = (ufixedpoint16(src[k + 2 * cn]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k + cn]) >> 2) + (ufixedpoint16(src[k + idxp1]) >> 2) + (ufixedpoint16(src[k]) >> 4) + (ufixedpoint16(src[k + idxp2]) >> 4);
            }
        }
    }
    else
    {
        // Points that fall left from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = (ufixedpoint16(src[k]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[cn + k]) >> 2) + (ufixedpoint16(src[2 * cn + k]) >> 4);
            dst[k + cn] = (ufixedpoint16(src[cn + k]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k]) >> 2) + (ufixedpoint16(src[2 * cn + k]) >> 2) + (ufixedpoint16(src[3 * cn + k]) >> 4);
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + (ufixedpoint16(src[idxm2 + k]) >> 4) + (ufixedpoint16(src[idxm1 + k]) >> 2);
                dst[k + cn] = dst[k + cn] + (ufixedpoint16(src[idxm1 + k]) >> 4);
            }
        }

        src += 2 * cn; dst += 2 * cn;
        int i = 2 * cn, lencn = (len - 2)*cn;
        v_uint16x8 v_6 = v_setall_u16(6);
        for (; i <= lencn - 16; i += 16, src += 16, dst += 16)
        {
            v_uint16x8 v_src00, v_src01, v_src10, v_src11, v_src20, v_src21, v_src30, v_src31, v_src40, v_src41;
            v_expand(v_load(src - 2*cn), v_src00, v_src01);
            v_expand(v_load(src - cn), v_src10, v_src11);
            v_expand(v_load(src), v_src20, v_src21);
            v_expand(v_load(src + cn), v_src30, v_src31);
            v_expand(v_load(src + 2*cn), v_src40, v_src41);
            v_store((uint16_t*)dst, (v_src20 * v_6 + ((v_src10 + v_src30) << 2) + v_src00 + v_src40) << 4);
            v_store((uint16_t*)dst + 8, (v_src21 * v_6 + ((v_src11 + v_src31) << 2) + v_src01 + v_src41) << 4);
        }
        for (; i < lencn; i++, src++, dst++)
            *((uint16_t*)dst) = (uint16_t(src[0]) * 6 + ((uint16_t(src[-cn]) + uint16_t(src[cn])) << 2) + uint16_t(src[-2 * cn]) + uint16_t(src[2 * cn])) << 4;

        // Points that fall right from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = (ufixedpoint16(src[k]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k - cn]) >> 2) + (ufixedpoint16(src[k + cn]) >> 2) + (ufixedpoint16(src[k - 2 * cn]) >> 4);
            dst[k + cn] = (ufixedpoint16(src[k + cn]) >> 4) * (uint8_t)6 + (ufixedpoint16(src[k]) >> 2) + (ufixedpoint16(src[k - cn]) >> 4);
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxp1 = (borderInterpolate(len, len, borderType) - (len - 2))*cn;
            int idxp2 = (borderInterpolate(len + 1, len, borderType) - (len - 2))*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + (ufixedpoint16(src[idxp1 + k]) >> 4);
                dst[k + cn] = dst[k + cn] + (ufixedpoint16(src[idxp1 + k]) >> 2) + (ufixedpoint16(src[idxp2 + k]) >> 4);
            }
        }
    }
}
template <typename ET, typename FT>
void hlineSmooth5Nabcba(const ET* src, int cn, const FT* m, int, FT* dst, int len, int borderType)
{
    if (len == 1)
    {
        FT msum = borderType != BORDER_CONSTANT ? ((m[0] + m[1])<<1) + m[2] : m[2];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else if (len == 2)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[1] * src[k + cn];
                dst[k + cn] = m[1] * src[k] + m[2] * src[k + cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(2, len, borderType)*cn;
            int idxp2 = borderInterpolate(3, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[1] * src[k + idxm1] + m[2] * src[k] + m[1] * src[k + cn] + m[0] * src[k + idxp1] + m[0] * src[k + idxm2];
                dst[k + cn] = m[0] * src[k + idxm1] + m[1] * src[k] + m[2] * src[k + cn] + m[1] * src[k + idxp1] + m[0] * src[k + idxp2];
            }
        }
    }
    else if (len == 3)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[1] * src[k + cn] + m[0] * src[k + 2 * cn];
                dst[k + cn] = m[1] * src[k] + m[2] * src[k + cn] + m[1] * src[k + 2 * cn];
                dst[k + 2 * cn] = m[0] * src[k] + m[1] * src[k + cn] + m[2] * src[k + 2 * cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(3, len, borderType)*cn;
            int idxp2 = borderInterpolate(4, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[1] * src[k + cn] + m[0] * src[k + 2 * cn] + m[0] * src[k + idxm2] + m[1] * src[k + idxm1];
                dst[k + cn] = m[1] * src[k] + m[2] * src[k + cn] + m[1] * src[k + 2 * cn] + m[0] * src[k + idxm1] + m[0] * src[k + idxp1];
                dst[k + 2 * cn] = m[0] * src[k] + m[1] * src[k + cn] + m[2] * src[k + 2 * cn] + m[1] * src[k + idxp1] + m[0] * src[k + idxp2];
            }
        }
    }
    else
    {
        // Points that fall left from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = m[2] * src[k] + m[1] * src[cn + k] + m[0] * src[2 * cn + k];
            dst[k + cn] = m[1] * src[k] + m[2] * src[cn + k] + m[1] * src[2 * cn + k] + m[0] * src[3 * cn + k];
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + m[0] * src[idxm2 + k] + m[1] * src[idxm1 + k];
                dst[k + cn] = dst[k + cn] + m[0] * src[idxm1 + k];
            }
        }

        src += 2 * cn; dst += 2 * cn;
        for (int i = 2 * cn; i < (len - 2)*cn; i++, src++, dst++)
            *dst = m[0] * src[-2 * cn] + m[1] * src[-cn] + m[2] * src[0] + m[3] * src[cn] + m[4] * src[2 * cn];

        // Points that fall right from border
        for (int k = 0; k < cn; k++)
        {
            dst[k] = m[0] * src[k - 2 * cn] + m[1] * src[k - cn] + m[2] * src[k] + m[3] * src[k + cn];
            dst[k + cn] = m[0] * src[k - cn] + m[1] * src[k] + m[2] * src[k + cn];
        }
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxp1 = (borderInterpolate(len, len, borderType) - (len - 2))*cn;
            int idxp2 = (borderInterpolate(len + 1, len, borderType) - (len - 2))*cn;
            for (int k = 0; k < cn; k++)
            {
                dst[k] = dst[k] + m[0] * src[idxp1 + k];
                dst[k + cn] = dst[k + cn] + m[1] * src[idxp1 + k] + m[0] * src[idxp2 + k];
            }
        }
    }
}
template <>
void hlineSmooth5Nabcba<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16* m, int, ufixedpoint16* dst, int len, int borderType)
{
    if (len == 1)
    {
        ufixedpoint16 msum = borderType != BORDER_CONSTANT ? ((m[0] + m[1]) << 1) + m[2] : m[2];
        for (int k = 0; k < cn; k++)
            dst[k] = msum * src[k];
    }
    else if (len == 2)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[1] * src[k + cn];
                dst[k + cn] = m[1] * src[k] + m[2] * src[k + cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(2, len, borderType)*cn;
            int idxp2 = borderInterpolate(3, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                ((uint16_t*)dst)[k] = ((uint16_t*)m)[1] * ((uint16_t)(src[k + idxm1]) + (uint16_t)(src[k + cn])) + ((uint16_t*)m)[2] * src[k] + ((uint16_t*)m)[0] * ((uint16_t)(src[k + idxp1]) + (uint16_t)(src[k + idxm2]));
                ((uint16_t*)dst)[k + cn] = ((uint16_t*)m)[0] * ((uint16_t)(src[k + idxm1]) + (uint16_t)(src[k + idxp2])) + ((uint16_t*)m)[1] * ((uint16_t)(src[k]) + (uint16_t)(src[k + idxp1])) + ((uint16_t*)m)[2] * src[k + cn];
            }
        }
    }
    else if (len == 3)
    {
        if (borderType == BORDER_CONSTANT)
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[1] * src[k + cn] + m[0] * src[k + 2 * cn];
                ((uint16_t*)dst)[k + cn] = ((uint16_t*)m)[1] * ((uint16_t)(src[k]) + (uint16_t)(src[k + 2 * cn])) + ((uint16_t*)m)[2] * src[k + cn];
                dst[k + 2 * cn] = m[0] * src[k] + m[1] * src[k + cn] + m[2] * src[k + 2 * cn];
            }
        else
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            int idxp1 = borderInterpolate(3, len, borderType)*cn;
            int idxp2 = borderInterpolate(4, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                ((uint16_t*)dst)[k] = ((uint16_t*)m)[2] * src[k] + ((uint16_t*)m)[1] * ((uint16_t)(src[k + cn]) + (uint16_t)(src[k + idxm1])) + ((uint16_t*)m)[0] * ((uint16_t)(src[k + 2 * cn]) + (uint16_t)(src[k + idxm2]));
                ((uint16_t*)dst)[k + cn] = ((uint16_t*)m)[2] * src[k + cn] + ((uint16_t*)m)[1] * ((uint16_t)(src[k]) + (uint16_t)(src[k + 2 * cn])) + ((uint16_t*)m)[0] * ((uint16_t)(src[k + idxm1]) + (uint16_t)(src[k + idxp1]));
                ((uint16_t*)dst)[k + 2 * cn] = ((uint16_t*)m)[0] * ((uint16_t)(src[k]) + (uint16_t)(src[k + idxp2])) + ((uint16_t*)m)[1] * ((uint16_t)(src[k + cn]) + (uint16_t)(src[k + idxp1])) + ((uint16_t*)m)[2] * src[k + 2 * cn];
            }
        }
    }
    else
    {
        // Points that fall left from border
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxm2 = borderInterpolate(-2, len, borderType)*cn;
            int idxm1 = borderInterpolate(-1, len, borderType)*cn;
            for (int k = 0; k < cn; k++)
            {
                ((uint16_t*)dst)[k] = ((uint16_t*)m)[2] * src[k] + ((uint16_t*)m)[1] * ((uint16_t)(src[cn + k]) + (uint16_t)(src[idxm1 + k])) + ((uint16_t*)m)[0] * ((uint16_t)(src[2 * cn + k]) + (uint16_t)(src[idxm2 + k]));
                ((uint16_t*)dst)[k + cn] = ((uint16_t*)m)[1] * ((uint16_t)(src[k]) + (uint16_t)(src[2 * cn + k])) + ((uint16_t*)m)[2] * src[cn + k] + ((uint16_t*)m)[0] * ((uint16_t)(src[3 * cn + k]) + (uint16_t)(src[idxm1 + k]));
            }
        }
        else
        {
            for (int k = 0; k < cn; k++)
            {
                dst[k] = m[2] * src[k] + m[1] * src[cn + k] + m[0] * src[2 * cn + k];
                ((uint16_t*)dst)[k + cn] = ((uint16_t*)m)[1] * ((uint16_t)(src[k]) + (uint16_t)(src[2 * cn + k])) + ((uint16_t*)m)[2] * src[cn + k] + ((uint16_t*)m)[0] * src[3 * cn + k];
            }
        }

        src += 2 * cn; dst += 2 * cn;
        int i = 2 * cn, lencn = (len - 2)*cn;
        v_uint16x8 v_mul0 = v_setall_u16(*((uint16_t*)m));
        v_uint16x8 v_mul1 = v_setall_u16(*((uint16_t*)(m + 1)));
        v_uint16x8 v_mul2 = v_setall_u16(*((uint16_t*)(m + 2)));
        for (; i <= lencn - 16; i += 16, src += 16, dst += 16)
        {
            v_uint16x8 v_src00, v_src01, v_src10, v_src11, v_src20, v_src21, v_src30, v_src31, v_src40, v_src41;
            v_expand(v_load(src - 2 * cn), v_src00, v_src01);
            v_expand(v_load(src - cn), v_src10, v_src11);
            v_expand(v_load(src), v_src20, v_src21);
            v_expand(v_load(src + cn), v_src30, v_src31);
            v_expand(v_load(src + 2 * cn), v_src40, v_src41);
            v_store((uint16_t*)dst, (v_src00 + v_src40) * v_mul0 + (v_src10 + v_src30)* v_mul1 + v_src20 * v_mul2);
            v_store((uint16_t*)dst + 8, (v_src01 + v_src41) * v_mul0 + (v_src11 + v_src31) * v_mul1 + v_src21 * v_mul2);
        }
        for (; i < lencn; i++, src++, dst++)
            *((uint16_t*)dst) = ((uint16_t*)m)[0] * ((uint16_t)(src[-2 * cn]) + (uint16_t)(src[2 * cn])) + ((uint16_t*)m)[1] * ((uint16_t)(src[-cn]) + (uint16_t)(src[cn])) + ((uint16_t*)m)[2] * src[0];

        // Points that fall right from border
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int idxp1 = (borderInterpolate(len, len, borderType) - (len - 2))*cn;
            int idxp2 = (borderInterpolate(len + 1, len, borderType) - (len - 2))*cn;
            for (int k = 0; k < cn; k++)
            {
                ((uint16_t*)dst)[k] = ((uint16_t*)m)[0] * ((uint16_t)(src[k - 2 * cn]) + (uint16_t)(src[idxp1 + k])) + ((uint16_t*)m)[1] * ((uint16_t)(src[k - cn]) + (uint16_t)(src[k + cn])) + ((uint16_t*)m)[2] * src[k];
                ((uint16_t*)dst)[k + cn] = ((uint16_t*)m)[0] * ((uint16_t)(src[k - cn]) + (uint16_t)(src[idxp2 + k])) + ((uint16_t*)m)[1] * ((uint16_t)(src[k]) + (uint16_t)(src[idxp1 + k])) + ((uint16_t*)m)[2] * src[k + cn];
            }
        }
        else
        {
            for (int k = 0; k < cn; k++)
            {
                ((uint16_t*)dst)[k] = ((uint16_t*)m)[0] * src[k - 2 * cn] + ((uint16_t*)m)[1] * ((uint16_t)(src[k - cn]) + (uint16_t)(src[k + cn])) + ((uint16_t*)m)[2] * src[k];
                dst[k + cn] = m[0] * src[k - cn] + m[1] * src[k] + m[2] * src[k + cn];
            }
        }
    }
}
template <typename ET, typename FT>
void hlineSmooth(const ET* src, int cn, const FT* m, int n, FT* dst, int len, int borderType)
{
    int pre_shift = n / 2;
    int post_shift = n - pre_shift;
    int i = 0;
    for (; i < min(pre_shift, len); i++, dst += cn) // Points that fall left from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[pre_shift-i] * src[k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (int j = i - pre_shift, mid = 0; j < 0; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
        int j, mid;
        for (j = 1, mid = pre_shift - i + 1; j < min(i + post_shift, len); j++, mid++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[mid] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)
            for (; j < i + post_shift; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
    }
    i *= cn;
    for (; i < (len - post_shift + 1)*cn; i++, src++, dst++)
    {
        *dst = m[0] * src[0];
        for (int j = 1; j < n; j++)
            *dst = *dst + m[j] * src[j*cn];
    }
    i /= cn;
    for (i -= pre_shift; i < len - pre_shift; i++, src += cn, dst += cn) // Points that fall right from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[0] * src[k];
        int j = 1;
        for (; j < len - i; j++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[j] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (; j < n; j++)
            {
                int src_idx = borderInterpolate(i + j, len, borderType) - i;
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[j] * src[src_idx*cn + k];
            }
    }
}
template <>
void hlineSmooth<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16* m, int n, ufixedpoint16* dst, int len, int borderType)
{
    int pre_shift = n / 2;
    int post_shift = n - pre_shift;
    int i = 0;
    for (; i < min(pre_shift, len); i++, dst += cn) // Points that fall left from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[pre_shift - i] * src[k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (int j = i - pre_shift, mid = 0; j < 0; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
        int j, mid;
        for (j = 1, mid = pre_shift - i + 1; j < min(i + post_shift, len); j++, mid++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[mid] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)
            for (; j < i + post_shift; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
    }
    i *= cn;
    int lencn = (len - post_shift + 1)*cn;
    for (; i <= lencn - 16; i+=16, src+=16, dst+=16)
    {
        v_uint16x8 v_src0, v_src1;
        v_uint16x8 v_mul = v_setall_u16(*((uint16_t*)m));
        v_expand(v_load(src), v_src0, v_src1);
        v_uint16x8 v_res0 = v_src0 * v_mul;
        v_uint16x8 v_res1 = v_src1 * v_mul;
        for (int j = 1; j < n; j++)
        {
            v_mul = v_setall_u16(*((uint16_t*)(m + j)));
            v_expand(v_load(src + j * cn), v_src0, v_src1);
            v_res0 += v_src0 * v_mul;
            v_res1 += v_src1 * v_mul;
        }
        v_store((uint16_t*)dst, v_res0);
        v_store((uint16_t*)dst+8, v_res1);
    }
    for (; i < lencn; i++, src++, dst++)
    {
            *dst = m[0] * src[0];
            for (int j = 1; j < n; j++)
                *dst = *dst + m[j] * src[j*cn];
    }
    i /= cn;
    for (i -= pre_shift; i < len - pre_shift; i++, src += cn, dst += cn) // Points that fall right from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[0] * src[k];
        int j = 1;
        for (; j < len - i; j++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[j] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (; j < n; j++)
            {
                int src_idx = borderInterpolate(i + j, len, borderType) - i;
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[j] * src[src_idx*cn + k];
            }
    }
}
template <typename ET, typename FT>
void hlineSmoothONa_yzy_a(const ET* src, int cn, const FT* m, int n, FT* dst, int len, int borderType)
{
    int pre_shift = n / 2;
    int post_shift = n - pre_shift;
    int i = 0;
    for (; i < min(pre_shift, len); i++, dst += cn) // Points that fall left from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[pre_shift - i] * src[k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (int j = i - pre_shift, mid = 0; j < 0; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
        int j, mid;
        for (j = 1, mid = pre_shift - i + 1; j < min(i + post_shift, len); j++, mid++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[mid] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)
            for (; j < i + post_shift; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
    }
    i *= cn;
    for (; i < (len - post_shift + 1)*cn; i++, src++, dst++)
    {
        *dst = m[pre_shift] * src[pre_shift*cn];
        for (int j = 0; j < pre_shift; j++)
            *dst = *dst + m[j] * src[j*cn] + m[j] * src[(n-1-j)*cn];
    }
    i /= cn;
    for (i -= pre_shift; i < len - pre_shift; i++, src += cn, dst += cn) // Points that fall right from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[0] * src[k];
        int j = 1;
        for (; j < len - i; j++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[j] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (; j < n; j++)
            {
                int src_idx = borderInterpolate(i + j, len, borderType) - i;
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[j] * src[src_idx*cn + k];
            }
    }
}
template <>
void hlineSmoothONa_yzy_a<uint8_t, ufixedpoint16>(const uint8_t* src, int cn, const ufixedpoint16* m, int n, ufixedpoint16* dst, int len, int borderType)
{
    int pre_shift = n / 2;
    int post_shift = n - pre_shift;
    int i = 0;
    for (; i < min(pre_shift, len); i++, dst += cn) // Points that fall left from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[pre_shift - i] * src[k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (int j = i - pre_shift, mid = 0; j < 0; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
        int j, mid;
        for (j = 1, mid = pre_shift - i + 1; j < min(i + post_shift, len); j++, mid++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[mid] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)
            for (; j < i + post_shift; j++, mid++)
            {
                int src_idx = borderInterpolate(j, len, borderType);
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[mid] * src[src_idx*cn + k];
            }
    }
    i *= cn;
    int lencn = (len - post_shift + 1)*cn;
    for (; i <= lencn - 16; i += 16, src += 16, dst += 16)
    {
        v_uint16x8 v_src00, v_src01, v_srcN00, v_srcN01;

        v_uint16x8 v_mul = v_setall_u16(*((uint16_t*)(m + pre_shift)));
        v_expand(v_load(src + pre_shift * cn), v_src00, v_src01);
        v_uint16x8 v_res0 = v_src00 * v_mul;
        v_uint16x8 v_res1 = v_src01 * v_mul;
        for (int j = 0; j < pre_shift; j ++)
        {
            v_mul = v_setall_u16(*((uint16_t*)(m + j)));
            v_expand(v_load(src + j * cn), v_src00, v_src01);
            v_expand(v_load(src + (n - 1 - j)*cn), v_srcN00, v_srcN01);
            v_res0 += (v_src00 + v_srcN00) * v_mul;
            v_res1 += (v_src01 + v_srcN01) * v_mul;
        }

        v_store((uint16_t*)dst, v_res0);
        v_store((uint16_t*)dst + 8, v_res1);
    }
    for (; i < lencn; i++, src++, dst++)
    {
        *dst = m[pre_shift] * src[pre_shift*cn];
        for (int j = 0; j < pre_shift; j++)
            *dst = *dst + m[j] * src[j*cn] + m[j] * src[(n - 1 - j)*cn];
    }
    i /= cn;
    for (i -= pre_shift; i < len - pre_shift; i++, src += cn, dst += cn) // Points that fall right from border
    {
        for (int k = 0; k < cn; k++)
            dst[k] = m[0] * src[k];
        int j = 1;
        for (; j < len - i; j++)
            for (int k = 0; k < cn; k++)
                dst[k] = dst[k] + m[j] * src[j*cn + k];
        if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
            for (; j < n; j++)
            {
                int src_idx = borderInterpolate(i + j, len, borderType) - i;
                for (int k = 0; k < cn; k++)
                    dst[k] = dst[k] + m[j] * src[src_idx*cn + k];
            }
    }
}
template <typename ET, typename FT>
void vlineSmooth1N(const FT* const * src, const FT* m, int, ET* dst, int len)
{
    const FT* src0 = src[0];
    for (int i = 0; i < len; i++)
        dst[i] = *m * src0[i];
}
template <>
void vlineSmooth1N<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16* m, int, uint8_t* dst, int len)
{
    const ufixedpoint16* src0 = src[0];
    v_uint16x8 v_mul = v_setall_u16(*((uint16_t*)m));
#if CV_SSE2
    v_uint16x8 v_1 = v_setall_u16(1);
    v_mul += v_mul;
#endif
    int i = 0;
    for (; i <= len - 16; i += 16)
    {
        v_uint16x8 v_src0 = v_load((uint16_t*)src0 + i);
        v_uint16x8 v_src1 = v_load((uint16_t*)src0 + i + 8);
        v_uint8x16 v_res;
#if CV_SSE2
        v_res.val = _mm_packus_epi16(_mm_srli_epi16(_mm_add_epi16(v_1.val, _mm_mulhi_epu16(v_src0.val, v_mul.val)),1),
                                     _mm_srli_epi16(_mm_add_epi16(v_1.val, _mm_mulhi_epu16(v_src1.val, v_mul.val)),1));
#else
        v_uint32x4 v_res0, v_res1, v_res2, v_res3;
        v_mul_expand(v_src0, v_mul, v_res0, v_res1);
        v_mul_expand(v_src1, v_mul, v_res2, v_res3);
        v_res = v_pack(v_rshr_pack<16>(v_res0, v_res1), v_rshr_pack<16>(v_res2, v_res3));
#endif
        v_store(dst + i, v_res);
    }
    for (; i < len; i++)
        dst[i] = m[0] * src0[i];
}
template <typename ET, typename FT>
void vlineSmooth1N1(const FT* const * src, const FT*, int, ET* dst, int len)
{
    const FT* src0 = src[0];
    for (int i = 0; i < len; i++)
        dst[i] = src0[i];
}
template <>
void vlineSmooth1N1<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16*, int, uint8_t* dst, int len)
{
    const ufixedpoint16* src0 = src[0];
    int i = 0;
    for (; i <= len - 8; i += 8)
        v_rshr_pack_store<8>(dst + i, v_load((uint16_t*)(src0 + i)));
    for (; i < len; i++)
        dst[i] = src0[i];
}
template <typename ET, typename FT>
void vlineSmooth3N(const FT* const * src, const FT* m, int, ET* dst, int len)
{
    for (int i = 0; i < len; i++)
        dst[i] = m[0] * src[0][i] + m[1] * src[1][i] + m[2] * src[2][i];
}
template <>
void vlineSmooth3N<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16* m, int, uint8_t* dst, int len)
{
    int i = 0;
    static const v_int16x8 v_128 = v_reinterpret_as_s16(v_setall_u16((uint16_t)1 << 15));
    v_int32x4 v_128_4 = v_setall_s32(128 << 16);
    if (len > 7)
    {
        ufixedpoint32 val[] = { (m[0] + m[1] + m[2]) * ufixedpoint16((uint8_t)128) };
        v_128_4 = v_setall_s32(*((int32_t*)val));
    }
    v_int16x8 v_mul01 = v_reinterpret_as_s16(v_setall_u32(*((uint32_t*)m)));
    v_int16x8 v_mul2 = v_reinterpret_as_s16(v_setall_u16(*((uint16_t*)(m + 2))));
    for (; i <= len - 32; i += 32)
    {
        v_int16x8 v_src00, v_src10, v_src01, v_src11, v_src02, v_src12, v_src03, v_src13;
        v_int16x8 v_tmp0, v_tmp1;

        v_src00 = v_load((int16_t*)(src[0]) + i);
        v_src01 = v_load((int16_t*)(src[0]) + i + 8);
        v_src02 = v_load((int16_t*)(src[0]) + i + 16);
        v_src03 = v_load((int16_t*)(src[0]) + i + 24);
        v_src10 = v_load((int16_t*)(src[1]) + i);
        v_src11 = v_load((int16_t*)(src[1]) + i + 8);
        v_src12 = v_load((int16_t*)(src[1]) + i + 16);
        v_src13 = v_load((int16_t*)(src[1]) + i + 24);
        v_zip(v_add_wrap(v_src00, v_128), v_add_wrap(v_src10, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res0 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res1 = v_dotprod(v_tmp1, v_mul01);
        v_zip(v_add_wrap(v_src01, v_128), v_add_wrap(v_src11, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res2 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res3 = v_dotprod(v_tmp1, v_mul01);
        v_zip(v_add_wrap(v_src02, v_128), v_add_wrap(v_src12, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res4 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res5 = v_dotprod(v_tmp1, v_mul01);
        v_zip(v_add_wrap(v_src03, v_128), v_add_wrap(v_src13, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res6 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res7 = v_dotprod(v_tmp1, v_mul01);

        v_int32x4 v_resj0, v_resj1;
        v_src00 = v_load((int16_t*)(src[2]) + i);
        v_src01 = v_load((int16_t*)(src[2]) + i + 8);
        v_src02 = v_load((int16_t*)(src[2]) + i + 16);
        v_src03 = v_load((int16_t*)(src[2]) + i + 24);
        v_mul_expand(v_add_wrap(v_src00, v_128), v_mul2, v_resj0, v_resj1);
        v_res0 += v_resj0;
        v_res1 += v_resj1;
        v_mul_expand(v_add_wrap(v_src01, v_128), v_mul2, v_resj0, v_resj1);
        v_res2 += v_resj0;
        v_res3 += v_resj1;
        v_mul_expand(v_add_wrap(v_src02, v_128), v_mul2, v_resj0, v_resj1);
        v_res4 += v_resj0;
        v_res5 += v_resj1;
        v_mul_expand(v_add_wrap(v_src03, v_128), v_mul2, v_resj0, v_resj1);
        v_res6 += v_resj0;
        v_res7 += v_resj1;

        v_res0 += v_128_4;
        v_res1 += v_128_4;
        v_res2 += v_128_4;
        v_res3 += v_128_4;
        v_res4 += v_128_4;
        v_res5 += v_128_4;
        v_res6 += v_128_4;
        v_res7 += v_128_4;

        v_store(dst + i     , v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res0, v_res1)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res2, v_res3))));
        v_store(dst + i + 16, v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res4, v_res5)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res6, v_res7))));
    }
    for (; i < len; i++)
        dst[i] = m[0] * src[0][i] + m[1] * src[1][i] + m[2] * src[2][i];
}
template <typename ET, typename FT>
void vlineSmooth3N121(const FT* const * src, const FT*, int, ET* dst, int len)
{
    for (int i = 0; i < len; i++)
        dst[i] = (FT::WT(src[0][i]) >> 2) + (FT::WT(src[2][i]) >> 2) + (FT::WT(src[1][i]) >> 1);
}
template <>
void vlineSmooth3N121<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16*, int, uint8_t* dst, int len)
{
    int i = 0;
    for (; i <= len - 16; i += 16)
    {
        v_uint32x4 v_src00, v_src01, v_src02, v_src03, v_src10, v_src11, v_src12, v_src13, v_src20, v_src21, v_src22, v_src23;
        v_expand(v_load((uint16_t*)(src[0]) + i), v_src00, v_src01);
        v_expand(v_load((uint16_t*)(src[0]) + i + 8), v_src02, v_src03);
        v_expand(v_load((uint16_t*)(src[1]) + i), v_src10, v_src11);
        v_expand(v_load((uint16_t*)(src[1]) + i + 8), v_src12, v_src13);
        v_expand(v_load((uint16_t*)(src[2]) + i), v_src20, v_src21);
        v_expand(v_load((uint16_t*)(src[2]) + i + 8), v_src22, v_src23);
        v_store(dst + i, v_pack(v_rshr_pack<10>(v_src00 + v_src20 + (v_src10 + v_src10), v_src01 + v_src21 + (v_src11 + v_src11)),
                                v_rshr_pack<10>(v_src02 + v_src22 + (v_src12 + v_src12), v_src03 + v_src23 + (v_src13 + v_src13))));
    }
    for (; i < len; i++)
        dst[i] = (((uint32_t)(((uint16_t*)(src[0]))[i]) + (uint32_t)(((uint16_t*)(src[2]))[i]) + ((uint32_t)(((uint16_t*)(src[1]))[i]) << 1)) + (1 << 9)) >> 10;
}
template <typename ET, typename FT>
void vlineSmooth5N(const FT* const * src, const FT* m, int, ET* dst, int len)
{
    for (int i = 0; i < len; i++)
        dst[i] = m[0] * src[0][i] + m[1] * src[1][i] + m[2] * src[2][i] + m[3] * src[3][i] + m[4] * src[4][i];
}
template <>
void vlineSmooth5N<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16* m, int, uint8_t* dst, int len)
{
    int i = 0;
    static const v_int16x8 v_128 = v_reinterpret_as_s16(v_setall_u16((uint16_t)1 << 15));
    v_int32x4 v_128_4 = v_setall_s32(128 << 16);
    if (len > 7)
    {
        ufixedpoint32 val[] = { (m[0] + m[1] + m[2] + m[3] + m[4]) * ufixedpoint16((uint8_t)128) };
        v_128_4 = v_setall_s32(*((int32_t*)val));
    }
    v_int16x8 v_mul01 = v_reinterpret_as_s16(v_setall_u32(*((uint32_t*)m)));
    v_int16x8 v_mul23 = v_reinterpret_as_s16(v_setall_u32(*((uint32_t*)(m + 2))));
    v_int16x8 v_mul4 = v_reinterpret_as_s16(v_setall_u16(*((uint16_t*)(m + 4))));
    for (; i <= len - 32; i += 32)
    {
        v_int16x8 v_src00, v_src10, v_src01, v_src11, v_src02, v_src12, v_src03, v_src13;
        v_int16x8 v_tmp0, v_tmp1;

        v_src00 = v_load((int16_t*)(src[0]) + i);
        v_src01 = v_load((int16_t*)(src[0]) + i + 8);
        v_src02 = v_load((int16_t*)(src[0]) + i + 16);
        v_src03 = v_load((int16_t*)(src[0]) + i + 24);
        v_src10 = v_load((int16_t*)(src[1]) + i);
        v_src11 = v_load((int16_t*)(src[1]) + i + 8);
        v_src12 = v_load((int16_t*)(src[1]) + i + 16);
        v_src13 = v_load((int16_t*)(src[1]) + i + 24);
        v_zip(v_add_wrap(v_src00, v_128), v_add_wrap(v_src10, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res0 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res1 = v_dotprod(v_tmp1, v_mul01);
        v_zip(v_add_wrap(v_src01, v_128), v_add_wrap(v_src11, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res2 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res3 = v_dotprod(v_tmp1, v_mul01);
        v_zip(v_add_wrap(v_src02, v_128), v_add_wrap(v_src12, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res4 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res5 = v_dotprod(v_tmp1, v_mul01);
        v_zip(v_add_wrap(v_src03, v_128), v_add_wrap(v_src13, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res6 = v_dotprod(v_tmp0, v_mul01);
        v_int32x4 v_res7 = v_dotprod(v_tmp1, v_mul01);

        v_src00 = v_load((int16_t*)(src[2]) + i);
        v_src01 = v_load((int16_t*)(src[2]) + i + 8);
        v_src02 = v_load((int16_t*)(src[2]) + i + 16);
        v_src03 = v_load((int16_t*)(src[2]) + i + 24);
        v_src10 = v_load((int16_t*)(src[3]) + i);
        v_src11 = v_load((int16_t*)(src[3]) + i + 8);
        v_src12 = v_load((int16_t*)(src[3]) + i + 16);
        v_src13 = v_load((int16_t*)(src[3]) + i + 24);
        v_zip(v_add_wrap(v_src00, v_128), v_add_wrap(v_src10, v_128), v_tmp0, v_tmp1);
        v_res0 += v_dotprod(v_tmp0, v_mul23);
        v_res1 += v_dotprod(v_tmp1, v_mul23);
        v_zip(v_add_wrap(v_src01, v_128), v_add_wrap(v_src11, v_128), v_tmp0, v_tmp1);
        v_res2 += v_dotprod(v_tmp0, v_mul23);
        v_res3 += v_dotprod(v_tmp1, v_mul23);
        v_zip(v_add_wrap(v_src02, v_128), v_add_wrap(v_src12, v_128), v_tmp0, v_tmp1);
        v_res4 += v_dotprod(v_tmp0, v_mul23);
        v_res5 += v_dotprod(v_tmp1, v_mul23);
        v_zip(v_add_wrap(v_src03, v_128), v_add_wrap(v_src13, v_128), v_tmp0, v_tmp1);
        v_res6 += v_dotprod(v_tmp0, v_mul23);
        v_res7 += v_dotprod(v_tmp1, v_mul23);

        v_int32x4 v_resj0, v_resj1;
        v_src00 = v_load((int16_t*)(src[4]) + i);
        v_src01 = v_load((int16_t*)(src[4]) + i + 8);
        v_src02 = v_load((int16_t*)(src[4]) + i + 16);
        v_src03 = v_load((int16_t*)(src[4]) + i + 24);
        v_mul_expand(v_add_wrap(v_src00, v_128), v_mul4, v_resj0, v_resj1);
        v_res0 += v_resj0;
        v_res1 += v_resj1;
        v_mul_expand(v_add_wrap(v_src01, v_128), v_mul4, v_resj0, v_resj1);
        v_res2 += v_resj0;
        v_res3 += v_resj1;
        v_mul_expand(v_add_wrap(v_src02, v_128), v_mul4, v_resj0, v_resj1);
        v_res4 += v_resj0;
        v_res5 += v_resj1;
        v_mul_expand(v_add_wrap(v_src03, v_128), v_mul4, v_resj0, v_resj1);
        v_res6 += v_resj0;
        v_res7 += v_resj1;

        v_res0 += v_128_4;
        v_res1 += v_128_4;
        v_res2 += v_128_4;
        v_res3 += v_128_4;
        v_res4 += v_128_4;
        v_res5 += v_128_4;
        v_res6 += v_128_4;
        v_res7 += v_128_4;

        v_store(dst + i     , v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res0, v_res1)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res2, v_res3))));
        v_store(dst + i + 16, v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res4, v_res5)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res6, v_res7))));
    }
    for (; i < len; i++)
        dst[i] = m[0] * src[0][i] + m[1] * src[1][i] + m[2] * src[2][i] + m[3] * src[3][i] + m[4] * src[4][i];
}
template <typename ET, typename FT>
void vlineSmooth5N14641(const FT* const * src, const FT*, int, ET* dst, int len)
{
    for (int i = 0; i < len; i++)
        dst[i] = (FT::WT(src[2][i])*(uint8_t)6 + ((FT::WT(src[1][i]) + FT::WT(src[3][i]))<<2) + FT::WT(src[0][i]) + FT::WT(src[4][i])) >> 4;
}
template <>
void vlineSmooth5N14641<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16*, int, uint8_t* dst, int len)
{
    int i = 0;
    v_uint32x4 v_6 = v_setall_u32(6);
    for (; i <= len - 16; i += 16)
    {
        v_uint32x4 v_src00, v_src10, v_src20, v_src30, v_src40;
        v_uint32x4 v_src01, v_src11, v_src21, v_src31, v_src41;
        v_uint32x4 v_src02, v_src12, v_src22, v_src32, v_src42;
        v_uint32x4 v_src03, v_src13, v_src23, v_src33, v_src43;
        v_expand(v_load((uint16_t*)(src[0]) + i), v_src00, v_src01);
        v_expand(v_load((uint16_t*)(src[0]) + i + 8), v_src02, v_src03);
        v_expand(v_load((uint16_t*)(src[1]) + i), v_src10, v_src11);
        v_expand(v_load((uint16_t*)(src[1]) + i + 8), v_src12, v_src13);
        v_expand(v_load((uint16_t*)(src[2]) + i), v_src20, v_src21);
        v_expand(v_load((uint16_t*)(src[2]) + i + 8), v_src22, v_src23);
        v_expand(v_load((uint16_t*)(src[3]) + i), v_src30, v_src31);
        v_expand(v_load((uint16_t*)(src[3]) + i + 8), v_src32, v_src33);
        v_expand(v_load((uint16_t*)(src[4]) + i), v_src40, v_src41);
        v_expand(v_load((uint16_t*)(src[4]) + i + 8), v_src42, v_src43);
        v_store(dst + i, v_pack(v_rshr_pack<12>(v_src20*v_6 + ((v_src10 + v_src30) << 2) + v_src00 + v_src40,
                                                v_src21*v_6 + ((v_src11 + v_src31) << 2) + v_src01 + v_src41),
                                v_rshr_pack<12>(v_src22*v_6 + ((v_src12 + v_src32) << 2) + v_src02 + v_src42,
                                                v_src23*v_6 + ((v_src13 + v_src33) << 2) + v_src03 + v_src43)));
    }
    for (; i < len; i++)
        dst[i] = ((uint32_t)(((uint16_t*)(src[2]))[i]) * 6 +
                  (((uint32_t)(((uint16_t*)(src[1]))[i]) + (uint32_t)(((uint16_t*)(src[3]))[i])) << 2) +
                  (uint32_t)(((uint16_t*)(src[0]))[i]) + (uint32_t)(((uint16_t*)(src[4]))[i]) + (1 << 11)) >> 12;
}
template <typename ET, typename FT>
void vlineSmooth(const FT* const * src, const FT* m, int n, ET* dst, int len)
{
    for (int i = 0; i < len; i++)
    {
        typename FT::WT val = m[0] * src[0][i];
        for (int j = 1; j < n; j++)
            val = val + m[j] * src[j][i];
        dst[i] = val;
    }
}
template <>
void vlineSmooth<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16* m, int n, uint8_t* dst, int len)
{
    int i = 0;
    static const v_int16x8 v_128 = v_reinterpret_as_s16(v_setall_u16((uint16_t)1 << 15));
    v_int32x4 v_128_4 = v_setall_s32(128 << 16);
    if (len > 7)
    {
        ufixedpoint16 msum = m[0] + m[1];
        for (int j = 2; j < n; j++)
            msum = msum + m[j];
        ufixedpoint32 val[] = { msum * ufixedpoint16((uint8_t)128) };
        v_128_4 = v_setall_s32(*((int32_t*)val));
    }
    for (; i <= len - 32; i += 32)
    {
        v_int16x8 v_src00, v_src10, v_src01, v_src11, v_src02, v_src12, v_src03, v_src13;
        v_int16x8 v_tmp0, v_tmp1;

        v_int16x8 v_mul = v_reinterpret_as_s16(v_setall_u32(*((uint32_t*)m)));

        v_src00 = v_load((int16_t*)(src[0]) + i);
        v_src01 = v_load((int16_t*)(src[0]) + i + 8);
        v_src02 = v_load((int16_t*)(src[0]) + i + 16);
        v_src03 = v_load((int16_t*)(src[0]) + i + 24);
        v_src10 = v_load((int16_t*)(src[1]) + i);
        v_src11 = v_load((int16_t*)(src[1]) + i + 8);
        v_src12 = v_load((int16_t*)(src[1]) + i + 16);
        v_src13 = v_load((int16_t*)(src[1]) + i + 24);
        v_zip(v_add_wrap(v_src00, v_128), v_add_wrap(v_src10, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res0 = v_dotprod(v_tmp0, v_mul);
        v_int32x4 v_res1 = v_dotprod(v_tmp1, v_mul);
        v_zip(v_add_wrap(v_src01, v_128), v_add_wrap(v_src11, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res2 = v_dotprod(v_tmp0, v_mul);
        v_int32x4 v_res3 = v_dotprod(v_tmp1, v_mul);
        v_zip(v_add_wrap(v_src02, v_128), v_add_wrap(v_src12, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res4 = v_dotprod(v_tmp0, v_mul);
        v_int32x4 v_res5 = v_dotprod(v_tmp1, v_mul);
        v_zip(v_add_wrap(v_src03, v_128), v_add_wrap(v_src13, v_128), v_tmp0, v_tmp1);
        v_int32x4 v_res6 = v_dotprod(v_tmp0, v_mul);
        v_int32x4 v_res7 = v_dotprod(v_tmp1, v_mul);

        int j = 2;
        for (; j < n - 1; j+=2)
        {
            v_mul = v_reinterpret_as_s16(v_setall_u32(*((uint32_t*)(m+j))));

            v_src00 = v_load((int16_t*)(src[j]) + i);
            v_src01 = v_load((int16_t*)(src[j]) + i + 8);
            v_src02 = v_load((int16_t*)(src[j]) + i + 16);
            v_src03 = v_load((int16_t*)(src[j]) + i + 24);
            v_src10 = v_load((int16_t*)(src[j+1]) + i);
            v_src11 = v_load((int16_t*)(src[j+1]) + i + 8);
            v_src12 = v_load((int16_t*)(src[j+1]) + i + 16);
            v_src13 = v_load((int16_t*)(src[j+1]) + i + 24);
            v_zip(v_add_wrap(v_src00, v_128), v_add_wrap(v_src10, v_128), v_tmp0, v_tmp1);
            v_res0 += v_dotprod(v_tmp0, v_mul);
            v_res1 += v_dotprod(v_tmp1, v_mul);
            v_zip(v_add_wrap(v_src01, v_128), v_add_wrap(v_src11, v_128), v_tmp0, v_tmp1);
            v_res2 += v_dotprod(v_tmp0, v_mul);
            v_res3 += v_dotprod(v_tmp1, v_mul);
            v_zip(v_add_wrap(v_src02, v_128), v_add_wrap(v_src12, v_128), v_tmp0, v_tmp1);
            v_res4 += v_dotprod(v_tmp0, v_mul);
            v_res5 += v_dotprod(v_tmp1, v_mul);
            v_zip(v_add_wrap(v_src03, v_128), v_add_wrap(v_src13, v_128), v_tmp0, v_tmp1);
            v_res6 += v_dotprod(v_tmp0, v_mul);
            v_res7 += v_dotprod(v_tmp1, v_mul);
        }
        if(j < n)
        {
            v_int32x4 v_resj0, v_resj1;
            v_mul = v_reinterpret_as_s16(v_setall_u16(*((uint16_t*)(m + j))));
            v_src00 = v_load((int16_t*)(src[j]) + i);
            v_src01 = v_load((int16_t*)(src[j]) + i + 8);
            v_src02 = v_load((int16_t*)(src[j]) + i + 16);
            v_src03 = v_load((int16_t*)(src[j]) + i + 24);
            v_mul_expand(v_add_wrap(v_src00, v_128), v_mul, v_resj0, v_resj1);
            v_res0 += v_resj0;
            v_res1 += v_resj1;
            v_mul_expand(v_add_wrap(v_src01, v_128), v_mul, v_resj0, v_resj1);
            v_res2 += v_resj0;
            v_res3 += v_resj1;
            v_mul_expand(v_add_wrap(v_src02, v_128), v_mul, v_resj0, v_resj1);
            v_res4 += v_resj0;
            v_res5 += v_resj1;
            v_mul_expand(v_add_wrap(v_src03, v_128), v_mul, v_resj0, v_resj1);
            v_res6 += v_resj0;
            v_res7 += v_resj1;
        }
        v_res0 += v_128_4;
        v_res1 += v_128_4;
        v_res2 += v_128_4;
        v_res3 += v_128_4;
        v_res4 += v_128_4;
        v_res5 += v_128_4;
        v_res6 += v_128_4;
        v_res7 += v_128_4;

        v_store(dst + i     , v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res0, v_res1)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res2, v_res3))));
        v_store(dst + i + 16, v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res4, v_res5)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res6, v_res7))));
    }
    for (; i < len; i++)
    {
        ufixedpoint32 val = m[0] * src[0][i];
        for (int j = 1; j < n; j++)
        {
            val = val + m[j] * src[j][i];
        }
        dst[i] = val;
    }
}
template <typename ET, typename FT>
void vlineSmoothONa_yzy_a(const FT* const * src, const FT* m, int n, ET* dst, int len)
{
    int pre_shift = n / 2;
    for (int i = 0; i < len; i++)
    {
        typename FT::WT val = m[pre_shift] * src[pre_shift][i];
        for (int j = 0; j < pre_shift; j++)
            val = val + m[j] * src[j][i] + m[j] * src[(n - 1 - j)][i];
        dst[i] = val;
    }
}
template <>
void vlineSmoothONa_yzy_a<uint8_t, ufixedpoint16>(const ufixedpoint16* const * src, const ufixedpoint16* m, int n, uint8_t* dst, int len)
{
    int pre_shift = n / 2;
    int i = 0;
    static const v_int16x8 v_128 = v_reinterpret_as_s16(v_setall_u16((uint16_t)1 << 15));
    v_int32x4 v_128_4 = v_setall_s32(128 << 16);
    if (len > 7)
    {
        ufixedpoint16 msum = m[0] + m[pre_shift] + m[n - 1];
        for (int j = 1; j < pre_shift; j++)
            msum = msum + m[j] + m[n - 1 - j];
        ufixedpoint32 val[] = { msum * ufixedpoint16((uint8_t)128) };
        v_128_4 = v_setall_s32(*((int32_t*)val));
    }
    for (; i <= len - 32; i += 32)
    {
        v_int16x8 v_src00, v_src10, v_src20, v_src30, v_src01, v_src11, v_src21, v_src31;
        v_int32x4 v_res0, v_res1, v_res2, v_res3, v_res4, v_res5, v_res6, v_res7;
        v_int16x8 v_tmp0, v_tmp1, v_tmp2, v_tmp3, v_tmp4, v_tmp5, v_tmp6, v_tmp7;

        v_int16x8 v_mul = v_reinterpret_as_s16(v_setall_u16(*((uint16_t*)(m + pre_shift))));
        v_src00 = v_load((int16_t*)(src[pre_shift]) + i);
        v_src10 = v_load((int16_t*)(src[pre_shift]) + i + 8);
        v_src20 = v_load((int16_t*)(src[pre_shift]) + i + 16);
        v_src30 = v_load((int16_t*)(src[pre_shift]) + i + 24);
        v_mul_expand(v_add_wrap(v_src00, v_128), v_mul, v_res0, v_res1);
        v_mul_expand(v_add_wrap(v_src10, v_128), v_mul, v_res2, v_res3);
        v_mul_expand(v_add_wrap(v_src20, v_128), v_mul, v_res4, v_res5);
        v_mul_expand(v_add_wrap(v_src30, v_128), v_mul, v_res6, v_res7);

        int j = 0;
        for (; j < pre_shift; j++)
        {
            v_mul = v_reinterpret_as_s16(v_setall_u16(*((uint16_t*)(m + j))));

            v_src00 = v_load((int16_t*)(src[j]) + i);
            v_src10 = v_load((int16_t*)(src[j]) + i + 8);
            v_src20 = v_load((int16_t*)(src[j]) + i + 16);
            v_src30 = v_load((int16_t*)(src[j]) + i + 24);
            v_src01 = v_load((int16_t*)(src[n - 1 - j]) + i);
            v_src11 = v_load((int16_t*)(src[n - 1 - j]) + i + 8);
            v_src21 = v_load((int16_t*)(src[n - 1 - j]) + i + 16);
            v_src31 = v_load((int16_t*)(src[n - 1 - j]) + i + 24);
            v_zip(v_add_wrap(v_src00, v_128), v_add_wrap(v_src01, v_128), v_tmp0, v_tmp1);
            v_res0 += v_dotprod(v_tmp0, v_mul);
            v_res1 += v_dotprod(v_tmp1, v_mul);
            v_zip(v_add_wrap(v_src10, v_128), v_add_wrap(v_src11, v_128), v_tmp2, v_tmp3);
            v_res2 += v_dotprod(v_tmp2, v_mul);
            v_res3 += v_dotprod(v_tmp3, v_mul);
            v_zip(v_add_wrap(v_src20, v_128), v_add_wrap(v_src21, v_128), v_tmp4, v_tmp5);
            v_res4 += v_dotprod(v_tmp4, v_mul);
            v_res5 += v_dotprod(v_tmp5, v_mul);
            v_zip(v_add_wrap(v_src30, v_128), v_add_wrap(v_src31, v_128), v_tmp6, v_tmp7);
            v_res6 += v_dotprod(v_tmp6, v_mul);
            v_res7 += v_dotprod(v_tmp7, v_mul);
        }

        v_res0 += v_128_4;
        v_res1 += v_128_4;
        v_res2 += v_128_4;
        v_res3 += v_128_4;
        v_res4 += v_128_4;
        v_res5 += v_128_4;
        v_res6 += v_128_4;
        v_res7 += v_128_4;

        v_store(dst + i     , v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res0, v_res1)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res2, v_res3))));
        v_store(dst + i + 16, v_pack(v_reinterpret_as_u16(v_rshr_pack<16>(v_res4, v_res5)),
                                     v_reinterpret_as_u16(v_rshr_pack<16>(v_res6, v_res7))));
    }
    for (; i < len; i++)
    {
        ufixedpoint32 val = m[0] * src[0][i];
        for (int j = 1; j < n; j++)
        {
            val = val + m[j] * src[j][i];
        }
        dst[i] = val;
    }
}
template <typename ET, typename FT>
class fixedSmoothInvoker : public ParallelLoopBody
{
public:
    fixedSmoothInvoker(const ET* _src, size_t _src_stride, ET* _dst, size_t _dst_stride,
                       int _width, int _height, int _cn, const FT* _kx, int _kxlen, const FT* _ky, int _kylen, int _borderType) : ParallelLoopBody(),
                       src(_src), dst(_dst), src_stride(_src_stride), dst_stride(_dst_stride),
                       width(_width), height(_height), cn(_cn), kx(_kx), ky(_ky), kxlen(_kxlen), kylen(_kylen), borderType(_borderType)
    {
        if (kxlen == 1)
        {
            if (kx[0] == FT::one())
                hlineSmoothFunc = hlineSmooth1N1;
            else
                hlineSmoothFunc = hlineSmooth1N;
        }
        else if (kxlen == 3)
        {
            if (kx[0] == (FT::one()>>2)&&kx[1] == (FT::one()>>1)&&kx[2] == (FT::one()>>2))
                hlineSmoothFunc = hlineSmooth3N121;
            else if ((kx[0] - kx[2]).isZero())
                    hlineSmoothFunc = hlineSmooth3Naba;
            else
                hlineSmoothFunc = hlineSmooth3N;
        }
        else if (kxlen == 5)
        {
            if (kx[2] == (FT::one()*(uint8_t)3>>3) &&
                kx[1] == (FT::one()>>2) && kx[3] == (FT::one()>>2) &&
                kx[0] == (FT::one()>>4) && kx[4] == (FT::one()>>4))
                hlineSmoothFunc = hlineSmooth5N14641;
            else if (kx[0] == kx[4] && kx[1] == kx[3])
                hlineSmoothFunc = hlineSmooth5Nabcba;
            else
                hlineSmoothFunc = hlineSmooth5N;
        }
        else if (kxlen % 2 == 1)
        {
            hlineSmoothFunc = hlineSmoothONa_yzy_a;
            for (int i = 0; i < kxlen / 2; i++)
                if (!(kx[i] == kx[kxlen - 1 - i]))
                {
                    hlineSmoothFunc = hlineSmooth;
                    break;
                }
        }
        else
            hlineSmoothFunc = hlineSmooth;
        if (kylen == 1)
        {
            if (ky[0] == FT::one())
                vlineSmoothFunc = vlineSmooth1N1;
            else
                vlineSmoothFunc = vlineSmooth1N;
        }
        else if (kylen == 3)
        {
            if (ky[0] == (FT::one() >> 2) && ky[1] == (FT::one() >> 1) && ky[2] == (FT::one() >> 2))
                vlineSmoothFunc = vlineSmooth3N121;
            else
                vlineSmoothFunc = vlineSmooth3N;
        }
        else if (kylen == 5)
        {
            if (ky[2] == (FT::one() * (uint8_t)3 >> 3) &&
                ky[1] == (FT::one() >> 2) && ky[3] == (FT::one() >> 2) &&
                ky[0] == (FT::one() >> 4) && ky[4] == (FT::one() >> 4))
                vlineSmoothFunc = vlineSmooth5N14641;
            else
                vlineSmoothFunc = vlineSmooth5N;
        }
        else if (kylen % 2 == 1)
        {
            vlineSmoothFunc = vlineSmoothONa_yzy_a;
            for (int i = 0; i < kylen / 2; i++)
                if (!(ky[i] == ky[kylen - 1 - i]))
                {
                    vlineSmoothFunc = vlineSmooth;
                    break;
                }
        }
        else
            vlineSmoothFunc = vlineSmooth;
    }
    virtual void operator() (const Range& range) const CV_OVERRIDE
    {
        AutoBuffer<FT> _buf(width*cn*kylen);
        FT* buf = _buf.data();
        AutoBuffer<FT*> _ptrs(kylen*2);
        FT** ptrs = _ptrs.data();

        if (kylen == 1)
        {
            ptrs[0] = buf;
            for (int i = range.start; i < range.end; i++)
            {
                hlineSmoothFunc(src + i * src_stride, cn, kx, kxlen, ptrs[0], width, borderType);
                vlineSmoothFunc(ptrs, ky, kylen, dst + i * dst_stride, width*cn);
            }
        }
        else if (borderType != BORDER_CONSTANT)// If BORDER_CONSTANT out of border values are equal to zero and could be skipped
        {
            int pre_shift = kylen / 2;
            int post_shift = kylen - pre_shift - 1;
            // First line evaluation
            int idst = range.start;
            int ifrom = max(0, idst - pre_shift);
            int ito = idst + post_shift + 1;
            int i = ifrom;
            int bufline = 0;
            for (; i < min(ito, height); i++, bufline++)
            {
                ptrs[bufline+kylen] = ptrs[bufline] = buf + bufline * width*cn;
                hlineSmoothFunc(src + i * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
            }
            for (; i < ito; i++, bufline++)
            {
                int src_idx = borderInterpolate(i, height, borderType);
                if (src_idx < ifrom)
                {
                    ptrs[bufline + kylen] = ptrs[bufline] = buf + bufline * width*cn;
                    hlineSmoothFunc(src + src_idx * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
                }
                else
                {
                    ptrs[bufline + kylen] = ptrs[bufline] = ptrs[src_idx - ifrom];
                }
            }
            for (int j = idst - pre_shift; j < 0; j++)
            {
                int src_idx = borderInterpolate(j, height, borderType);
                if (src_idx >= ito)
                {
                    ptrs[2*kylen + j] = ptrs[kylen + j] = buf + (kylen + j) * width*cn;
                    hlineSmoothFunc(src + src_idx * src_stride, cn, kx, kxlen, ptrs[kylen + j], width, borderType);
                }
                else
                {
                    ptrs[2*kylen + j] = ptrs[kylen + j] = ptrs[src_idx];
                }
            }
            vlineSmoothFunc(ptrs + bufline, ky, kylen, dst + idst*dst_stride, width*cn); idst++;

            // border mode dependent part evaluation
            // i points to last src row to evaluate in convolution
            bufline %= kylen; ito = min(height, range.end + post_shift);
            for (; i < min(kylen, ito); i++, idst++)
            {
                ptrs[bufline + kylen] = ptrs[bufline] = buf + bufline * width*cn;
                hlineSmoothFunc(src + i * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
                bufline = (bufline + 1) % kylen;
                vlineSmoothFunc(ptrs + bufline, ky, kylen, dst + idst*dst_stride, width*cn);
            }
            // Points inside the border
            for (; i < ito; i++, idst++)
            {
                hlineSmoothFunc(src + i * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
                bufline = (bufline + 1) % kylen;
                vlineSmoothFunc(ptrs + bufline, ky, kylen, dst + idst*dst_stride, width*cn);
            }
            // Points that could fall below border
            for (; i < range.end + post_shift; i++, idst++)
            {
                int src_idx = borderInterpolate(i, height, borderType);
                if ((i - src_idx) > kylen)
                    hlineSmoothFunc(src + src_idx * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
                else
                    ptrs[bufline + kylen] = ptrs[bufline] = ptrs[(bufline + kylen - (i - src_idx)) % kylen];
                bufline = (bufline + 1) % kylen;
                vlineSmoothFunc(ptrs + bufline, ky, kylen, dst + idst*dst_stride, width*cn);
            }
        }
        else
        {
            int pre_shift = kylen / 2;
            int post_shift = kylen - pre_shift - 1;
            // First line evaluation
            int idst = range.start;
            int ifrom = idst - pre_shift;
            int ito = min(idst + post_shift + 1, height);
            int i = max(0, ifrom);
            int bufline = 0;
            for (; i < ito; i++, bufline++)
            {
                ptrs[bufline + kylen] = ptrs[bufline] = buf + bufline * width*cn;
                hlineSmoothFunc(src + i * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
            }

            if (bufline == 1)
                vlineSmooth1N(ptrs, ky - min(ifrom, 0), bufline, dst + idst*dst_stride, width*cn);
            else if (bufline == 3)
                vlineSmooth3N(ptrs, ky - min(ifrom, 0), bufline, dst + idst*dst_stride, width*cn);
            else if (bufline == 5)
                vlineSmooth5N(ptrs, ky - min(ifrom, 0), bufline, dst + idst*dst_stride, width*cn);
            else
                vlineSmooth(ptrs, ky - min(ifrom, 0), bufline, dst + idst*dst_stride, width*cn);
            idst++;

            // border mode dependent part evaluation
            // i points to last src row to evaluate in convolution
            bufline %= kylen; ito = min(height, range.end + post_shift);
            for (; i < min(kylen, ito); i++, idst++)
            {
                ptrs[bufline + kylen] = ptrs[bufline] = buf + bufline * width*cn;
                hlineSmoothFunc(src + i * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
                bufline++;
                if (bufline == 3)
                    vlineSmooth3N(ptrs, ky + kylen - bufline, i + 1, dst + idst*dst_stride, width*cn);
                else if (bufline == 5)
                    vlineSmooth5N(ptrs, ky + kylen - bufline, i + 1, dst + idst*dst_stride, width*cn);
                else
                    vlineSmooth(ptrs, ky + kylen - bufline, i + 1, dst + idst*dst_stride, width*cn);
                bufline %= kylen;
            }
            // Points inside the border
            if (i - max(0, ifrom) >= kylen)
            {
                for (; i < ito; i++, idst++)
                {
                    hlineSmoothFunc(src + i * src_stride, cn, kx, kxlen, ptrs[bufline], width, borderType);
                    bufline = (bufline + 1) % kylen;
                    vlineSmoothFunc(ptrs + bufline, ky, kylen, dst + idst*dst_stride, width*cn);
                }

                // Points that could fall below border
                // i points to first src row to evaluate in convolution
                bufline = (bufline + 1) % kylen;
                for (i = idst - pre_shift; i < range.end - pre_shift; i++, idst++, bufline++)
                    if (height - i == 3)
                        vlineSmooth3N(ptrs + bufline, ky, height - i, dst + idst*dst_stride, width*cn);
                    else if (height - i == 5)
                        vlineSmooth5N(ptrs + bufline, ky, height - i, dst + idst*dst_stride, width*cn);
                    else
                        vlineSmooth(ptrs + bufline, ky, height - i, dst + idst*dst_stride, width*cn);
            }
            else
            {
                // i points to first src row to evaluate in convolution
                for (i = idst - pre_shift; i < min(range.end - pre_shift, 0); i++, idst++)
                    if (height == 3)
                        vlineSmooth3N(ptrs, ky - i, height, dst + idst*dst_stride, width*cn);
                    else if (height == 5)
                        vlineSmooth5N(ptrs, ky - i, height, dst + idst*dst_stride, width*cn);
                    else
                        vlineSmooth(ptrs, ky - i, height, dst + idst*dst_stride, width*cn);
                for (; i < range.end - pre_shift; i++, idst++)
                    if (height - i == 3)
                        vlineSmooth3N(ptrs + i - max(0, ifrom), ky, height - i, dst + idst*dst_stride, width*cn);
                    else if (height - i == 5)
                        vlineSmooth5N(ptrs + i - max(0, ifrom), ky, height - i, dst + idst*dst_stride, width*cn);
                    else
                        vlineSmooth(ptrs + i - max(0, ifrom), ky, height - i, dst + idst*dst_stride, width*cn);
            }
        }
    }
private:
    const ET* src;
    ET* dst;
    size_t src_stride, dst_stride;
    int width, height, cn;
    const FT *kx, *ky;
    int kxlen, kylen;
    int borderType;
    void(*hlineSmoothFunc)(const ET* src, int cn, const FT* m, int n, FT* dst, int len, int borderType);
    void(*vlineSmoothFunc)(const FT* const * src, const FT* m, int n, ET* dst, int len);

    fixedSmoothInvoker(const fixedSmoothInvoker&);
    fixedSmoothInvoker& operator=(const fixedSmoothInvoker&);
};

static void getGaussianKernel(int n, double sigma, int ktype, Mat& res) { res = getGaussianKernel(n, sigma, ktype); }
template <typename T> static void getGaussianKernel(int n, double sigma, int, std::vector<T>& res) { res = getFixedpointGaussianKernel<T>(n, sigma); }

template <typename T>
static void createGaussianKernels( T & kx, T & ky, int type, Size &ksize,
                                   double sigma1, double sigma2 )
{
    int depth = CV_MAT_DEPTH(type);
    if( sigma2 <= 0 )
        sigma2 = sigma1;

    // automatic detection of kernel size from sigma
    if( ksize.width <= 0 && sigma1 > 0 )
        ksize.width = cvRound(sigma1*(depth == CV_8U ? 3 : 4)*2 + 1)|1;
    if( ksize.height <= 0 && sigma2 > 0 )
        ksize.height = cvRound(sigma2*(depth == CV_8U ? 3 : 4)*2 + 1)|1;

    CV_Assert( ksize.width > 0 && ksize.width % 2 == 1 &&
        ksize.height > 0 && ksize.height % 2 == 1 );

    sigma1 = std::max( sigma1, 0. );
    sigma2 = std::max( sigma2, 0. );

    getGaussianKernel( ksize.width, sigma1, std::max(depth, CV_32F), kx );
    if( ksize.height == ksize.width && std::abs(sigma1 - sigma2) < DBL_EPSILON )
        ky = kx;
    else
        getGaussianKernel( ksize.height, sigma2, std::max(depth, CV_32F), ky );
}

}

cv::Ptr<cv::FilterEngine> cv::createGaussianFilter( int type, Size ksize,
                                        double sigma1, double sigma2,
                                        int borderType )
{
    Mat kx, ky;
    createGaussianKernels(kx, ky, type, ksize, sigma1, sigma2);

    return createSeparableLinearFilter( type, type, kx, ky, Point(-1,-1), 0, borderType );
}

namespace cv
{
#ifdef HAVE_OPENCL

static bool ocl_GaussianBlur_8UC1(InputArray _src, OutputArray _dst, Size ksize, int ddepth,
                                  InputArray _kernelX, InputArray _kernelY, int borderType)
{
    const ocl::Device & dev = ocl::Device::getDefault();
    int type = _src.type(), sdepth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);

    if ( !(dev.isIntel() && (type == CV_8UC1) &&
         (_src.offset() == 0) && (_src.step() % 4 == 0) &&
         ((ksize.width == 5 && (_src.cols() % 4 == 0)) ||
         (ksize.width == 3 && (_src.cols() % 16 == 0) && (_src.rows() % 2 == 0)))) )
        return false;

    Mat kernelX = _kernelX.getMat().reshape(1, 1);
    if (kernelX.cols % 2 != 1)
        return false;
    Mat kernelY = _kernelY.getMat().reshape(1, 1);
    if (kernelY.cols % 2 != 1)
        return false;

    if (ddepth < 0)
        ddepth = sdepth;

    Size size = _src.size();
    size_t globalsize[2] = { 0, 0 };
    size_t localsize[2] = { 0, 0 };

    if (ksize.width == 3)
    {
        globalsize[0] = size.width / 16;
        globalsize[1] = size.height / 2;
    }
    else if (ksize.width == 5)
    {
        globalsize[0] = size.width / 4;
        globalsize[1] = size.height / 1;
    }

    const char * const borderMap[] = { "BORDER_CONSTANT", "BORDER_REPLICATE", "BORDER_REFLECT", 0, "BORDER_REFLECT_101" };
    char build_opts[1024];
    sprintf(build_opts, "-D %s %s%s", borderMap[borderType & ~BORDER_ISOLATED],
            ocl::kernelToStr(kernelX, CV_32F, "KERNEL_MATRIX_X").c_str(),
            ocl::kernelToStr(kernelY, CV_32F, "KERNEL_MATRIX_Y").c_str());

    ocl::Kernel kernel;

    if (ksize.width == 3)
        kernel.create("gaussianBlur3x3_8UC1_cols16_rows2", cv::ocl::imgproc::gaussianBlur3x3_oclsrc, build_opts);
    else if (ksize.width == 5)
        kernel.create("gaussianBlur5x5_8UC1_cols4", cv::ocl::imgproc::gaussianBlur5x5_oclsrc, build_opts);

    if (kernel.empty())
        return false;

    UMat src = _src.getUMat();
    _dst.create(size, CV_MAKETYPE(ddepth, cn));
    if (!(_dst.offset() == 0 && _dst.step() % 4 == 0))
        return false;
    UMat dst = _dst.getUMat();

    int idxArg = kernel.set(0, ocl::KernelArg::PtrReadOnly(src));
    idxArg = kernel.set(idxArg, (int)src.step);
    idxArg = kernel.set(idxArg, ocl::KernelArg::PtrWriteOnly(dst));
    idxArg = kernel.set(idxArg, (int)dst.step);
    idxArg = kernel.set(idxArg, (int)dst.rows);
    idxArg = kernel.set(idxArg, (int)dst.cols);

    return kernel.run(2, globalsize, (localsize[0] == 0) ? NULL : localsize, false);
}

#endif

#ifdef HAVE_OPENVX

namespace ovx {
    template <> inline bool skipSmallImages<VX_KERNEL_GAUSSIAN_3x3>(int w, int h) { return w*h < 320 * 240; }
}
static bool openvx_gaussianBlur(InputArray _src, OutputArray _dst, Size ksize,
                                double sigma1, double sigma2, int borderType)
{
    if (sigma2 <= 0)
        sigma2 = sigma1;
    // automatic detection of kernel size from sigma
    if (ksize.width <= 0 && sigma1 > 0)
        ksize.width = cvRound(sigma1*6 + 1) | 1;
    if (ksize.height <= 0 && sigma2 > 0)
        ksize.height = cvRound(sigma2*6 + 1) | 1;

    if (_src.type() != CV_8UC1 ||
        _src.cols() < 3 || _src.rows() < 3 ||
        ksize.width != 3 || ksize.height != 3)
        return false;

    sigma1 = std::max(sigma1, 0.);
    sigma2 = std::max(sigma2, 0.);

    if (!(sigma1 == 0.0 || (sigma1 - 0.8) < DBL_EPSILON) || !(sigma2 == 0.0 || (sigma2 - 0.8) < DBL_EPSILON) ||
        ovx::skipSmallImages<VX_KERNEL_GAUSSIAN_3x3>(_src.cols(), _src.rows()))
        return false;

    Mat src = _src.getMat();
    Mat dst = _dst.getMat();

    if ((borderType & BORDER_ISOLATED) == 0 && src.isSubmatrix())
        return false; //Process isolated borders only
    vx_enum border;
    switch (borderType & ~BORDER_ISOLATED)
    {
    case BORDER_CONSTANT:
        border = VX_BORDER_CONSTANT;
        break;
    case BORDER_REPLICATE:
        border = VX_BORDER_REPLICATE;
        break;
    default:
        return false;
    }

    try
    {
        ivx::Context ctx = ovx::getOpenVXContext();

        Mat a;
        if (dst.data != src.data)
            a = src;
        else
            src.copyTo(a);

        ivx::Image
            ia = ivx::Image::createFromHandle(ctx, VX_DF_IMAGE_U8,
                ivx::Image::createAddressing(a.cols, a.rows, 1, (vx_int32)(a.step)), a.data),
            ib = ivx::Image::createFromHandle(ctx, VX_DF_IMAGE_U8,
                ivx::Image::createAddressing(dst.cols, dst.rows, 1, (vx_int32)(dst.step)), dst.data);

        //ATTENTION: VX_CONTEXT_IMMEDIATE_BORDER attribute change could lead to strange issues in multi-threaded environments
        //since OpenVX standard says nothing about thread-safety for now
        ivx::border_t prevBorder = ctx.immediateBorder();
        ctx.setImmediateBorder(border, (vx_uint8)(0));
        ivx::IVX_CHECK_STATUS(vxuGaussian3x3(ctx, ia, ib));
        ctx.setImmediateBorder(prevBorder);
    }
    catch (ivx::RuntimeError & e)
    {
        VX_DbgThrow(e.what());
    }
    catch (ivx::WrapperError & e)
    {
        VX_DbgThrow(e.what());
    }
    return true;
}

#endif

#ifdef HAVE_IPP
// IW 2017u2 has bug which doesn't allow use of partial inMem with tiling
#if IPP_DISABLE_GAUSSIANBLUR_PARALLEL
#define IPP_GAUSSIANBLUR_PARALLEL 0
#else
#define IPP_GAUSSIANBLUR_PARALLEL 1
#endif

#ifdef HAVE_IPP_IW

class ipp_gaussianBlurParallel: public ParallelLoopBody
{
public:
    ipp_gaussianBlurParallel(::ipp::IwiImage &src, ::ipp::IwiImage &dst, int kernelSize, float sigma, ::ipp::IwiBorderType &border, bool *pOk):
        m_src(src), m_dst(dst), m_kernelSize(kernelSize), m_sigma(sigma), m_border(border), m_pOk(pOk) {
        *m_pOk = true;
    }
    ~ipp_gaussianBlurParallel()
    {
    }

    virtual void operator() (const Range& range) const CV_OVERRIDE
    {
        CV_INSTRUMENT_REGION_IPP()

        if(!*m_pOk)
            return;

        try
        {
            ::ipp::IwiTile tile = ::ipp::IwiRoi(0, range.start, m_dst.m_size.width, range.end - range.start);
            CV_INSTRUMENT_FUN_IPP(::ipp::iwiFilterGaussian, m_src, m_dst, m_kernelSize, m_sigma, ::ipp::IwDefault(), m_border, tile);
        }
        catch(::ipp::IwException e)
        {
            *m_pOk = false;
            return;
        }
    }
private:
    ::ipp::IwiImage &m_src;
    ::ipp::IwiImage &m_dst;

    int m_kernelSize;
    float m_sigma;
    ::ipp::IwiBorderType &m_border;

    volatile bool *m_pOk;
    const ipp_gaussianBlurParallel& operator= (const ipp_gaussianBlurParallel&);
};

#endif

static bool ipp_GaussianBlur(InputArray _src, OutputArray _dst, Size ksize,
                   double sigma1, double sigma2, int borderType )
{
#ifdef HAVE_IPP_IW
    CV_INSTRUMENT_REGION_IPP()

#if IPP_VERSION_X100 < 201800 && ((defined _MSC_VER && defined _M_IX86) || (defined __GNUC__ && defined __i386__))
    CV_UNUSED(_src); CV_UNUSED(_dst); CV_UNUSED(ksize); CV_UNUSED(sigma1); CV_UNUSED(sigma2); CV_UNUSED(borderType);
    return false; // bug on ia32
#else
    if(sigma1 != sigma2)
        return false;

    if(sigma1 < FLT_EPSILON)
        return false;

    if(ksize.width != ksize.height)
        return false;

    // Acquire data and begin processing
    try
    {
        Mat src = _src.getMat();
        Mat dst = _dst.getMat();
        ::ipp::IwiImage       iwSrc      = ippiGetImage(src);
        ::ipp::IwiImage       iwDst      = ippiGetImage(dst);
        ::ipp::IwiBorderSize  borderSize = ::ipp::iwiSizeToBorderSize(ippiGetSize(ksize));
        ::ipp::IwiBorderType  ippBorder(ippiGetBorder(iwSrc, borderType, borderSize));
        if(!ippBorder)
            return false;

        const int threads = ippiSuggestThreadsNum(iwDst, 2);
        if(IPP_GAUSSIANBLUR_PARALLEL && threads > 1) {
            bool ok;
            ipp_gaussianBlurParallel invoker(iwSrc, iwDst, ksize.width, (float) sigma1, ippBorder, &ok);

            if(!ok)
                return false;
            const Range range(0, (int) iwDst.m_size.height);
            parallel_for_(range, invoker, threads*4);

            if(!ok)
                return false;
        } else {
            CV_INSTRUMENT_FUN_IPP(::ipp::iwiFilterGaussian, iwSrc, iwDst, ksize.width, sigma1, ::ipp::IwDefault(), ippBorder);
        }
    }
    catch (::ipp::IwException ex)
    {
        return false;
    }

    return true;
#endif
#else
    CV_UNUSED(_src); CV_UNUSED(_dst); CV_UNUSED(ksize); CV_UNUSED(sigma1); CV_UNUSED(sigma2); CV_UNUSED(borderType);
    return false;
#endif
}
#endif
}

void cv::GaussianBlur( InputArray _src, OutputArray _dst, Size ksize,
                   double sigma1, double sigma2,
                   int borderType )
{
    CV_INSTRUMENT_REGION()

    int type = _src.type();
    Size size = _src.size();
    _dst.create( size, type );

    if( (borderType & ~BORDER_ISOLATED) != BORDER_CONSTANT &&
        ((borderType & BORDER_ISOLATED) != 0 || !_src.getMat().isSubmatrix()) )
    {
        if( size.height == 1 )
            ksize.height = 1;
        if( size.width == 1 )
            ksize.width = 1;
    }

    if( ksize.width == 1 && ksize.height == 1 )
    {
        _src.copyTo(_dst);
        return;
    }

    bool useOpenCL = (ocl::isOpenCLActivated() && _dst.isUMat() && _src.dims() <= 2 &&
               ((ksize.width == 3 && ksize.height == 3) ||
               (ksize.width == 5 && ksize.height == 5)) &&
               _src.rows() > ksize.height && _src.cols() > ksize.width);
    (void)useOpenCL;

    int sdepth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);

    if(sdepth == CV_8U && ((borderType & BORDER_ISOLATED) || !_src.getMat().isSubmatrix()))
    {
        std::vector<ufixedpoint16> fkx, fky;
        createGaussianKernels(fkx, fky, type, ksize, sigma1, sigma2);
        Mat src = _src.getMat();
        Mat dst = _dst.getMat();
        if (src.data == dst.data)
            src = src.clone();
        fixedSmoothInvoker<uint8_t, ufixedpoint16> invoker(src.ptr<uint8_t>(), src.step1(), dst.ptr<uint8_t>(), dst.step1(), dst.cols, dst.rows, dst.channels(), &fkx[0], (int)fkx.size(), &fky[0], (int)fky.size(), borderType & ~BORDER_ISOLATED);
        parallel_for_(Range(0, dst.rows), invoker, std::max(1, std::min(getNumThreads(), getNumberOfCPUs())));
        return;
    }


    Mat kx, ky;
    createGaussianKernels(kx, ky, type, ksize, sigma1, sigma2);

    CV_OCL_RUN(useOpenCL, ocl_GaussianBlur_8UC1(_src, _dst, ksize, CV_MAT_DEPTH(type), kx, ky, borderType));

    CV_OCL_RUN(_dst.isUMat() && _src.dims() <= 2 && (size_t)_src.rows() > kx.total() && (size_t)_src.cols() > kx.total(),
               ocl_sepFilter2D(_src, _dst, sdepth, kx, ky, Point(-1, -1), 0, borderType))

    Mat src = _src.getMat();
    Mat dst = _dst.getMat();

    Point ofs;
    Size wsz(src.cols, src.rows);
    if(!(borderType & BORDER_ISOLATED))
        src.locateROI( wsz, ofs );

    CALL_HAL(gaussianBlur, cv_hal_gaussianBlur, src.ptr(), src.step, dst.ptr(), dst.step, src.cols, src.rows, sdepth, cn,
             ofs.x, ofs.y, wsz.width - src.cols - ofs.x, wsz.height - src.rows - ofs.y, ksize.width, ksize.height,
             sigma1, sigma2, borderType&~BORDER_ISOLATED);

    CV_OVX_RUN(true,
               openvx_gaussianBlur(src, dst, ksize, sigma1, sigma2, borderType))

    CV_IPP_RUN_FAST(ipp_GaussianBlur(src, dst, ksize, sigma1, sigma2, borderType));

    sepFilter2D(src, dst, sdepth, kx, ky, Point(-1, -1), 0, borderType);
}

/****************************************************************************************\
                                      Median Filter
\****************************************************************************************/

namespace cv
{
typedef ushort HT;

/**
 * This structure represents a two-tier histogram. The first tier (known as the
 * "coarse" level) is 4 bit wide and the second tier (known as the "fine" level)
 * is 8 bit wide. Pixels inserted in the fine level also get inserted into the
 * coarse bucket designated by the 4 MSBs of the fine bucket value.
 *
 * The structure is aligned on 16 bits, which is a prerequisite for SIMD
 * instructions. Each bucket is 16 bit wide, which means that extra care must be
 * taken to prevent overflow.
 */
typedef struct
{
    HT coarse[16];
    HT fine[16][16];
} Histogram;


#if CV_SIMD128

static inline void histogram_add_simd( const HT x[16], HT y[16] )
{
    v_store(y, v_load(x) + v_load(y));
    v_store(y + 8, v_load(x + 8) + v_load(y + 8));
}

static inline void histogram_sub_simd( const HT x[16], HT y[16] )
{
    v_store(y, v_load(y) - v_load(x));
    v_store(y + 8, v_load(y + 8) - v_load(x + 8));
}

#endif


static inline void histogram_add( const HT x[16], HT y[16] )
{
    int i;
    for( i = 0; i < 16; ++i )
        y[i] = (HT)(y[i] + x[i]);
}

static inline void histogram_sub( const HT x[16], HT y[16] )
{
    int i;
    for( i = 0; i < 16; ++i )
        y[i] = (HT)(y[i] - x[i]);
}

static inline void histogram_muladd( int a, const HT x[16],
        HT y[16] )
{
    for( int i = 0; i < 16; ++i )
        y[i] = (HT)(y[i] + a * x[i]);
}

static void
medianBlur_8u_O1( const Mat& _src, Mat& _dst, int ksize )
{
/**
 * HOP is short for Histogram OPeration. This macro makes an operation \a op on
 * histogram \a h for pixel value \a x. It takes care of handling both levels.
 */
#define HOP(h,x,op) \
    h.coarse[x>>4] op, \
    *((HT*)h.fine + x) op

#define COP(c,j,x,op) \
    h_coarse[ 16*(n*c+j) + (x>>4) ] op, \
    h_fine[ 16 * (n*(16*c+(x>>4)) + j) + (x & 0xF) ] op

    int cn = _dst.channels(), m = _dst.rows, r = (ksize-1)/2;
    CV_Assert(cn > 0 && cn <= 4);
    size_t sstep = _src.step, dstep = _dst.step;
    Histogram CV_DECL_ALIGNED(16) H[4];
    HT CV_DECL_ALIGNED(16) luc[4][16];

    int STRIPE_SIZE = std::min( _dst.cols, 512/cn );

    std::vector<HT> _h_coarse(1 * 16 * (STRIPE_SIZE + 2*r) * cn + 16);
    std::vector<HT> _h_fine(16 * 16 * (STRIPE_SIZE + 2*r) * cn + 16);
    HT* h_coarse = alignPtr(&_h_coarse[0], 16);
    HT* h_fine = alignPtr(&_h_fine[0], 16);
#if CV_SIMD128
    volatile bool useSIMD = hasSIMD128();
#endif

    for( int x = 0; x < _dst.cols; x += STRIPE_SIZE )
    {
        int i, j, k, c, n = std::min(_dst.cols - x, STRIPE_SIZE) + r*2;
        const uchar* src = _src.ptr() + x*cn;
        uchar* dst = _dst.ptr() + (x - r)*cn;

        memset( h_coarse, 0, 16*n*cn*sizeof(h_coarse[0]) );
        memset( h_fine, 0, 16*16*n*cn*sizeof(h_fine[0]) );

        // First row initialization
        for( c = 0; c < cn; c++ )
        {
            for( j = 0; j < n; j++ )
                COP( c, j, src[cn*j+c], += (cv::HT)(r+2) );

            for( i = 1; i < r; i++ )
            {
                const uchar* p = src + sstep*std::min(i, m-1);
                for ( j = 0; j < n; j++ )
                    COP( c, j, p[cn*j+c], ++ );
            }
        }

        for( i = 0; i < m; i++ )
        {
            const uchar* p0 = src + sstep * std::max( 0, i-r-1 );
            const uchar* p1 = src + sstep * std::min( m-1, i+r );

            memset( H, 0, cn*sizeof(H[0]) );
            memset( luc, 0, cn*sizeof(luc[0]) );
            for( c = 0; c < cn; c++ )
            {
                // Update column histograms for the entire row.
                for( j = 0; j < n; j++ )
                {
                    COP( c, j, p0[j*cn + c], -- );
                    COP( c, j, p1[j*cn + c], ++ );
                }

                // First column initialization
                for( k = 0; k < 16; ++k )
                    histogram_muladd( 2*r+1, &h_fine[16*n*(16*c+k)], &H[c].fine[k][0] );

#if CV_SIMD128
                if( useSIMD )
                {
                    for( j = 0; j < 2*r; ++j )
                        histogram_add_simd( &h_coarse[16*(n*c+j)], H[c].coarse );

                    for( j = r; j < n-r; j++ )
                    {
                        int t = 2*r*r + 2*r, b, sum = 0;
                        HT* segment;

                        histogram_add_simd( &h_coarse[16*(n*c + std::min(j+r,n-1))], H[c].coarse );

                        // Find median at coarse level
                        for ( k = 0; k < 16 ; ++k )
                        {
                            sum += H[c].coarse[k];
                            if ( sum > t )
                            {
                                sum -= H[c].coarse[k];
                                break;
                            }
                        }
                        CV_Assert( k < 16 );

                        /* Update corresponding histogram segment */
                        if ( luc[c][k] <= j-r )
                        {
                            memset( &H[c].fine[k], 0, 16 * sizeof(HT) );
                            for ( luc[c][k] = cv::HT(j-r); luc[c][k] < MIN(j+r+1,n); ++luc[c][k] )
                                histogram_add_simd( &h_fine[16*(n*(16*c+k)+luc[c][k])], H[c].fine[k] );

                            if ( luc[c][k] < j+r+1 )
                            {
                                histogram_muladd( j+r+1 - n, &h_fine[16*(n*(16*c+k)+(n-1))], &H[c].fine[k][0] );
                                luc[c][k] = (HT)(j+r+1);
                            }
                        }
                        else
                        {
                            for ( ; luc[c][k] < j+r+1; ++luc[c][k] )
                            {
                                histogram_sub_simd( &h_fine[16*(n*(16*c+k)+MAX(luc[c][k]-2*r-1,0))], H[c].fine[k] );
                                histogram_add_simd( &h_fine[16*(n*(16*c+k)+MIN(luc[c][k],n-1))], H[c].fine[k] );
                            }
                        }

                        histogram_sub_simd( &h_coarse[16*(n*c+MAX(j-r,0))], H[c].coarse );

                        /* Find median in segment */
                        segment = H[c].fine[k];
                        for ( b = 0; b < 16 ; b++ )
                        {
                            sum += segment[b];
                            if ( sum > t )
                            {
                                dst[dstep*i+cn*j+c] = (uchar)(16*k + b);
                                break;
                            }
                        }
                        CV_Assert( b < 16 );
                    }
                }
                else
#endif
                {
                    for( j = 0; j < 2*r; ++j )
                        histogram_add( &h_coarse[16*(n*c+j)], H[c].coarse );

                    for( j = r; j < n-r; j++ )
                    {
                        int t = 2*r*r + 2*r, b, sum = 0;
                        HT* segment;

                        histogram_add( &h_coarse[16*(n*c + std::min(j+r,n-1))], H[c].coarse );

                        // Find median at coarse level
                        for ( k = 0; k < 16 ; ++k )
                        {
                            sum += H[c].coarse[k];
                            if ( sum > t )
                            {
                                sum -= H[c].coarse[k];
                                break;
                            }
                        }
                        CV_Assert( k < 16 );

                        /* Update corresponding histogram segment */
                        if ( luc[c][k] <= j-r )
                        {
                            memset( &H[c].fine[k], 0, 16 * sizeof(HT) );
                            for ( luc[c][k] = cv::HT(j-r); luc[c][k] < MIN(j+r+1,n); ++luc[c][k] )
                                histogram_add( &h_fine[16*(n*(16*c+k)+luc[c][k])], H[c].fine[k] );

                            if ( luc[c][k] < j+r+1 )
                            {
                                histogram_muladd( j+r+1 - n, &h_fine[16*(n*(16*c+k)+(n-1))], &H[c].fine[k][0] );
                                luc[c][k] = (HT)(j+r+1);
                            }
                        }
                        else
                        {
                            for ( ; luc[c][k] < j+r+1; ++luc[c][k] )
                            {
                                histogram_sub( &h_fine[16*(n*(16*c+k)+MAX(luc[c][k]-2*r-1,0))], H[c].fine[k] );
                                histogram_add( &h_fine[16*(n*(16*c+k)+MIN(luc[c][k],n-1))], H[c].fine[k] );
                            }
                        }

                        histogram_sub( &h_coarse[16*(n*c+MAX(j-r,0))], H[c].coarse );

                        /* Find median in segment */
                        segment = H[c].fine[k];
                        for ( b = 0; b < 16 ; b++ )
                        {
                            sum += segment[b];
                            if ( sum > t )
                            {
                                dst[dstep*i+cn*j+c] = (uchar)(16*k + b);
                                break;
                            }
                        }
                        CV_Assert( b < 16 );
                    }
                }
            }
        }
    }

#undef HOP
#undef COP
}

static void
medianBlur_8u_Om( const Mat& _src, Mat& _dst, int m )
{
    #define N  16
    int     zone0[4][N];
    int     zone1[4][N*N];
    int     x, y;
    int     n2 = m*m/2;
    Size    size = _dst.size();
    const uchar* src = _src.ptr();
    uchar*  dst = _dst.ptr();
    int     src_step = (int)_src.step, dst_step = (int)_dst.step;
    int     cn = _src.channels();
    const uchar*  src_max = src + size.height*src_step;
    CV_Assert(cn > 0 && cn <= 4);

    #define UPDATE_ACC01( pix, cn, op ) \
    {                                   \
        int p = (pix);                  \
        zone1[cn][p] op;                \
        zone0[cn][p >> 4] op;           \
    }

    //CV_Assert( size.height >= nx && size.width >= nx );
    for( x = 0; x < size.width; x++, src += cn, dst += cn )
    {
        uchar* dst_cur = dst;
        const uchar* src_top = src;
        const uchar* src_bottom = src;
        int k, c;
        int src_step1 = src_step, dst_step1 = dst_step;

        if( x % 2 != 0 )
        {
            src_bottom = src_top += src_step*(size.height-1);
            dst_cur += dst_step*(size.height-1);
            src_step1 = -src_step1;
            dst_step1 = -dst_step1;
        }

        // init accumulator
        memset( zone0, 0, sizeof(zone0[0])*cn );
        memset( zone1, 0, sizeof(zone1[0])*cn );

        for( y = 0; y <= m/2; y++ )
        {
            for( c = 0; c < cn; c++ )
            {
                if( y > 0 )
                {
                    for( k = 0; k < m*cn; k += cn )
                        UPDATE_ACC01( src_bottom[k+c], c, ++ );
                }
                else
                {
                    for( k = 0; k < m*cn; k += cn )
                        UPDATE_ACC01( src_bottom[k+c], c, += m/2+1 );
                }
            }

            if( (src_step1 > 0 && y < size.height-1) ||
                (src_step1 < 0 && size.height-y-1 > 0) )
                src_bottom += src_step1;
        }

        for( y = 0; y < size.height; y++, dst_cur += dst_step1 )
        {
            // find median
            for( c = 0; c < cn; c++ )
            {
                int s = 0;
                for( k = 0; ; k++ )
                {
                    int t = s + zone0[c][k];
                    if( t > n2 ) break;
                    s = t;
                }

                for( k *= N; ;k++ )
                {
                    s += zone1[c][k];
                    if( s > n2 ) break;
                }

                dst_cur[c] = (uchar)k;
            }

            if( y+1 == size.height )
                break;

            if( cn == 1 )
            {
                for( k = 0; k < m; k++ )
                {
                    int p = src_top[k];
                    int q = src_bottom[k];
                    zone1[0][p]--;
                    zone0[0][p>>4]--;
                    zone1[0][q]++;
                    zone0[0][q>>4]++;
                }
            }
            else if( cn == 3 )
            {
                for( k = 0; k < m*3; k += 3 )
                {
                    UPDATE_ACC01( src_top[k], 0, -- );
                    UPDATE_ACC01( src_top[k+1], 1, -- );
                    UPDATE_ACC01( src_top[k+2], 2, -- );

                    UPDATE_ACC01( src_bottom[k], 0, ++ );
                    UPDATE_ACC01( src_bottom[k+1], 1, ++ );
                    UPDATE_ACC01( src_bottom[k+2], 2, ++ );
                }
            }
            else
            {
                assert( cn == 4 );
                for( k = 0; k < m*4; k += 4 )
                {
                    UPDATE_ACC01( src_top[k], 0, -- );
                    UPDATE_ACC01( src_top[k+1], 1, -- );
                    UPDATE_ACC01( src_top[k+2], 2, -- );
                    UPDATE_ACC01( src_top[k+3], 3, -- );

                    UPDATE_ACC01( src_bottom[k], 0, ++ );
                    UPDATE_ACC01( src_bottom[k+1], 1, ++ );
                    UPDATE_ACC01( src_bottom[k+2], 2, ++ );
                    UPDATE_ACC01( src_bottom[k+3], 3, ++ );
                }
            }

            if( (src_step1 > 0 && src_bottom + src_step1 < src_max) ||
                (src_step1 < 0 && src_bottom + src_step1 >= src) )
                src_bottom += src_step1;

            if( y >= m/2 )
                src_top += src_step1;
        }
    }
#undef N
#undef UPDATE_ACC
}


struct MinMax8u
{
    typedef uchar value_type;
    typedef int arg_type;
    enum { SIZE = 1 };
    arg_type load(const uchar* ptr) { return *ptr; }
    void store(uchar* ptr, arg_type val) { *ptr = (uchar)val; }
    void operator()(arg_type& a, arg_type& b) const
    {
        int t = CV_FAST_CAST_8U(a - b);
        b += t; a -= t;
    }
};

struct MinMax16u
{
    typedef ushort value_type;
    typedef int arg_type;
    enum { SIZE = 1 };
    arg_type load(const ushort* ptr) { return *ptr; }
    void store(ushort* ptr, arg_type val) { *ptr = (ushort)val; }
    void operator()(arg_type& a, arg_type& b) const
    {
        arg_type t = a;
        a = std::min(a, b);
        b = std::max(b, t);
    }
};

struct MinMax16s
{
    typedef short value_type;
    typedef int arg_type;
    enum { SIZE = 1 };
    arg_type load(const short* ptr) { return *ptr; }
    void store(short* ptr, arg_type val) { *ptr = (short)val; }
    void operator()(arg_type& a, arg_type& b) const
    {
        arg_type t = a;
        a = std::min(a, b);
        b = std::max(b, t);
    }
};

struct MinMax32f
{
    typedef float value_type;
    typedef float arg_type;
    enum { SIZE = 1 };
    arg_type load(const float* ptr) { return *ptr; }
    void store(float* ptr, arg_type val) { *ptr = val; }
    void operator()(arg_type& a, arg_type& b) const
    {
        arg_type t = a;
        a = std::min(a, b);
        b = std::max(b, t);
    }
};

#if CV_SIMD128

struct MinMaxVec8u
{
    typedef uchar value_type;
    typedef v_uint8x16 arg_type;
    enum { SIZE = 16 };
    arg_type load(const uchar* ptr) { return v_load(ptr); }
    void store(uchar* ptr, const arg_type &val) { v_store(ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
        arg_type t = a;
        a = v_min(a, b);
        b = v_max(b, t);
    }
};


struct MinMaxVec16u
{
    typedef ushort value_type;
    typedef v_uint16x8 arg_type;
    enum { SIZE = 8 };
    arg_type load(const ushort* ptr) { return v_load(ptr); }
    void store(ushort* ptr, const arg_type &val) { v_store(ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
        arg_type t = a;
        a = v_min(a, b);
        b = v_max(b, t);
    }
};


struct MinMaxVec16s
{
    typedef short value_type;
    typedef v_int16x8 arg_type;
    enum { SIZE = 8 };
    arg_type load(const short* ptr) { return v_load(ptr); }
    void store(short* ptr, const arg_type &val) { v_store(ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
        arg_type t = a;
        a = v_min(a, b);
        b = v_max(b, t);
    }
};


struct MinMaxVec32f
{
    typedef float value_type;
    typedef v_float32x4 arg_type;
    enum { SIZE = 4 };
    arg_type load(const float* ptr) { return v_load(ptr); }
    void store(float* ptr, const arg_type &val) { v_store(ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
        arg_type t = a;
        a = v_min(a, b);
        b = v_max(b, t);
    }
};

#else

typedef MinMax8u MinMaxVec8u;
typedef MinMax16u MinMaxVec16u;
typedef MinMax16s MinMaxVec16s;
typedef MinMax32f MinMaxVec32f;

#endif

template<class Op, class VecOp>
static void
medianBlur_SortNet( const Mat& _src, Mat& _dst, int m )
{
    typedef typename Op::value_type T;
    typedef typename Op::arg_type WT;
    typedef typename VecOp::arg_type VT;

    const T* src = _src.ptr<T>();
    T* dst = _dst.ptr<T>();
    int sstep = (int)(_src.step/sizeof(T));
    int dstep = (int)(_dst.step/sizeof(T));
    Size size = _dst.size();
    int i, j, k, cn = _src.channels();
    Op op;
    VecOp vop;
    volatile bool useSIMD = hasSIMD128();

    if( m == 3 )
    {
        if( size.width == 1 || size.height == 1 )
        {
            int len = size.width + size.height - 1;
            int sdelta = size.height == 1 ? cn : sstep;
            int sdelta0 = size.height == 1 ? 0 : sstep - cn;
            int ddelta = size.height == 1 ? cn : dstep;

            for( i = 0; i < len; i++, src += sdelta0, dst += ddelta )
                for( j = 0; j < cn; j++, src++ )
                {
                    WT p0 = src[i > 0 ? -sdelta : 0];
                    WT p1 = src[0];
                    WT p2 = src[i < len - 1 ? sdelta : 0];

                    op(p0, p1); op(p1, p2); op(p0, p1);
                    dst[j] = (T)p1;
                }
            return;
        }

        size.width *= cn;
        for( i = 0; i < size.height; i++, dst += dstep )
        {
            const T* row0 = src + std::max(i - 1, 0)*sstep;
            const T* row1 = src + i*sstep;
            const T* row2 = src + std::min(i + 1, size.height-1)*sstep;
            int limit = useSIMD ? cn : size.width;

            for(j = 0;; )
            {
                for( ; j < limit; j++ )
                {
                    int j0 = j >= cn ? j - cn : j;
                    int j2 = j < size.width - cn ? j + cn : j;
                    WT p0 = row0[j0], p1 = row0[j], p2 = row0[j2];
                    WT p3 = row1[j0], p4 = row1[j], p5 = row1[j2];
                    WT p6 = row2[j0], p7 = row2[j], p8 = row2[j2];

                    op(p1, p2); op(p4, p5); op(p7, p8); op(p0, p1);
                    op(p3, p4); op(p6, p7); op(p1, p2); op(p4, p5);
                    op(p7, p8); op(p0, p3); op(p5, p8); op(p4, p7);
                    op(p3, p6); op(p1, p4); op(p2, p5); op(p4, p7);
                    op(p4, p2); op(p6, p4); op(p4, p2);
                    dst[j] = (T)p4;
                }

                if( limit == size.width )
                    break;

                for( ; j <= size.width - VecOp::SIZE - cn; j += VecOp::SIZE )
                {
                    VT p0 = vop.load(row0+j-cn), p1 = vop.load(row0+j), p2 = vop.load(row0+j+cn);
                    VT p3 = vop.load(row1+j-cn), p4 = vop.load(row1+j), p5 = vop.load(row1+j+cn);
                    VT p6 = vop.load(row2+j-cn), p7 = vop.load(row2+j), p8 = vop.load(row2+j+cn);

                    vop(p1, p2); vop(p4, p5); vop(p7, p8); vop(p0, p1);
                    vop(p3, p4); vop(p6, p7); vop(p1, p2); vop(p4, p5);
                    vop(p7, p8); vop(p0, p3); vop(p5, p8); vop(p4, p7);
                    vop(p3, p6); vop(p1, p4); vop(p2, p5); vop(p4, p7);
                    vop(p4, p2); vop(p6, p4); vop(p4, p2);
                    vop.store(dst+j, p4);
                }

                limit = size.width;
            }
        }
    }
    else if( m == 5 )
    {
        if( size.width == 1 || size.height == 1 )
        {
            int len = size.width + size.height - 1;
            int sdelta = size.height == 1 ? cn : sstep;
            int sdelta0 = size.height == 1 ? 0 : sstep - cn;
            int ddelta = size.height == 1 ? cn : dstep;

            for( i = 0; i < len; i++, src += sdelta0, dst += ddelta )
                for( j = 0; j < cn; j++, src++ )
                {
                    int i1 = i > 0 ? -sdelta : 0;
                    int i0 = i > 1 ? -sdelta*2 : i1;
                    int i3 = i < len-1 ? sdelta : 0;
                    int i4 = i < len-2 ? sdelta*2 : i3;
                    WT p0 = src[i0], p1 = src[i1], p2 = src[0], p3 = src[i3], p4 = src[i4];

                    op(p0, p1); op(p3, p4); op(p2, p3); op(p3, p4); op(p0, p2);
                    op(p2, p4); op(p1, p3); op(p1, p2);
                    dst[j] = (T)p2;
                }
            return;
        }

        size.width *= cn;
        for( i = 0; i < size.height; i++, dst += dstep )
        {
            const T* row[5];
            row[0] = src + std::max(i - 2, 0)*sstep;
            row[1] = src + std::max(i - 1, 0)*sstep;
            row[2] = src + i*sstep;
            row[3] = src + std::min(i + 1, size.height-1)*sstep;
            row[4] = src + std::min(i + 2, size.height-1)*sstep;
            int limit = useSIMD ? cn*2 : size.width;

            for(j = 0;; )
            {
                for( ; j < limit; j++ )
                {
                    WT p[25];
                    int j1 = j >= cn ? j - cn : j;
                    int j0 = j >= cn*2 ? j - cn*2 : j1;
                    int j3 = j < size.width - cn ? j + cn : j;
                    int j4 = j < size.width - cn*2 ? j + cn*2 : j3;
                    for( k = 0; k < 5; k++ )
                    {
                        const T* rowk = row[k];
                        p[k*5] = rowk[j0]; p[k*5+1] = rowk[j1];
                        p[k*5+2] = rowk[j]; p[k*5+3] = rowk[j3];
                        p[k*5+4] = rowk[j4];
                    }

                    op(p[1], p[2]); op(p[0], p[1]); op(p[1], p[2]); op(p[4], p[5]); op(p[3], p[4]);
                    op(p[4], p[5]); op(p[0], p[3]); op(p[2], p[5]); op(p[2], p[3]); op(p[1], p[4]);
                    op(p[1], p[2]); op(p[3], p[4]); op(p[7], p[8]); op(p[6], p[7]); op(p[7], p[8]);
                    op(p[10], p[11]); op(p[9], p[10]); op(p[10], p[11]); op(p[6], p[9]); op(p[8], p[11]);
                    op(p[8], p[9]); op(p[7], p[10]); op(p[7], p[8]); op(p[9], p[10]); op(p[0], p[6]);
                    op(p[4], p[10]); op(p[4], p[6]); op(p[2], p[8]); op(p[2], p[4]); op(p[6], p[8]);
                    op(p[1], p[7]); op(p[5], p[11]); op(p[5], p[7]); op(p[3], p[9]); op(p[3], p[5]);
                    op(p[7], p[9]); op(p[1], p[2]); op(p[3], p[4]); op(p[5], p[6]); op(p[7], p[8]);
                    op(p[9], p[10]); op(p[13], p[14]); op(p[12], p[13]); op(p[13], p[14]); op(p[16], p[17]);
                    op(p[15], p[16]); op(p[16], p[17]); op(p[12], p[15]); op(p[14], p[17]); op(p[14], p[15]);
                    op(p[13], p[16]); op(p[13], p[14]); op(p[15], p[16]); op(p[19], p[20]); op(p[18], p[19]);
                    op(p[19], p[20]); op(p[21], p[22]); op(p[23], p[24]); op(p[21], p[23]); op(p[22], p[24]);
                    op(p[22], p[23]); op(p[18], p[21]); op(p[20], p[23]); op(p[20], p[21]); op(p[19], p[22]);
                    op(p[22], p[24]); op(p[19], p[20]); op(p[21], p[22]); op(p[23], p[24]); op(p[12], p[18]);
                    op(p[16], p[22]); op(p[16], p[18]); op(p[14], p[20]); op(p[20], p[24]); op(p[14], p[16]);
                    op(p[18], p[20]); op(p[22], p[24]); op(p[13], p[19]); op(p[17], p[23]); op(p[17], p[19]);
                    op(p[15], p[21]); op(p[15], p[17]); op(p[19], p[21]); op(p[13], p[14]); op(p[15], p[16]);
                    op(p[17], p[18]); op(p[19], p[20]); op(p[21], p[22]); op(p[23], p[24]); op(p[0], p[12]);
                    op(p[8], p[20]); op(p[8], p[12]); op(p[4], p[16]); op(p[16], p[24]); op(p[12], p[16]);
                    op(p[2], p[14]); op(p[10], p[22]); op(p[10], p[14]); op(p[6], p[18]); op(p[6], p[10]);
                    op(p[10], p[12]); op(p[1], p[13]); op(p[9], p[21]); op(p[9], p[13]); op(p[5], p[17]);
                    op(p[13], p[17]); op(p[3], p[15]); op(p[11], p[23]); op(p[11], p[15]); op(p[7], p[19]);
                    op(p[7], p[11]); op(p[11], p[13]); op(p[11], p[12]);
                    dst[j] = (T)p[12];
                }

                if( limit == size.width )
                    break;

                for( ; j <= size.width - VecOp::SIZE - cn*2; j += VecOp::SIZE )
                {
                    VT p[25];
                    for( k = 0; k < 5; k++ )
                    {
                        const T* rowk = row[k];
                        p[k*5] = vop.load(rowk+j-cn*2); p[k*5+1] = vop.load(rowk+j-cn);
                        p[k*5+2] = vop.load(rowk+j); p[k*5+3] = vop.load(rowk+j+cn);
                        p[k*5+4] = vop.load(rowk+j+cn*2);
                    }

                    vop(p[1], p[2]); vop(p[0], p[1]); vop(p[1], p[2]); vop(p[4], p[5]); vop(p[3], p[4]);
                    vop(p[4], p[5]); vop(p[0], p[3]); vop(p[2], p[5]); vop(p[2], p[3]); vop(p[1], p[4]);
                    vop(p[1], p[2]); vop(p[3], p[4]); vop(p[7], p[8]); vop(p[6], p[7]); vop(p[7], p[8]);
                    vop(p[10], p[11]); vop(p[9], p[10]); vop(p[10], p[11]); vop(p[6], p[9]); vop(p[8], p[11]);
                    vop(p[8], p[9]); vop(p[7], p[10]); vop(p[7], p[8]); vop(p[9], p[10]); vop(p[0], p[6]);
                    vop(p[4], p[10]); vop(p[4], p[6]); vop(p[2], p[8]); vop(p[2], p[4]); vop(p[6], p[8]);
                    vop(p[1], p[7]); vop(p[5], p[11]); vop(p[5], p[7]); vop(p[3], p[9]); vop(p[3], p[5]);
                    vop(p[7], p[9]); vop(p[1], p[2]); vop(p[3], p[4]); vop(p[5], p[6]); vop(p[7], p[8]);
                    vop(p[9], p[10]); vop(p[13], p[14]); vop(p[12], p[13]); vop(p[13], p[14]); vop(p[16], p[17]);
                    vop(p[15], p[16]); vop(p[16], p[17]); vop(p[12], p[15]); vop(p[14], p[17]); vop(p[14], p[15]);
                    vop(p[13], p[16]); vop(p[13], p[14]); vop(p[15], p[16]); vop(p[19], p[20]); vop(p[18], p[19]);
                    vop(p[19], p[20]); vop(p[21], p[22]); vop(p[23], p[24]); vop(p[21], p[23]); vop(p[22], p[24]);
                    vop(p[22], p[23]); vop(p[18], p[21]); vop(p[20], p[23]); vop(p[20], p[21]); vop(p[19], p[22]);
                    vop(p[22], p[24]); vop(p[19], p[20]); vop(p[21], p[22]); vop(p[23], p[24]); vop(p[12], p[18]);
                    vop(p[16], p[22]); vop(p[16], p[18]); vop(p[14], p[20]); vop(p[20], p[24]); vop(p[14], p[16]);
                    vop(p[18], p[20]); vop(p[22], p[24]); vop(p[13], p[19]); vop(p[17], p[23]); vop(p[17], p[19]);
                    vop(p[15], p[21]); vop(p[15], p[17]); vop(p[19], p[21]); vop(p[13], p[14]); vop(p[15], p[16]);
                    vop(p[17], p[18]); vop(p[19], p[20]); vop(p[21], p[22]); vop(p[23], p[24]); vop(p[0], p[12]);
                    vop(p[8], p[20]); vop(p[8], p[12]); vop(p[4], p[16]); vop(p[16], p[24]); vop(p[12], p[16]);
                    vop(p[2], p[14]); vop(p[10], p[22]); vop(p[10], p[14]); vop(p[6], p[18]); vop(p[6], p[10]);
                    vop(p[10], p[12]); vop(p[1], p[13]); vop(p[9], p[21]); vop(p[9], p[13]); vop(p[5], p[17]);
                    vop(p[13], p[17]); vop(p[3], p[15]); vop(p[11], p[23]); vop(p[11], p[15]); vop(p[7], p[19]);
                    vop(p[7], p[11]); vop(p[11], p[13]); vop(p[11], p[12]);
                    vop.store(dst+j, p[12]);
                }

                limit = size.width;
            }
        }
    }
}

#ifdef HAVE_OPENCL

static bool ocl_medianFilter(InputArray _src, OutputArray _dst, int m)
{
    size_t localsize[2] = { 16, 16 };
    size_t globalsize[2];
    int type = _src.type(), depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);

    if ( !((depth == CV_8U || depth == CV_16U || depth == CV_16S || depth == CV_32F) && cn <= 4 && (m == 3 || m == 5)) )
        return false;

    Size imgSize = _src.size();
    bool useOptimized = (1 == cn) &&
                        (size_t)imgSize.width >= localsize[0] * 8  &&
                        (size_t)imgSize.height >= localsize[1] * 8 &&
                        imgSize.width % 4 == 0 &&
                        imgSize.height % 4 == 0 &&
                        (ocl::Device::getDefault().isIntel());

    cv::String kname = format( useOptimized ? "medianFilter%d_u" : "medianFilter%d", m) ;
    cv::String kdefs = useOptimized ?
                         format("-D T=%s -D T1=%s -D T4=%s%d -D cn=%d -D USE_4OPT", ocl::typeToStr(type),
                         ocl::typeToStr(depth), ocl::typeToStr(depth), cn*4, cn)
                         :
                         format("-D T=%s -D T1=%s -D cn=%d", ocl::typeToStr(type), ocl::typeToStr(depth), cn) ;

    ocl::Kernel k(kname.c_str(), ocl::imgproc::medianFilter_oclsrc, kdefs.c_str() );

    if (k.empty())
        return false;

    UMat src = _src.getUMat();
    _dst.create(src.size(), type);
    UMat dst = _dst.getUMat();

    k.args(ocl::KernelArg::ReadOnlyNoSize(src), ocl::KernelArg::WriteOnly(dst));

    if( useOptimized )
    {
        globalsize[0] = DIVUP(src.cols / 4, localsize[0]) * localsize[0];
        globalsize[1] = DIVUP(src.rows / 4, localsize[1]) * localsize[1];
    }
    else
    {
        globalsize[0] = (src.cols + localsize[0] + 2) / localsize[0] * localsize[0];
        globalsize[1] = (src.rows + localsize[1] - 1) / localsize[1] * localsize[1];
    }

    return k.run(2, globalsize, localsize, false);
}

#endif

}

#ifdef HAVE_OPENVX
namespace cv
{
    namespace ovx {
        template <> inline bool skipSmallImages<VX_KERNEL_MEDIAN_3x3>(int w, int h) { return w*h < 1280 * 720; }
    }
    static bool openvx_medianFilter(InputArray _src, OutputArray _dst, int ksize)
    {
        if (_src.type() != CV_8UC1 || _dst.type() != CV_8U
#ifndef VX_VERSION_1_1
            || ksize != 3
#endif
            )
            return false;

        Mat src = _src.getMat();
        Mat dst = _dst.getMat();

        if (
#ifdef VX_VERSION_1_1
             ksize != 3 ? ovx::skipSmallImages<VX_KERNEL_NON_LINEAR_FILTER>(src.cols, src.rows) :
#endif
             ovx::skipSmallImages<VX_KERNEL_MEDIAN_3x3>(src.cols, src.rows)
           )
            return false;

        try
        {
            ivx::Context ctx = ovx::getOpenVXContext();
#ifdef VX_VERSION_1_1
            if ((vx_size)ksize > ctx.nonlinearMaxDimension())
                return false;
#endif

            Mat a;
            if (dst.data != src.data)
                a = src;
            else
                src.copyTo(a);

            ivx::Image
                ia = ivx::Image::createFromHandle(ctx, VX_DF_IMAGE_U8,
                    ivx::Image::createAddressing(a.cols, a.rows, 1, (vx_int32)(a.step)), a.data),
                ib = ivx::Image::createFromHandle(ctx, VX_DF_IMAGE_U8,
                    ivx::Image::createAddressing(dst.cols, dst.rows, 1, (vx_int32)(dst.step)), dst.data);

            //ATTENTION: VX_CONTEXT_IMMEDIATE_BORDER attribute change could lead to strange issues in multi-threaded environments
            //since OpenVX standard says nothing about thread-safety for now
            ivx::border_t prevBorder = ctx.immediateBorder();
            ctx.setImmediateBorder(VX_BORDER_REPLICATE);
#ifdef VX_VERSION_1_1
            if (ksize == 3)
#endif
            {
                ivx::IVX_CHECK_STATUS(vxuMedian3x3(ctx, ia, ib));
            }
#ifdef VX_VERSION_1_1
            else
            {
                ivx::Matrix mtx;
                if(ksize == 5)
                    mtx = ivx::Matrix::createFromPattern(ctx, VX_PATTERN_BOX, ksize, ksize);
                else
                {
                    vx_size supportedSize;
                    ivx::IVX_CHECK_STATUS(vxQueryContext(ctx, VX_CONTEXT_NONLINEAR_MAX_DIMENSION, &supportedSize, sizeof(supportedSize)));
                    if ((vx_size)ksize > supportedSize)
                    {
                        ctx.setImmediateBorder(prevBorder);
                        return false;
                    }
                    Mat mask(ksize, ksize, CV_8UC1, Scalar(255));
                    mtx = ivx::Matrix::create(ctx, VX_TYPE_UINT8, ksize, ksize);
                    mtx.copyFrom(mask);
                }
                ivx::IVX_CHECK_STATUS(vxuNonLinearFilter(ctx, VX_NONLINEAR_FILTER_MEDIAN, ia, mtx, ib));
            }
#endif
            ctx.setImmediateBorder(prevBorder);
        }
        catch (ivx::RuntimeError & e)
        {
            VX_DbgThrow(e.what());
        }
        catch (ivx::WrapperError & e)
        {
            VX_DbgThrow(e.what());
        }

        return true;
    }
}
#endif

#ifdef HAVE_IPP
namespace cv
{
static bool ipp_medianFilter(Mat &src0, Mat &dst, int ksize)
{
    CV_INSTRUMENT_REGION_IPP()

#if IPP_VERSION_X100 < 201801
    // Degradations for big kernel
    if(ksize > 7)
        return false;
#endif

    {
        int         bufSize;
        IppiSize    dstRoiSize = ippiSize(dst.cols, dst.rows), maskSize = ippiSize(ksize, ksize);
        IppDataType ippType = ippiGetDataType(src0.type());
        int         channels = src0.channels();
        IppAutoBuffer<Ipp8u> buffer;

        if(src0.isSubmatrix())
            return false;

        Mat src;
        if(dst.data != src0.data)
            src = src0;
        else
            src0.copyTo(src);

        if(ippiFilterMedianBorderGetBufferSize(dstRoiSize, maskSize, ippType, channels, &bufSize) < 0)
            return false;

        buffer.allocate(bufSize);

        switch(ippType)
        {
        case ipp8u:
            if(channels == 1)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_8u_C1R, src.ptr<Ipp8u>(), (int)src.step, dst.ptr<Ipp8u>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else if(channels == 3)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_8u_C3R, src.ptr<Ipp8u>(), (int)src.step, dst.ptr<Ipp8u>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else if(channels == 4)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_8u_C4R, src.ptr<Ipp8u>(), (int)src.step, dst.ptr<Ipp8u>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else
                return false;
        case ipp16u:
            if(channels == 1)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_16u_C1R, src.ptr<Ipp16u>(), (int)src.step, dst.ptr<Ipp16u>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else if(channels == 3)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_16u_C3R, src.ptr<Ipp16u>(), (int)src.step, dst.ptr<Ipp16u>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else if(channels == 4)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_16u_C4R, src.ptr<Ipp16u>(), (int)src.step, dst.ptr<Ipp16u>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else
                return false;
        case ipp16s:
            if(channels == 1)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_16s_C1R, src.ptr<Ipp16s>(), (int)src.step, dst.ptr<Ipp16s>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else if(channels == 3)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_16s_C3R, src.ptr<Ipp16s>(), (int)src.step, dst.ptr<Ipp16s>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else if(channels == 4)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_16s_C4R, src.ptr<Ipp16s>(), (int)src.step, dst.ptr<Ipp16s>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else
                return false;
        case ipp32f:
            if(channels == 1)
                return CV_INSTRUMENT_FUN_IPP(ippiFilterMedianBorder_32f_C1R, src.ptr<Ipp32f>(), (int)src.step, dst.ptr<Ipp32f>(), (int)dst.step, dstRoiSize, maskSize, ippBorderRepl, 0, buffer) >= 0;
            else
                return false;
        default:
            return false;
        }
    }
}
}
#endif

void cv::medianBlur( InputArray _src0, OutputArray _dst, int ksize )
{
    CV_INSTRUMENT_REGION()

    CV_Assert( (ksize % 2 == 1) && (_src0.dims() <= 2 ));

    if( ksize <= 1 || _src0.empty() )
    {
        _src0.copyTo(_dst);
        return;
    }

    CV_OCL_RUN(_dst.isUMat(),
               ocl_medianFilter(_src0,_dst, ksize))

    Mat src0 = _src0.getMat();
    _dst.create( src0.size(), src0.type() );
    Mat dst = _dst.getMat();

    CALL_HAL(medianBlur, cv_hal_medianBlur, src0.data, src0.step, dst.data, dst.step, src0.cols, src0.rows, src0.depth(),
             src0.channels(), ksize);

    CV_OVX_RUN(true,
               openvx_medianFilter(_src0, _dst, ksize))

    CV_IPP_RUN_FAST(ipp_medianFilter(src0, dst, ksize));

#ifdef HAVE_TEGRA_OPTIMIZATION
    if (tegra::useTegra() && tegra::medianBlur(src0, dst, ksize))
        return;
#endif

    bool useSortNet = ksize == 3 || (ksize == 5
#if !(CV_SIMD128)
            && ( src0.depth() > CV_8U || src0.channels() == 2 || src0.channels() > 4 )
#endif
        );

    Mat src;
    if( useSortNet )
    {
        if( dst.data != src0.data )
            src = src0;
        else
            src0.copyTo(src);

        if( src.depth() == CV_8U )
            medianBlur_SortNet<MinMax8u, MinMaxVec8u>( src, dst, ksize );
        else if( src.depth() == CV_16U )
            medianBlur_SortNet<MinMax16u, MinMaxVec16u>( src, dst, ksize );
        else if( src.depth() == CV_16S )
            medianBlur_SortNet<MinMax16s, MinMaxVec16s>( src, dst, ksize );
        else if( src.depth() == CV_32F )
            medianBlur_SortNet<MinMax32f, MinMaxVec32f>( src, dst, ksize );
        else
            CV_Error(CV_StsUnsupportedFormat, "");

        return;
    }
    else
    {
        cv::copyMakeBorder( src0, src, 0, 0, ksize/2, ksize/2, BORDER_REPLICATE|BORDER_ISOLATED);

        int cn = src0.channels();
        CV_Assert( src.depth() == CV_8U && (cn == 1 || cn == 3 || cn == 4) );

        double img_size_mp = (double)(src0.total())/(1 << 20);
        if( ksize <= 3 + (img_size_mp < 1 ? 12 : img_size_mp < 4 ? 6 : 2)*
            (CV_SIMD128 && hasSIMD128() ? 1 : 3))
            medianBlur_8u_Om( src, dst, ksize );
        else
            medianBlur_8u_O1( src, dst, ksize );
    }
}

/****************************************************************************************\
                                   Bilateral Filtering
\****************************************************************************************/

namespace cv
{

class BilateralFilter_8u_Invoker :
    public ParallelLoopBody
{
public:
    BilateralFilter_8u_Invoker(Mat& _dest, const Mat& _temp, int _radius, int _maxk,
        int* _space_ofs, float *_space_weight, float *_color_weight) :
        temp(&_temp), dest(&_dest), radius(_radius),
        maxk(_maxk), space_ofs(_space_ofs), space_weight(_space_weight), color_weight(_color_weight)
    {
    }

    virtual void operator() (const Range& range) const CV_OVERRIDE
    {
        int i, j, cn = dest->channels(), k;
        Size size = dest->size();
#if CV_SIMD128
        int CV_DECL_ALIGNED(16) buf[4];
        bool haveSIMD128 = hasSIMD128();
#endif

        for( i = range.start; i < range.end; i++ )
        {
            const uchar* sptr = temp->ptr(i+radius) + radius*cn;
            uchar* dptr = dest->ptr(i);

            if( cn == 1 )
            {
                for( j = 0; j < size.width; j++ )
                {
                    float sum = 0, wsum = 0;
                    int val0 = sptr[j];
                    k = 0;
#if CV_SIMD128
                    if( haveSIMD128 )
                    {
                        v_float32x4 _val0 = v_setall_f32(static_cast<float>(val0));
                        v_float32x4 vsumw = v_setzero_f32();
                        v_float32x4 vsumc = v_setzero_f32();

                        for( ; k <= maxk - 4; k += 4 )
                        {
                            v_float32x4 _valF = v_float32x4(sptr[j + space_ofs[k]],
                                sptr[j + space_ofs[k + 1]],
                                sptr[j + space_ofs[k + 2]],
                                sptr[j + space_ofs[k + 3]]);
                            v_float32x4 _val = v_abs(_valF - _val0);
                            v_store(buf, v_round(_val));

                            v_float32x4 _cw = v_float32x4(color_weight[buf[0]],
                                color_weight[buf[1]],
                                color_weight[buf[2]],
                                color_weight[buf[3]]);
                            v_float32x4 _sw = v_load(space_weight+k);
#if defined(_MSC_VER) && _MSC_VER == 1700/* MSVS 2012 */ && CV_AVX
                            // details: https://github.com/opencv/opencv/issues/11004
                            vsumw += _cw * _sw;
                            vsumc += _cw * _sw * _valF;
#else
                            v_float32x4 _w = _cw * _sw;
                            _cw = _w * _valF;

                            vsumw += _w;
                            vsumc += _cw;
#endif
                        }
                        float *bufFloat = (float*)buf;
                        v_float32x4 sum4 = v_reduce_sum4(vsumw, vsumc, vsumw, vsumc);
                        v_store(bufFloat, sum4);
                        sum += bufFloat[1];
                        wsum += bufFloat[0];
                    }
#endif
                    for( ; k < maxk; k++ )
                    {
                        int val = sptr[j + space_ofs[k]];
                        float w = space_weight[k]*color_weight[std::abs(val - val0)];
                        sum += val*w;
                        wsum += w;
                    }
                    // overflow is not possible here => there is no need to use cv::saturate_cast
                    CV_DbgAssert(fabs(wsum) > 0);
                    dptr[j] = (uchar)cvRound(sum/wsum);
                }
            }
            else
            {
                assert( cn == 3 );
                for( j = 0; j < size.width*3; j += 3 )
                {
                    float sum_b = 0, sum_g = 0, sum_r = 0, wsum = 0;
                    int b0 = sptr[j], g0 = sptr[j+1], r0 = sptr[j+2];
                    k = 0;
#if CV_SIMD128
                    if( haveSIMD128 )
                    {
                        v_float32x4 vsumw = v_setzero_f32();
                        v_float32x4 vsumb = v_setzero_f32();
                        v_float32x4 vsumg = v_setzero_f32();
                        v_float32x4 vsumr = v_setzero_f32();
                        const v_float32x4 _b0 = v_setall_f32(static_cast<float>(b0));
                        const v_float32x4 _g0 = v_setall_f32(static_cast<float>(g0));
                        const v_float32x4 _r0 = v_setall_f32(static_cast<float>(r0));

                        for( ; k <= maxk - 4; k += 4 )
                        {
                            const uchar* const sptr_k0  = sptr + j + space_ofs[k];
                            const uchar* const sptr_k1  = sptr + j + space_ofs[k+1];
                            const uchar* const sptr_k2  = sptr + j + space_ofs[k+2];
                            const uchar* const sptr_k3  = sptr + j + space_ofs[k+3];

                            v_float32x4 __b = v_cvt_f32(v_reinterpret_as_s32(v_load_expand_q(sptr_k0)));
                            v_float32x4 __g = v_cvt_f32(v_reinterpret_as_s32(v_load_expand_q(sptr_k1)));
                            v_float32x4 __r = v_cvt_f32(v_reinterpret_as_s32(v_load_expand_q(sptr_k2)));
                            v_float32x4 __z = v_cvt_f32(v_reinterpret_as_s32(v_load_expand_q(sptr_k3)));
                            v_float32x4 _b, _g, _r, _z;

                            v_transpose4x4(__b, __g, __r, __z, _b, _g, _r, _z);

                            v_float32x4 bt = v_abs(_b -_b0);
                            v_float32x4 gt = v_abs(_g -_g0);
                            v_float32x4 rt = v_abs(_r -_r0);

                            bt = rt + bt + gt;
                            v_store(buf, v_round(bt));

                            v_float32x4 _w  = v_float32x4(color_weight[buf[0]],color_weight[buf[1]],
                                                    color_weight[buf[2]],color_weight[buf[3]]);
                            v_float32x4 _sw = v_load(space_weight+k);

#if defined(_MSC_VER) && _MSC_VER == 1700/* MSVS 2012 */ && CV_AVX
                            // details: https://github.com/opencv/opencv/issues/11004
                            vsumw += _w * _sw;
                            vsumb += _w * _sw * _b;
                            vsumg += _w * _sw * _g;
                            vsumr += _w * _sw * _r;
#else
                            _w *= _sw;
                            _b *=  _w;
                            _g *=  _w;
                            _r *=  _w;

                            vsumw += _w;
                            vsumb += _b;
                            vsumg += _g;
                            vsumr += _r;
#endif
                        }
                        float *bufFloat = (float*)buf;
                        v_float32x4 sum4 = v_reduce_sum4(vsumw, vsumb, vsumg, vsumr);
                        v_store(bufFloat, sum4);
                        wsum += bufFloat[0];
                        sum_b += bufFloat[1];
                        sum_g += bufFloat[2];
                        sum_r += bufFloat[3];
                    }
#endif

                    for( ; k < maxk; k++ )
                    {
                        const uchar* sptr_k = sptr + j + space_ofs[k];
                        int b = sptr_k[0], g = sptr_k[1], r = sptr_k[2];
                        float w = space_weight[k]*color_weight[std::abs(b - b0) +
                                                               std::abs(g - g0) + std::abs(r - r0)];
                        sum_b += b*w; sum_g += g*w; sum_r += r*w;
                        wsum += w;
                    }
                    CV_DbgAssert(fabs(wsum) > 0);
                    wsum = 1.f/wsum;
                    b0 = cvRound(sum_b*wsum);
                    g0 = cvRound(sum_g*wsum);
                    r0 = cvRound(sum_r*wsum);
                    dptr[j] = (uchar)b0; dptr[j+1] = (uchar)g0; dptr[j+2] = (uchar)r0;
                }
            }
        }
    }

private:
    const Mat *temp;
    Mat *dest;
    int radius, maxk, *space_ofs;
    float *space_weight, *color_weight;
};

#ifdef HAVE_OPENCL

static bool ocl_bilateralFilter_8u(InputArray _src, OutputArray _dst, int d,
                                   double sigma_color, double sigma_space,
                                   int borderType)
{
#ifdef __ANDROID__
    if (ocl::Device::getDefault().isNVidia())
        return false;
#endif

    int type = _src.type(), depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);
    int i, j, maxk, radius;

    if (depth != CV_8U || cn > 4)
        return false;

    if (sigma_color <= 0)
        sigma_color = 1;
    if (sigma_space <= 0)
        sigma_space = 1;

    double gauss_color_coeff = -0.5 / (sigma_color * sigma_color);
    double gauss_space_coeff = -0.5 / (sigma_space * sigma_space);

    if ( d <= 0 )
        radius = cvRound(sigma_space * 1.5);
    else
        radius = d / 2;
    radius = MAX(radius, 1);
    d = radius * 2 + 1;

    UMat src = _src.getUMat(), dst = _dst.getUMat(), temp;
    if (src.u == dst.u)
        return false;

    copyMakeBorder(src, temp, radius, radius, radius, radius, borderType);
    std::vector<float> _space_weight(d * d);
    std::vector<int> _space_ofs(d * d);
    float * const space_weight = &_space_weight[0];
    int * const space_ofs = &_space_ofs[0];

    // initialize space-related bilateral filter coefficients
    for( i = -radius, maxk = 0; i <= radius; i++ )
        for( j = -radius; j <= radius; j++ )
        {
            double r = std::sqrt((double)i * i + (double)j * j);
            if ( r > radius )
                continue;
            space_weight[maxk] = (float)std::exp(r * r * gauss_space_coeff);
            space_ofs[maxk++] = (int)(i * temp.step + j * cn);
        }

    char cvt[3][40];
    String cnstr = cn > 1 ? format("%d", cn) : "";
    String kernelName("bilateral");
    size_t sizeDiv = 1;
    if ((ocl::Device::getDefault().isIntel()) &&
        (ocl::Device::getDefault().type() == ocl::Device::TYPE_GPU))
    {
            //Intel GPU
            if (dst.cols % 4 == 0 && cn == 1) // For single channel x4 sized images.
            {
                kernelName = "bilateral_float4";
                sizeDiv = 4;
            }
     }
     ocl::Kernel k(kernelName.c_str(), ocl::imgproc::bilateral_oclsrc,
            format("-D radius=%d -D maxk=%d -D cn=%d -D int_t=%s -D uint_t=uint%s -D convert_int_t=%s"
            " -D uchar_t=%s -D float_t=%s -D convert_float_t=%s -D convert_uchar_t=%s -D gauss_color_coeff=(float)%f",
            radius, maxk, cn, ocl::typeToStr(CV_32SC(cn)), cnstr.c_str(),
            ocl::convertTypeStr(CV_8U, CV_32S, cn, cvt[0]),
            ocl::typeToStr(type), ocl::typeToStr(CV_32FC(cn)),
            ocl::convertTypeStr(CV_32S, CV_32F, cn, cvt[1]),
            ocl::convertTypeStr(CV_32F, CV_8U, cn, cvt[2]), gauss_color_coeff));
    if (k.empty())
        return false;

    Mat mspace_weight(1, d * d, CV_32FC1, space_weight);
    Mat mspace_ofs(1, d * d, CV_32SC1, space_ofs);
    UMat ucolor_weight, uspace_weight, uspace_ofs;

    mspace_weight.copyTo(uspace_weight);
    mspace_ofs.copyTo(uspace_ofs);

    k.args(ocl::KernelArg::ReadOnlyNoSize(temp), ocl::KernelArg::WriteOnly(dst),
           ocl::KernelArg::PtrReadOnly(uspace_weight),
           ocl::KernelArg::PtrReadOnly(uspace_ofs));

    size_t globalsize[2] = { (size_t)dst.cols / sizeDiv, (size_t)dst.rows };
    return k.run(2, globalsize, NULL, false);
}

#endif
static void
bilateralFilter_8u( const Mat& src, Mat& dst, int d,
    double sigma_color, double sigma_space,
    int borderType )
{
    int cn = src.channels();
    int i, j, maxk, radius;
    Size size = src.size();

    CV_Assert( (src.type() == CV_8UC1 || src.type() == CV_8UC3) && src.data != dst.data );

    if( sigma_color <= 0 )
        sigma_color = 1;
    if( sigma_space <= 0 )
        sigma_space = 1;

    double gauss_color_coeff = -0.5/(sigma_color*sigma_color);
    double gauss_space_coeff = -0.5/(sigma_space*sigma_space);

    if( d <= 0 )
        radius = cvRound(sigma_space*1.5);
    else
        radius = d/2;
    radius = MAX(radius, 1);
    d = radius*2 + 1;

    Mat temp;
    copyMakeBorder( src, temp, radius, radius, radius, radius, borderType );

    std::vector<float> _color_weight(cn*256);
    std::vector<float> _space_weight(d*d);
    std::vector<int> _space_ofs(d*d);
    float* color_weight = &_color_weight[0];
    float* space_weight = &_space_weight[0];
    int* space_ofs = &_space_ofs[0];

    // initialize color-related bilateral filter coefficients

    for( i = 0; i < 256*cn; i++ )
        color_weight[i] = (float)std::exp(i*i*gauss_color_coeff);

    // initialize space-related bilateral filter coefficients
    for( i = -radius, maxk = 0; i <= radius; i++ )
    {
        j = -radius;

        for( ; j <= radius; j++ )
        {
            double r = std::sqrt((double)i*i + (double)j*j);
            if( r > radius )
                continue;
            space_weight[maxk] = (float)std::exp(r*r*gauss_space_coeff);
            space_ofs[maxk++] = (int)(i*temp.step + j*cn);
        }
    }

    BilateralFilter_8u_Invoker body(dst, temp, radius, maxk, space_ofs, space_weight, color_weight);
    parallel_for_(Range(0, size.height), body, dst.total()/(double)(1<<16));
}


class BilateralFilter_32f_Invoker :
    public ParallelLoopBody
{
public:

    BilateralFilter_32f_Invoker(int _cn, int _radius, int _maxk, int *_space_ofs,
        const Mat& _temp, Mat& _dest, float _scale_index, float *_space_weight, float *_expLUT) :
        cn(_cn), radius(_radius), maxk(_maxk), space_ofs(_space_ofs),
        temp(&_temp), dest(&_dest), scale_index(_scale_index), space_weight(_space_weight), expLUT(_expLUT)
    {
    }

    virtual void operator() (const Range& range) const CV_OVERRIDE
    {
        int i, j, k;
        Size size = dest->size();
#if CV_SIMD128
        int CV_DECL_ALIGNED(16) idxBuf[4];
        bool haveSIMD128 = hasSIMD128();
#endif

        for( i = range.start; i < range.end; i++ )
        {
            const float* sptr = temp->ptr<float>(i+radius) + radius*cn;
            float* dptr = dest->ptr<float>(i);

            if( cn == 1 )
            {
                for( j = 0; j < size.width; j++ )
                {
                    float sum = 0, wsum = 0;
                    float val0 = sptr[j];
                    k = 0;
#if CV_SIMD128
                    if( haveSIMD128 )
                    {
                        v_float32x4 vecwsum = v_setzero_f32();
                        v_float32x4 vecvsum = v_setzero_f32();
                        const v_float32x4 _val0 = v_setall_f32(sptr[j]);
                        const v_float32x4 _scale_index = v_setall_f32(scale_index);

                        for (; k <= maxk - 4; k += 4)
                        {
                            v_float32x4 _sw = v_load(space_weight + k);
                            v_float32x4 _val = v_float32x4(sptr[j + space_ofs[k]],
                                sptr[j + space_ofs[k + 1]],
                                sptr[j + space_ofs[k + 2]],
                                sptr[j + space_ofs[k + 3]]);
                            v_float32x4 _alpha = v_abs(_val - _val0) * _scale_index;

                            v_int32x4 _idx = v_round(_alpha);
                            v_store(idxBuf, _idx);
                            _alpha -= v_cvt_f32(_idx);

                            v_float32x4 _explut = v_float32x4(expLUT[idxBuf[0]],
                                expLUT[idxBuf[1]],
                                expLUT[idxBuf[2]],
                                expLUT[idxBuf[3]]);
                            v_float32x4 _explut1 = v_float32x4(expLUT[idxBuf[0] + 1],
                                expLUT[idxBuf[1] + 1],
                                expLUT[idxBuf[2] + 1],
                                expLUT[idxBuf[3] + 1]);

                            v_float32x4 _w = _sw * (_explut + (_alpha * (_explut1 - _explut)));
                            _val *= _w;

                            vecwsum += _w;
                            vecvsum += _val;
                        }
                        float *bufFloat = (float*)idxBuf;
                        v_float32x4 sum4 = v_reduce_sum4(vecwsum, vecvsum, vecwsum, vecvsum);
                        v_store(bufFloat, sum4);
                        sum += bufFloat[1];
                        wsum += bufFloat[0];
                    }
#endif

                    for( ; k < maxk; k++ )
                    {
                        float val = sptr[j + space_ofs[k]];
                        float alpha = (float)(std::abs(val - val0)*scale_index);
                        int idx = cvFloor(alpha);
                        alpha -= idx;
                        float w = space_weight[k]*(expLUT[idx] + alpha*(expLUT[idx+1] - expLUT[idx]));
                        sum += val*w;
                        wsum += w;
                    }
                    CV_DbgAssert(fabs(wsum) > 0);
                    dptr[j] = (float)(sum/wsum);
                }
            }
            else
            {
                CV_Assert( cn == 3 );
                for( j = 0; j < size.width*3; j += 3 )
                {
                    float sum_b = 0, sum_g = 0, sum_r = 0, wsum = 0;
                    float b0 = sptr[j], g0 = sptr[j+1], r0 = sptr[j+2];
                    k = 0;
#if CV_SIMD128
                    if( haveSIMD128 )
                    {
                        v_float32x4 sumw = v_setzero_f32();
                        v_float32x4 sumb = v_setzero_f32();
                        v_float32x4 sumg = v_setzero_f32();
                        v_float32x4 sumr = v_setzero_f32();
                        const v_float32x4 _b0 = v_setall_f32(b0);
                        const v_float32x4 _g0 = v_setall_f32(g0);
                        const v_float32x4 _r0 = v_setall_f32(r0);
                        const v_float32x4 _scale_index = v_setall_f32(scale_index);

                        for( ; k <= maxk-4; k += 4 )
                        {
                            v_float32x4 _sw = v_load(space_weight + k);

                            const float* const sptr_k0 = sptr + j + space_ofs[k];
                            const float* const sptr_k1 = sptr + j + space_ofs[k+1];
                            const float* const sptr_k2 = sptr + j + space_ofs[k+2];
                            const float* const sptr_k3 = sptr + j + space_ofs[k+3];

                            v_float32x4 _v0 = v_load(sptr_k0);
                            v_float32x4 _v1 = v_load(sptr_k1);
                            v_float32x4 _v2 = v_load(sptr_k2);
                            v_float32x4 _v3 = v_load(sptr_k3);
                            v_float32x4 _b, _g, _r, _dummy;

                            v_transpose4x4(_v0, _v1, _v2, _v3, _b, _g, _r, _dummy);

                            v_float32x4 _bt = v_abs(_b - _b0);
                            v_float32x4 _gt = v_abs(_g - _g0);
                            v_float32x4 _rt = v_abs(_r - _r0);
                            v_float32x4 _alpha = _scale_index * (_bt + _gt + _rt);

                            v_int32x4 _idx = v_round(_alpha);
                            v_store((int*)idxBuf, _idx);
                            _alpha -= v_cvt_f32(_idx);

                            v_float32x4 _explut = v_float32x4(expLUT[idxBuf[0]],
                                expLUT[idxBuf[1]],
                                expLUT[idxBuf[2]],
                                expLUT[idxBuf[3]]);
                            v_float32x4 _explut1 = v_float32x4(expLUT[idxBuf[0] + 1],
                                expLUT[idxBuf[1] + 1],
                                expLUT[idxBuf[2] + 1],
                                expLUT[idxBuf[3] + 1]);

                            v_float32x4 _w = _sw * (_explut + (_alpha * (_explut1 - _explut)));

                            _b *=  _w;
                            _g *=  _w;
                            _r *=  _w;
                            sumw += _w;
                            sumb += _b;
                            sumg += _g;
                            sumr += _r;
                        }
                        v_float32x4 sum4 = v_reduce_sum4(sumw, sumb, sumg, sumr);
                        float *bufFloat = (float*)idxBuf;
                        v_store(bufFloat, sum4);
                        wsum += bufFloat[0];
                        sum_b += bufFloat[1];
                        sum_g += bufFloat[2];
                        sum_r += bufFloat[3];
                    }
#endif

                    for(; k < maxk; k++ )
                    {
                        const float* sptr_k = sptr + j + space_ofs[k];
                        float b = sptr_k[0], g = sptr_k[1], r = sptr_k[2];
                        float alpha = (float)((std::abs(b - b0) +
                            std::abs(g - g0) + std::abs(r - r0))*scale_index);
                        int idx = cvFloor(alpha);
                        alpha -= idx;
                        float w = space_weight[k]*(expLUT[idx] + alpha*(expLUT[idx+1] - expLUT[idx]));
                        sum_b += b*w; sum_g += g*w; sum_r += r*w;
                        wsum += w;
                    }
                    CV_DbgAssert(fabs(wsum) > 0);
                    wsum = 1.f/wsum;
                    b0 = sum_b*wsum;
                    g0 = sum_g*wsum;
                    r0 = sum_r*wsum;
                    dptr[j] = b0; dptr[j+1] = g0; dptr[j+2] = r0;
                }
            }
        }
    }

private:
    int cn, radius, maxk, *space_ofs;
    const Mat* temp;
    Mat *dest;
    float scale_index, *space_weight, *expLUT;
};


static void
bilateralFilter_32f( const Mat& src, Mat& dst, int d,
                     double sigma_color, double sigma_space,
                     int borderType )
{
    int cn = src.channels();
    int i, j, maxk, radius;
    double minValSrc=-1, maxValSrc=1;
    const int kExpNumBinsPerChannel = 1 << 12;
    int kExpNumBins = 0;
    float lastExpVal = 1.f;
    float len, scale_index;
    Size size = src.size();

    CV_Assert( (src.type() == CV_32FC1 || src.type() == CV_32FC3) && src.data != dst.data );

    if( sigma_color <= 0 )
        sigma_color = 1;
    if( sigma_space <= 0 )
        sigma_space = 1;

    double gauss_color_coeff = -0.5/(sigma_color*sigma_color);
    double gauss_space_coeff = -0.5/(sigma_space*sigma_space);

    if( d <= 0 )
        radius = cvRound(sigma_space*1.5);
    else
        radius = d/2;
    radius = MAX(radius, 1);
    d = radius*2 + 1;
    // compute the min/max range for the input image (even if multichannel)

    minMaxLoc( src.reshape(1), &minValSrc, &maxValSrc );
    if(std::abs(minValSrc - maxValSrc) < FLT_EPSILON)
    {
        src.copyTo(dst);
        return;
    }

    // temporary copy of the image with borders for easy processing
    Mat temp;
    copyMakeBorder( src, temp, radius, radius, radius, radius, borderType );
    const double insteadNaNValue = -5. * sigma_color;
    patchNaNs( temp, insteadNaNValue ); // this replacement of NaNs makes the assumption that depth values are nonnegative
                                        // TODO: make insteadNaNValue avalible in the outside function interface to control the cases breaking the assumption
    // allocate lookup tables
    std::vector<float> _space_weight(d*d);
    std::vector<int> _space_ofs(d*d);
    float* space_weight = &_space_weight[0];
    int* space_ofs = &_space_ofs[0];

    // assign a length which is slightly more than needed
    len = (float)(maxValSrc - minValSrc) * cn;
    kExpNumBins = kExpNumBinsPerChannel * cn;
    std::vector<float> _expLUT(kExpNumBins+2);
    float* expLUT = &_expLUT[0];

    scale_index = kExpNumBins/len;

    // initialize the exp LUT
    for( i = 0; i < kExpNumBins+2; i++ )
    {
        if( lastExpVal > 0.f )
        {
            double val =  i / scale_index;
            expLUT[i] = (float)std::exp(val * val * gauss_color_coeff);
            lastExpVal = expLUT[i];
        }
        else
            expLUT[i] = 0.f;
    }

    // initialize space-related bilateral filter coefficients
    for( i = -radius, maxk = 0; i <= radius; i++ )
        for( j = -radius; j <= radius; j++ )
        {
            double r = std::sqrt((double)i*i + (double)j*j);
            if( r > radius )
                continue;
            space_weight[maxk] = (float)std::exp(r*r*gauss_space_coeff);
            space_ofs[maxk++] = (int)(i*(temp.step/sizeof(float)) + j*cn);
        }

    // parallel_for usage

    BilateralFilter_32f_Invoker body(cn, radius, maxk, space_ofs, temp, dst, scale_index, space_weight, expLUT);
    parallel_for_(Range(0, size.height), body, dst.total()/(double)(1<<16));
}

#ifdef HAVE_IPP
#define IPP_BILATERAL_PARALLEL 1

#ifdef HAVE_IPP_IW
class ipp_bilateralFilterParallel: public ParallelLoopBody
{
public:
    ipp_bilateralFilterParallel(::ipp::IwiImage &_src, ::ipp::IwiImage &_dst, int _radius, Ipp32f _valSquareSigma, Ipp32f _posSquareSigma, ::ipp::IwiBorderType _borderType, bool *_ok):
        src(_src), dst(_dst)
    {
        pOk = _ok;

        radius          = _radius;
        valSquareSigma  = _valSquareSigma;
        posSquareSigma  = _posSquareSigma;
        borderType      = _borderType;

        *pOk = true;
    }
    ~ipp_bilateralFilterParallel() {}

    virtual void operator() (const Range& range) const CV_OVERRIDE
    {
        if(*pOk == false)
            return;

        try
        {
            ::ipp::IwiTile tile = ::ipp::IwiRoi(0, range.start, dst.m_size.width, range.end - range.start);
            CV_INSTRUMENT_FUN_IPP(::ipp::iwiFilterBilateral, src, dst, radius, valSquareSigma, posSquareSigma, ::ipp::IwDefault(), borderType, tile);
        }
        catch(::ipp::IwException)
        {
            *pOk = false;
            return;
        }
    }
private:
    ::ipp::IwiImage &src;
    ::ipp::IwiImage &dst;

    int                  radius;
    Ipp32f               valSquareSigma;
    Ipp32f               posSquareSigma;
    ::ipp::IwiBorderType borderType;

    bool  *pOk;
    const ipp_bilateralFilterParallel& operator= (const ipp_bilateralFilterParallel&);
};
#endif

static bool ipp_bilateralFilter(Mat &src, Mat &dst, int d, double sigmaColor, double sigmaSpace, int borderType)
{
#ifdef HAVE_IPP_IW
    CV_INSTRUMENT_REGION_IPP()

    int         radius         = IPP_MAX(((d <= 0)?cvRound(sigmaSpace*1.5):d/2), 1);
    Ipp32f      valSquareSigma = (Ipp32f)((sigmaColor <= 0)?1:sigmaColor*sigmaColor);
    Ipp32f      posSquareSigma = (Ipp32f)((sigmaSpace <= 0)?1:sigmaSpace*sigmaSpace);

    // Acquire data and begin processing
    try
    {
        ::ipp::IwiImage      iwSrc = ippiGetImage(src);
        ::ipp::IwiImage      iwDst = ippiGetImage(dst);
        ::ipp::IwiBorderSize borderSize(radius);
        ::ipp::IwiBorderType ippBorder(ippiGetBorder(iwSrc, borderType, borderSize));
        if(!ippBorder)
            return false;

        const int threads = ippiSuggestThreadsNum(iwDst, 2);
        if(IPP_BILATERAL_PARALLEL && threads > 1) {
            bool  ok      = true;
            Range range(0, (int)iwDst.m_size.height);
            ipp_bilateralFilterParallel invoker(iwSrc, iwDst, radius, valSquareSigma, posSquareSigma, ippBorder, &ok);
            if(!ok)
                return false;

            parallel_for_(range, invoker, threads*4);

            if(!ok)
                return false;
        } else {
            CV_INSTRUMENT_FUN_IPP(::ipp::iwiFilterBilateral, iwSrc, iwDst, radius, valSquareSigma, posSquareSigma, ::ipp::IwDefault(), ippBorder);
        }
    }
    catch (::ipp::IwException)
    {
        return false;
    }
    return true;
#else
    CV_UNUSED(src); CV_UNUSED(dst); CV_UNUSED(d); CV_UNUSED(sigmaColor); CV_UNUSED(sigmaSpace); CV_UNUSED(borderType);
    return false;
#endif
}
#endif

}

void cv::bilateralFilter( InputArray _src, OutputArray _dst, int d,
                      double sigmaColor, double sigmaSpace,
                      int borderType )
{
    CV_INSTRUMENT_REGION()

    _dst.create( _src.size(), _src.type() );

    CV_OCL_RUN(_src.dims() <= 2 && _dst.isUMat(),
               ocl_bilateralFilter_8u(_src, _dst, d, sigmaColor, sigmaSpace, borderType))

    Mat src = _src.getMat(), dst = _dst.getMat();

    CV_IPP_RUN_FAST(ipp_bilateralFilter(src, dst, d, sigmaColor, sigmaSpace, borderType));

    if( src.depth() == CV_8U )
        bilateralFilter_8u( src, dst, d, sigmaColor, sigmaSpace, borderType );
    else if( src.depth() == CV_32F )
        bilateralFilter_32f( src, dst, d, sigmaColor, sigmaSpace, borderType );
    else
        CV_Error( CV_StsUnsupportedFormat,
        "Bilateral filtering is only implemented for 8u and 32f images" );
}

//////////////////////////////////////////////////////////////////////////////////////////

CV_IMPL void
cvSmooth( const void* srcarr, void* dstarr, int smooth_type,
          int param1, int param2, double param3, double param4 )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst0 = cv::cvarrToMat(dstarr), dst = dst0;

    CV_Assert( dst.size() == src.size() &&
        (smooth_type == CV_BLUR_NO_SCALE || dst.type() == src.type()) );

    if( param2 <= 0 )
        param2 = param1;

    if( smooth_type == CV_BLUR || smooth_type == CV_BLUR_NO_SCALE )
        cv::boxFilter( src, dst, dst.depth(), cv::Size(param1, param2), cv::Point(-1,-1),
            smooth_type == CV_BLUR, cv::BORDER_REPLICATE );
    else if( smooth_type == CV_GAUSSIAN )
        cv::GaussianBlur( src, dst, cv::Size(param1, param2), param3, param4, cv::BORDER_REPLICATE );
    else if( smooth_type == CV_MEDIAN )
        cv::medianBlur( src, dst, param1 );
    else
        cv::bilateralFilter( src, dst, param1, param3, param4, cv::BORDER_REPLICATE );

    if( dst.data != dst0.data )
        CV_Error( CV_StsUnmatchedFormats, "The destination image does not have the proper type" );
}

/* End of file. */


void myBoxFilter(cv::InputArray _src, cv::OutputArray _dst, int ddepth,
	cv::Size ksize, cv::Point anchor, bool normalize, int borderType) {
	cv::boxFilter(_src, _dst, ddepth,
		ksize, anchor,
		normalize, borderType);
}