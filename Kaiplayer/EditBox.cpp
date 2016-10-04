﻿
#include "EditBox.h"
#include "Grid.h"
#include "KainoteApp.h"
#include <wx/regex.h>
#include <wx/tglbtn.h>
#include "FontDialog.h"
#include "ColorPicker.h"
#include "Visuals.h"

BEGIN_EVENT_TABLE(EBStaticText,wxStaticText)
	EVT_ERASE_BACKGROUND(EBStaticText::OnEraseBackground)
	END_EVENT_TABLE()

	EBStaticText::EBStaticText(wxWindow *parent, const wxString &txt, const wxSize &size)
	:wxStaticText(parent,9990,txt, wxDefaultPosition, size)
{
}

void EBStaticText::OnEraseBackground(wxEraseEvent &event)
{
}

DescTxtCtrl::DescTxtCtrl(wxWindow *parent, const wxSize &size, const wxString &desc)
	:wxTextCtrl(parent,-1,"",wxDefaultPosition, size)
{
	description =desc;
}
void DescTxtCtrl::ChangeValue(wxString &val)
{
	if(val=="" && !HasFocus()){
		SetForegroundColour("#A0A0A0"); 
		wxTextCtrl::SetValue(description);
	}
	else{
		SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		wxTextCtrl::ChangeValue(val);
	}
}

void DescTxtCtrl::OnFocus(wxFocusEvent &evt)
{
	if(GetForegroundColour()=="#A0A0A0"){SetValue("");SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));}
	evt.Skip();
}
void DescTxtCtrl::OnKillFocus(wxFocusEvent &evt)
{
	if(GetValue()==""){SetForegroundColour("#A0A0A0"); SetValue(description);}
	evt.Skip();
}

BEGIN_EVENT_TABLE(DescTxtCtrl,wxTextCtrl)
	EVT_SET_FOCUS(DescTxtCtrl::OnFocus)
	EVT_KILL_FOCUS(DescTxtCtrl::OnKillFocus)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(TagButton, wxButton)
	EVT_MOUSE_EVENTS(TagButton::OnMouseEvent)
END_EVENT_TABLE()

	txtdialog::txtdialog(wxWindow *parent, int id, const wxString &txtt, int _type, const wxPoint &position)
	:wxDialog(parent,id,_("Wpisz tag ASS"),position)
{
	wxBoxSizer *siz=new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *siz1=new wxBoxSizer(wxHORIZONTAL);
	wxString types[3]={_("Tag wstawiany w miejse kursora"), _("Tag wstawiany na początku tekstu"), _("Zwykły tekst")};
	type=new wxChoice(this,-1,wxDefaultPosition, wxDefaultSize,3,types);
	type->SetSelection(_type);
	txt=new wxTextCtrl(this,-1,txtt,wxDefaultPosition,wxSize(150,25), wxTE_PROCESS_ENTER);
	txt->SetSelection(0,txtt.Len()-1);
	siz->Add(type,0,wxEXPAND|wxALL,4);
	siz->Add(txt,0,wxEXPAND|wxLEFT|wxRIGHT,4);
	siz1->Add(new wxButton(this, wxID_OK,_("Zapisz tag")),0,wxEXPAND|wxALL,4);
	siz1->Add(new wxButton(this, wxID_CANCEL,_("Anuluj")),0,wxEXPAND|wxALL,4);
	siz->Add(siz1,0,wxEXPAND,0);
	SetSizerAndFit(siz);
}

TagButton::TagButton(wxWindow *parent, int id, const wxString &name, const wxSize &size)
	: wxButton(parent,id,name,wxDefaultPosition,size)
{
	wxString rest;
	type=0;
	tag=Options.GetString(wxString::Format("Editbox tag button%i",GetId()-15000)).BeforeFirst('\f', &rest);
	if(tag!=""){SetToolTip(tag);type=wxAtoi(rest);}
}

void TagButton::OnMouseEvent(wxMouseEvent& event)
{
	if(event.RightUp()||(tag=="" && event.LeftDown())){
		tagtxt=new txtdialog(this,-1,tag,0,ClientToScreen(event.GetPosition()));
		if(tagtxt->ShowModal()==wxID_OK){
			tag=tagtxt->txt->GetValue();
			type=tagtxt->type->GetSelection();
			wxString svtag = tag;
			Options.SetString(wxString::Format("Editbox tag button%i",GetId()-15000),(tag!="")? svtag<<"\f"<<type : svtag);
			Options.SaveOptions(true,false);
			if(tag!=""){SetToolTip(tag);}
		}
		tagtxt->Destroy();
		return;
	}

	event.Skip();
}



EditBox::EditBox(wxWindow *parent, Grid *grid1, kainoteFrame* kaif,int idd)
	: wxWindow(parent, idd, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxCLIP_CHILDREN)
{

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	ebrow=0;
	grid=grid1;
	grid->Edit=this;
	isdetached=OnVideo=splittedTags=false;
	Visual=0;
	ABox=NULL;
	line=NULL;

	wxArrayString ans;
	ans.Add("an1");
	ans.Add("an2");
	ans.Add("an3");
	ans.Add("an4");
	ans.Add("an5");
	ans.Add("an6");
	ans.Add("an7");
	ans.Add("an8");
	ans.Add("an9");

	Bfont = new wxButton(this, ID_FONT, "", wxDefaultPosition, wxSize(26,26));
	Bfont->SetBitmap(wxBITMAP_PNG ("FONTS"));
	Bcol1 = new wxButton(this, ID_COL1, "", wxDefaultPosition, wxSize(26,26));
	Bcol1->SetBitmap(wxBITMAP_PNG ("Kolor1"));
	Bcol2 = new wxButton(this, ID_COL2, "", wxDefaultPosition, wxSize(26,26));
	Bcol2->SetBitmap(wxBITMAP_PNG ("Kolor2"));
	Bcol3 = new wxButton(this, ID_COL3, "", wxDefaultPosition, wxSize(26,26));
	Bcol3->SetBitmap(wxBITMAP_PNG ("Kolor3"));
	Bcol4 = new wxButton(this, ID_COL4, "", wxDefaultPosition, wxSize(26,26));
	Bcol4->SetBitmap(wxBITMAP_PNG ("Kolor4"));
	Bbold = new wxButton(this, PutBold, "", wxDefaultPosition, wxSize(26,26));
	Bbold->SetBitmap(wxBITMAP_PNG ("BOLD"));
	Bital = new wxButton(this, PutItalic, "", wxDefaultPosition, wxSize(26,26));
	Bital->SetBitmap(wxBITMAP_PNG ("ITALIC"));
	Bund = new wxButton(this, ID_UND, "", wxDefaultPosition, wxSize(26,26));
	Bund->SetBitmap(wxBITMAP_PNG ("UNDER"));
	Bstrike = new wxButton(this, ID_STRIKE, "", wxDefaultPosition, wxSize(26,26));
	Bstrike->SetBitmap(wxBITMAP_PNG ("STRIKE"));
	Ban = new wxChoice(this, ID_AN, wxDefaultPosition, wxSize(48,24),ans, wxNO_FULL_REPAINT_ON_RESIZE);
	Ban->Select(1);

	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer4->Add(Bfont,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bbold,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bital,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bund,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bstrike,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bcol1,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bcol2,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bcol3,0,wxLEFT|wxBOTTOM,2);
	BoxSizer4->Add(Bcol4,0,wxLEFT|wxBOTTOM|wxRIGHT,2);
	BoxSizer4->Add(Ban,0,wxTOP,1);
	for(int i=0; i<Options.GetInt("Editbox tag buttons"); i++)
	{
		BoxSizer4->Add(new TagButton(this,15000+i,wxString::Format("T%i",i+1),wxSize(26,26)),0,wxLEFT|wxBOTTOM,2);
		Connect(15000+i,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnButtonTag);
	}


	TlMode= new wxCheckBox(this,ID_TLMODE,_("Tryb tłumaczenia"));
	TlMode->SetValue(false);
	TlMode->Enable(false);
	Chars = new EBStaticText(this,_("Linie: 0/86"));
	Chtime = new EBStaticText(this,_("Znaki na sekundę: 0<=15"));
	Times = new wxRadioButton(this,ID_TIMES_FRAMES,_("Czas"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP); 
	Times->SetValue(true);
	Times->Enable(false);
	Frames = new wxRadioButton(this,ID_TIMES_FRAMES,_("Klatki")); 
	Frames->Enable(false);
	Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &EditBox::OnChangeTimeDisplay, this, ID_TIMES_FRAMES);

	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer5->Add(Chars,0,wxALIGN_CENTER|wxLEFT|wxEXPAND,2);
	BoxSizer5->Add(Chtime,0,wxALIGN_CENTER|wxLEFT|wxEXPAND,6);
	BoxSizer5->Add(TlMode,0,wxALIGN_CENTER|wxLEFT,6);
	BoxSizer5->Add(Times,0,wxALIGN_CENTER|wxLEFT,2);
	BoxSizer5->Add(Frames,0,wxALIGN_CENTER|wxLEFT,2);


	Bcpall = new wxButton(this, ID_CPALL, _("Wklej wszystko"));
	Bcpall->Hide();
	Bcpsel = new wxButton(this, ID_CPSEL, _("Wklej zaznaczone"));
	Bcpsel->Hide();
	Bhide = new wxButton(this, ID_HIDE, _("Ukryj oryginał"));
	Bhide->Hide();
	AutoMoveTags = new wxToggleButton(this, ID_AUTOMOVETAGS, _("Przenoszenie tagów"));
	AutoMoveTags->Hide();

	BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer6->Add(Bcpsel,0,wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM,2);
	BoxSizer6->Add(Bcpall,0,wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM,2);
	BoxSizer6->Add(Bhide,0,wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM,2);
	BoxSizer6->Add(AutoMoveTags,0,wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM,2);


	TextEdit = new MTextEditor(this, 16667, Options.GetBool("Editbox Spellchecker"), wxDefaultPosition, wxSize(-1,30));
	TextEdit->EB=this;

	TextEditTl = new MTextEditor(this, 16667, false, wxDefaultPosition, wxSize(-1,30));
	TextEditTl->EB=this;
	TextEditTl->Hide();
	Comment = new wxCheckBox(this, ID_CHECKBOX1, _("Komentarz"), wxDefaultPosition, wxSize(82,-1));
	Comment->SetValue(false);
	LayerEdit = new NumCtrl(this, 16668, "",-10000000,10000000,true, wxDefaultPosition, wxSize(50,-1));
	StartEdit = new TimeCtrl(this, 16668, "", wxDefaultPosition, wxSize(87,-1),wxTE_CENTRE);
	EndEdit = new TimeCtrl(this, 16668, "", wxDefaultPosition, wxSize(87,-1),wxTE_CENTRE);
	DurEdit = new TimeCtrl(this, 16668, "", wxDefaultPosition, wxSize(87,-1),wxTE_CENTRE);
	wxArrayString styles;
	styles.Add("Default");
	StyleChoice = new wxChoice(this, IDSTYLE, wxDefaultPosition, wxSize(100,-1),styles);//wxSize(145,-1)
	//druga linia

	ActorEdit = new DescTxtCtrl(this, wxSize(90,-1), _("Aktor"));
	MarginLEdit = new NumCtrl(this, 16668, "",0,9999,true, wxDefaultPosition, wxSize(42,-1),wxTE_CENTRE);
	MarginREdit = new NumCtrl(this, 16668, "",0,9999,true, wxDefaultPosition, wxSize(42,-1),wxTE_CENTRE);
	MarginVEdit = new NumCtrl(this, 16668, "",0,9999,true, wxDefaultPosition, wxSize(42,-1),wxTE_CENTRE);
	EffectEdit = new DescTxtCtrl(this, wxSize(90,-1), _("Efekt"));

	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer2->Add(Comment,0,wxLEFT|wxALIGN_CENTER,4);
	BoxSizer2->Add(LayerEdit,0,wxLEFT,2);
	BoxSizer2->Add(StartEdit,0,wxLEFT,2);
	BoxSizer2->Add(EndEdit,0,wxLEFT,2);
	BoxSizer2->Add(DurEdit,0,wxLEFT,2);
	BoxSizer2->Add(StyleChoice,4,wxLEFT|wxEXPAND,2);
	BoxSizer2->Add(ActorEdit,3,wxLEFT|wxEXPAND,2);
	BoxSizer2->Add(MarginLEdit,0,wxLEFT,2);
	BoxSizer2->Add(MarginREdit,0,wxLEFT,2);
	BoxSizer2->Add(MarginVEdit,0,wxLEFT,2);
	BoxSizer2->Add(EffectEdit,3,wxLEFT |wxEXPAND,2);
	//BoxSizer1->AddSpacer(5);
	//BoxSizer2->Add(BoxSizer3,0,wxEXPAND,0);

	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer1->Add(BoxSizer4, 0, wxLEFT | wxRIGHT | wxTOP, 2);
	BoxSizer1->Add(BoxSizer5, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 2);
	BoxSizer1->Add(TextEditTl, 5, wxEXPAND|wxLEFT|wxRIGHT, 2);
	BoxSizer1->Add(BoxSizer6, 0, wxLEFT | wxRIGHT, 2);
	BoxSizer1->Add(TextEdit, 5, wxEXPAND|wxLEFT|wxRIGHT, 2);
	BoxSizer1->Add(BoxSizer2,0,wxEXPAND|wxALL,2);
	//BoxSizer1->Add(BoxSizer3,0,wxLEFT | wxRIGHT | wxBOTTOM,2);
	BoxSizer3 = NULL;

	SetSizer(BoxSizer1);



	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EditBox::OnCommit);
	Connect(ID_TLMODE,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EditBox::OnTlMode); 
	Connect(IDSTYLE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EditBox::OnCommit);    
	Connect(PutBold,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnBoldClick);
	Connect(PutItalic,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnItalClick);
	Connect(ID_UND,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnUndClick);
	Connect(ID_STRIKE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnStrikeClick);
	Connect(ID_AN,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EditBox::OnAnChoice);
	Connect(ID_FONT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnFontClick);
	Connect(ID_COL1,ID_COL4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnColorClick);
	Connect(ID_CPALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnCpAll);
	Connect(ID_CPSEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnCpSel);
	Connect(ID_HIDE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnHideOrig);
	Connect(ID_AUTOMOVETAGS,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&EditBox::OnAutoMoveTags);
	Connect(MENU_ZATW,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EditBox::OnCommit);
	Connect(MENU_NLINE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EditBox::OnNewline);
	Connect(SplitLine,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EditBox::OnSplit);
	Connect(StartDifference, EndDifference,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&EditBox::OnPasteDiff);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&EditBox::OnSize);
	if(!Options.GetBool("Disable live editing")){
		Connect(16668,NUMBER_CHANGED,(wxObjectEventFunction)&EditBox::OnEdit);
		Connect(16667,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EditBox::OnEdit);
	}
	Connect(16667,CURSOR_MOVED,(wxObjectEventFunction)&EditBox::OnCursorMoved);
	DoTooltips();
}

EditBox::~EditBox()
{
	wxDELETE(line);
}

void EditBox::SetIt(int Row, bool setaudio, bool save, bool nochangeline)
{
	TabPanel* pan=(TabPanel*)GetParent();
	if(nochangeline&&ebrow==Row){goto done;}
	if(Options.GetBool("Grid save without enter")&&ebrow!=Row&&save){

		Send(false);
	}
	ebrow=Row;
	grid->mtimerow=Row;
	wxDELETE(line);
	line=grid->GetDial(ebrow)->Copy();
	Comment->SetValue(line->IsComment);
	LayerEdit->SetInt(line->Layer);
	StartEdit->SetTime(line->Start);
	EndEdit->SetTime(line->End);
	DurEdit->SetTime(line->End - line->Start);
	StyleChoice->SetSelection(StyleChoice->FindString(line->Style,true));
	ActorEdit->ChangeValue(line->Actor);
	MarginLEdit->SetInt(line->MarginL);
	MarginREdit->SetInt(line->MarginR);
	MarginVEdit->SetInt(line->MarginV);
	EffectEdit->ChangeValue(line->Effect);
	//TextEdit->SetTextS((TextEditTl->IsShown())? GetTags(line->Text) : line->Text , false);
	SetTextWithTags();

	if(setaudio && ABox && ABox->IsShown()){ABox->audioDisplay->SetDialogue(line,ebrow);}



	//ustawia znaki na sekundę i ilość linii
	UpdateChars((TextEditTl->IsShown() && line->TextTl!="")? line->TextTl : line->Text);
	//ustawia clip/inny visual gdy jest włączony
	if(Visual>1){
		pan->Video->SetVisual(line->Start.mstime, line->End.mstime);
		pan->Video->Render();
	}
	
	//resetuje edycję na wideo
	if(OnVideo){
		if(pan->Video->IsShown()){
			pan->Video->OpenSubs(grid->SaveText()); 
			if(pan->Video->GetState()==Paused){pan->Video->Render();}
		}
		OnVideo=false;
	}
done:
	int pas=Options.GetInt("Play After Selection");
	if(pas>0){
		if(pas==1){
			if(ABox){
				wxWindow *focused= wxWindow::FindFocus();
				wxCommandEvent evt;ABox->OnPlaySelection(evt);
				focused->SetFocus();
			}
		}else{
			if(pan->Video->IsShown()){
				Dialogue *next=grid->GetDial(MIN(ebrow+1, grid->GetCount()-1));
				int ed=line->End.mstime, nst=next->Start.mstime;
				int htpf= pan->Video->avtpf/2;
				pan->Video->PlayLine(line->Start.mstime,(nst>ed && pas>2)? nst-htpf : ed-htpf);
			}
		}
	}
	//ustawia czas i msy na polu tekstowym wideo
	if(pan->Video->IsShown()){
		STime kkk;
		kkk.mstime=pan->Video->Tell();
		wxString dane;
		dane<<kkk.raw(SRT)<<";  ";
		if(!pan->Video->IsDshow){dane<<pan->Video->lastframe<<";  ";}
		int sdiff=kkk.mstime - line->Start.mstime;
		int ediff=kkk.mstime - line->End.mstime;
		dane<<sdiff<<" ms, "<<ediff<<" ms";
		pan->Video->mstimes->SetValue(dane);
		//wxRect rc=grid->GetMetrics(ebrow);

	}

}

void EditBox::UpdateChars(wxString text)
{
	wxString result;
	bool isbad=false;
	int ilzn=grid->CalcChars(text,&result,&isbad);
	wxColour textcolour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT); 
	Chars->SetLabelText(_("Linie: ")+result+"43");
	Chars->SetForegroundColour((isbad)? *wxRED : textcolour);
	int chtime= ilzn / ((line->End.mstime-line->Start.mstime) / 1000.0f);
	if(chtime<0 || chtime>999){chtime=999;}
	Chtime->SetLabelText(wxString::Format(_("Znaki na sekundę: %i<=15"),chtime));
	Chtime->SetForegroundColour((chtime>15)? *wxRED : textcolour);
	BoxSizer5->Layout();
	Frames->Refresh(false);
	Times->Refresh(false);
}

//Pobieranie danych z kontrolek editboxa
//selline przechodzi do następnej linii
//dummy nie zapisuje linii do grida
//visualdummy nie odświeża klatki wideo wykorzystywane przy visualu clipów
void EditBox::Send(bool selline, bool dummy, bool visualdummy)
{
	long cellm=0;

	if(line->IsComment != Comment->GetValue()){
		line->IsComment= !line->IsComment;
		cellm |= COMMENT;
	}

	if(LayerEdit->IsModified()){
		line->Layer=LayerEdit->GetInt();
		cellm |= LAYER;
		LayerEdit->SetModified(dummy);
	}

	if(StartEdit->IsModified()||StartEdit->HasFocus()){
		line->Start=StartEdit->GetTime();
		if(line->Start.mstime>line->End.mstime){line->End=line->Start;}
		cellm |=START;
		StartEdit->SetModified(dummy);
	}
	if(EndEdit->IsModified()||EndEdit->HasFocus()){
		line->End=EndEdit->GetTime();
		if(line->Start.mstime>line->End.mstime){line->End=line->Start;}
		cellm |= END;
		EndEdit->SetModified(dummy);
	}
	if(DurEdit->IsModified()){
		line->End=EndEdit->GetTime();
		if(line->Start.mstime>line->End.mstime){line->End=line->Start;}
		cellm |= END;
		DurEdit->SetModified(dummy);
	}

	wxString checkstyle = StyleChoice->GetString(StyleChoice->GetSelection());
	if(line->Style!=checkstyle && checkstyle!="" || StyleChoice->HasFocus()){
		line->Style=checkstyle; 
		cellm |= STYLE;
	}
	if(ActorEdit->IsModified()){
		line->Actor=ActorEdit->GetValue();
		cellm |= ACTOR;
		ActorEdit->SetModified(dummy);
	}
	if(MarginLEdit->IsModified()){
		line->MarginL=MarginLEdit->GetInt();
		cellm |= MARGINL;
		MarginLEdit->SetModified(dummy);
	}
	if(MarginREdit->IsModified()){
		line->MarginR=MarginREdit->GetInt();
		cellm |= MARGINR;
		MarginREdit->SetModified(dummy);
	}
	if(MarginVEdit->IsModified()){
		line->MarginV=MarginVEdit->GetInt();
		cellm |= MARGINV;
		MarginVEdit->SetModified(dummy);
	}
	if(EffectEdit->IsModified()){
		line->Effect=EffectEdit->GetValue();
		cellm |= EFFECT;
		EffectEdit->SetModified(dummy);
	}

	if(TextEdit->Modified()){
		if(TextEditTl->IsShown()){
			line->TextTl=TextEdit->GetValue();
			cellm |= TXTTL;
		}
		else{
			line->Text=TextEdit->GetValue();
			cellm |= TXT;
		}
		TextEdit->modified=dummy;
	}
	if(TextEditTl->Modified()&&TextEditTl->IsShown()){
		line->Text=TextEditTl->GetValue();
		cellm |= TXT;
		TextEditTl->modified=dummy;
	}

	if(cellm){
		if(ebrow<grid->GetCount() && !dummy){
			OnVideo=false;
			grid->ChangeLine(line, ebrow, cellm, selline, visualdummy);
		}
	}
	else if(selline){grid->NextLine();}
}


void EditBox::PutinText(wxString text, bool focus, bool onlysel, wxString *texttoPutin)
{
	bool oneline=(grid->sel.size()<2);
	if(oneline && !onlysel){
		long whre;
		wxString txt=TextEdit->GetValue();
		MTextEditor *Editor = TextEdit;
		if(grid->transl && txt=="" ){
			txt = TextEditTl->GetValue(); 
			Editor = TextEditTl;
		}
		if(!InBracket){
			txt.insert(Placed.x,"{"+text+"}");
			whre=cursorpos+text.Len()+2;
		}else{
			if(Placed.x<Placed.y){
				txt.erase(txt.begin()+Placed.x, txt.begin()+Placed.y+1);
				whre=(focus)? cursorpos+text.Len()-(Placed.y-Placed.x) : Placed.x;
			}
			else{whre=(focus)? cursorpos+1+text.Len() : Placed.x;}
			txt.insert(Placed.x,text);
		}
		if(texttoPutin){
			*texttoPutin=txt;
			return;
		}
		Editor->SetTextS(txt,true);
		if(focus){Editor->SetFocus();}
		Editor->SetSelection(whre,whre);//}else{Placed.x=whre;}
	}else{
		wxString tmp;
		wxArrayInt sels=grid->GetSels();
		for(size_t i=0;i<sels.size();i++){
			Dialogue *dialc=grid->CopyDial(sels[i]);
			wxString txt=(grid->transl && dialc->TextTl!="")? dialc->TextTl : dialc->Text;
			FindVal(lasttag,&tmp,txt);

			if(InBracket){
				//wxLogMessage("placed %i, %i: \r\n",Placed.x,Placed.y);
				if(Placed.x<Placed.y){txt.erase(txt.begin()+Placed.x, txt.begin()+Placed.y+1);}
				txt.insert(Placed.x,text);
				if(grid->transl && dialc->TextTl!=""){
					dialc->TextTl=txt;}
				else{dialc->Text=txt;}
			}else{
				if(grid->transl && dialc->TextTl!=""){
					dialc->TextTl.Prepend("{"+text+"}");}
				else{dialc->Text.Prepend("{"+text+"}");}
			}
		}
		grid->SetModified();
		grid->Refresh(false);
	}

}

void EditBox::PutinNonass(wxString text, wxString tag)
{
	if(grid->form==TMP)return;
	long from, to, whre;
	size_t start=0, len=0;
	bool match=false;
	TextEdit->GetSelection(&from,&to);
	wxString txt=TextEdit->GetValue();
	bool oneline=(grid->sel.size()<2);
	if(oneline){//zmiany tylko w editboxie
		if(grid->form==SRT){

			wxRegEx srttag("\\</?"+text+"\\>", wxRE_ADVANCED|wxRE_ICASE);
			if(srttag.Matches(txt.SubString(from-4,from+4))){
				srttag.GetMatch(&start, &len, 0);
				if(len+start>=4 && start<=4)
				{
					whre=from-4+start;
					txt.Remove(whre,len);
					txt.insert(whre,"<"+tag+">");
					whre+=3;
					match=true;
				}
			}
			if(!match){txt.insert(from,"<"+tag+">");from+=3;to+=3;whre=from;}
			if(from!=to){
				match=false;
				if(srttag.Matches(txt.SubString(to-4,to+4))){
					srttag.GetMatch(&start, &len, 0);
					if(len+start>=4 && start<=4)
					{
						txt.Remove(to-4+start,len);
						txt.insert(to-4+start,"</"+tag+">");
						whre=to+start;
						match=true;
					}
				}
				if(!match){txt.insert(to,"</"+tag+">");whre=to+4;}
			}

		}else if(grid->form==MDVD){


			wxRegEx srttag("\\{"+text+"}", wxRE_ADVANCED|wxRE_ICASE);
			int wheres=txt.SubString(0,from).Find('|',true);
			if(wheres==-1){wheres=0;}
			if(srttag.Matches(txt.Mid(wheres))){
				if(srttag.GetMatch(&start, &len, 0))
				{
					whre=wheres+start;
					txt.Remove(whre,len);
					txt.insert(whre,"{"+tag+"}");
					match=true;
				}
			}
			if(!match){txt.insert(wheres,"{"+tag+"}");}

		}

		TextEdit->SetTextS(txt,true);
		TextEdit->SetFocus();
		TextEdit->SetSelection(whre,whre);
	}
	else
	{//zmiany wszystkich zaznaczonych linijek
		wxString chars=(grid->form==SRT)? "<" : "{";
		wxString chare=(grid->form==SRT)? ">" : "}";
		wxArrayInt sels=grid->GetSels();
		for(size_t i=0;i<sels.size();i++)
		{
			Dialogue *dialc=grid->CopyDial(sels[i]);
			wxString txt=dialc->Text;
			//dialc->spells.Clear();
			if(txt.StartsWith(chars))
			{
				wxRegEx rex(chars+tag+chare,wxRE_ADVANCED|wxRE_ICASE);
				rex.ReplaceAll(&txt,"");
				dialc->Text=chars+text+chare+txt;
			}
			else
			{
				dialc->Text.Prepend(chars+text+chare);
			}
		}
		grid->SetModified();
		grid->Refresh(false);
	}

}

void EditBox::OnFontClick(wxCommandEvent& event)
{
	char form=grid->form;
	Styles *mstyle=(form<SRT)? grid->GetStyle(0,line->Style)->Copy() : new Styles();

	wxString tmp;
	if(form<SRT){

		if(FindVal("b(0|1)",&tmp)){if(mstyle->Bold&&tmp=="0"){mstyle->Bold=false;}else if(!mstyle->Bold&&tmp=="1"){mstyle->Bold=true;}}
		if(FindVal("i(0|1)",&tmp)){if(mstyle->Italic&&tmp=="0"){mstyle->Italic=false;}else if(!mstyle->Italic&&tmp=="1"){mstyle->Italic=true;}}
		if(FindVal("u(0|1)",&tmp)){if(mstyle->Underline&&tmp=="0"){mstyle->Underline=false;}else if(!mstyle->Underline&&tmp=="1"){mstyle->Underline=true;}}
		if(FindVal("s(0|1)",&tmp)){if(mstyle->StrikeOut&&tmp=="0"){mstyle->StrikeOut=false;}else if(!mstyle->StrikeOut&&tmp=="1"){mstyle->StrikeOut=true;}}
		if(FindVal("fs([0-9]+)",&tmp)){mstyle->Fontsize=tmp;}
		if(FindVal("fn(.*)",&tmp)){mstyle->Fontname=tmp;}
	}
	FontDialog FD(this,mstyle);
	if (FD.ShowModal() == wxID_OK) {
		//Getfont należy bezwzględnie zwolinić
		Styles *retstyl=FD.GetFont();
		if (retstyl->Fontname!=mstyle->Fontname)
		{
			if(form<SRT){PutinText("\\fn"+retstyl->Fontname);}
			else{PutinNonass("F:"+retstyl->Fontname, "f:([^}]*)");}
		}
		if (retstyl->Fontsize!=mstyle->Fontsize)
		{
			if(form<SRT){
				FindVal("fs([0-9]+)",&tmp);
				PutinText("\\fs"+retstyl->Fontsize);}
			else{PutinNonass("S:"+retstyl->Fontname, "s:([^}]*)");}
		}
		if (retstyl->Bold!=mstyle->Bold)
		{
			if(form<SRT){wxString bld=(retstyl->Bold)?"1":"0";
			FindVal("b(0|1)",&tmp);
			PutinText("\\b"+bld);}
			else{PutinNonass("y:b",(retstyl->Bold)?"Y:b" : "");}
		}
		if (retstyl->Italic!=mstyle->Italic)
		{
			if(form<SRT){wxString ital=(retstyl->Italic)?"1":"0";
			FindVal("i(0|1)",&tmp);
			PutinText("\\i"+ital);}
			else{PutinNonass("y:i", (retstyl->Italic)?"Y:i" : "");}
		}
		if (retstyl->Underline!=mstyle->Underline)
		{
			FindVal("u(0|1)",&tmp);
			wxString under=(retstyl->Underline)?"1":"0";
			PutinText("\\u"+under);
		}
		if (retstyl->StrikeOut!=mstyle->StrikeOut)
		{
			FindVal("s(0|1)",&tmp);
			wxString strike=(retstyl->StrikeOut)?"1":"0";
			PutinText("\\s"+strike);
		}
		delete retstyl;
	}
	delete mstyle;
}

void EditBox::AllColClick(int kol)
{
	num="";
	num<<kol;
	wxString iskol;
	wxString tmptext=TextEdit->GetValue();
	MTextEditor *Editor = TextEdit;
	if(grid->transl && tmptext=="" ){
		tmptext = TextEditTl->GetValue(); 
		Editor = TextEditTl;
	}
	wxString tag=(kol==1)? "?c&(.*)" : "c&(.*)";
	Styles *style=grid->GetStyle(0,line->Style);
	wxColour acol=(kol==1)? style->PrimaryColour.GetWX() :
		(kol==2)? style->SecondaryColour.GetWX() :
		(kol==3)? style->OutlineColour.GetWX() :
		style->BackColour.GetWX();
	DialogColorPicker *ColourDialog = DialogColorPicker::Get(this, (!FindVal(num+tag, &iskol))?
		acol : (grid->form<SRT)? AssColor("&"+iskol).GetWX() : wxColour("#FFFFFF"));

	wxPoint mst=wxGetMousePosition();
	int dw, dh;
	wxSize siz=ColourDialog->GetSize();
	siz.x;
	wxDisplaySize (&dw, &dh);
	mst.x-=(siz.x/2);
	mst.x=MID(0,mst.x, dw-siz.x);
	mst.y+=15;
	ColourDialog->Move(mst);
	ColourDialog->Connect(11111,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EditBox::OnColorChange,0,this);
	if ( ColourDialog->ShowModal() == wxID_OK) {
		Editor->SetSelection(Placed.x,Placed.x);
	}else{
		Editor->SetTextS(tmptext);wxCommandEvent evt;OnEdit(evt);
	}
	Editor->SetFocus();
}

void EditBox::OnColorClick(wxCommandEvent& event)
{
	AllColClick(event.GetId()-ID_COL1+1);
}

void EditBox::OnCommit(wxCommandEvent& event)
{
	TabPanel* pan=(TabPanel*)GetParent();
	pan->Video->blockpaint=true;
	if(splittedTags&&(TextEdit->modified || TextEditTl->modified)){TextEdit->modified=true; TextEditTl->modified=true;}
	Send(false, false, Visual!=0);
	if(Visual){
		pan->Video->SetVisual(line->Start.mstime,line->End.mstime);
	}
	if(StyleChoice->HasFocus()||Comment->HasFocus()){grid->SetFocus();}
	if(ABox){ABox->audioDisplay->SetDialogue(line,ebrow);}
	pan->Video->blockpaint=false;
}

void EditBox::OnNewline(wxCommandEvent& event)
{
	if(Visual){TextEdit->modified=true;}
	if(splittedTags&&(TextEdit->modified || TextEditTl->modified)){TextEdit->modified=true; TextEditTl->modified=true;}
	Send(!(StartEdit->HasFocus() || EndEdit->HasFocus()) || !Options.GetBool("Times Stop On line"));
}

void EditBox::OnBoldClick(wxCommandEvent& event)
{
	if(grid->form<SRT){
		Styles *mstyle=grid->GetStyle(0,line->Style);
		wxString wart=(mstyle->Bold)?"0":"1";
		bool issel=true;
		if(FindVal("b(0|1)",&wart,"",&issel)){wart = (wart=="1")? "0" :"1";}
		PutinText("\\b"+wart);
		if(!issel)return;
		wart=(mstyle->Bold)?"1":"0";
		if(FindVal("b(0|1)",&wart)){if(wart=="1"){wart="0";}else{wart="1";}}
		PutinText("\\b"+wart);
	}
	else if(grid->form==SRT){PutinNonass("b", "b");}
	else {PutinNonass("y:b", "Y:b");}
}

void EditBox::OnItalClick(wxCommandEvent& event)
{
	if(grid->form<SRT){Styles *mstyle=grid->GetStyle(0,line->Style);
	wxString wart=(mstyle->Italic)?"0":"1";
	bool issel=true;
	if(FindVal("i(0|1)",&wart,"",&issel)){if(wart=="1"){wart="0";}else{wart="1";}}
	PutinText("\\i"+wart);
	if(!issel)return;
	wart=(mstyle->Italic)?"1":"0";
	if(FindVal("i(0|1)",&wart)){if(wart=="1"){wart="0";}else{wart="1";}}
	PutinText("\\i"+wart);
	}
	else if(grid->form==SRT){PutinNonass("i", "i");}
	else if(grid->form==MDVD){PutinNonass("y:i", "Y:i");}
	else{PutinNonass("/", "/" );}
}

void EditBox::OnUndClick(wxCommandEvent& event)
{
	if(grid->form<SRT){
		Styles *mstyle=grid->GetStyle(0,line->Style);
		wxString wart=(mstyle->Underline)?"0":"1";
		bool issel=true;
		if(FindVal("u(0|1)",&wart,"",&issel)){if(wart=="1"){wart="0";}else{wart="1";}}
		PutinText("\\u"+wart);
		if(!issel)return;
		wart=(mstyle->Underline)?"1":"0";
		if(FindVal("u(0|1)",&wart)){if(wart=="1"){wart="0";}else{wart="1";}}
		PutinText("\\u"+wart);
	}
	else if(grid->form==SRT){PutinNonass("u", "u");}
}

void EditBox::OnStrikeClick(wxCommandEvent& event)
{
	if(grid->form<SRT){
		Styles *mstyle=grid->GetStyle(0,line->Style);
		wxString wart=(mstyle->StrikeOut)?"0":"1";
		bool issel=true;
		if(FindVal("s(0|1)",&wart,"",&issel)){if(wart=="1"){wart="0";}else{wart="1";}}
		PutinText("\\s"+wart);
		if(!issel)return;
		wart=(mstyle->StrikeOut)?"0":"1";
		if(FindVal("s(0|1)",&wart)){if(wart=="1"){wart="0";}else{wart="1";}}
		PutinText("\\s"+wart);
	}
	else if(grid->form==SRT){PutinNonass("s", "s");}
}

void EditBox::OnAnChoice(wxCommandEvent& event)
{
	TextEdit->SetSelection(0,0);
	if(grid->transl){TextEditTl->SetSelection(0,0);}
	lasttag="an([0-9])";
	PutinText("\\"+Ban->GetString(Ban->GetSelection()),true, true);
}

void EditBox::OnTlMode(wxCommandEvent& event)
{
	bool show=!TextEditTl->IsShown();
	if(grid->SetTlMode(show)){TlMode->SetValue(true);return;}
	SetTl(show);
	SetIt(ebrow);
}

void EditBox::SetTl(bool tl)
{
	TextEditTl->Show(tl);
	Bcpall->Show(tl);
	Bcpsel->Show(tl);
	Bhide->Show(tl);
	AutoMoveTags->Show(tl);
	AutoMoveTags->SetValue(Options.GetBool("Auto Move Tags"));
	BoxSizer1->Layout();
	if(TlMode->GetValue()!=tl){TlMode->SetValue(tl);}
	kainoteFrame *Kai = (kainoteFrame*)Notebook::GetTabs()->GetParent();
	Kai->Toolbar->UpdateId(SaveTranslation, tl);
}

void EditBox::OnCpAll(wxCommandEvent& event)
{
	TextEdit->SetTextS(TextEditTl->GetValue(),true);
	TextEdit->SetFocus();
}

void EditBox::OnCpSel(wxCommandEvent& event)
{
	long from, to, fromtl, totl;
	TextEditTl->GetSelection(&from,&to);
	if(from!=to){
		wxString txt=TextEditTl->GetValue();
		wxString txt1=TextEdit->GetValue();
		TextEdit->GetSelection(&fromtl,&totl);
		wxString txtt=txt.SubString(from,to-1);
		txt1.insert(fromtl,txtt);
		TextEdit->SetTextS(txt1,true);
		TextEdit->SetFocus();
		long whre=txtt.Len();
		TextEdit->SetSelection(fromtl+whre,fromtl+whre);
	}
}



void EditBox::RefreshStyle(bool resetline)
{
	StyleChoice->Clear();
	for(int i=0;i<grid->StylesSize();i++)
	{
		StyleChoice->Append(grid->GetStyle(i)->Name);
		if(grid->GetStyle(i)->Name==line->Style){StyleChoice->SetSelection(i);}
	}
	if(resetline){
		if(grid->GetCount()>0){
			SetIt(0);}
		else{ebrow=0;}
	}
}


void EditBox::DoTooltips()
{
	Bfont->SetToolTip(_("Wybór czcionki"));
	Bcol1->SetToolTip(_("Kolor podstawowy"));
	Bcol2->SetToolTip(_("Kolor zastępczy do karaoke"));
	Bcol3->SetToolTip(_("Kolor obwódki"));
	Bcol4->SetToolTip(_("Kolor cienia"));
	Ban->SetToolTip(_("Położenie tekstu"));
	//Global->SetToolTip(_("Wstawia wszystkie tagi na początku zaznaczonych linijek."));
	Bbold->SetToolTip(_("Pogrubienie"));
	Bital->SetToolTip(_("Pochylenie"));
	Bund->SetToolTip(_("Podkreślenie"));
	Bstrike->SetToolTip(_("Przekreślenie"));
	TlMode->SetToolTip(_("Tryb tłumaczenia wyświetla i zapisuje zarówno tekst obcojęzyczny, jak i tekst tłumaczenia"));
	Bcpall->SetToolTip(_("Kopiuje cały tekst obcojęzyczny do pola z tłumaczeniem"));
	Bcpsel->SetToolTip(_("Kopiuje zaznaczony tekst obcojęzyczny do pola z tłumaczeniem"));
	//TextEdit->SetToolTip(_("Tekst linijki / tekst tłumaczenia, gdy tryb tłumaczenia jest włączony."));
	//TextEditTl->SetToolTip(_("tekst obcojęzyczny."));
	Comment->SetToolTip(_("Ustawia linijkę jako komentarz. Komentarze nie są wyświetlane"));
	LayerEdit->SetToolTip(_("Warstwa linijki, wyższe warstwy są na wierzchu"));
	StartEdit->SetToolTip(_("Czas początkowy linijki"));
	EndEdit->SetToolTip(_("Czas końcowy linijki"));
	DurEdit->SetToolTip(_("Czas trwania linijki"));
	StyleChoice->SetToolTip(_("Styl linijki"));
	ActorEdit->SetToolTip(_("Oznaczenie aktora linijki. Nie wpływa na wygląd napisów"));
	MarginLEdit->SetToolTip(_("Margines lewy linijki"));
	MarginREdit->SetToolTip(_("Margines prawy linijki"));
	MarginVEdit->SetToolTip(_("Margines górny i dolny linijki"));
	EffectEdit->SetToolTip(_("Efekt linijki. Służy do oznaczania linijek, na których zastosowane ma być karaoke bądź efekty VSFiltera"));
	Chars->SetToolTip(_("Ilość znaków w każdej linijce.\nNie więcej niż 43 znaki na linijkę (maksymalnie 2 linijki)"));
	Chtime->SetToolTip(_("Znaki na sekundę.\nNie powinny przekraczać 15 znaków na sekundę"));
}

void EditBox::OnSize(wxSizeEvent& event)
{

	//if(grid->form>SSA){event.Skip(); return;}
	int w,h;
	GetClientSize(&w,&h);
	if(isdetached && w>800){
		BoxSizer1->Detach(BoxSizer3);

		for(int i = 5; i>=0; i--){BoxSizer3->Detach(i); }
		BoxSizer2->Insert(0,Comment,0,wxLEFT|wxALIGN_CENTER,4);
		BoxSizer2->Add(ActorEdit,5,wxEXPAND|wxLEFT,2);
		BoxSizer2->Add(MarginLEdit,0,wxLEFT,2);
		BoxSizer2->Add(MarginREdit,0,wxLEFT,2);
		BoxSizer2->Add(MarginVEdit,0,wxLEFT,2);
		BoxSizer2->Add(EffectEdit,5,wxLEFT |wxEXPAND,2);
		delete BoxSizer3; BoxSizer3=NULL;
		SetSizer(BoxSizer1);

		isdetached=false;
	}
	else if(!isdetached && w<=800)
	{
		for(int i = 10; i>=6; i--){BoxSizer2->Detach(i);}
		BoxSizer2->Detach(0);
		BoxSizer3=new wxBoxSizer(wxHORIZONTAL);
		BoxSizer3->Add(Comment,0,wxLEFT|wxALIGN_CENTER,4);
		BoxSizer3->Add(ActorEdit,5,wxEXPAND|wxLEFT,2);
		BoxSizer3->Add(MarginLEdit,0,wxLEFT,2);
		BoxSizer3->Add(MarginREdit,0,wxLEFT,2);
		BoxSizer3->Add(MarginVEdit,0,wxLEFT,2);
		BoxSizer3->Add(EffectEdit,5,wxLEFT |wxEXPAND,2);
		BoxSizer1->Add(BoxSizer3,0,wxEXPAND|wxLEFT | wxRIGHT | wxBOTTOM,2);
		SetSizer(BoxSizer1);

		isdetached=true;
	}

	event.Skip();

}

void EditBox::HideControls()
{
	bool state1=Bcol2->IsShown();
	bool state=grid->form<SRT;

	Ban->Enable(state);
	Bcol2->Enable(state);
	Bcol3->Enable(state);
	Bcol4->Enable(state);
	Comment->Enable(state);
	LayerEdit->Enable(state);
	StyleChoice->Enable(state);
	ActorEdit->Enable(state);
	MarginLEdit->Enable(state);
	MarginREdit->Enable(state);
	MarginVEdit->Enable(state);
	EffectEdit->Enable(state);

	state=grid->form<SRT || grid->form==MDVD;
	Bcol1->Enable(state);
	Bfont->Enable(state);

	state=grid->form<=SRT || grid->form==MDVD;
	Bbold->Enable(state);

	state=grid->form<=SRT;
	Bund->Enable(state);
	Bstrike->Enable(state);

	state=grid->form!=TMP;
	EndEdit->Enable(state);
	DurEdit->Enable(state);
	Bital->Enable(state);
	/*
	state=grid->form<SRT;

	if(!state1 && state)
	{//przejście na ass
	BoxSizer4->Detach(StartEdit);
	BoxSizer4->Detach(EndEdit);
	BoxSizer4->Detach(DurEdit);
	BoxSizer2->Insert(2,StartEdit,0,wxLEFT,2);
	BoxSizer2->Insert(3,EndEdit,0,wxLEFT,2);
	BoxSizer2->Insert(4,DurEdit,0,wxLEFT,2);
	wxSizeEvent evt2;
	OnSize(evt2);

	}
	else if(state1 && !state)
	{//przejście na inne formaty
	BoxSizer2->Detach(StartEdit);
	BoxSizer2->Detach(EndEdit);
	BoxSizer2->Detach(DurEdit);
	BoxSizer4->Prepend(DurEdit,0,wxLEFT|wxALIGN_CENTER,2);
	BoxSizer4->Prepend(EndEdit,0,wxLEFT|wxALIGN_CENTER,2);
	BoxSizer4->Prepend(StartEdit,0,wxLEFT|wxALIGN_CENTER,2);
	}
	Layout();*/
}

void EditBox::ClearErrs()
{
	Notebook *nb= Notebook::GetTabs();
	for(size_t i = 0; i < nb->Size(); i++)
	{
		nb->Page(i)->Grid1->SpellErrors.clear();
	}
	grid->Refresh(false);
}

void EditBox::OnSplit(wxCommandEvent& event)
{
	wxString Splitchar=(grid->form<=SRT)? "\\N" : "|";
	bool istl=(grid->transl && TextEdit->GetValue()=="");
	//Editor
	MTextEditor *tedit=(istl)? TextEditTl : TextEdit;
	wxString txt=tedit->GetValue();
	long strt, ennd;
	tedit->GetSelection(&strt,&ennd);
	if(strt>0 && txt[strt-1]==' '){strt--;}
	if(txt[ennd]==' '){ennd++;}

	if(strt!=ennd){txt.Remove(strt,ennd-strt);}
	txt.insert(strt,Splitchar);
	tedit->SetTextS(txt,true);
	long whre=strt+Splitchar.Len();
	tedit->SetSelection(whre,whre);
}

void EditBox::OnHideOrig(wxCommandEvent& event)
{
	wxString texttl = TextEditTl->GetValue();
	texttl="{"+texttl+"}";
	TextEdit->SetFocus();
	TextEditTl->SetTextS(texttl, true);
}

void EditBox::OnPasteDiff(wxCommandEvent& event)
{
	if(Notebook::GetTab()->Video->GetState()==None){wxBell(); return;}
	int idd=event.GetId();
	int vidtime=Notebook::GetTab()->Video->Tell();
	if(vidtime < line->Start.mstime || vidtime > line->End.mstime){wxBell(); return;}
	int diff=(idd==StartDifference)? vidtime - line->Start.mstime : abs(vidtime - line->End.mstime); 
	long poss, pose;
	TextEdit->GetSelection(&poss,&pose);
	wxString kkk;
	kkk<<diff;
	TextEdit->Replace(poss,pose,kkk);
	int npos=poss+kkk.Len();
	TextEdit->SetSelection(npos, npos);
}
//znajduje tagi w polu tekstowym
//w wyszukiwaniu nie używać // a także szukać tylko do końca taga, nie do następnego taga
bool EditBox::FindVal(wxString tag, wxString *Finded, wxString text, bool *endsel)
{
	lasttag=tag;
	long from=0, to=0;
	bool brkt=true;
	bool inbrkt=true;
	bool fromOriginal = false;
	wxString txt;
	if(text==""){
		txt = TextEdit->GetValue(); 
		if(grid->transl && txt=="" ){
			fromOriginal = true;
			txt = TextEditTl->GetValue(); 
		}
	}else{txt=text;}
	if(txt==""){Placed.x=0;Placed.y=0; InBracket=false; cursorpos=0; if(endsel){*endsel=false;} return false;}
	if(grid->sel.size()<2){
		MTextEditor *Editor = (fromOriginal)? TextEditTl : TextEdit;
		Editor->GetSelection(&from,&to);
	}

	if(endsel && from == to){ *endsel=false;}
	wxRegEx rex("^"+tag,wxRE_ADVANCED);


	int klamras=txt.SubString(0,from).Find('{',true);
	int klamrae=txt.SubString(0,(from-2<1)?1:(from-2)).Find('}',true);
	if(klamras==-1||(klamras<klamrae&&klamrae!=-1)){InBracket=false;inbrkt=false;klamrae=from;brkt=false;}
	else{
		InBracket=true;
		int tmpfrom=from-2;
		do{
			klamrae=txt.find('}',(tmpfrom<1)? 1 : tmpfrom);
			tmpfrom=klamrae+1;
		}while(klamrae!=-1 && klamrae<(int)txt.Len()-1 && txt[klamrae+1]=='{');
		if(klamrae<0){klamrae=txt.Len()-1;}
	}

	Placed.x=klamrae;
	Placed.y=klamrae;
	if(endsel && *endsel){
		cursorpos=to;
		if(InBracket){cursorpos--;}
	}else{
		cursorpos=klamrae;}
	bool isT=false;
	bool firstT=false;
	int endT;
	int lslash=endT=klamrae+1;
	wxString finded[2];
	wxPoint fpoints[2];
	if(klamrae==txt.Len()){klamrae--;}

	for(int i=klamrae; i>=0; i--){
		wxUniChar ch=txt[i];
		if(ch=='\\' && brkt){
			wxString ftag=txt.SubString(i+1,lslash-1);
			if(ftag.EndsWith(")")){
				if(ftag.Find('(')==-1||ftag.StartsWith("t(")){
					isT=true;
					endT=lslash-1;
				}
			}
			if(ftag.StartsWith("t(")){

				if(i<=from && from<endT){

					if(finded[1]!="" && fpoints[1].y<=endT){
						Placed=fpoints[1];*Finded=finded[1];return true;
					}else if(finded[0]!=""){
						if(fpoints[0].y<=endT){break;}
					}else{
						Placed.x=endT;Placed.y=Placed.x;InBracket=true;return false;
					}

				}
				isT=false;
				lslash=i;
				continue;
			}

			int reps=rex.ReplaceAll(&ftag,"\\1");
			if(reps>0){

				if((ftag.EndsWith(")")&&!ftag.StartsWith("("))||ftag.EndsWith("}")){ftag.RemoveLast(1);lslash--;}

				if(finded[0]==""&&!isT){finded[0]=ftag; fpoints[0].x=i; fpoints[0].y=lslash-1;}
				else{finded[1]=ftag; fpoints[1].x=i; fpoints[1].y=lslash-1;}
				if(!isT && finded[0]!=""){
					break;
				}
			}

			lslash=i;
		}else if(ch=='{'){
			brkt=false;
			if(txt[MAX(0,i-1)]!='}'){inbrkt=false;}
		}else if(ch=='}'){
			lslash=i;
			brkt=true;
		}

	}

	if(!isT && finded[0]!=""){
		if(inbrkt){Placed=fpoints[0];} *Finded=finded[0]; return true;
	}



	return false;
}


void EditBox::OnEdit(wxCommandEvent& event)
{
	TabPanel* panel= (TabPanel*)GetParent();
	bool visible=false;
	if(StartEdit->HasFocus()||EndEdit->HasFocus()){
		line->End=EndEdit->GetTime();
		line->Start=StartEdit->GetTime();
		if(line->Start>line->End){line->End=line->Start;EndEdit->SetTime(line->End);}
		DurEdit->SetTime(line->End - line->Start);
	}
	else if(DurEdit->HasFocus()){
		line->End.mstime=line->Start.mstime + DurEdit->GetTime().mstime;
		EndEdit->SetTime(line->End);
	}

	wxString *text=NULL;
	if(panel->Video->GetState()==Paused){
		text=grid->GetVisible(&visible);
	}
	else if(panel->Video->GetState()==Playing){
		visible=true;
		text=grid->SaveText();

	}

	OnVideo=true;
	if(visible && panel->Video->IsShown()){
		panel->Video->OpenSubs(text);
		if(panel->Video->GetState()==Paused){panel->Video->Render();}
	}else if(text){delete text;}
}

void EditBox::OnColorChange(wxCommandEvent& event)
{
	if(grid->form<SRT){
		wxString iskol;
		wxString tag=(num=="1")? "?c&(.*)" : "c&(.*)";
		FindVal(num+tag, &iskol);

		PutinText("\\"+num+"c"+event.GetString()+"&", false);
		if(event.GetInt()){
			FindVal(num+"a&(.*)", &iskol);
			PutinText("\\"+num+wxString::Format("a&H%02X&",event.GetInt()), false);
		}
	}
	else{PutinNonass("C:"+event.GetString().Mid(2),"C:([^}]*)");}
	OnEdit(event);
}

void EditBox::OnButtonTag(wxCommandEvent& event)
{
	wxString type;
	wxString tag=Options.GetString(wxString::Format("Editbox tag button%i",event.GetId()-15000)).BeforeFirst('\f', &type);


	if(type!="2"){
		if(type=="1"){TextEdit->SetSelection(0,0);}
		if(!tag.StartsWith("\\")){tag.Prepend("\\");}
		wxString delims="1234567890-&()[]";
		bool found=false;
		wxString findtag;
		for(int i=2; i<(int)tag.Len(); i++)
		{
			if(delims.Find(tag[i])!=-1)
			{
				found=true;
				findtag=tag.SubString(1,i-1);
				break;
			}
		}
		if(!found){findtag=tag.AfterFirst('\\');}
		wxString iskol;

		FindVal(findtag+"(.*)", &iskol);

		PutinText(tag);
	}else{
		long from, to;
		wxString txt= TextEdit->GetValue();
		MTextEditor *Editor = TextEdit;
		if(grid->transl && txt==""){ txt = TextEditTl->GetValue(); Editor = TextEditTl;}
		Editor->GetSelection(&from, &to);
		
		if(from!=to){
			txt.erase(txt.begin()+from, txt.begin()+to);
		}
		int klamras=txt.Mid(from).Find('{');
		int klamrae=txt.Mid(from).Find('}');

		if(klamrae!=-1 && (klamras==-1 || klamras>klamrae)){
			from+=klamrae+1;
		}
		txt.insert(from, tag);
		from+=tag.Len();
		Editor->SetTextS(txt, true);
		Editor->SetSelection(from, from);
	}

}

void EditBox::OnAutoMoveTags(wxCommandEvent& event)
{
	SetTextWithTags();
	Options.SetBool("Auto Move Tags", AutoMoveTags->GetValue());
	Options.SaveOptions();
}

void EditBox::SetTextWithTags()
{
	if(grid->transl && line->TextTl=="" && line->Text.StartsWith("{") && AutoMoveTags->GetValue()){
		int getr=line->Text.Find('}');
		if(getr>1){
			wxString null;
			wxString txt=line->Text.substr(0,getr+1);
			TextEdit->SetTextS(txt, false);
			TextEditTl->SetTextS(((int)line->Text.Len()>getr+1)? line->Text.Mid(getr+1): null, false);
			splittedTags=true;
			int pos=txt.Len();
			TextEdit->SetSelection(pos,pos);
			return;
		}
	}
	splittedTags=false;
	TextEdit->SetTextS((TextEditTl->IsShown())? line->TextTl : line->Text , false);
	if(TextEditTl->IsShown()){TextEditTl->SetTextS(line->Text, false);}
}

void EditBox::OnCursorMoved(wxCommandEvent& event)
{
	if(Visual==SCALE||Visual==ROTATEZ||Visual==ROTATEXY){
		TabPanel* pan=(TabPanel*)GetParent();
		pan->Video->SetVisual();
	}
}

void EditBox::OnChangeTimeDisplay(wxCommandEvent& event)
{
	grid->showFrames=Frames->GetValue();
	grid->RepaintWindow(START|END);
}