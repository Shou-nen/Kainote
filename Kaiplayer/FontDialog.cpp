﻿
#include "FontDialog.h"
#include <wx/fontenum.h>
#include <wx/regex.h>
#include "Config.h"
#include "SubsGrid.h"

bool compare(wxString first, wxString second)
{
	return (first.CmpNoCase(second)<0);
}

FontList::FontList(wxWindow *parent,long id,const wxPoint &pos, const wxSize &size)
	:wxWindow(parent,id,pos,size)
{
	scrollBar = new wxScrollBar(this,ID_SCROLL1,wxDefaultPosition,wxDefaultSize,wxSB_VERTICAL);
	scrollBar->SetScrollbar(0,10,100,10);
	fonts = wxFontEnumerator::GetFacenames();
	std::sort(fonts.begin(),fonts.end(),compare);

	font= wxFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma",wxFONTENCODING_DEFAULT);

	wxClientDC dc(this);
	dc.SetFont(font);
	int fw,fh;
	dc.GetTextExtent("#TWFfGH", &fw, &fh, NULL, NULL, &font);
	Height=fh+4;

	bmp=NULL;
	scPos=0;
	sel=0;
	holding=false;
	Refresh();

}

FontList::~FontList(){
	if(bmp)delete bmp; bmp=0;
}



void FontList::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	int w = 0;
	int h = 0;
	int sw=0;
	int sh=0;
	GetClientSize(&w,&h);
	scrollBar->GetSize(&sw,&sh);
	scrollBar->SetSize(w-sw,0,sw,h);
	w -= sw;

	bool direct = false;

	if (direct) {
		DrawFld(dc,w,h);
	}

	else {

		if (bmp) {
			if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
				delete bmp;
				bmp = NULL;
			}
		}
		if (!bmp) bmp = new wxBitmap(w,h);

		// Draw bitmap
		wxMemoryDC bmpDC;
		bmpDC.SelectObject(*bmp);
		DrawFld(bmpDC,w,h);
		dc.Blit(0,0,w,h,&bmpDC,0,0);
	}

}

void FontList::DrawFld(wxDC &dc,int w, int h)
{
	int fw=0,fh=0,posX=1,posY=1;

	dc.SetPen(wxPen(wxColour("#808080")));
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	dc.DrawRectangle(0,0,w,h);


	int panelrows=(h/Height)+1;
	int scrows;
	if((scPos+panelrows)>=(int)fonts.size()+1){scrows=fonts.size();scPos=(scrows-panelrows)+1;
	if(panelrows>(int)fonts.size()){scPos=0;scrollBar->Enable(false);}else{scrollBar->SetScrollbar(scPos,panelrows,fonts.size()+1,panelrows-1);}}
	else{scrows=(scPos+panelrows);scrollBar->Enable(true);scrollBar->SetScrollbar(scPos,panelrows,fonts.size()+1,panelrows-1);}

	dc.SetPen(wxPen(wxColour("#000000")));

	for(int i=scPos; i<scrows; i++)
	{
		if(i==sel){
			dc.SetBrush(wxBrush(wxColour("#359AFF")));
			dc.DrawRectangle(posX,posY,w-2,Height);
		}else{dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));}

		font.SetFaceName(fonts[i]);
		dc.SetFont(font);

		wxRect cur(posX+4,posY,w-8,Height);	
		dc.SetClippingRegion(cur);
		dc.DrawLabel(fonts[i],cur,wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
		dc.DestroyClippingRegion();

		posY+=Height;
	}

}

void FontList::OnSize(wxSizeEvent& event)
{
	Refresh(false);
}

void FontList::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();

	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void FontList::OnMouseEvent(wxMouseEvent& event)
{
	int w,h;
	GetClientSize (&w, &h);
	bool click = event.LeftDown();
	bool left_up = event.LeftUp();
	bool dclick = event.LeftDClick();
	int curY=(event.GetY());
	int row = (curY / Height)+scPos;

	if (left_up && !holding) {
		return;
	}

	// Get focus
	if (event.ButtonDown()) {
		SetFocus();}

	if (event.GetWheelRotation() != 0) {
		int step = 3 * event.GetWheelRotation() / event.GetWheelDelta();
		scPos -= step;
		Refresh(false);
		return;
	}

	if (left_up && holding) {
		holding = false;
		ReleaseMouse();
	}

	if (row < scPos || row >= (int)fonts.size()) {
		return;
	}

	if (click) {
		holding = true;
		sel=row;
		wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
		AddPendingEvent(evt);
		Refresh(false);
		CaptureMouse();
	}

	

	if (holding) {
		// Find direction
		int minVis = scPos+1;
		int maxVis = scPos+h/Height-1;
		int delta = 0;
		if (row < minVis && row!=0) delta = -1;
		if (row > maxVis) delta = +1;

		if (delta) {
			scPos=(MID(row - (h / Height), scPos + delta, row));
			Refresh(false);
			// End the hold if this was a mousedown to avoid accidental
			// selection of extra lines
			if (click) {
				holding = false;
				left_up = true;
				ReleaseMouse();
			}
		}

		if(sel!=row){
			sel=row;
			Refresh(false);
			wxCommandEvent evt(wxEVT_COMMAND_LISTBOX_SELECTED, GetId());
			AddPendingEvent(evt);
		}
	}


}

void FontList::Insert(wxString facename,int pos)
{
	fonts.Insert(facename,pos);
}

void FontList::SetSelection(int pos)
{
	if(pos<0 || pos>=(int)fonts.size()){wxBell();return;}
	if (scPos<pos || scPos>pos+7){scPos -= (sel-pos);}
	sel=pos;
	
	Refresh(false);
}

void FontList::SetSelectionByName(wxString name)
{
	int sell=fonts.Index(name,false);
	if(sell!=-1){SetSelection(sell);}
}


void FontList::SetSelectionByPartialName(wxString PartialName)
{
	if(PartialName==""){SetSelection(0);return;}
	int sell=-1;
	PartialName=PartialName.Lower();
	
	for(size_t i=0; i<fonts.size(); i++){
		wxString fontname = fonts[i].Lower();
		
		if(fontname.StartsWith(PartialName)){
			sell=i;
			break;
		}
	}

	if(sell!=-1){
		SetSelection(sell);
	}
}

wxString FontList::GetString(int line)
{
	return fonts[line];
}

int FontList::GetSelection()
{
	return sel;
}

void FontList::Scroll(int step)
{
	sel+=step;
	scPos+=step;
	if (scPos<sel-7 || scPos>sel){scPos=sel;}
	Refresh(false);
}

BEGIN_EVENT_TABLE(FontList,wxWindow)
	EVT_PAINT(FontList::OnPaint)
	EVT_SIZE(FontList::OnSize)
	EVT_COMMAND_SCROLL(ID_SCROLL1,FontList::OnScroll)
	EVT_MOUSE_EVENTS(FontList::OnMouseEvent)
	//EVT_KEY_DOWN(FontList::OnKeyPress)
END_EVENT_TABLE()

	FontDialog::FontDialog(wxWindow *parent, Styles *acst)
	:wxDialog(parent,-1,_("Wybierz czcionkę"))
{

	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_NORMAL, WXK_RETURN, wxID_OK);
	entries[1].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
	entries[2].Set(wxACCEL_NORMAL, WXK_UP,ID_SCROLLUP);
	entries[3].Set(wxACCEL_NORMAL, WXK_DOWN,ID_SCROLLDOWN);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);


	wxBoxSizer *Main= new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *Cfont= new wxStaticBoxSizer(wxHORIZONTAL,this,_("Czcionka"));
	wxStaticBoxSizer *prev= new wxStaticBoxSizer(wxVERTICAL,this,_("Podgląd"));
	wxBoxSizer *Fattr= new wxBoxSizer(wxVERTICAL);
	//wxBoxSizer *Flist= new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *Bsizer= new wxBoxSizer(wxHORIZONTAL);
	FontName = new wxTextCtrl(this,ID_FONTNAME,acst->Fontname,wxDefaultPosition,wxSize(150,-1),wxTE_PROCESS_ENTER);

	Fonts= new FontList(this,ID_FONTLIST,wxDefaultPosition,wxSize(180,-1));
	Fonts->SetSelectionByName(acst->Fontname);
	//Flist->Add(FontName,0,wxEXPAND|wxBOTTOM,3);
	//Flist->Add(Fonts,0,wxEXPAND);

	Preview = new StylePreview(this,-1,wxDefaultPosition, wxSize(-1,110));
	Preview->DrawPreview(acst);
	FontSize= new NumCtrl(this,ID_FONTSIZE1,acst->Fontsize,1,10000,false,wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	Bold= new wxCheckBox(this,ID_FONTATTR,_("Pogrubienie"));
	Bold->SetValue(acst->Bold);
	Italic= new wxCheckBox(this,ID_FONTATTR,_("Kursywa"));
	Italic->SetValue(acst->Italic);
	Underl= new wxCheckBox(this,ID_FONTATTR,_("Podkreślenie"));
	Underl->SetValue(acst->Underline);
	Strike= new wxCheckBox(this,ID_FONTATTR,_("Przekreślenie"));
	Strike->SetValue(acst->StrikeOut);
	Buttok= new wxButton(this,wxID_OK,"OK");
	Buttcancel= new wxButton(this,wxID_CANCEL,_("Anuluj"));
	Fattr->Add(FontName,0,wxEXPAND|wxLEFT|wxRIGHT,5);
	Fattr->Add(FontSize,0,wxEXPAND|wxALL,5);
	Fattr->Add(Bold,1,wxEXPAND|wxALL,5);
	Fattr->Add(Italic,1,wxEXPAND|wxALL,5);
	Fattr->Add(Underl,1,wxEXPAND|wxALL,5);
	Fattr->Add(Strike,1,wxEXPAND|wxALL,5);

	Cfont->Add(Fonts,0,wxEXPAND);
	Cfont->Add(Fattr,0,wxEXPAND);

	prev->Add(Preview,0,wxEXPAND|wxALL,5);

	Bsizer->Add(Buttok,0,wxALL,5);
	Bsizer->Add(Buttcancel,0,wxALL,5);

	Main->Add(Cfont,0,wxEXPAND|wxALL,5);
	Main->Add(prev,0,wxEXPAND|wxBOTTOM|wxLEFT|wxRIGHT,5);
	Main->Add(Bsizer,0,wxBOTTOM|wxALIGN_CENTER,5);

	SetSizerAndFit(Main);

	CenterOnParent();
	UpdatePreview();

	Connect(ID_FONTLIST,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&FontDialog::OnFontChanged);
	Connect(ID_FONTNAME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FontDialog::OnUpdateText);
	Connect(ID_FONTSIZE1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_FONTATTR,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FontDialog::OnUpdatePreview);
	Connect(ID_SCROLLUP,ID_SCROLLDOWN,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&FontDialog::OnScrollList);

}

FontDialog::~FontDialog()
{
	//Options.SetString("Preview Text",Preview->GetValue());
}

Styles *FontDialog::GetFont()
{
	Styles *val=new Styles();
	val->Bold=Bold->GetValue();
	val->Italic=Italic->GetValue();
	val->Underline=Underl->GetValue();
	val->StrikeOut=Strike->GetValue();
	val->Fontname=Fonts->GetString(Fonts->GetSelection());
	wxString kkk;
	kkk<<FontSize->GetValue();
	if(kkk!=""){val->Fontsize=kkk;}
	return val;
}

void FontDialog::OnFontChanged(wxCommandEvent& event)
{
	FontName->ChangeValue(Fonts->GetString(Fonts->GetSelection()));
	UpdatePreview();
}

void FontDialog::OnUpdatePreview(wxCommandEvent& event)
{
	UpdatePreview();
}

void FontDialog::UpdatePreview()
{
	Styles *styl=GetFont();
	Preview->DrawPreview(styl);
	delete styl;
}

void FontDialog::OnUpdateText(wxCommandEvent& event)
{
	Fonts->SetSelectionByPartialName(FontName->GetValue());
}

void FontDialog::OnScrollList(wxCommandEvent& event)
{

	int step=(event.GetId()==ID_SCROLLUP)? -1 : 1;
	Fonts->Scroll(step);
	//wxLogStatus("weszło");
	FontName->ChangeValue(Fonts->GetString(Fonts->GetSelection()));
	UpdatePreview();
}