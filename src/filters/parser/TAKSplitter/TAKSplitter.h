/*
 * $Id:
 *
 * Copyright (C) 2013 Alexandr Vodiannikov aka "Aleksoid1978" (Aleksoid1978@mail.ru)
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

#pragma once

#include "../BaseSplitter/BaseSplitter.h"

#pragma warning(disable: 4005 4244)
extern "C" {
	#include <stdint.h>
}

#define TAKSplitterName L"MPC TAK Splitter"
#define TAKSourceName   L"MPC TAK Source"

enum TAKCodecType {
	TAK_CODEC_MONO_STEREO	= 2,
	TAK_CODEC_MULTICHANNEL	= 4,
};

enum TAKMetaDataType {
	TAK_METADATA_END = 0,
	TAK_METADATA_STREAMINFO,
	TAK_METADATA_SEEKTABLE,
	TAK_METADATA_SIMPLE_WAVE_DATA,
	TAK_METADATA_ENCODER,
	TAK_METADATA_PADDING,
	TAK_METADATA_MD5,
	TAK_METADATA_LAST_FRAME,
};

struct TAKStreamInfo {
	enum TAKCodecType	codec;
	int					data_type;
	int					sample_rate;
	int					channels;
	int					bps;
	uint64_t			ch_layout;
	uint64_t			samples;

	TAKStreamInfo() {
		memset(this, 0, sizeof(*this));
	}
};

class __declspec(uuid("AA04C78C-3671-43F6-ABFE-6C265BAB2345"))
	CTAKSplitterFilter : public CBaseSplitterFilter
{
	REFERENCE_TIME m_rtStart;
	DWORD m_nAvgBytesPerSec;

	__int64 m_endpos;

protected:
	CAutoPtr<CBaseSplitterFileEx> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

	void ParseTAKStreamInfo(BYTE* buff, int size, TAKStreamInfo& si);

public:
	CTAKSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// CBaseFilter

	STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);
};

class __declspec(uuid("A98DC1BA-E70D-47A0-BAD1-28C64A859FB1"))
	CTAKSourceFilter : public CTAKSplitterFilter
{
public:
	CTAKSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};