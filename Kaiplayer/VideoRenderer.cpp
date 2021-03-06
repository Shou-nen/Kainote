﻿//  Copyright (c) 2016, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.


#include "Visuals.h"
#include "Videobox.h"
#include "VideoRenderer.h"
#include "DShowPlayer.h"
#include "KainoteApp.h"

#include <Dvdmedia.h>
#include "CsriMod.h"
#include <thread>
#include "OpennWrite.h"


#if byvertices
struct CUSTOMVERTEX
{
	D3DXVECTOR3 position; // The position
	FLOAT       tu, tv;   // The texture coordinates
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)
#endif

#pragma comment(lib, "Dxva2.lib")
const IID IID_IDirectXVideoProcessorService = { 0xfc51a552, 0xd5e7, 0x11d9, { 0xaf, 0x55, 0x00, 0x05, 0x4e, 0x43, 0xff, 0x02 } };


void CreateVERTEX(VERTEX *v, float X, float Y, D3DCOLOR Color, float Z)
{
	v->fX = X;
	v->fY = Y;
	v->fZ = Z;
	v->Color = Color;
}

VideoRenderer::VideoRenderer(wxWindow *_parent, const wxSize &size)
	:wxWindow(_parent, -1, wxDefaultPosition, size)//wxFULL_REPAINT_ON_RESIZE
	, panelHeight(44)
	, AR(0.0)
	, fps(0.0)
	, isFullscreen(false)
	, hasZoom(false)
{
	hwnd = GetHWND();

	//---------------------------- format
	d3dformat = D3DFORMAT('2YUY');//D3DFORMAT('21VN');
	//-----------------------------------
	vformat = NV12;
	time = 0;
	playend = 0;
	vstate = None;
	d3dobject = NULL;
	d3device = NULL;
	bars = NULL;
	VFF = NULL;
	instance = NULL;
	//vobsub = NULL;
	framee = NULL;
	format = NULL;
	lines = NULL;
	Visual = NULL;
	resized = seek = block = cross = pbar = hasVisualEdition = false;
	IsDshow = true;
	devicelost = false;
	panelOnFullscreen = false;
	MainStream = NULL;
	datas = NULL;
	player = NULL;
	vplayer = NULL;
	windowRect.bottom = 0;
	windowRect.right = 0;
	windowRect.left = 0;
	windowRect.top = 0;
	m_font = NULL;
	numframe = 0;
	diff = 0;
	avframetime = 42;
	zoomParcent = 1.f;
#if byvertices
	vertex=NULL;
	texture=NULL;
#endif
	dxvaProcessor = NULL;
	dxvaService = NULL;
}

bool VideoRenderer::InitDX(bool reset)
{

	if (!reset){
		d3dobject = Direct3DCreate9(D3D_SDK_VERSION);
		PTR(d3dobject, _("Nie można utworzyć objektu Direct3D"));
	}
	else{
		SAFE_RELEASE(MainStream);
		SAFE_RELEASE(bars);
		SAFE_RELEASE(lines);
		SAFE_RELEASE(m_font);

#if byvertices
		SAFE_RELEASE(texture);
		SAFE_RELEASE(vertex);
#endif
		if (IsDshow){
			SAFE_RELEASE(dxvaProcessor);
			SAFE_RELEASE(dxvaService);
		}
	}

	HRESULT hr;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.BackBufferWidth = windowRect.right;
	d3dpp.BackBufferHeight = windowRect.bottom;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;//
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_INTERVAL_DEFAULT;

	if (reset){
		hr = d3device->Reset(&d3dpp);
		if (FAILED(hr)){
			KaiLog(_("Nie można zresetować Direct3D"));
			return false;
		}
	}
	else{
		hr = d3dobject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3device);//| D3DCREATE_FPU_PRESERVE
		if (FAILED(hr)){
			HR(d3dobject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &d3device),
				_("Nie można utworzyć urządzenia D3D9"));
		}
	}

	hr = d3device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	hr = d3device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

	hr = d3device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	hr = d3device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = d3device->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = d3device->SetRenderState(D3DRS_DITHERENABLE, TRUE);

	hr = d3device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = d3device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = d3device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	hr = d3device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = d3device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	hr = d3device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);

	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	hr = d3device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	HR(hr, _("Zawiodło któreś z ustawień DirectX"));

	D3DXMATRIX matOrtho;
	D3DXMATRIX matIdentity;

	D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, windowRect.right, windowRect.bottom, 0, 0.0f, 1.0f);
	D3DXMatrixIdentity(&matIdentity);

	HR(d3device->SetTransform(D3DTS_PROJECTION, &matOrtho), _("Nie można ustawić macierzy projekcji"));
	HR(d3device->SetTransform(D3DTS_WORLD, &matIdentity), _("Nie można ustawić macierzy świata"));
	HR(d3device->SetTransform(D3DTS_VIEW, &matIdentity), _("Nie można ustawić macierzy widoku"));

#if byvertices
	hr = d3device->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP );
	hr = d3device->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP );

	// Add filtering
	hr = d3device->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	hr = d3device->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	HR(hr,_("Zawiodło któreś z ustawień DirectX vertices"));
	HR(d3device->CreateTexture(vwidth,vheight, 1, D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&texture, NULL), "Nie można utworzyć tekstury" );

	HR(texture->GetSurfaceLevel(0, &bars), "nie można utworzyć powierzchni");

	HR(d3device->CreateOffscreenPlainSurface(vwidth,vheight,d3dformat, D3DPOOL_DEFAULT,&MainStream,0),"Nie można utworzyć powierzchni");

	HR(d3device->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX),D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT, &vertex, NULL ),
		"Nie można utworzyć bufora wertex")

		CUSTOMVERTEX* pVertices;
	HR ( hr = vertex->Lock( 0, 0, (void**)&pVertices, 0 ), "nie można zablokować bufora vertex" ); 

	pVertices[0].position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVertices[0].tu       = 0.0f;
	pVertices[0].tv       = 0.0f; 
	pVertices[1].position = D3DXVECTOR3(vwidth, 0.0f, 0.0f);
	pVertices[1].tu       = 1.0f;
	pVertices[1].tv       = 0.0f;
	pVertices[2].position = D3DXVECTOR3(vwidth, vheight, 0.0f);
	pVertices[2].tu       = 1.0f;
	pVertices[2].tv       = 1.0f; 
	pVertices[3].position = D3DXVECTOR3(0.0f, vheight, 0.0f);
	pVertices[3].tu       = 0.0f;
	pVertices[3].tv       = 1.0f;

	vertex->Unlock();
#endif

	//if (d3dformat != ddsd.Format) {
	//wLogStatus("Textura ma niewłaściwy format"); return false;	
	//}
	if (IsDshow){
		HR(d3device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bars), _("Nie można stworzyć powierzchni"));
		HR(DXVA2CreateVideoService(d3device, IID_IDirectXVideoProcessorService, (VOID**)&dxvaService),
			_("Nie można stworzyć DXVA processor service"));
		DXVA2_VideoDesc videoDesc;
		videoDesc.SampleWidth = vwidth;
		videoDesc.SampleHeight = vheight;
		videoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
		videoDesc.SampleFormat.NominalRange = DXVA2_NominalRange_0_255;
		videoDesc.SampleFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
		videoDesc.SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
		videoDesc.SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
		videoDesc.SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;
		videoDesc.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
		videoDesc.Format = D3DFMT_X8R8G8B8;
		videoDesc.InputSampleFreq.Numerator = 60;
		videoDesc.InputSampleFreq.Denominator = 1;
		videoDesc.OutputFrameFreq.Numerator = 60;
		videoDesc.OutputFrameFreq.Denominator = 1;

		UINT count, count1;//, count2;
		GUID* guids = NULL;

		HR(dxvaService->GetVideoProcessorDeviceGuids(&videoDesc, &count, &guids), _("Nie moźna pobrać GUIDów DXVA"));
		D3DFORMAT* formats = NULL;
		//D3DFORMAT* formats2 = NULL;
		bool isgood = false;
		GUID dxvaGuid;
		DXVA2_VideoProcessorCaps DXVAcaps;
		for (UINT i = 0; i < count; i++){
			hr = dxvaService->GetVideoProcessorRenderTargets(guids[i], &videoDesc, &count1, &formats);
			if (FAILED(hr)){ KaiLog(_("Nie można uzyskać formatów DXVA")); continue; }
			for (UINT j = 0; j < count1; j++)
			{
				if (formats[j] == D3DFMT_X8R8G8B8)
				{
					isgood = true; //break;
				}

			}

			CoTaskMemFree(formats);
			if (!isgood){ KaiLog(_("Ten format nie jest obsługiwany przez DXVA")); continue; }
			isgood = false;

			hr = dxvaService->GetVideoProcessorCaps(guids[i], &videoDesc, D3DFMT_X8R8G8B8, &DXVAcaps);
			if (FAILED(hr)){ KaiLog(_("GetVideoProcessorCaps zawiodło")); continue; }
			if (DXVAcaps.NumForwardRefSamples > 0 || DXVAcaps.NumBackwardRefSamples > 0)
			{
				/*wLogStatus(L"NumForwardRefSamples albo NumBackwardRefSample jest większe od zera");*/continue;
			}

			//if(DXVAcaps.DeviceCaps!=4){continue;}//DXVAcaps.InputPool
			hr = dxvaService->CreateSurface(vwidth, vheight, 0, d3dformat, D3DPOOL_DEFAULT, 0,
				DXVA2_VideoSoftwareRenderTarget, &MainStream, NULL);
			if (FAILED(hr)){ KaiLog(wxString::Format(_("Nie można stworzyć powierzchni DXVA %i"), (int)i)); continue; }

			hr = dxvaService->CreateVideoProcessor(guids[i], &videoDesc, D3DFMT_X8R8G8B8, 0, &dxvaProcessor);
			if (FAILED(hr)){ KaiLog(_("Nie można stworzyć processora DXVA")); continue; }
			dxvaGuid = guids[i]; isgood = true;
			break;
		}
		CoTaskMemFree(guids);
		PTR(isgood, L"Nie ma żadnych guidów");



	}
	else{
#ifndef byvertices
		HR(d3device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bars), _("Nie można stworzyć powierzchni"));

		HR(d3device->CreateOffscreenPlainSurface(vwidth, vheight, d3dformat, D3DPOOL_DEFAULT, &MainStream, 0),
			_("Nie można stworzyć plain surface"));//D3DPOOL_DEFAULT
#endif
	}

	HR(D3DXCreateLine(d3device, &lines), _("Nie można stworzyć linii D3DX"));
	HR(D3DXCreateFont(d3device, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, L"Tahoma", &m_font), _("Nie można stworzyć czcionki D3DX"));

	return true;
}
//w stosuj false tylko w przypadku gdy odświeżasz coś namalowanego na wideo, 
//w reszcie przypadków ma być pełne odświeżanie klatki

void VideoRenderer::Render(bool Frame, bool wait)
{
	if (Frame && !IsDshow && !devicelost){
		VFF->Render(wait);
		resized = false;
		return;
	}
	wxCriticalSectionLocker lock(mutexRender);
	resized = false;
	HRESULT hr = S_OK;

	if (devicelost)
	{
		if (FAILED(hr = d3device->TestCooperativeLevel()))
		{
			if (D3DERR_DEVICELOST == hr ||
				D3DERR_DRIVERINTERNALERROR == hr){
				//wxLogMessage("cooperative level device lost");
				return;
			}

			if (D3DERR_DEVICENOTRESET == hr)
			{
				Clear();
				InitDX();
				if (IsDshow){ RecreateSurface(); }
				if (Visual){
					Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
						backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
				}
				devicelost = false;
				Render(true, false);
				return;
			}
			return;
		}
		devicelost = false;
	}

	hr = d3device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	if (IsDshow){
		DXVA2_VideoProcessBltParams blt = { 0 };
		DXVA2_VideoSample samples = { 0 };
		LONGLONG start_100ns = time * 10000;
		LONGLONG end_100ns = start_100ns + 170000;
		blt.TargetFrame = start_100ns;
		blt.TargetRect = windowRect;

		// DXVA2_VideoProcess_Constriction
		blt.ConstrictionSize.cx = windowRect.right - windowRect.left;
		blt.ConstrictionSize.cy = windowRect.bottom - windowRect.top;
		DXVA2_AYUVSample16 color;

		color.Cr = 0x8000;
		color.Cb = 0x8000;
		color.Y = 0x0F00;
		color.Alpha = 0xFFFF;
		blt.BackgroundColor = color;

		// DXVA2_VideoProcess_YUV2RGBExtended
		blt.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_Unknown;
		blt.DestFormat.NominalRange = DXVA2_NominalRange_0_255;//EX_COLOR_INFO[g_ExColorInfo][1];
		blt.DestFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;
		blt.DestFormat.VideoLighting = DXVA2_VideoLighting_dim;
		blt.DestFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
		blt.DestFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;

		blt.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
		// Initialize main stream video sample.
		//
		samples.Start = start_100ns;
		samples.End = end_100ns;

		// DXVA2_VideoProcess_YUV2RGBExtended
		samples.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
		samples.SampleFormat.NominalRange = DXVA2_NominalRange_0_255;
		samples.SampleFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_BT709;//EX_COLOR_INFO[g_ExColorInfo][0];
		samples.SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
		samples.SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
		samples.SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;

		samples.SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;

		samples.SrcSurface = MainStream;

		samples.SrcRect = mainStreamRect;

		samples.DstRect = backBufferRect;

		// DXVA2_VideoProcess_PlanarAlpha
		samples.PlanarAlpha = DXVA2_Fixed32OpaqueAlpha();

		hr = dxvaProcessor->VideoProcessBlt(bars, &blt, &samples, 1, NULL);

	}
	else{
		hr = d3device->StretchRect(MainStream, &mainStreamRect, bars, &backBufferRect, D3DTEXF_LINEAR);
		if (FAILED(hr)){ KaiLog(_("Nie można nałożyć powierzchni na siebie")); }
	}

	hr = d3device->BeginScene();

#if byvertices


	// Render the vertex buffer contents
	hr = d3device->SetStreamSource( 0, vertex, 0, sizeof(CUSTOMVERTEX) );
	hr = d3device->SetVertexShader( NULL );
	hr = d3device->SetFVF( D3DFVF_CUSTOMVERTEX);
	hr = d3device->SetTexture( 0, texture );
	hr = d3device->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );
#endif

	if (Visual){ Visual->Draw(time); }

	if (cross){
		DRAWOUTTEXT(m_font, coords, crossRect, (crossRect.left < vectors[0].x) ? 10 : 8, 0xFFFFFFFF)
			hr = lines->SetWidth(3);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[0], 2, 0xFF000000);
		hr = lines->Draw(&vectors[2], 2, 0xFF000000);
		hr = lines->End();
		hr = lines->SetWidth(1);
		D3DXVECTOR2 v1[4];
		v1[0] = vectors[0];
		v1[0].x += 0.5f;
		v1[1] = vectors[1];
		v1[1].x += 0.5f;
		v1[2] = vectors[2];
		v1[2].y += 0.5f;
		v1[3] = vectors[3];
		v1[3].y += 0.5f;
		hr = lines->Begin();
		hr = lines->Draw(&v1[0], 2, 0xFFFFFFFF);
		hr = lines->Draw(&v1[2], 2, 0xFFFFFFFF);
		hr = lines->End();
	}

	if (pbar){
		DRAWOUTTEXT(m_font, pbtime, progressBarRect, DT_LEFT | DT_TOP, 0xFFFFFFFF)
			hr = lines->SetWidth(1);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[4], 5, 0xFF000000);
		hr = lines->Draw(&vectors[9], 5, 0xFFFFFFFF);
		hr = lines->End();
		hr = lines->SetWidth(7);
		hr = lines->Begin();
		hr = lines->Draw(&vectors[14], 2, 0xFFFFFFFF);
		hr = lines->End();
	}
	if (hasZoom){ DrawZoom(); }
	// End the scene
	hr = d3device->EndScene();
	hr = d3device->Present(NULL, &windowRect, NULL, NULL);
	if (D3DERR_DEVICELOST == hr ||
		D3DERR_DRIVERINTERNALERROR == hr){
		if (!devicelost){
			devicelost = true;
		}
		Render(true, false);
	}

}


bool VideoRenderer::DrawTexture(byte *nframe, bool copy)
{

	wxCriticalSectionLocker lock(mutexRender);
	byte *fdata = NULL;
	byte *texbuf;
	byte bytes = (vformat == RGB32) ? 4 : (vformat == YUY2) ? 2 : 1;
	//DWORD black = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x10101010;
	//DWORD blackuv = (vformat == RGB32) ? 0 : (vformat == YUY2) ? 0x80108010 : 0x8080;

	D3DLOCKED_RECT d3dlr;

	if (nframe){
		fdata = nframe;
		if (copy){
			byte *cpy = (byte*)datas;
			memcpy(cpy, fdata, vheight * pitch);
		}
	}
	else{
		KaiLog(_("Brak bufora klatki")); return false;
	}


	if (instance){
		//for swap -pitch and buffer set to last element - pitch
		framee->strides[0] = (swapFrame) ? -(vwidth * bytes) : vwidth * bytes;
		framee->planes[0] = (swapFrame) ? fdata + (vwidth * (vheight - 1) * bytes) : fdata;
		csri_render(instance, framee, (time / 1000.0));
	}


#ifdef byvertices
	HR(MainStream->LockRect( &d3dlr,0, 0), _("Nie można zablokować bufora tekstury"));//D3DLOCK_NOSYSLOCK
#else
	HR(MainStream->LockRect(&d3dlr, 0, D3DLOCK_NOSYSLOCK), _("Nie można zablokować bufora tekstury"));
#endif
	texbuf = static_cast<byte *>(d3dlr.pBits);

	diff = d3dlr.Pitch - (vwidth*bytes);
	if (swapFrame){
		int framePitch = vwidth * bytes;
		byte * reversebyte = fdata + (framePitch * vheight) - framePitch;
		for (int j = 0; j < vheight; ++j){
			memcpy(texbuf, reversebyte, framePitch);
			texbuf += framePitch + diff;
			reversebyte -= framePitch;
		}
	}
	else if (!diff){
		memcpy(texbuf, fdata, (vheight * pitch));
	}
	else if (diff > 0){

		if (vformat >= YV12){
			for (int i = 0; i < vheight; ++i){
				memcpy(texbuf, fdata, vwidth);
				texbuf += (vwidth + diff);
				fdata += vwidth;
			}
			int hheight = vheight / 2;
			int fwidth = (vformat == NV12) ? vwidth : vwidth / 2;
			int fdiff = (vformat == NV12) ? diff : diff / 2;

			for (int i = 0; i < hheight; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + fdiff);
				fdata += fwidth;
			}
			if (vformat < NV12){
				for (int i = 0; i < hheight; ++i){
					memcpy(texbuf, fdata, fwidth);
					texbuf += (fwidth + fdiff);
					fdata += fwidth;
				}
			}
		}
		else
		{
			int fwidth = vwidth * bytes;
			for (int i = 0; i < vheight; i++){
				memcpy(texbuf, fdata, fwidth);
				texbuf += (fwidth + diff);
				fdata += fwidth;
			}
		}

	}
	else{
		KaiLog(wxString::Format(L"bad pitch diff %i pitch %i dxpitch %i", diff, pitch, d3dlr.Pitch));
	}

	MainStream->UnlockRect();

	return true;
}

void VideoRenderer::RecreateSurface()
{
	int all = vheight * pitch;
	char *cpy = new char[all];
	byte *cpy1 = (byte*)cpy;
	byte *data1 = (byte*)datas;
	memcpy(cpy1, data1, all);
	DrawTexture(cpy1);
	delete[] cpy;
}

VideoRenderer::~VideoRenderer()
{
	Stop();

	vstate = None;

	SAFE_DELETE(VFF);
	Clear();
	SAFE_DELETE(Visual);
	SAFE_DELETE(vplayer);
	SAFE_DELETE(framee);
	SAFE_DELETE(format);
	if (instance) { csri_close(instance); }

	if (datas){ delete[] datas; datas = NULL; }

}

void VideoRenderer::Clear()
{
	SAFE_RELEASE(MainStream);
	SAFE_RELEASE(bars);
#if byvertices
	SAFE_RELEASE(vertex);
	SAFE_RELEASE(texture);
#endif
	SAFE_RELEASE(dxvaProcessor);
	SAFE_RELEASE(dxvaService);
	SAFE_RELEASE(d3device);
	SAFE_RELEASE(d3dobject);
	SAFE_RELEASE(lines);
	SAFE_RELEASE(m_font);
	hasZoom = false;
}



bool VideoRenderer::OpenFile(const wxString &fname, wxString *textsubs, bool Dshow, bool vobsub, bool changeAudio)
{
	wxMutexLocker lock(mutexOpenFile);
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;
	TabPanel *tab = ((TabPanel*)GetParent());
	VideoFfmpeg *tmpvff = NULL;
	if (vstate == Playing){ ((VideoCtrl*)this)->Stop(); }

	if (!Dshow){
		bool success;
		tmpvff = new VideoFfmpeg(fname, this, (isFullscreen) ? ((VideoCtrl*)this)->TD : (wxWindow *)Kaia->Frame, &success);
		//this is safe mode, when new video not load, 
		//the last opened will not be released
		if (!success || !tmpvff){
			SAFE_DELETE(tmpvff);
			return false;
		}
		//when loading only audio do not remove video
		if (tmpvff->width < 0 && tmpvff->GetSampleRate() > 0){
			VideoFfmpeg *tmp = VFF;
			VFF = tmpvff;
			Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
			player = tab->Edit->ABox->audioDisplay;
			VFF = tmp;
			return false;
		}
	}
	SAFE_DELETE(VFF);

	if (vstate != None){
		resized = seek = cross = pbar = false;
		vstate = None;
		Clear();
	}
	IsDshow = Dshow;
	time = 0;
	numframe = 0;

	if (!Dshow){
		SAFE_DELETE(vplayer);
		VFF = tmpvff;
		d3dformat = D3DFMT_X8R8G8B8;
		vformat = RGB32;
		vwidth = VFF->width;
		vheight = VFF->height;
		fps = VFF->fps;
		ax = VFF->arwidth;
		ay = VFF->arheight;
		if (vwidth % 2 != 0){ vwidth++; }
		pitch = vwidth * 4;
		if (changeAudio){
			if (VFF->GetSampleRate() > 0){
				Kaia->Frame->OpenAudioInTab(tab, 40000, fname);
				player = tab->Edit->ABox->audioDisplay;
			}
			else if (player){ Kaia->Frame->OpenAudioInTab(tab, CloseAudio, L""); }
		}
		if (!VFF || VFF->width < 0){
			return false;
		}
	}
	else{

		if (!vplayer){ vplayer = new DShowPlayer(this); }

		if (!vplayer->OpenFile(fname, vobsub)){
			return false;
		}
		wxSize videoSize = vplayer->GetVideoSize();
		vwidth = videoSize.x; vheight = videoSize.y;
		if (vwidth % 2 != 0){ vwidth++; }

		pitch = vwidth * vplayer->inf.bytes;
		fps = vplayer->inf.fps;
		vformat = vplayer->inf.CT;
		ax = vplayer->inf.ARatioX;
		ay = vplayer->inf.ARatioY;
		d3dformat = (vformat == 5) ? D3DFORMAT('21VN') : (vformat == 3) ? D3DFORMAT('21VY') :
			(vformat == 2) ? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;
		//KaiLog(wxString::Format(L"vformat %i", (int)vformat));
		swapFrame = (vformat == 0 && !vplayer->HasVobsub());
		if (player){
			Kaia->Frame->OpenAudioInTab(((TabPanel*)GetParent()), CloseAudio, L"");
		}
	}
	diff = 0;
	frameDuration = (1000.0f / fps);
	if (ay == 0 || ax == 0){ AR = 0.0f; }
	else{ AR = (float)ay / (float)ax; }

	mainStreamRect.bottom = vheight;
	mainStreamRect.right = vwidth;
	mainStreamRect.left = 0;
	mainStreamRect.top = 0;
	if (datas){ delete[] datas; datas = NULL; }
	datas = new char[vheight*pitch];

	if (!InitDX()){ return false; }
	UpdateRects();

	if (!framee){ framee = new csri_frame; }
	if (!format){ format = new csri_fmt; }
	for (int i = 1; i < 4; i++){
		framee->planes[i] = NULL;
		framee->strides[i] = NULL;
	}

	framee->pixfmt = (vformat == 5) ? CSRI_F_YV12A : (vformat == 3) ? CSRI_F_YV12 :
		(vformat == 2) ? CSRI_F_YUY2 : CSRI_F_BGR_;

	format->width = vwidth;
	format->height = vheight;
	format->pixfmt = framee->pixfmt;

	if (!vobsub){
		OpenSubs(textsubs, false);
	}
	else{
		SAFE_DELETE(textsubs);
		OpenSubs(0, false);
	}
	vstate = Stopped;
	if (IsDshow && vplayer)
		vplayer->GetChapters(&chapters);
	else if (!IsDshow)
		VFF->GetChapters(&chapters);

	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
			backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
	}
	return true;
}

bool VideoRenderer::Play(int end)
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS);
	VideoCtrl *vb = ((VideoCtrl*)this);
	if (!(IsShown() || (vb->TD && vb->TD->IsShown()))){ return false; }
	TabPanel* pan = (TabPanel*)GetParent();
	if (hasVisualEdition){
		wxString *txt = pan->Grid->SaveText();
		OpenSubs(txt, false, true);
		SAFE_DELETE(Visual->dummytext);
		hasVisualEdition = false;
	}
	else if (hasDummySubs && pan->editor){
		OpenSubs(pan->Grid->SaveText(), false, true);
	}

	if (end > 0){ playend = end; }
	else if (IsDshow){ playend = 0; }
	else{ playend = GetDuration(); }
	if (IsDshow){ if (time < GetDuration() - frameDuration) vplayer->Play(); else return false; }

	vstate = Playing;

	if (!IsDshow){
		time = VFF->Timecodes[numframe];
		lasttime = timeGetTime() - time;
		if (player){ player->Play(time, -1, false); }
		VFF->Play();
	}
	return true;
}

bool VideoRenderer::PlayLine(int start, int eend)
{
	int duration = GetDuration();
	if (vstate == None || start >= eend || start >= duration){ return false; }
	if (duration < eend){ eend = duration; }
	SetPosition(start, true, true, false);
	Play(eend);
	return true;
}

bool VideoRenderer::Pause()
{
	if (vstate == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate = Paused;
		if (!IsDshow){
			if (player){ player->Stop(false); }
		}
		else{
			vplayer->Pause();
		}
	}
	else if (vstate != None){
		Play();
	}
	else{ return false; }
	return true;
}

bool VideoRenderer::Stop()
{
	if (vstate == Playing){
		SetThreadExecutionState(ES_CONTINUOUS);
		vstate = Stopped;
		if (IsDshow){
			vplayer->Stop();
			playend = 0;
		}
		if (!IsDshow && player){
			player->Stop();
			//VFF->SetPosition(0, true);
			playend = GetDuration();
		}
		time = 0;
		return true;
	}
	return false;
}

void VideoRenderer::SetPosition(int _time, bool starttime/*=true*/, bool corect/*=true*/, bool async /*= true*/)
{

	if (IsDshow){
		bool playing = vstate == Playing;
		TabPanel* tab = (TabPanel*)GetParent();
		time = MID(0, _time, GetDuration());
		if (corect){
			time /= frameDuration;
			if (starttime){ time++; }
			time *= frameDuration;
		}
		//albo to przypadek albo ustawianie pozycji przed ustawianiem clipów jest rozwiązaniem dość częstego krasza
		//przy wielu plikach jednocześnie, był zawsze po seekingu
		playend = 0;
		seek = true;
		vplayer->SetPosition(time);
		if (hasVisualEdition){
			SAFE_DELETE(Visual->dummytext);
			if (Visual->Visual == VECTORCLIP){
				Visual->SetClip(Visual->GetVisual(), true, false, false);
			}
			else{
				OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
				if (vstate == Playing){ hasVisualEdition = false; }
			}
		}
		else if (hasDummySubs && tab->editor){
			OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
		}
	}
	else{
		if (vstate == Playing || !async)
			SetFFMS2Position(_time, starttime);
		else
			VFF->SetPosition(_time, starttime);
	}
}

void VideoRenderer::SetFFMS2Position(int _time, bool starttime){
	TabPanel* tab = (TabPanel*)GetParent();
	bool playing = vstate == Playing;
	numframe = VFF->GetFramefromMS(_time, (time > _time) ? 0 : numframe);
	if (!starttime){
		numframe--;
		if (VFF->Timecodes[numframe] >= _time){ numframe--; }
	}
	time = VFF->Timecodes[numframe];
	lasttime = timeGetTime() - time;
	playend = GetDuration();

	if (hasVisualEdition){
		SAFE_DELETE(Visual->dummytext);
		if (Visual->Visual == VECTORCLIP){
			Visual->SetClip(Visual->GetVisual(), true, false, false);
		}
		else{
			OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
			if (playing){ hasVisualEdition = false; }
		}
	}
	else if (hasDummySubs){
		OpenSubs((playing) ? tab->Grid->SaveText() : tab->Grid->GetVisible(), true, playing);
	}
	if (vstate == Playing){
		if (player){
			player->player->SetCurrentPosition(player->GetSampleAtMS(time));
		}
	}
	else{
		//rebuild spectrum cause position can be changed
		//and it causes random bugs
		if (player){ player->UpdateImage(false, true); }
		VFF->Render();
		RefreshTime();
	}
}

bool VideoRenderer::OpenSubs(wxString *textsubs, bool redraw, bool fromFile)
{
	wxCriticalSectionLocker lock(mutexRender);
	if (instance) csri_close(instance);
	instance = NULL;

	if (!textsubs) {
		if (redraw && vstate != None && IsDshow && datas){
			RecreateSurface();
		}
		hasDummySubs = true;
		return true;
	}

	if (hasVisualEdition && Visual->Visual == VECTORCLIP && Visual->dummytext){
		wxString toAppend = Visual->dummytext->Trim().AfterLast(L'\n');
		if (fromFile){
			OpenWrite ow(*textsubs, false);
			ow.PartFileWrite(toAppend);
			ow.CloseFile();
		}
		else{
			(*textsubs) << toAppend;
		}
	}

	hasDummySubs = !fromFile;

	wxScopedCharBuffer buffer = textsubs->mb_str(wxConvUTF8);
	int size = strlen(buffer);


	// Select renderer
	csri_rend *vobsub = Options.GetVSFilter();
	if (!vobsub){ KaiLog(_("CSRI odmówiło posłuszeństwa.")); delete textsubs; return false; }

	instance = (fromFile) ? csri_open_file(vobsub, buffer, NULL) : csri_open_mem(vobsub, buffer, size, NULL);
	if (!instance){ KaiLog(_("Instancja VobSuba nie utworzyła się.")); delete textsubs; return false; }

	if (!format || csri_request_fmt(instance, format)){
		KaiLog(_("CSRI nie obsługuje tego formatu."));
		csri_close(instance);
		instance = NULL;
		delete textsubs; return false;
	}

	if (redraw && vstate != None && IsDshow && datas){
		RecreateSurface();
	}

	delete textsubs;
	return true;
}

int VideoRenderer::GetCurrentPosition()
{
	return time;
}

int VideoRenderer::GetCurrentFrame()
{
	return numframe;
}

int VideoRenderer::GetFrameTime(bool start)
{
	if (VFF){
		if (start){
			int prevFrameTime = VFF->GetMSfromFrame(numframe - 1);
			return time + ((prevFrameTime - time) / 2);
		}
		else{
			if (numframe + 1 >= VFF->NumFrames){
				int prevFrameTime = VFF->GetMSfromFrame(numframe - 1);
				return time + ((time - prevFrameTime) / 2);
			}
			else{
				int nextFrameTime = VFF->GetMSfromFrame(numframe + 1);
				return time + ((nextFrameTime - time) / 2);
			}
		}
	}
	else{
		int halfFrame = (start) ? -(frameDuration / 2.0f) : (frameDuration / 2.0f) + 1;
		return time + halfFrame;
	}
}

void VideoRenderer::GetStartEndDelay(int startTime, int endTime, int *retStart, int *retEnd)
{
	if (!retStart || !retEnd){ return; }
	if (VFF){
		int frameStartTime = VFF->GetFramefromMS(startTime);
		int frameEndTime = VFF->GetFramefromMS(endTime, frameStartTime);
		*retStart = VFF->GetMSfromFrame(frameStartTime) - startTime;
		*retEnd = VFF->GetMSfromFrame(frameEndTime) - endTime;
	}
	else{
		int frameStartTime = (((float)startTime / 1000.f) * fps);
		int frameEndTime = (((float)endTime / 1000.f) * fps);
		frameStartTime++;
		frameEndTime++;
		*retStart = (((frameStartTime * 1000) / fps) + 0.5f) - startTime;
		*retEnd = (((frameEndTime * 1000) / fps) + 0.5f) - endTime;
	}
}

int VideoRenderer::GetFrameTimeFromTime(int _time, bool start)
{
	if (VFF){
		if (start){
			int frameFromTime = VFF->GetFramefromMS(_time);
			int prevFrameTime = VFF->GetMSfromFrame(frameFromTime - 1);
			int frameTime = VFF->GetMSfromFrame(frameFromTime);
			return frameTime + ((prevFrameTime - frameTime) / 2);
		}
		else{
			int frameFromTime = VFF->GetFramefromMS(_time);
			int nextFrameTime = VFF->GetMSfromFrame(frameFromTime + 1);
			int frameTime = VFF->GetMSfromFrame(frameFromTime);
			return frameTime + ((nextFrameTime - frameTime) / 2);
		}
	}
	else{
		int halfFrame = (start) ? -(frameDuration / 2.0f) : (frameDuration / 2.0f) + 1;
		return _time + halfFrame;
	}
}

int VideoRenderer::GetFrameTimeFromFrame(int frame, bool start)
{
	if (VFF){
		if (start){
			int prevFrameTime = VFF->GetMSfromFrame(frame - 1);
			int frameTime = VFF->GetMSfromFrame(frame);
			return frameTime + ((prevFrameTime - frameTime) / 2);
		}
		else{
			int nextFrameTime = VFF->GetMSfromFrame(frame + 1);
			int frameTime = VFF->GetMSfromFrame(frame);
			return frameTime + ((nextFrameTime - frameTime) / 2);
		}
	}
	else{
		int halfFrame = (start) ? -(frameDuration / 2.0f) : (frameDuration / 2.0f) + 1;
		return (frame * (1000.f / fps)) + halfFrame;
	}
}

int VideoRenderer::GetPlayEndTime(int _time)
{
	if (VFF){
		int frameFromTime = VFF->GetFramefromMS(_time);
		int prevFrameTime = VFF->GetMSfromFrame(frameFromTime - 1);
		return prevFrameTime;
	}
	else{
		int newTime = _time;
		newTime /= frameDuration;
		newTime = (newTime * frameDuration) + 1.f;
		if (_time == newTime && newTime % 10 == 0){ newTime -= 5; }
		return newTime;
	}
}

int VideoRenderer::GetDuration()
{
	if (IsDshow){ return vplayer->GetDuration(); }
	return VFF->Duration*1000.0;
}


//sets new rects after change video resolution
bool VideoRenderer::UpdateRects(bool changeZoom)
{

	VideoCtrl* Video = (VideoCtrl*) this;
	wxRect rt;
	TabPanel* tab = (TabPanel*)Video->GetParent();
	if (isFullscreen){
		hwnd = Video->TD->GetHWND();
		rt = Video->TD->GetClientRect();
		if (panelOnFullscreen){ rt.height -= Video->TD->panelsize; }
		pbar = Options.GetBool(VideoProgressBar);
		cross = false;
	}
	else{
		hwnd = GetHWND();
		rt = GetClientRect();
		rt.height -= panelHeight;
		pbar = false;
	}
	if (!rt.height || !rt.width){ return false; }

	windowRect.bottom = rt.height;
	windowRect.right = rt.width;
	windowRect.left = rt.x;
	windowRect.top = rt.y;

	/*if(tab->editor && !isFullscreen){
		backBufferRect=windowRect;
		}
		else
		{*/
	int arwidth = rt.height / AR;
	int arheight = rt.width * AR;

	if (arwidth > rt.width)
	{
		int onebar = (rt.height - arheight) / 2;
		//KaiLog(wxString::Format("onebar w %i, h %i, %i", onebar, rt.height, arheight));
		/*if(zoomParcent>1){
			int zoomARHeight = ((zoomRect.width - zoomRect.x)) * AR;
			onebar = (zoomRect.width - zoomRect.x > rt.width)? (rt.height - zoomARHeight)/2 : 0;
			wLogStatus("height %i %i %i, %i", zoomARHeight,arheight,rt.height,onebar);
			}*/
		backBufferRect.bottom = arheight + onebar;
		//if(backBufferRect.bottom % 2 != 0){backBufferRect.bottom++;}
		backBufferRect.right = rt.width;//zostaje bez zmian
		backBufferRect.left = 0;
		backBufferRect.top = onebar;
	}
	else if (arheight > rt.height)
	{
		int onebar = (rt.width - arwidth) / 2;
		//KaiLog(wxString::Format("onebar w %i, h %i, %i", onebar, rt.width, arwidth));
		/*if(zoomParcent>1){
			int zoomARWidth = ((zoomRect.height - zoomRect.y)) / AR;
			onebar = (zoomRect.height - zoomRect.y > rt.height)? (rt.width - zoomARWidth)/2 : 0;
			wLogStatus("width %i %i %i, %i", zoomARWidth,arwidth,rt.width,onebar);
			}*/
		backBufferRect.bottom = rt.height;//zostaje bez zmian
		backBufferRect.right = arwidth + onebar;
		//if(backBufferRect.right % 2 != 0){backBufferRect.right++;}
		backBufferRect.left = onebar;
		backBufferRect.top = 0;
	}
	else
	{
		//KaiLog(wxString::Format("equal %i %i", windowRect.right, windowRect.bottom));
		backBufferRect = windowRect;
	}
	//}
	if (/*zoomRect.width>0 && */changeZoom){
		wxSize s(backBufferRect.right, backBufferRect.bottom);
		float videoToScreenX = (float)s.x / (float)vwidth;
		float videoToScreenY = (float)s.y / (float)vheight;
		zoomRect.x = (mainStreamRect.left * videoToScreenX) + backBufferRect.left;
		zoomRect.y = (mainStreamRect.top * videoToScreenY) + backBufferRect.top;
		zoomRect.height = (mainStreamRect.bottom * videoToScreenY);
		zoomRect.width = (mainStreamRect.right * videoToScreenX);
		if (Visual){
			SetVisualZoom();
		}
	}
	return true;
}

//Changing resolution of video window
void VideoRenderer::UpdateVideoWindow()
{

	wxCriticalSectionLocker lock(mutexRender);
	/*block=true;*/
	if (!UpdateRects()){/*block=false;*/return; }

	if (!InitDX(true)){
		//need tests, if lost device return any error when reseting or not
		Clear();
		if (!InitDX()){
			return;
		}
	}

	if (IsDshow && datas){
		RecreateSurface();
	}


	resized = true;
	if (Visual){
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
		SAFE_DELETE(Visual->dummytext);
		Visual->SetCurVisual();
		hasVisualEdition = true;
	}
	SetScaleAndZoom();
	/*block=false;*/
}
void VideoRenderer::SetZoom(){
	if (vstate == None){ return; }
	hasZoom = !hasZoom;
	if (zoomRect.width < 1){ zoomRect = FloatRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom); }
	Render();
}

void VideoRenderer::ResetZoom()
{
	if (vstate == None){ return; }
	zoomRect = FloatRect(backBufferRect.left, backBufferRect.top, backBufferRect.right, backBufferRect.bottom);
	wxSize size(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	float videoToScreenXX = size.x / (float)vwidth;
	float videoToScreenYY = size.y / (float)vheight;
	mainStreamRect.left = (zoomRect.x - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.top = (zoomRect.y - backBufferRect.top) / videoToScreenYY;
	mainStreamRect.right = (zoomRect.width - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.bottom = (zoomRect.height - backBufferRect.top) / videoToScreenYY;
	zoomParcent = size.x / (zoomRect.width - zoomRect.x/* + backBufferRect.left*/);
	Render();
	SetScaleAndZoom();
}

void VideoRenderer::Zoom(const wxSize &size)
{
	//wxSize s1(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	hasZoom = true;
	float videoToScreenXX = size.x / (float)vwidth;
	float videoToScreenYY = size.y / (float)vheight;
	mainStreamRect.left = (zoomRect.x - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.top = (zoomRect.y - backBufferRect.top) / videoToScreenYY;
	mainStreamRect.right = (zoomRect.width - backBufferRect.left) / videoToScreenXX;
	mainStreamRect.bottom = (zoomRect.height - backBufferRect.top) / videoToScreenYY;
	zoomParcent = size.x / (zoomRect.width - zoomRect.x/* + backBufferRect.left*/);
	if (isFullscreen){ UpdateRects(false); }
	if (Visual){
		SetVisualZoom();
		if (Visual && (Visual->Visual < CLIPRECT || Visual->Visual > VECTORDRAW)){
			SAFE_DELETE(Visual->dummytext);
			Visual->SetCurVisual();
			hasVisualEdition = true;
		}
	}
	Render(false);
	SetScaleAndZoom();
}

void VideoRenderer::SetVisualZoom()
{
	float videoToScreenX = (float)(backBufferRect.right - backBufferRect.left) / (float)(vwidth);
	float videoToScreenY = (float)(backBufferRect.bottom - backBufferRect.top) / (float)(vheight);
	float zoomX = mainStreamRect.left * videoToScreenX;
	float zoomY = mainStreamRect.top * videoToScreenY;
	D3DXVECTOR2 zoomScale((float)vwidth / (float)(mainStreamRect.right - mainStreamRect.left),
		(float)vheight / (float)(mainStreamRect.bottom - mainStreamRect.top));
	Visual->SetZoom(D3DXVECTOR2(zoomX - (backBufferRect.left / zoomScale.x),
		zoomY - (backBufferRect.top / zoomScale.y)), zoomScale);
}

void VideoRenderer::OpenKeyframes(const wxString &filename)
{
	if (VFF){
		VFF->OpenKeyframes(filename);
		return;
	}
	else if (((TabPanel*)GetParent())->Edit->ABox){
		// skip return when audio do not have own provider or file didn't have video for take timecodes.
		if (((TabPanel*)GetParent())->Edit->ABox->OpenKeyframes(filename)){
			return;
		}
	}
	//if there is no FFMS2 or audiobox we store keyframes path;
	keyframesFileName = filename;
}

void VideoRenderer::DrawZoom()
{
	D3DXVECTOR2 v2[5];
	wxSize s(backBufferRect.right, backBufferRect.bottom);
	v2[0].x = zoomRect.x;
	v2[0].y = zoomRect.y;
	v2[1].x = v2[0].x;
	v2[1].y = zoomRect.height - 1;
	v2[2].x = zoomRect.width - 1;
	v2[2].y = v2[1].y;
	v2[3].x = v2[2].x;
	v2[3].y = v2[0].y;
	v2[4].x = v2[0].x;
	v2[4].y = v2[0].y;


	VERTEX v24[12];
	CreateVERTEX(&v24[0], 0, 0, 0x88000000);
	CreateVERTEX(&v24[1], s.x, 0, 0x88000000);
	CreateVERTEX(&v24[2], v2[2].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[3], v2[0].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[4], v2[0].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[5], 0, s.y, 0x88000000);
	CreateVERTEX(&v24[6], s.x, s.y, 0x88000000);
	CreateVERTEX(&v24[7], 0, s.y, 0x88000000);
	CreateVERTEX(&v24[8], v2[0].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[9], v2[2].x, v2[2].y, 0x88000000);
	CreateVERTEX(&v24[10], v2[2].x, v2[0].y, 0x88000000);
	CreateVERTEX(&v24[11], s.x, 0, 0x88000000);

	HRN(d3device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE), L"FVF failed");
	HRN(d3device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, v24, sizeof(VERTEX)), L"Primitive failed");
	HRN(d3device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, &v24[6], sizeof(VERTEX)), L"Primitive failed");
	lines->SetWidth(1);
	lines->Begin();
	lines->Draw(v2, 5, 0xFFBB0000);
	lines->End();

}

void VideoRenderer::ZoomMouseHandle(wxMouseEvent &evt)
{
	int x = evt.GetX();
	int y = evt.GetY();
	//wxWindow *win = this;
	VideoCtrl *vb = (VideoCtrl*)this;
	//if(isFullscreen){win = vb->TD; wxGetMousePosition(&x,&y);}

	wxSize s(backBufferRect.right, backBufferRect.bottom);
	wxSize s1(backBufferRect.right - backBufferRect.left, backBufferRect.bottom - backBufferRect.top);
	float ar = (float)s1.x / (float)s1.y;

	FloatRect tmp = zoomRect;
	//wxWindow *window = (isFullscreen)? (wxWindow*)((VideoCtrl*)this)->TD : this; 

	bool rotation = evt.GetWheelRotation() != 0;

	if (evt.ButtonUp()){
		if (HasCapture()){ ReleaseMouse(); }
		if (!vb->hasArrow){ SetCursor(wxCURSOR_ARROW); vb->hasArrow = true; }
	}


	if (!(evt.LeftDown() || evt.LeftIsDown())){
		bool setarrow = false;

		if (abs(x - zoomRect.x) < 5){
			setarrow = true;
			SetCursor(wxCURSOR_SIZEWE);
			vb->hasArrow = false;
		}
		if (abs(y - zoomRect.y) < 5){
			setarrow = true;
			SetCursor(wxCURSOR_SIZENS);
			vb->hasArrow = false;
		}
		if (abs(x - zoomRect.width) < 5){
			setarrow = true;
			SetCursor(wxCURSOR_SIZEWE);
			vb->hasArrow = false;
		}
		if (abs(y - zoomRect.height) < 5){
			setarrow = true;
			SetCursor(wxCURSOR_SIZENS);
			vb->hasArrow = false;
		}

		if (!setarrow && !vb->hasArrow){ SetCursor(wxCURSOR_ARROW); vb->hasArrow = true; }
	}
	if (evt.LeftDown()){
		if (!HasCapture()){ CaptureMouse(); }
		grabbed = -1;
		if (abs(x - zoomRect.x) < 5){
			zoomDiff.x = zoomRect.x - x;
			grabbed = 0;
		}
		else if (abs(y - zoomRect.y) < 5){
			zoomDiff.y = zoomRect.y - y;
			grabbed = 1;
		}
		else if (abs(x - zoomRect.width) < 5){
			zoomDiff.x = zoomRect.width - x;
			grabbed = 2;
		}
		else if (abs(y - zoomRect.height) < 5){
			zoomDiff.y = zoomRect.height - y;
			grabbed = 3;
		}
		else{
			zoomDiff.x = x - zoomRect.x;
			zoomDiff.y = y - zoomRect.y;
		}

	}
	else if (evt.LeftIsDown() || rotation){
		int minx = backBufferRect.left;
		int miny = backBufferRect.top;
		if (rotation){
			int step = 5 * evt.GetWheelRotation() / evt.GetWheelDelta();
			zoomRect.x -= step;
			zoomRect.y -= step / ar;
			zoomRect.width += step;
			zoomRect.height += step / ar;
		}
		else if (grabbed < 0){
			float oldzx = zoomRect.x;
			float oldzy = zoomRect.y;
			if (zoomRect.x >= minx && zoomRect.width < s.x || (zoomRect.width == s.x && zoomRect.x > x - zoomDiff.x)){
				zoomRect.x = x - zoomDiff.x;
			}
			if (zoomRect.y >= miny && zoomRect.height < s.y || (zoomRect.height == s.y && zoomRect.y > y - zoomDiff.y)){
				zoomRect.y = y - zoomDiff.y;
			}
			if (zoomRect.x >= minx && zoomRect.width <= s.x){
				zoomRect.width += (zoomRect.x - oldzx);
			}
			if (zoomRect.y >= miny && zoomRect.height <= s.y){
				zoomRect.height += (zoomRect.y - oldzy);
			}
			zoomRect.x = MID(minx, zoomRect.x, s.x);
			zoomRect.y = MID(miny, zoomRect.y, s.y);
			zoomRect.width = MIN(zoomRect.width, s.x);
			zoomRect.height = MIN(zoomRect.height, s.y);
			Zoom(s1);
			return;
		}
		else if (grabbed < 2){
			if (grabbed == 0){
				float oldzx = zoomRect.x;
				zoomRect.x = x - zoomDiff.x;
				//if(zoomRect.x<minx){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				zoomRect.y += (zoomRect.x - oldzx) / ar;
				zoomRect.width -= (zoomRect.x - oldzx);
				zoomRect.height -= (zoomRect.x - oldzx) / ar;
			}
			else{
				float oldzy = zoomRect.y;
				zoomRect.y = y - zoomDiff.y;
				//if(zoomRect.y<miny){zoomRect = FloatRect(minx, miny, s.x, s.y);Zoom(s);return;}
				zoomRect.x += (zoomRect.y - oldzy) * ar;
				zoomRect.height -= (zoomRect.y - oldzy);
				zoomRect.width -= (zoomRect.y - oldzy) * ar;
			}
		}
		else{
			//wLogStatus("zoom1 %f %f %f %f", zoomRect.x, zoomRect.y, zoomRect.width, zoomRect.height);
			if (grabbed == 2){
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.x < x - zoomDiff.x) ){return;}
				float oldzw = zoomRect.width;
				zoomRect.width = (x - zoomDiff.x);
				zoomRect.height += (zoomRect.width - oldzw) / ar;
			}
			else{
				//if(zoomRect.width - zoomRect.x < 100 || (zoomRect.width - zoomRect.x == 100 && zoomRect.y < y - zoomDiff.y) ){return;}
				float oldzh = zoomRect.height;
				zoomRect.height = (y - zoomDiff.y);
				zoomRect.width += (zoomRect.height - oldzh) * ar;
			}

		}
		////wLogStatus("zoom1 %f %f %f %f", zoomRect.x, zoomRect.y, zoomRect.width, zoomRect.height);
		if (zoomRect.width > s.x){
			zoomRect.x -= zoomRect.width - s.x;
			zoomRect.width = s.x;
		}
		if (zoomRect.height > s.y){
			zoomRect.y -= zoomRect.height - s.y;
			zoomRect.height = s.y;
		}
		if (zoomRect.x < minx){
			zoomRect.width -= (zoomRect.x - minx);
			zoomRect.x = minx;
		}
		if (zoomRect.y < miny){
			zoomRect.height -= (zoomRect.y - miny);
			zoomRect.y = miny;
		}

		zoomRect.width = MIN(zoomRect.width, s.x);
		zoomRect.height = MIN(zoomRect.height, s.y);
		zoomRect.x = MID(minx, zoomRect.x, s.x);
		zoomRect.y = MID(miny, zoomRect.y, s.y);
		if (zoomRect.width - zoomRect.x < 100){
			zoomRect = tmp;
		}
		Zoom(s1);
	}

}

void VideoRenderer::GetFpsnRatio(float *fps, long *arx, long *ary)
{
	if (IsDshow){ vplayer->GetFpsnRatio(fps, arx, ary); return; }
	*fps = VFF->fps;
	*arx = VFF->arwidth;
	*ary = VFF->arheight;
}

void VideoRenderer::GetVideoSize(int *width, int *height)
{
	if (IsDshow){
		wxSize sz = vplayer->GetVideoSize();
		*width = sz.x;
		*height = sz.y;
		return;
	}
	*width = VFF->width;
	*height = VFF->height;
}

wxSize VideoRenderer::GetVideoSize()
{
	wxSize sz;
	if (IsDshow){ sz = vplayer->GetVideoSize(); return sz; }
	sz.x = VFF->width;
	sz.y = VFF->height;
	return sz;
}

void VideoRenderer::DrawLines(wxPoint point)
{
	wxMutexLocker lock(mutexLines);
	int w, h;
	GetClientSize(&w, &h);
	w /= 2; h /= 2;
	crossRect.top = (h > point.y) ? point.y - 12 : point.y - 40;
	crossRect.bottom = (h > point.y) ? point.y + 23 : point.y - 5;
	crossRect.left = (w < point.x) ? point.x - 100 : point.x + 5;
	crossRect.right = (w < point.x) ? point.x - 5 : point.x + 100;

	vectors[0].x = point.x;
	vectors[0].y = 0;
	vectors[1].x = point.x;
	vectors[1].y = backBufferRect.bottom;
	vectors[2].x = 0;
	vectors[2].y = point.y;
	vectors[3].x = backBufferRect.right;
	vectors[3].y = point.y;
	cross = true;
	if (vstate == Paused && !block){
		Render(resized);
	}
}

void VideoRenderer::DrawProgBar()
{
	//pozycja zegara
	wxMutexLocker lock(mutexProgBar);
	int w, h;
	VideoCtrl *vb = (VideoCtrl*)this;
	vb->TD->GetClientSize(&w, &h);
	progressBarRect.top = 16;
	progressBarRect.bottom = 60;
	progressBarRect.left = w - 167;
	progressBarRect.right = w - 3;
	//koordynaty czarnej ramki
	vectors[4].x = w - 170;
	vectors[4].y = 5;
	vectors[5].x = w - 5;
	vectors[5].y = 5;
	vectors[6].x = w - 5;
	vectors[6].y = 15;
	vectors[7].x = w - 170;
	vectors[7].y = 15;
	vectors[8].x = w - 170;
	vectors[8].y = 5;
	//koordynaty białej ramki
	vectors[9].x = w - 169;
	vectors[9].y = 6;
	vectors[10].x = w - 6;
	vectors[10].y = 6;
	vectors[11].x = w - 6;
	vectors[11].y = 14;
	vectors[12].x = w - 169;
	vectors[12].y = 14;
	vectors[13].x = w - 169;
	vectors[13].y = 6;
	//koordynaty paska postępu
	int rw = w - 168;
	vectors[14].x = rw;
	vectors[14].y = 10.5;
	vectors[15].x = (GetDuration() > 0) ? (((float)time / (float)GetDuration()) * 161) + rw : 161 + rw;
	vectors[15].y = 10.5;
}

void VideoRenderer::SetVolume(int vol)
{
	if (vstate == None){ return; }
	if (!IsDshow){
		vol = 7600 + vol;
		double dvol = vol / 7600.0;
		int sliderValue = (dvol * 99) + 1;
		TabPanel *tab = (TabPanel*)GetParent();
		if (tab->Edit->ABox){
			tab->Edit->ABox->SetVolume(sliderValue);
		}
		//float value = pow(float(sliderValue)/50.0f,3);
		//if(player){player->player->SetVolume(value);}
	}
	else
	{
		vplayer->SetVolume(vol);
	}
}

int VideoRenderer::GetVolume()
{
	if (vstate == None){ return 0; }
	if (!IsDshow && player){
		double dvol = player->player->GetVolume();
		dvol = sqrt(dvol);
		dvol *= 8100.0;
		dvol -= 8100.0;
		return dvol;
	}
	else if (IsDshow){
		return vplayer->GetVolume();
	}
	return 0;
}

void VideoRenderer::ChangePositionByFrame(int step)
{
	if (vstate == Playing || vstate == None){ return; }
	if (!IsDshow){
		//if (!VFF->isBusy){
		numframe = MID(0, numframe + step, VFF->NumFrames - 1);
		time = VFF->Timecodes[numframe];
		TabPanel* pan = (TabPanel*)GetParent();
		if (hasVisualEdition || hasDummySubs){
			OpenSubs(pan->Grid->SaveText(), false, true);
			hasVisualEdition = false;
		}
		if (player){ player->UpdateImage(true, true); }
		Render(true, false);
		//}
	}
	else{
		time += (frameDuration * step);
		SetPosition(time, true, false);
	}
	VideoCtrl *vb = (VideoCtrl*)this;
	vb->RefreshTime();

}


wxArrayString VideoRenderer::GetStreams()
{
	if (vplayer){ return vplayer->GetStreams(); }
	wxArrayString streams;
	return streams;
}

void VideoRenderer::EnableStream(long index)
{
	if (vplayer->stream){
		seek = true;
		auto hr = vplayer->stream->Enable(index, AMSTREAMSELECTENABLE_ENABLE);
		if (FAILED(hr)){
			KaiLog(L"Cannot change stream");
		}
	}
}



void VideoRenderer::ChangeVobsub(bool vobsub)
{
	if (!vplayer){ return; }
	kainoteApp *Kaia = (kainoteApp*)wxTheApp;

	int tmptime = time;
	TabPanel *pan = Kaia->Frame->GetTab();
	OpenSubs((vobsub) ? NULL : pan->Grid->SaveText(), true, true);
	vplayer->OpenFile(pan->VideoPath, vobsub);
	vformat = vplayer->inf.CT;
	D3DFORMAT tmpd3dformat = (vformat == 5) ? D3DFORMAT('21VN') : (vformat == 3) ? D3DFORMAT('21VY') :
		(vformat == 2) ? D3DFMT_YUY2 : D3DFMT_X8R8G8B8;
	swapFrame = (vformat == 0 && !vplayer->HasVobsub());
	if (tmpd3dformat != d3dformat){
		d3dformat = tmpd3dformat;
		int tmppitch = vwidth * vplayer->inf.bytes;
		if (tmppitch != pitch){
			pitch = tmppitch;
			if (datas){ delete[] datas; datas = NULL; }
			datas = new char[vheight * pitch];
		}
		UpdateVideoWindow();
	}
	SetPosition(tmptime);
	if (vstate == Paused){ vplayer->Play(); vplayer->Pause(); }
	else if (vstate == Playing){ vplayer->Play(); }
	int pos = pan->Video->volslider->GetValue();
	SetVolume(-(pos * pos));
	pan->Video->ChangeStream();
}

void VideoRenderer::SetVisual(bool remove/*=false*/, bool settext/*=false*/, bool noRefresh /*= false*/)
{
	TabPanel* pan = (TabPanel*)GetParent();

	if (remove){
		SAFE_DELETE(Visual); pan->Edit->Visual = 0;
		hasVisualEdition = false;
		if (!noRefresh){
			OpenSubs(pan->Grid->GetVisible());
			Render();
		}
	}
	else{

		int vis = pan->Edit->Visual;
		if (!Visual){
			Visual = Visuals::Get(vis, this);
		}
		else if (Visual->Visual != vis){
			bool vectorclip = Visual->Visual == VECTORCLIP;
			delete Visual;
			Visual = Visuals::Get(vis, this);
			if (vectorclip && !settext){ OpenSubs(pan->Grid->GetVisible()); }
		}
		else{ SAFE_DELETE(Visual->dummytext); }
		if (settext){ OpenSubs(pan->Grid->GetVisible()); }
		Visual->SizeChanged(wxRect(backBufferRect.left, backBufferRect.top,
			backBufferRect.right, backBufferRect.bottom), lines, m_font, d3device);
		SetVisualZoom();
		Visual->SetVisual(pan->Edit->line->Start.mstime, pan->Edit->line->End.mstime,
			pan->Edit->line->IsComment, noRefresh);
		hasVisualEdition = true;
	}
}

void VideoRenderer::ResetVisual()
{
	SAFE_DELETE(Visual->dummytext);
	Visual->SetCurVisual();
	hasVisualEdition = true;
	Render();
}

bool VideoRenderer::EnumFilters(Menu *menu)
{
	if (vplayer){ return vplayer->EnumFilters(menu); }
	return false;
}

bool VideoRenderer::FilterConfig(wxString name, int idx, wxPoint pos)
{
	if (vplayer){ return vplayer->FilterConfig(name, idx, pos); }
	return false;
}

byte *VideoRenderer::GetFramewithSubs(bool subs, bool *del)
{
	bool dssubs = (IsDshow && subs && Notebook::GetTab()->editor);
	bool ffnsubs = (!IsDshow && !subs);
	byte *cpy1;
	byte bytes = (vformat == RGB32) ? 4 : (vformat == YUY2) ? 2 : 1;
	int all = vheight*pitch;
	if (dssubs || ffnsubs){
		*del = true;
		char *cpy = new char[all];
		cpy1 = (byte*)cpy;
	}
	else{ *del = false; }
	if (ffnsubs){
		VFF->GetFrame(time, cpy1);
	}
	else if (instance && dssubs){
		byte *data1 = (byte*)datas;
		memcpy(cpy1, data1, all);
		framee->strides[0] = vwidth * bytes;
		framee->planes[0] = cpy1;
		csri_render(instance, framee, (time / 1000.0));
	}
	return (dssubs || ffnsubs) ? cpy1 : (byte*)datas;
}

void VideoRenderer::GoToNextKeyframe()
{
	if (!VFF){ return; }
	for (size_t i = 0; i < VFF->KeyFrames.size(); i++){
		if (VFF->KeyFrames[i] > time){
			SetPosition(VFF->KeyFrames[i]);
			return;
		}
	}
	SetPosition(VFF->KeyFrames[0]);
}
void VideoRenderer::GoToPrevKeyframe()
{
	if (!VFF){ return; }
	for (int i = VFF->KeyFrames.size() - 1; i >= 0; i--){
		if (VFF->KeyFrames[i] < time){
			SetPosition(VFF->KeyFrames[i]);
			return;
		}
	}
	SetPosition(VFF->KeyFrames[VFF->KeyFrames.size() - 1]);
}

//bool VideoRenderer::GetStartEndDurationFromMS(Dialogue *dial, STime &duration)
//{
//	duration = dial->End - dial->Start;
//	if (VFF){
//		//start frame
//		dial->Start.orgframe = VFF->GetFramefromMS(dial->Start.mstime);
//		//end frame
//		dial->End.orgframe = VFF->GetFramefromMS(dial->End.mstime);
//		dial->End.orgframe--;
//		//duration frame
//		duration.orgframe = dial->End.orgframe - dial->Start.orgframe + 1;
//	}
//}