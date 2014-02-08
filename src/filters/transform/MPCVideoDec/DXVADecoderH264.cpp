/*
 * (C) 2006-2014 see Authors.txt
 *
 * This file is part of MPC-BE.
 *
 * MPC-BE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-BE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "../../../DSUtil/DSUtil.h"
#include "DXVADecoderH264.h"
#include "MPCVideoDec.h"
#include "DXVAAllocator.h"
#include "FfmpegContext.h"
extern "C" {
	#include <ffmpeg/libavcodec/avcodec.h>
}

#if 0
	#define TRACE_H264 TRACE
#else
	#define TRACE_H264(...)
#endif

CDXVADecoderH264::CDXVADecoderH264(CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber)
	: CDXVADecoder(pFilter, pAMVideoAccelerator, nMode, nPicEntryNumber)
{
	m_bUseLongSlice = (GetDXVA1Config()->bConfigBitstreamRaw != 2);
	Init();
}

CDXVADecoderH264::CDXVADecoderH264(CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber, DXVA2_ConfigPictureDecode* pDXVA2Config)
	: CDXVADecoder(pFilter, pDirectXVideoDec, nMode, nPicEntryNumber, pDXVA2Config)
{
	m_bUseLongSlice = (GetDXVA2Config()->ConfigBitstreamRaw != 2);
	Init();
}

CDXVADecoderH264::~CDXVADecoderH264()
{
	DbgLog((LOG_TRACE, 3, L"CDXVADecoderH264::Destroy()"));
}

void CDXVADecoderH264::Init()
{
	DbgLog((LOG_TRACE, 3, L"CDXVADecoderH264::Init()"));

	memset(&m_DXVAPicParams, 0, sizeof(DXVA_PicParams_H264));
	memset(&m_pSliceLong, 0, sizeof(m_pSliceLong));
	memset(&m_pSliceShort, 0, sizeof(m_pSliceShort));

	m_DXVAPicParams.MbsConsecutiveFlag					= 1;
	m_DXVAPicParams.Reserved16Bits						= 3;
	if (m_pFilter->GetPCIVendor() == PCIV_Intel) {
		m_DXVAPicParams.Reserved16Bits					= 0x534c;
	} else if (IsATIUVD(m_pFilter->GetPCIVendor(), m_pFilter->GetPCIDevice())) {
		m_DXVAPicParams.Reserved16Bits					= 0;
	}
	m_DXVAPicParams.ContinuationFlag					= 1;
	m_DXVAPicParams.MinLumaBipredSize8x8Flag			= 1;	// Improve accelerator performances
	m_DXVAPicParams.StatusReportFeedbackNumber			= 0;	// Use to report status

	for (int i = 0; i < _countof(m_DXVAPicParams.RefFrameList); i++) {
		m_DXVAPicParams.RefFrameList[i].bPicEntry		= 255;
	}

	m_nNALLength	= 4;
	m_nMaxSlices	= 0;
	m_nSlices		= 0;

	switch (GetMode()) {
		case H264_VLD :
			AllocExecuteParams (4);
			break;
		default :
			ASSERT(FALSE);
	}

	FFH264SetDxvaSliceLong(m_pFilter->GetAVCtx(), m_pSliceLong);

	Flush();
}

void CDXVADecoderH264::CopyBitstream(BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize)
{
	CH264Nalu	Nalu;
	UINT		m_nSize = nSize;
	int			nDxvaNalLength;

	m_nSlices = 0;

	Nalu.SetBuffer(pBuffer, m_nSize, m_nNALLength);
	nSize = 0;
	while (Nalu.ReadNext()) {
		switch (Nalu.GetType()) {
			case NALU_TYPE_SLICE:
			case NALU_TYPE_IDR:
				// Skip the NALU if the data length is below 0
				if ((int)Nalu.GetDataLength() < 0) {
					break;
				}

				// For AVC1, put startcode 0x000001
				pDXVABuffer[0] = pDXVABuffer[1] = 0; pDXVABuffer[2] = 1;

				// Copy NALU
				memcpy_sse(pDXVABuffer + 3, Nalu.GetDataBuffer(), Nalu.GetDataLength());

				// Update slice control buffer
				nDxvaNalLength									= Nalu.GetDataLength() + 3;
				m_pSliceShort[m_nSlices].BSNALunitDataLocation	= nSize;
				m_pSliceShort[m_nSlices].SliceBytesInBuffer		= nDxvaNalLength;

				nSize											+= nDxvaNalLength;
				pDXVABuffer										+= nDxvaNalLength;
				m_nSlices++;
				break;
		}
	}

	// Complete bitstream buffer with zero padding (buffer size should be a multiple of 128)
	if (nSize % 128) {
		int nDummy = 128 - (nSize % 128);

		memset(pDXVABuffer, 0, nDummy);
		m_pSliceShort[m_nSlices - 1].SliceBytesInBuffer	+= nDummy;
		nSize											+= nDummy;		
	}
}

void CDXVADecoderH264::Flush()
{
	m_DXVAPicParams.UsedForReferenceFlags	= 0;
	m_nSurfaceIndex							= -1;
	m_pSampleToDeliver						= NULL;

	__super::Flush();
}

HRESULT CDXVADecoderH264::DecodeFrame(BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
	HRESULT						hr					= S_FALSE;
	UINT						nSlices				= 0;
	UINT						nNalOffset			= 0;
	UINT						SecondFieldOffset	= 0;
	UINT						nSize_Result		= 0;
	int							Sync				= 0;
	int							got_picture			= 0;
	CH264Nalu					Nalu;

	CHECK_HR_FALSE (FFH264DecodeFrame(m_pFilter->GetAVCtx(), m_pFilter->GetFrame(), pDataIn, nSize, rtStart, 
					&SecondFieldOffset, &Sync, &m_nNALLength, &got_picture));

	if (m_nSurfaceIndex == -1) {
		return S_FALSE;
	}

	Nalu.SetBuffer(pDataIn, nSize, m_nNALLength);
	while (Nalu.ReadNext()) {
		switch (Nalu.GetType()) {
			case NALU_TYPE_SLICE:
			case NALU_TYPE_IDR:
				if (m_bUseLongSlice) {
					m_pSliceLong[nSlices].BSNALunitDataLocation	= nNalOffset;
					m_pSliceLong[nSlices].SliceBytesInBuffer	= Nalu.GetDataLength() + 3;
					m_pSliceLong[nSlices].slice_id				= nSlices;
					FF264UpdateRefFrameSliceLong(&m_DXVAPicParams, &m_pSliceLong[nSlices], m_pFilter->GetAVCtx());

					if (nSlices) {
						m_pSliceLong[nSlices - 1].NumMbsForSlice = m_pSliceLong[nSlices].NumMbsForSlice = m_pSliceLong[nSlices].first_mb_in_slice - m_pSliceLong[nSlices - 1].first_mb_in_slice;
					}
				}
				nSlices++;
				nNalOffset += (UINT)(Nalu.GetDataLength() + 3);
				if (nSlices > MAX_SLICES) {
					break;
				}
				break;
		}
	}

	if (!nSlices) {
		return S_FALSE;
	}

	// If parsing fail (probably no PPS/SPS), continue anyway it may arrived later (happen on truncated streams)
	CHECK_HR_FALSE (FFH264BuildPicParams(m_pFilter->GetAVCtx(), m_pFilter->GetPCIVendor(), m_pFilter->GetPCIDevice(), &m_DXVAPicParams, &m_DXVAScalingMatrix));
	FFH264SetCurrentPicture(m_nSurfaceIndex, &m_DXVAPicParams, m_pFilter->GetAVCtx());

	{
		m_DXVAPicParams.StatusReportFeedbackNumber++;
		
		CHECK_HR (BeginFrame(m_nSurfaceIndex, m_pSampleToDeliver));
		// Send picture parameters
		CHECK_HR (AddExecuteBuffer(DXVA2_PictureParametersBufferType, sizeof(m_DXVAPicParams), &m_DXVAPicParams));
		// Add bitstream
		CHECK_HR (AddExecuteBuffer(DXVA2_BitStreamDateBufferType, SecondFieldOffset ? SecondFieldOffset : nSize, pDataIn, &nSize_Result));
		// Add quantization matrix
		CHECK_HR (AddExecuteBuffer(DXVA2_InverseQuantizationMatrixBufferType, sizeof(DXVA_Qmatrix_H264), (void*)&m_DXVAScalingMatrix));
		// Add slice control
		if (m_bUseLongSlice) {
			CHECK_HR (AddExecuteBuffer(DXVA2_SliceControlBufferType, sizeof(DXVA_Slice_H264_Long) * m_nSlices, m_pSliceLong));
		} else {
			CHECK_HR (AddExecuteBuffer(DXVA2_SliceControlBufferType, sizeof(DXVA_Slice_H264_Short) * m_nSlices, m_pSliceShort));
		}
		// Decode frame
		CHECK_HR (Execute());
		CHECK_HR (EndFrame(m_nSurfaceIndex));

		if (SecondFieldOffset) {
			
			CHECK_HR (BeginFrame(m_nSurfaceIndex, m_pSampleToDeliver));
			// Send picture parameters
			CHECK_HR (AddExecuteBuffer(DXVA2_PictureParametersBufferType, sizeof(m_DXVAPicParams), &m_DXVAPicParams));
			// Add bitstream
			CHECK_HR (AddExecuteBuffer(DXVA2_BitStreamDateBufferType, nSize - SecondFieldOffset, pDataIn + SecondFieldOffset, &nSize_Result));
			// Add quantization matrix
			CHECK_HR (AddExecuteBuffer(DXVA2_InverseQuantizationMatrixBufferType, sizeof(DXVA_Qmatrix_H264), (void*)&m_DXVAScalingMatrix));
			// Add slice control
			if (m_bUseLongSlice) {
				CHECK_HR (AddExecuteBuffer(DXVA2_SliceControlBufferType, sizeof(DXVA_Slice_H264_Long) * m_nSlices, m_pSliceLong));
			} else {
				CHECK_HR (AddExecuteBuffer(DXVA2_SliceControlBufferType, sizeof(DXVA_Slice_H264_Short) * m_nSlices, m_pSliceShort));
			}
			// Decode frame
			CHECK_HR (Execute());
			CHECK_HR (EndFrame(m_nSurfaceIndex));
		}

	}

	if (got_picture) {
		AddToStore(m_nSurfaceIndex, m_pSampleToDeliver, m_DXVAPicParams.RefPicFlag, rtStart, rtStop);
	}

#if defined(_DEBUG) && 0
	DisplayStatus();
#endif

	FFH264UpdateRefFramesList(&m_DXVAPicParams, m_pFilter->GetAVCtx());

	if (got_picture) {
		hr = DisplayNextFrame();
	}

	m_bFlushed = false;
	return hr;
}

HRESULT CDXVADecoderH264::DisplayStatus()
{
	HRESULT 			hr = E_INVALIDARG;
	DXVA_Status_H264 	Status;

	memset(&Status, 0, sizeof(Status));
	CHECK_HR (CDXVADecoder::QueryStatus(&Status, sizeof(Status)));

	TRACE_H264 ("CDXVADecoderH264::DisplayStatus() : Status for the frame %u : bBufType = %u, bStatus = %u, wNumMbsAffected = %u\n",
				Status.StatusReportFeedbackNumber,
				Status.bBufType,
				Status.bStatus,
				Status.wNumMbsAffected);

	return hr;
}

int CDXVADecoderH264::FindOldestFrame()
{
	int		nPos	= -1;
	AVFrame	*pic	= m_pFilter->GetFrame();

	SurfaceWrapper* pSurfaceWrapper = (SurfaceWrapper*)pic->data[3];
	if (pSurfaceWrapper) {
		int nSurfaceIndex = pSurfaceWrapper->nSurfaceIndex;
		if (nSurfaceIndex >= 0 && nSurfaceIndex < m_nPicEntryNumber && m_pPictureStore[nSurfaceIndex].pSample) {
			nPos = nSurfaceIndex;
			m_pPictureStore[nPos].rtStart = m_pFilter->GetFrame()->pkt_pts;
			m_pFilter->ReorderBFrames(m_pPictureStore[nPos].rtStart, m_pPictureStore[nPos].rtStop);
			m_pFilter->UpdateFrameTime(m_pPictureStore[nPos].rtStart, m_pPictureStore[nPos].rtStop);
		}
	}

	return nPos;
}

HRESULT CDXVADecoderH264::get_buffer_dxva(AVFrame *pic)
{
	m_pSampleToDeliver.Release();
	m_nSurfaceIndex = -1;
	HRESULT hr = S_OK;
	CHECK_HR(GetFreeSurfaceIndex(m_nSurfaceIndex, &m_pSampleToDeliver, 0, 0));
	
	SurfaceWrapper* pSurfaceWrapper = DNew SurfaceWrapper();
	pSurfaceWrapper->opaque			= (void*)this;
	pSurfaceWrapper->nSurfaceIndex	= m_nSurfaceIndex;
	pSurfaceWrapper->pSample		= m_pSampleToDeliver;

	pic->data[3]	= (uint8_t *)pSurfaceWrapper;
	pic->buf[3]		= av_buffer_create(NULL, 0, release_buffer_dxva, pSurfaceWrapper, 0);
	
	return hr;
}
