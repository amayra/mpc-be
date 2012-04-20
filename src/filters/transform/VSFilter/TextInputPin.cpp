/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "DirectVobSubFilter.h"
#include "TextInputPin.h"
#include "../../../DSUtil/DSUtil.h"

CTextInputPin::CTextInputPin(CDirectVobSubFilter* pFilter, CCritSec* pLock, CCritSec* pSubLock, HRESULT* phr)
	: CSubtitleInputPin(pFilter, pLock, pSubLock, phr)
	, m_pDVS(pFilter)
{
}

void CTextInputPin::AddSubStream(ISubStream* pSubStream)
{
	m_pDVS->AddSubStream(pSubStream);
}

void CTextInputPin::RemoveSubStream(ISubStream* pSubStream)
{
	m_pDVS->RemoveSubStream(pSubStream);
}

void CTextInputPin::InvalidateSubtitle(REFERENCE_TIME rtStart, ISubStream* pSubStream)
{
	m_pDVS->InvalidateSubtitle(rtStart, (DWORD_PTR)(ISubStream*)pSubStream);
}
