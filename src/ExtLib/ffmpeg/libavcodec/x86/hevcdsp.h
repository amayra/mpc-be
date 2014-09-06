/*
 * HEVC video decoder
 *
 * Copyright (C) 2012 - 2013 Guillaume Martres
 * Copyright (C) 2013 - 2014 Pierre-Edouard Lepere
 *
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_X86_HEVCDSP_H
#define AVCODEC_X86_HEVCDSP_H

#include <stddef.h>
#include <stdint.h>

// ==> Start patch MPC
struct SAOParams;

///////////////////////////////////////////////////////////////////////////////
// SAO functions
///////////////////////////////////////////////////////////////////////////////
void ff_hevc_sao_edge_filter_0_8_sse(uint8_t *_dst, uint8_t *_src,
                                     ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,
                                     int *borders, int _width, int _height, int c_idx,
                                     uint8_t *vert_edge, uint8_t *horiz_edge, uint8_t *diag_edge);
void ff_hevc_sao_edge_filter_1_8_sse(uint8_t *_dst, uint8_t *_src,
                                     ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,
                                     int *borders, int _width, int _height, int c_idx,
                                     uint8_t *vert_edge, uint8_t *horiz_edge, uint8_t *diag_edge);
void ff_hevc_sao_edge_filter_0_10_sse(uint8_t *_dst, uint8_t *_src,
                                      ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,
                                      int *borders, int _width, int _height, int c_idx,
                                      uint8_t *vert_edge, uint8_t *horiz_edge, uint8_t *diag_edge);
void ff_hevc_sao_edge_filter_1_10_sse(uint8_t *_dst, uint8_t *_src,
                                      ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,
                                      int *borders, int _width, int _height, int c_idx,
                                      uint8_t *vert_edge, uint8_t *horiz_edge, uint8_t *diag_edge);
void ff_hevc_sao_edge_filter_0_12_sse(uint8_t *_dst, uint8_t *_src,
                                      ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,
                                      int *borders, int _width, int _height, int c_idx,
                                      uint8_t *vert_edge, uint8_t *horiz_edge, uint8_t *diag_edge);
void ff_hevc_sao_edge_filter_1_12_sse(uint8_t *_dst, uint8_t *_src,
                                      ptrdiff_t _stride_dst, ptrdiff_t _stride_src, struct SAOParams *sao,
                                      int *borders, int _width, int _height, int c_idx,
                                      uint8_t *vert_edge, uint8_t *horiz_edge, uint8_t *diag_edge);

void ff_hevc_sao_band_filter_0_8_sse(uint8_t *_dst, uint8_t *_src, ptrdiff_t _stride_dst, ptrdiff_t _stride_src,
                                     struct SAOParams *sao, int *borders, int width, int height, int c_idx);
void ff_hevc_sao_band_filter_0_10_sse(uint8_t *_dst, uint8_t *_src, ptrdiff_t _stride_dst, ptrdiff_t _stride_src,
                                      struct SAOParams *sao, int *borders, int width, int height, int c_idx);
void ff_hevc_sao_band_filter_0_12_sse(uint8_t *_dst, uint8_t *_src, ptrdiff_t _stride_dst, ptrdiff_t _stride_src,
                                      struct SAOParams *sao, int *borders, int width, int height, int c_idx);
// ==> End patch MPC

#define idct_dc_proto(size, bitd, opt) \
                void ff_hevc_idct##size##_dc_add_##bitd##_##opt(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride)

#define PEL_LINK(dst, idx1, idx2, idx3, name, D, opt) \
dst[idx1][idx2][idx3] = ff_hevc_put_hevc_ ## name ## _ ## D ## _##opt; \
dst ## _bi[idx1][idx2][idx3] = ff_hevc_put_hevc_bi_ ## name ## _ ## D ## _##opt; \
dst ## _uni[idx1][idx2][idx3] = ff_hevc_put_hevc_uni_ ## name ## _ ## D ## _##opt; \
dst ## _uni_w[idx1][idx2][idx3] = ff_hevc_put_hevc_uni_w_ ## name ## _ ## D ## _##opt; \
dst ## _bi_w[idx1][idx2][idx3] = ff_hevc_put_hevc_bi_w_ ## name ## _ ## D ## _##opt


#define PEL_PROTOTYPE(name, D, opt) \
void ff_hevc_put_hevc_ ## name ## _ ## D ## _##opt(int16_t *dst, uint8_t *_src, ptrdiff_t _srcstride, int height, intptr_t mx, intptr_t my,int width); \
void ff_hevc_put_hevc_bi_ ## name ## _ ## D ## _##opt(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride, int16_t *src2, int height, intptr_t mx, intptr_t my, int width); \
void ff_hevc_put_hevc_uni_ ## name ## _ ## D ## _##opt(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride, int height, intptr_t mx, intptr_t my, int width); \
void ff_hevc_put_hevc_uni_w_ ## name ## _ ## D ## _##opt(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride, int height, int denom, int wx, int ox, intptr_t mx, intptr_t my, int width); \
void ff_hevc_put_hevc_bi_w_ ## name ## _ ## D ## _##opt(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride, int16_t *src2, int height, int denom, int wx0, int wx1, int ox0, int ox1, intptr_t mx, intptr_t my, int width)


///////////////////////////////////////////////////////////////////////////////
// MC functions
///////////////////////////////////////////////////////////////////////////////

#define EPEL_PROTOTYPES(fname, bitd, opt) \
        PEL_PROTOTYPE(fname##4,  bitd, opt); \
        PEL_PROTOTYPE(fname##6,  bitd, opt); \
        PEL_PROTOTYPE(fname##8,  bitd, opt); \
        PEL_PROTOTYPE(fname##12, bitd, opt); \
        PEL_PROTOTYPE(fname##16, bitd, opt); \
        PEL_PROTOTYPE(fname##24, bitd, opt); \
        PEL_PROTOTYPE(fname##32, bitd, opt); \
        PEL_PROTOTYPE(fname##48, bitd, opt); \
        PEL_PROTOTYPE(fname##64, bitd, opt)

#define QPEL_PROTOTYPES(fname, bitd, opt) \
        PEL_PROTOTYPE(fname##4,  bitd, opt); \
        PEL_PROTOTYPE(fname##8,  bitd, opt); \
        PEL_PROTOTYPE(fname##12, bitd, opt); \
        PEL_PROTOTYPE(fname##16, bitd, opt); \
        PEL_PROTOTYPE(fname##24, bitd, opt); \
        PEL_PROTOTYPE(fname##32, bitd, opt); \
        PEL_PROTOTYPE(fname##48, bitd, opt); \
        PEL_PROTOTYPE(fname##64, bitd, opt)

#define WEIGHTING_PROTOTYPE(width, bitd, opt) \
void ff_hevc_put_hevc_uni_w##width##_##bitd##_##opt(uint8_t *dst, ptrdiff_t dststride, int16_t *_src, ptrdiff_t _srcstride, int height, int denom,  int _wx, int _ox); \
void ff_hevc_put_hevc_bi_w##width##_##bitd##_##opt(uint8_t *dst, ptrdiff_t dststride, int16_t *_src, ptrdiff_t _srcstride, int16_t *_src2, int height, int denom,  int _wx0,  int _wx1, int _ox0, int _ox1)

#define WEIGHTING_PROTOTYPES(bitd, opt) \
        WEIGHTING_PROTOTYPE(2, bitd, opt); \
        WEIGHTING_PROTOTYPE(4, bitd, opt); \
        WEIGHTING_PROTOTYPE(6, bitd, opt); \
        WEIGHTING_PROTOTYPE(8, bitd, opt); \
        WEIGHTING_PROTOTYPE(12, bitd, opt); \
        WEIGHTING_PROTOTYPE(16, bitd, opt); \
        WEIGHTING_PROTOTYPE(24, bitd, opt); \
        WEIGHTING_PROTOTYPE(32, bitd, opt); \
        WEIGHTING_PROTOTYPE(48, bitd, opt); \
        WEIGHTING_PROTOTYPE(64, bitd, opt)


///////////////////////////////////////////////////////////////////////////////
// QPEL_PIXELS EPEL_PIXELS
///////////////////////////////////////////////////////////////////////////////
EPEL_PROTOTYPES(pel_pixels ,  8, sse4);
EPEL_PROTOTYPES(pel_pixels , 10, sse4);
EPEL_PROTOTYPES(pel_pixels , 12, sse4);
///////////////////////////////////////////////////////////////////////////////
// EPEL
///////////////////////////////////////////////////////////////////////////////
EPEL_PROTOTYPES(epel_h ,  8, sse4);
EPEL_PROTOTYPES(epel_h , 10, sse4);
EPEL_PROTOTYPES(epel_h , 12, sse4);

EPEL_PROTOTYPES(epel_v ,  8, sse4);
EPEL_PROTOTYPES(epel_v , 10, sse4);
EPEL_PROTOTYPES(epel_v , 12, sse4);

EPEL_PROTOTYPES(epel_hv ,  8, sse4);
EPEL_PROTOTYPES(epel_hv , 10, sse4);
EPEL_PROTOTYPES(epel_hv , 12, sse4);

///////////////////////////////////////////////////////////////////////////////
// QPEL
///////////////////////////////////////////////////////////////////////////////
QPEL_PROTOTYPES(qpel_h ,  8, sse4);
QPEL_PROTOTYPES(qpel_h , 10, sse4);
QPEL_PROTOTYPES(qpel_h , 12, sse4);

QPEL_PROTOTYPES(qpel_v,  8, sse4);
QPEL_PROTOTYPES(qpel_v, 10, sse4);
QPEL_PROTOTYPES(qpel_v, 12, sse4);

QPEL_PROTOTYPES(qpel_hv,  8, sse4);
QPEL_PROTOTYPES(qpel_hv, 10, sse4);
QPEL_PROTOTYPES(qpel_hv, 12, sse4);


WEIGHTING_PROTOTYPES(8, sse4);
WEIGHTING_PROTOTYPES(10, sse4);
WEIGHTING_PROTOTYPES(12, sse4);

///////////////////////////////////////////////////////////////////////////////
// TRANSFORM_ADD
///////////////////////////////////////////////////////////////////////////////
void ff_hevc_transform_add4_8_mmxext(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add8_8_sse2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add16_8_sse2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add32_8_sse2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);

void ff_hevc_transform_add8_8_avx(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add16_8_avx(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add32_8_avx(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);

void ff_hevc_transform_add4_10_mmxext(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add8_10_sse2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add16_10_sse2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add32_10_sse2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);

void ff_hevc_transform_add16_10_avx2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);
void ff_hevc_transform_add32_10_avx2(uint8_t *dst, int16_t *coeffs, ptrdiff_t stride);

// ==> Start patch MPC
void ff_hevc_transform_skip_8_sse(uint8_t *_dst, int16_t *coeffs, ptrdiff_t _stride);

void ff_hevc_transform_4x4_luma_8_sse2(int16_t *coeffs);
void ff_hevc_transform_4x4_luma_10_sse2(int16_t *coeffs);
void ff_hevc_transform_4x4_luma_12_sse2(int16_t *coeffs);

#define IDCT_FUNC(s, b) void ff_hevc_transform_ ## s ## x ## s ##_## b ##_sse2\
            (int16_t *coeffs, int col_limit);

IDCT_FUNC(4, 8)
IDCT_FUNC(4, 10)
IDCT_FUNC(4, 12)
IDCT_FUNC(8, 8)
IDCT_FUNC(8, 10)
IDCT_FUNC(8, 12)
IDCT_FUNC(16, 8)
IDCT_FUNC(16, 10)
IDCT_FUNC(16, 12)
IDCT_FUNC(32, 8)
IDCT_FUNC(32, 10)
IDCT_FUNC(32, 12)
// ==> End patch MPC

#endif // AVCODEC_X86_HEVCDSP_H
