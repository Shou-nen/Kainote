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


#include "ShiftTimes.h"
#include "Config.h"
#include "Stylelistbox.h"
#include "KainoteMain.h"
#include "EditBox.h"
#include "KaiDialog.h"
#include "KaiMessageBox.h"

class ProfileEdition : public KaiDialog
{
public:
	ProfileEdition(wxWindow* parent, const wxArrayString &profiles, const wxPoint &pos);
	virtual ~ProfileEdition(){};
	wxString GetProfileName(bool *exist){ 
		*exist = overwrite;

		return profilesList->GetValue(); 
	};
private:
	void OnOKClick(wxCommandEvent &evt);
	KaiChoice *profilesList;
	const wxArrayString &profilesNames;
	bool overwrite = false;
	wxPoint dialogPos;
};

ProfileEdition::ProfileEdition(wxWindow* parent, const wxArrayString &profiles, const wxPoint &pos)
	:KaiDialog(parent, -1, _("Wybierz nazwę profilu"), pos)
	, profilesNames(profiles)
{
	DialogSizer *dSizer = new DialogSizer(wxVERTICAL);
	KaiStaticText *description = new KaiStaticText(this, -1, _("Wprowadź nazwę profilu,\nbądź wybierz istniejący, by nadpisać"));
	KaiTextValidator valid(wxFILTER_EXCLUDE_CHAR_LIST);
	wxArrayString excludes;
	excludes.Add(L"\\");
	excludes.Add(L"/");
	excludes.Add(L":");
	excludes.Add(L"|");
	excludes.Add(L"\f");
	valid.SetExcludes(excludes);
	profilesList = new KaiChoice(this, -1, L"", wxDefaultPosition, wxDefaultSize, profiles, 0, valid);
	profilesList->SetMaxLength(25);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	MappedButton *OK = new MappedButton(this, wxID_OK, L"OK");
	MappedButton *cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ProfileEdition::OnOKClick, this, wxID_OK);
	buttonSizer->Add(OK, 1, wxALL | wxEXPAND, 2);
	buttonSizer->Add(cancel, 1, wxALL | wxEXPAND, 2);
	dSizer->Add(description, 0, wxALL | wxEXPAND, 2);
	dSizer->Add(profilesList, 0, wxALL | wxEXPAND, 2);
	dSizer->Add(buttonSizer, 0, wxALL, 2);
	SetSizerAndFit(dSizer);
	CenterOnParent(wxHORIZONTAL);
	dialogPos = pos;
}

void ProfileEdition::OnOKClick(wxCommandEvent &evt)
{
	wxString thisName = profilesList->GetValue();
	for (auto name : profilesNames){
		if (name == thisName){
			int result = KaiMessageBox(wxString::Format(_("Na pewno chcesz nadpisać profil o nazwie %s"),
				profilesList->GetString(profilesList->GetSelection())), _("Informacja"), wxYES_NO, GetParent(), dialogPos);
			if (result != wxYES)
				return;

			overwrite = true;
			break;
		}
	}
	

	EndModal(wxID_OK);

}


ShiftTimesWindow::ShiftTimesWindow(wxWindow* parent, KainoteFrame* kfparent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxWindow(parent, id, pos, size, style | wxVERTICAL)
{
	Kai = kfparent;
	form = ASS;
	panel = new wxWindow(this, -1);
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	scroll = new KaiScrollbar(this, 5558, wxDefaultPosition, wxDefaultSize, wxVERTICAL);
	scroll->Hide();
	scroll->SetScrollRate(30);
	isscrollbar = false;
	//SetScrollRate(0,5);
	scPos = 0;

	wxAcceleratorEntry ctentries[1];
	ctentries[0].Set(wxACCEL_NORMAL, WXK_RETURN, GLOBAL_SHIFT_TIMES);

	wxAcceleratorTable ctaccel(1, ctentries);
	SetAcceleratorTable(ctaccel);
	CreateControls(Options.GetInt(PostprocessorEnabling) < 16);
	RefVals();

}

ShiftTimesWindow::~ShiftTimesWindow()
{
	SaveOptions();
}

bool ShiftTimesWindow::SetBackgroundColour(const wxColour &col)
{
	wxWindow::SetBackgroundColour(Options.GetColour(WindowBackground));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));
	return true;
}

bool ShiftTimesWindow::SetForegroundColour(const wxColour &col)
{
	wxWindow::SetForegroundColour(Options.GetColour(WindowText));
	panel->SetForegroundColour(Options.GetColour(WindowText));
	return true;
}

void ShiftTimesWindow::Contents(bool addopts)
{
	bool state;
	form = Kai->GetTab()->Grid->subsFormat;
	VideoCtrl *vb = Kai->GetTab()->Video;
	if (form < SRT){
		state = true;
		WhichLines->EnableItem(3);
		WhichLines->EnableItem(4);
		WhichLines->EnableItem(5);
	}
	else{
		WhichLines->EnableItem(3, false);
		WhichLines->EnableItem(4, false);
		WhichLines->EnableItem(5, false);
		state = false;
	}
	if (!LeadIn){
		bool lastEnable = DisplayFrames->IsEnabled();
		DisplayFrames->Enable(vb->VFF != NULL);
		bool Enable = DisplayFrames->IsEnabled();
		bool dispFrames = DisplayFrames->GetValue();
		if (lastEnable != Enable){
			if (!vb->VFF && (dispFrames || !Enable)){
				ChangeDisplayUnits(true);
			}
			else if (vb->VFF && dispFrames){
				ChangeDisplayUnits(false);
			}
		}
		MoveTagTimes->Enable(state);
	}
	AddStyles->Enable(state);
	Stylestext->Enable(state);
	if (!LeadIn){
		WhichTimes->Enable(form != TMP);
		if (vb->GetState() != None){ state = true; }
		else{ state = false; }
		MoveToVideoTime->Enable(state);
		state = (Kai->GetTab()->Edit->ABox && Kai->GetTab()->Edit->ABox->audioDisplay->hasMark);
		MoveToAudioTime->Enable(state);
	}
	if (LeadIn){
		state = (form != TMP);
		LeadIn->Enable(state);
		LeadOut->Enable(state);
		Continous->Enable(state);
		SnapKF->Enable(state && vb->VFF);
	}
	//if(addopts){RefVals();}

}



void ShiftTimesWindow::OnAddStyles(wxCommandEvent& event)
{
	wxString result = GetCheckedElements(Kai);
	Stylestext->SetValue(result);
	if (result != L""){ WhichLines->SetSelection(5); }
}

void ShiftTimesWindow::SaveOptions()
{
	if (!LeadIn){
		if (TimeText->HasShownFrames()){
			Options.SetInt(MoveTimesFrames, TimeText->GetTime().orgframe);
		}
		else{
			Options.SetInt(MoveTimesTime, TimeText->GetTime().mstime);
		}

		//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 
		//8 Move to audio time 16 display times / frames 32 move tag times;
		Options.SetInt(MoveTimesOptions, (int)Forward->GetValue() | ((int)StartVAtime->GetValue() << 1) |
			((int)MoveToVideoTime->GetValue() << 2) | ((int)(MoveToAudioTime->GetValue()) << 3) |
			((int)DisplayFrames->GetValue() << 4) | ((int)MoveTagTimes->GetValue() << 5));

		Options.SetInt(MoveTimesWhichTimes, WhichTimes->GetSelection());
		Options.SetInt(MoveTimesCorrectEndTimes, EndTimeCorrection->GetSelection());
	}
	Options.SetInt(MoveTimesWhichLines, WhichLines->GetSelection());
	if (form == ASS){
		wxString sstyles = Stylestext->GetValue();
		Options.SetString(MoveTimesStyles, sstyles);
	}
	if (LeadIn){
		Options.SetInt(PostprocessorLeadIn, LITime->GetInt());
		Options.SetInt(PostprocessorLeadOut, LOTime->GetInt());
		Options.SetInt(PostprocessorThresholdStart, ThresStart->GetInt());
		Options.SetInt(PostprocessorThresholdEnd, ThresEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeStart, BeforeStart->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterStart, AfterStart->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeEnd, BeforeEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterEnd, AfterEnd->GetInt());
		//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
		//int peres= (LeadIn->GetValue())? 1 : 0
		Options.SetInt(PostprocessorEnabling, (int)LeadIn->GetValue() + ((int)LeadOut->GetValue() * 2) + ((int)Continous->GetValue() * 4) + ((int)SnapKF->GetValue() * 8) + 16);
	}
}

void ShiftTimesWindow::CreateControls(bool normal /*= true*/)
{
	//wxFont thisFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, L"Tahoma", wxFONTENCODING_DEFAULT);
	panel->SetFont(*Options.GetFont(-2)/*thisFont*/);
	panel->SetForegroundColour(Options.GetColour(WindowText));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));
	Main = new wxBoxSizer(wxVERTICAL);

	coll = new MappedButton(panel, 22999, (normal) ? _("Post processor") : _("Przesuwanie czasów"));
	Main->AddSpacer(2);
	Main->Add(coll, 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

	wxArrayString choices;
	KaiStaticBoxSizer *linesizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Które linijki"));
	choices.Add(_("Wszystkie linijki"));
	choices.Add(_("Zaznaczone linijki"));
	choices.Add(_("Od zaznaczonej linijki"));
	choices.Add(_("Czasy wyższe i równe"));
	choices.Add(_("Czasy niższe i równe"));
	choices.Add(_("Według wybranych stylów"));
	WhichLines = new KaiChoice(panel, 22888, wxDefaultPosition, wxDefaultSize, choices, KAI_SCROLL_ON_FOCUS);

	wxBoxSizer *stylesizer = new wxBoxSizer(wxHORIZONTAL);
	AddStyles = new MappedButton(panel, ID_BSTYLE, L"+", L"", wxDefaultPosition, wxDefaultSize, -1, MAKE_SQUARE_BUTTON);
	Stylestext = new KaiTextCtrl(panel, -1, L"", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	stylesizer->Add(AddStyles, 0, wxALL, 2);
	stylesizer->Add(Stylestext, 1, wxEXPAND | wxBOTTOM | wxTOP | wxRIGHT, 2);

	linesizer->Add(WhichLines, 0, wxEXPAND | wxRIGHT | wxTOP | wxLEFT, 2);
	linesizer->Add(stylesizer, 1, wxEXPAND);
	
	if (normal){
		//profiles
		profileSizer = new KaiStaticBoxSizer(wxHORIZONTAL, panel, _("Edycja profilów"));
		NewProfile = new MappedButton(panel, 31229, L"+", -1);
		NewProfile->SetToolTip(_("Dodawanie i edycja profilów"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ShiftTimesWindow::OnAddProfile, this, 31229);
		RemoveProfile = new MappedButton(panel, 31230, L"-", -1);
		NewProfile->SetToolTip(_("Usuwanie profilów"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ShiftTimesWindow::OnRemoveProfile, this, 31230);
		wxArrayString profileList;
		GetProfilesNames(profileList);
		ProfilesList = new KaiChoice(panel, 31231, wxDefaultPosition, wxDefaultSize, profileList);
		Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ShiftTimesWindow::OnChangeProfile, this, 31231);
		profileSizer->Add(NewProfile, 0, wxALL, 2);
		profileSizer->Add(RemoveProfile, 0, wxBOTTOM | wxTOP | wxRIGHT, 2);
		profileSizer->Add(ProfilesList, 1, wxEXPAND | wxBOTTOM | wxTOP | wxRIGHT, 2);
		//time frame
		KaiStaticBoxSizer *timesizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Czas"));
		wxGridSizer *timegrid = new wxGridSizer(2, 0, 0);
		MoveTime = new MappedButton(panel, GLOBAL_SHIFT_TIMES, _("Przesuń"), _("Przesuń czas napisów"), wxDefaultPosition, wxSize(60, -1), GLOBAL_HOTKEY);
		TimeText = new TimeCtrl(panel, 22890, L"0:00:00.00", wxDefaultPosition, wxSize(60, -1), wxALIGN_CENTER | wxTE_PROCESS_ENTER);
		Forward = new KaiRadioButton(panel, 22891, _("W przód"));
		Backward = new KaiRadioButton(panel, 22891, _("W tył"));
		DisplayFrames = new KaiCheckBox(panel, 31221, _("Klatki"));
		MoveTagTimes = new KaiCheckBox(panel, 22889, _("Czasy tagów"));
		Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &ShiftTimesWindow::OnChangeDisplayUnits, this, 31221);
		timegrid->Add(TimeText, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
		timegrid->Add(MoveTime, 0, wxEXPAND | wxRIGHT, 2);
		timegrid->Add(Forward, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);
		timegrid->Add(Backward, 1, wxEXPAND | wxRIGHT, 2);
		timegrid->Add(DisplayFrames, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);
		timegrid->Add(MoveTagTimes, 0, wxEXPAND | wxRIGHT, 2);

		timesizer->Add(timegrid, 0, wxEXPAND, 0);


		//ramka przesuwania wg audio / wideo
		KaiStaticBoxSizer *VAtiming = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Przesuwanie wg wideo / audio"));

		wxBoxSizer *SE = new wxBoxSizer(wxHORIZONTAL);
		StartVAtime = new KaiRadioButton(panel, 22891, _("Początek"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		EndVAtime = new KaiRadioButton(panel, 22891, _("Koniec"));

		SE->Add(StartVAtime, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);
		SE->Add(EndVAtime, 1, wxEXPAND | wxRIGHT, 2);

		MoveToVideoTime = new KaiCheckBox(panel, ID_VIDEO, _("Przesuń znacznik\ndo czasu wideo"));
		MoveToVideoTime->SetForegroundColour(WindowWarningElements);
		MoveToVideoTime->Enable(false);

		MoveToAudioTime = new KaiCheckBox(panel, ID_AUDIO, _("Przesuń znacznik\ndo czasu audio"));
		MoveToAudioTime->SetForegroundColour(WindowWarningElements);
		MoveToAudioTime->Enable(false);

		Connect(ID_VIDEO, ID_AUDIO, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&ShiftTimesWindow::AudioVideoTime);
		//TextColorPicker *picker = new TextColorPicker(this, AssColor(wxString("#AABBCC")));
		VAtiming->Add(SE, 0, wxEXPAND | wxTOP, 2);
		VAtiming->Add(MoveToVideoTime, 1, wxEXPAND | wxLEFT, 2);
		VAtiming->Add(MoveToAudioTime, 1, wxEXPAND | wxLEFT, 2);
		//VAtiming->Add(picker,0,wxEXPAND|wxLEFT,2);

		KaiStaticBoxSizer *timessizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Sposób przesuwania czasów"));
		choices.clear();
		choices.Add(_("Obydwa czasy"));
		choices.Add(_("Czas początkowy"));
		choices.Add(_("Czas końcowy"));

		WhichTimes = new KaiChoice(panel, 22888, wxDefaultPosition, wxDefaultSize, choices, KAI_SCROLL_ON_FOCUS);
		WhichTimes->Enable(form != TMP);

		timessizer->Add(WhichTimes, 0, wxEXPAND | wxRIGHT | wxTOP | wxLEFT, 2);

		KaiStaticBoxSizer *cesizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Korekcja czasów końcowych"));
		choices.clear();
		choices.Add(_("Pozostaw bez zmian"));
		choices.Add(_("Skoryguj nachodzące czasy"));
		choices.Add(_("Nowe czasy"));
		EndTimeCorrection = new KaiChoice(panel, 22888, wxDefaultPosition, wxSize(130, -1), choices, KAI_SCROLL_ON_FOCUS);
		EndTimeCorrection->SetSelection(0);
		cesizer->Add(EndTimeCorrection, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);

		LeadIn = NULL;
		Main->Add(profileSizer, 0, wxEXPAND | wxALL, 2);
		Main->Add(timesizer, 0, wxEXPAND | wxALL, 2);
		Main->Add(VAtiming, 0, wxEXPAND | wxALL, 2);
		Main->Add(linesizer, 0, wxEXPAND | wxALL, 2);
		Main->Add(timessizer, 0, wxEXPAND | wxALL, 2);
		Main->Add(cesizer, 0, wxEXPAND | wxALL, 2);

		panel->SetSizerAndFit(Main);
		Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ShiftTimesWindow::OnEdition, this, 22888);
		Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &ShiftTimesWindow::OnEdition, this, 22889);
		Bind(wxEVT_COMMAND_TEXT_UPDATED, &ShiftTimesWindow::OnEdition, this, 22890);
		Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &ShiftTimesWindow::OnEdition, this, 22891);
	}
	else{
		int pe = Options.GetInt(PostprocessorEnabling);
		liosizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Wstęp i zakończenie"));
		MoveTime = new MappedButton(panel, GLOBAL_SHIFT_TIMES, _("Uruchom postprocessor"), _("Uruchom postprocessor"), wxDefaultPosition, wxDefaultSize, GLOBAL_HOTKEY);
		Main->Add(MoveTime, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 6);
		wxBoxSizer *leadinSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *leadoutSizer = new wxBoxSizer(wxHORIZONTAL);
		LeadIn = new KaiCheckBox(panel, -1, _("Wstęp"), wxDefaultPosition, wxSize(-1, -1));
		LeadIn->SetValue((pe & 1) > 0);
		LITime = new NumCtrl(panel, -1, Options.GetString(PostprocessorLeadIn), -10000, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		LeadOut = new KaiCheckBox(panel, -1, _("Zakończenie"), wxDefaultPosition, wxSize(-1, -1));
		LeadOut->SetValue((pe & 2) > 0);
		LOTime = new NumCtrl(panel, -1, Options.GetString(PostprocessorLeadOut), -10000, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);

		leadinSizer->Add(LeadIn, 1, wxEXPAND, 0);
		leadinSizer->Add(LITime, 0);
		leadoutSizer->Add(LeadOut, 1, wxEXPAND, 0);
		leadoutSizer->Add(LOTime, 0);

		liosizer->Add(leadinSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
		liosizer->Add(leadoutSizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);

		consizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Ustaw czasy jako ciągłe"));
		Continous = new KaiCheckBox(panel, -1, _("Włącz"));
		Continous->SetValue((pe & 4) > 0);

		consizer->Add(Continous, 0, wxEXPAND | wxALL, 2);

		wxBoxSizer *ThresStartSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *ThresEndSizer = new wxBoxSizer(wxHORIZONTAL);
		ThresStart = new NumCtrl(panel, -1, Options.GetString(PostprocessorThresholdStart), 0, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		ThresEnd = new NumCtrl(panel, -1, Options.GetString(PostprocessorThresholdEnd), 0, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);

		ThresStartSizer->Add(new KaiStaticText(panel, -1, _("Próg czasu początku"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		ThresStartSizer->Add(ThresStart, 0);
		ThresEndSizer->Add(new KaiStaticText(panel, -1, _("Próg czasu końca"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		ThresEndSizer->Add(ThresEnd, 0);

		consizer->Add(ThresStartSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		consizer->Add(ThresEndSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);

		snapsizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Wyrównaj do klatek kluczowych"));
		SnapKF = new KaiCheckBox(panel, -1, _("Włącz"));
		SnapKF->Enable(false);
		SnapKF->SetValue((pe & 8) > 0);
		snapsizer->Add(SnapKF, 0, wxEXPAND | wxALL, 2);

		wxBoxSizer *BeforeStartSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *AfterStartSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *BeforeEndSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *AfterEndSizer = new wxBoxSizer(wxHORIZONTAL);
		BeforeStart = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeBeforeStart), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		AfterStart = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeAfterStart), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		BeforeEnd = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeBeforeEnd), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		AfterEnd = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeAfterEnd), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);

		BeforeStartSizer->Add(new KaiStaticText(panel, -1, _("Przed czasem początku"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		BeforeStartSizer->Add(BeforeStart, 0);
		AfterStartSizer->Add(new KaiStaticText(panel, -1, _("Po czasie początku"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		AfterStartSizer->Add(AfterStart, 0);
		BeforeEndSizer->Add(new KaiStaticText(panel, -1, _("Przed czasem końca"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		BeforeEndSizer->Add(BeforeEnd, 0);
		AfterEndSizer->Add(new KaiStaticText(panel, -1, _("Po czasie końca"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		AfterEndSizer->Add(AfterEnd, 0);

		snapsizer->Add(BeforeStartSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		snapsizer->Add(AfterStartSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		snapsizer->Add(BeforeEndSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		snapsizer->Add(AfterEndSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);

		Main->Add(linesizer, 0, wxEXPAND | wxALL, 2);
		Main->Add((wxSizer*)liosizer, 0, wxEXPAND | wxALL, 2);
		Main->Add((wxSizer*)consizer, 0, wxEXPAND | wxALL, 2);
		Main->Add((wxSizer*)snapsizer, 0, wxEXPAND | wxALL, 2);
		panel->SetSizerAndFit(Main);
	}
	DoTooltips(normal);
	Connect(GLOBAL_SHIFT_TIMES, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ShiftTimesWindow::OnOKClick);
	Connect(ID_BSTYLE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ShiftTimesWindow::OnAddStyles);
	Connect(22999, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ShiftTimesWindow::CollapsePane);
}

void ShiftTimesWindow::OnOKClick(wxCommandEvent& event)
{
	SaveOptions();
	int acid = event.GetId();
	TabPanel *tab = Kai->GetTab();
	if (acid == GLOBAL_SHIFT_TIMES){
		tab->Grid->ChangeTimes((!LeadIn) ? TimeText->HasShownFrames() : false);
	}
	else if (acid == ID_CLOSE){
		Hide();
		tab->BoxSizer1->Layout();
	}
	tab->Grid->SetFocus();
}


void ShiftTimesWindow::OnSize(wxSizeEvent& event)
{
	int h, gw, gh;
	TabPanel* cur = (TabPanel*)GetParent();
	cur->Grid->GetClientSize(&gw, &gh);
	int w;
	panel->GetBestSize(&w, &h);
	int ctw, cth;
	GetSize(&ctw, &cth);
	if (!isscrollbar && gh < h)//pojawianie scrollbara
	{
		isscrollbar = true;
		SetMinSize(wxSize(w + 17, h));
		cur->BoxSizer3->Layout();
		scroll->SetSize(w - 1, 0, 17, gh);
		scroll->SetScrollbar(scPos, gh, h, gh - 10);
		scroll->Show();

	}
	else if (isscrollbar && gh >= h)//znikanie scrollbara
	{
		isscrollbar = false;
		scPos = 0;
		scroll->Hide();
		scroll->SetScrollbar(scPos, gh, h, gh - 10);
		SetMinSize(wxSize(w, h));
		panel->SetPosition(wxPoint(0, scPos));
		cur->BoxSizer3->Layout();
	}
	else if (scroll->IsShown()){
		scroll->SetSize(ctw - 18, 0, 17, gh);
		scroll->SetScrollbar(scPos, gh, h, gh - 10);
		if (scPos != scroll->GetScrollPos()){
			scPos = scroll->GetScrollPos();
			panel->SetPosition(wxPoint(0, -scPos));
			//panel->Refresh(false);
		}
	}
	else if (!isscrollbar && ctw != w){
		SetMinSize(wxSize(w, h));
		cur->BoxSizer3->Layout();
	}

}

void ShiftTimesWindow::DoTooltips(bool normal /*= true*/)
{
	WhichLines->SetToolTip(_("Wybór linijek do przesunięcia"));
	AddStyles->SetToolTip(_("Wybierz style z listy"));
	Stylestext->SetToolTip(_("Przesuń według następujących stylów (oddzielone średnikiem)"));
	if (normal){
		TimeText->SetToolTip(_("Czas przesunięcia"));
		MoveToVideoTime->SetToolTip(_("Przesuwanie zaznaczonej linijki\ndo czasu wideo ± czas przesunięcia"));
		MoveToAudioTime->SetToolTip(_("Przesuwanie zaznaczonej linijki do czasu\nznacznika audio ± czas przesunięcia"));
		StartVAtime->SetToolTip(_("Przesuwa czas początkowy do czasu wideo / audio"));
		EndVAtime->SetToolTip(_("Przesuwa czas końcowy do czasu wideo / audio"));
		Forward->SetToolTip(_("Opóźnia napisy"));
		Backward->SetToolTip(_("Przyspiesza napisy"));
		DisplayFrames->SetToolTip(_("Przesuwa napisy o ustawiony czas / klatki"));
		MoveTagTimes->SetToolTip(_("Przesuwa czasy tagów \\move, \\t, \\fad tak,\nby ich pozycja na wideo się nie zmieniła\n(spowalnia przesuwanie czasów)"));
		WhichTimes->SetToolTip(_("Wybór czasów do przesunięcia"));
		EndTimeCorrection->SetToolTip(_("Korekcja czasów końcowych, gdy są niewłaściwe albo nachodzą na siebie"));
	}
	else{
		LeadIn->SetToolTip(_("Wstawia wstęp do czasu początkowego, dobre przy stosowaniu fad"));
		LITime->SetToolTip(_("Czas wstępu w milisekundach"));
		LeadOut->SetToolTip(_("Wstawia zakończenie do czasu końcowego, dobre przy stosowaniu fad"));
		LOTime->SetToolTip(_("Czas zakończenia w milisekundach"));
		ThresStart->SetToolTip(_("Próg wydłużania czasu początkowego"));
		ThresEnd->SetToolTip(_("Próg wydłużania czasu końcowego"));
		BeforeStart->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\nprzed czasem początkowym w milisekundach"));
		AfterStart->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\npo czasie początkowym w milisekundach"));
		BeforeEnd->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\nprzed czasem końcowym w milisekundach"));
		AfterEnd->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\npo czasie końcowym w milisekundach"));
	}
}

void ShiftTimesWindow::AudioVideoTime(wxCommandEvent &event)
{
	int id = event.GetId();
	if (id == ID_VIDEO && MoveToVideoTime->GetValue()){
		MoveToAudioTime->SetValue(false);
	}
	else if (id == ID_AUDIO && MoveToAudioTime->GetValue()){
		MoveToVideoTime->SetValue(false);
	}
	OnEdition(event);
}

void ShiftTimesWindow::RefVals(ShiftTimesWindow *secondWindow)
{
	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 
	//8 Move to audio time 16 display times / frames 32 move tag times;
	if (secondWindow && (!secondWindow->LeadIn != !LeadIn)){
		wxCommandEvent evt;
		evt.SetId(22999);
		evt.SetInt(1000);
		CollapsePane(evt);
		Contents();
	}
	int mto = Options.GetInt(MoveTimesOptions);

	if (!LeadIn){

		STime ct = (secondWindow) ? secondWindow->TimeText->GetTime() : STime(Options.GetInt(MoveTimesTime), Options.GetInt(MoveTimesFrames));
		bool dispTimes = DisplayFrames->GetValue();
		DisplayFrames->SetValue((secondWindow) ? secondWindow->DisplayFrames->GetValue() : (mto & 16) > 0);
		TabPanel *tab = ((TabPanel*)GetParent());
		if (secondWindow && (secondWindow->DisplayFrames->GetValue() != dispTimes)){
			//it uses times as true
			ChangeDisplayUnits(!DisplayFrames->GetValue());
			
		}
		else if (!tab->Video->VFF){
			if (DisplayFrames->GetValue()){ ChangeDisplayUnits(true); }
			DisplayFrames->Enable(false);
		}
		if (secondWindow){
			wxArrayString list;
			secondWindow->ProfilesList->GetArray(&list);
			ProfilesList->PutArray(&list);	
			ProfilesList->SetSelection(secondWindow->ProfilesList->GetSelection());
		}
		TimeText->SetTime(ct);
		MoveToVideoTime->SetValue((secondWindow) ? secondWindow->MoveToVideoTime->GetValue() : (mto & 4) > 0);
		MoveToAudioTime->SetValue((secondWindow) ? secondWindow->MoveToAudioTime->GetValue() : (mto & 8) > 0);
		Forward->SetValue((secondWindow) ? secondWindow->Forward->GetValue() : (mto & 1) > 0);
		Backward->SetValue((secondWindow) ? secondWindow->Backward->GetValue() : (mto & 1) == 0);
		DisplayFrames->SetValue((secondWindow) ? secondWindow->DisplayFrames->GetValue() : (mto & 16) > 0);
		MoveTagTimes->SetValue((secondWindow) ? secondWindow->MoveTagTimes->GetValue() : (mto & 32) > 0);
	}
	Stylestext->SetValue((secondWindow) ? secondWindow->Stylestext->GetValue() : Options.GetString(MoveTimesStyles));

	int cm = (secondWindow) ? secondWindow->WhichLines->GetSelection() : Options.GetInt(MoveTimesWhichLines);
	if (cm > (int)WhichLines->GetCount()){ cm = 0; }
	WhichLines->SetSelection(cm);
	if (!LeadIn){
		WhichTimes->SetSelection((secondWindow) ? secondWindow->WhichTimes->GetSelection() : Options.GetInt(MoveTimesWhichTimes));

		if ((secondWindow) ? secondWindow->StartVAtime->GetValue() : (mto & 2) > 0){ StartVAtime->SetValue(true); }
		else{ EndVAtime->SetValue(true); }

		EndTimeCorrection->SetSelection((secondWindow) ? secondWindow->EndTimeCorrection->GetSelection() : Options.GetInt(MoveTimesCorrectEndTimes));
	}
	int enables = Options.GetInt(PostprocessorEnabling);

	if (LeadIn && secondWindow && secondWindow->LeadIn){
		LeadIn->SetValue((secondWindow) ? secondWindow->LeadIn->GetValue() : (enables & 1) > 0);
		LeadOut->SetValue((secondWindow) ? secondWindow->LeadOut->GetValue() : (enables & 2) > 0);
		Continous->SetValue((secondWindow) ? secondWindow->Continous->GetValue() : (enables & 4) > 0);
		SnapKF->SetValue((secondWindow) ? secondWindow->SnapKF->GetValue() : (enables & 8) > 0);
		LITime->SetInt((secondWindow) ? secondWindow->LITime->GetInt() : Options.GetInt(PostprocessorLeadIn));
		LOTime->SetInt((secondWindow) ? secondWindow->LOTime->GetInt() : Options.GetInt(PostprocessorLeadOut));
		ThresStart->SetInt((secondWindow) ? secondWindow->ThresStart->GetInt() : Options.GetInt(PostprocessorThresholdStart));
		ThresEnd->SetInt((secondWindow) ? secondWindow->ThresEnd->GetInt() : Options.GetInt(PostprocessorThresholdEnd));
		BeforeStart->SetInt((secondWindow) ? secondWindow->BeforeStart->GetInt() : Options.GetInt(PostprocessorKeyframeBeforeStart));
		AfterStart->SetInt((secondWindow) ? secondWindow->AfterStart->GetInt() : Options.GetInt(PostprocessorKeyframeAfterStart));
		BeforeEnd->SetInt((secondWindow) ? secondWindow->BeforeEnd->GetInt() : Options.GetInt(PostprocessorKeyframeBeforeEnd));
		AfterEnd->SetInt((secondWindow) ? secondWindow->AfterEnd->GetInt() : Options.GetInt(PostprocessorKeyframeAfterEnd));
	}

}

void ShiftTimesWindow::CollapsePane(wxCommandEvent &event)
{
	bool collapsed = (LeadIn == NULL);
	int pe = Options.GetInt(PostprocessorEnabling);
	Options.SetInt(PostprocessorEnabling, pe ^ 16);
	if (!collapsed){
		Options.SetInt(PostprocessorLeadIn, LITime->GetInt());
		Options.SetInt(PostprocessorLeadOut, LOTime->GetInt());
		Options.SetInt(PostprocessorThresholdStart, ThresStart->GetInt());
		Options.SetInt(PostprocessorThresholdEnd, ThresEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeStart, BeforeStart->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterStart, AfterStart->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeEnd, BeforeEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterEnd, AfterEnd->GetInt());
		//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
		//int peres= (LeadIn->GetValue())? 1 : 0
		Options.SetInt(PostprocessorEnabling, (int)LeadIn->GetValue() + ((int)LeadOut->GetValue() * 2) + ((int)Continous->GetValue() * 4) + ((int)SnapKF->GetValue() * 8));
	}
	Freeze();
	panel->Destroy();
	LeadIn = NULL;
	panel = new wxWindow(this, -1);
	CreateControls(!collapsed);
	Thaw();
	if (collapsed){

		if (event.GetId() == 22999){
			//((TabPanel*)GetParent())->BoxSizer3->Layout();
			isscrollbar = false;
			wxSizeEvent evt;
			OnSize(evt);
		}
		//wxSizeEvent evt;
		//OnSize(evt);
	}
	else{

		//wxSizeEvent evt;
		//OnSize(evt);
		int w, h, gw, gh;
		TabPanel* cur = (TabPanel*)GetParent();
		cur->Grid->GetClientSize(&gw, &gh);
		panel->GetBestSize(&w, &h);
		scPos = 0;
		if (gh < h)//pojawianie scrollbara
		{
			SetMinSize(wxSize(w + 17, h));
			cur->BoxSizer3->Layout();
			scroll->SetSize(w, 0, 17, gh);
			scroll->SetScrollbar(scPos, gh, h, gh - 10);

		}
		else if (gh >= h)//znikanie scrollbara
		{
			isscrollbar = false;
			scroll->Hide();
			scroll->SetScrollbar(scPos, gh, h, gh - 10);
			SetMinSize(wxSize(w, h));
			cur->BoxSizer3->Layout();
		}

		panel->SetPosition(wxPoint(0, -scPos));
		//panel->Update();
	}
	//trick to prevent recursive stack overflow
	if (event.GetInt() != 1000){
		Contents(false);
		RefVals();
	}
}

void ShiftTimesWindow::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	if (scPos != newPos) {
		scPos = newPos;
		panel->SetPosition(wxPoint(0, -scPos));
		panel->Update();
	}
}

void ShiftTimesWindow::OnMouseScroll(wxMouseEvent& event)
{
	if (event.GetWheelRotation() != 0){
		int step = 30 * event.GetWheelRotation() / event.GetWheelDelta();
		scPos = scroll->SetScrollPos(scPos - step);
		panel->SetPosition(wxPoint(0, -scPos));
		panel->Update();
	}
}

void ShiftTimesWindow::OnChangeDisplayUnits(wxCommandEvent& event)
{
	ChangeDisplayUnits(!DisplayFrames->GetValue());
	OnEdition(event);
}

void ShiftTimesWindow::ChangeDisplayUnits(bool times)
{
	STime ct = TimeText->GetTime();
	if (times){
		TimeText->ShowFrames(false);
		ct.mstime = Options.GetInt(MoveTimesTime);
		TimeText->SetTime(ct);
	}
	else{
		TimeText->ShowFrames(true);
		ct.orgframe = Options.GetInt(MoveTimesFrames);
		TimeText->SetTime(ct);
	}
}

void ShiftTimesWindow::GetProfilesNames(wxArrayString &list)
{
	wxArrayString fullProfiles;
	Options.GetTable(SHIFT_TIMES_PROFILES, fullProfiles, L"\f", wxTOKEN_STRTOK);
	for (auto profile : fullProfiles){
		wxString profileName = profile.BeforeFirst(L':');
		list.Add(profileName);
	}
}

void ShiftTimesWindow::CreateProfile(const wxString &name, bool overwrite)
{
	wxString moveToAudioTime = (MoveToAudioTime->GetValue()) ? L"1" : L"0";
	wxString moveToVideoTime = (MoveToVideoTime->GetValue()) ? L"1" : L"0";
	wxString moveToStartTimes = (StartVAtime->GetValue()) ? L"1" : L"0";
	wxString moveTagTimes = (MoveTagTimes->GetValue()) ? L"1" : L"0";
	wxString forward = (Forward->GetValue()) ? L"1" : L"0";
	wxString frames = (DisplayFrames->GetValue()) ? L"1" : L"0";
	wxString profile = Options.GetString(SHIFT_TIMES_PROFILES);
	
	wxString newProfile;
	newProfile << name << L": Time: " << TimeText->GetTime().mstime <<
		L" Forward: " << forward << L" Frames: " << frames <<
		L" MoveTagTimes: " << moveTagTimes << L" MoveToStartTimes: " <<
		moveToStartTimes << L" MoveToVideoTime: " << moveToVideoTime <<
		L" MoveToVideoTime: " << moveToAudioTime << L" WhichLines: " <<
		WhichLines->GetSelection() << L" StylesText: " <<
		Stylestext->GetValue() << L" WhichTimes: " << WhichTimes->GetSelection() <<
		L" EndTimeCorrection: " << EndTimeCorrection->GetSelection() << L"\f";

	if (overwrite){
		wxStringTokenizer tokenizer(profile, L"\f", wxTOKEN_STRTOK);
		while (tokenizer.HasMoreTokens()){
			wxString token = tokenizer.GetNextToken();
			if (!token.StartsWith(name + L":")){
				newProfile << token << L"\f";
			}
		}
		profile.Empty();
	}

	Options.SetString(SHIFT_TIMES_PROFILES, profile.Prepend(newProfile));
	wxArrayString profilesList;
	GetProfilesNames(profilesList);
	ProfilesList->PutArray(&profilesList);
	ProfilesList->SetSelection(0);
}

void ShiftTimesWindow::SetProfile(const wxString &name)
{
	wxArrayString fullProfiles;
	wxString profileWithoutName;
	Options.GetTable(SHIFT_TIMES_PROFILES, fullProfiles, L"\f", wxTOKEN_STRTOK);
	//get text profile by name
	for (auto profile : fullProfiles){
		wxString profileName = profile.BeforeFirst(L':');
		if (profileName == name){
			profileWithoutName = profile.AfterFirst(L':');
			profileWithoutName.Remove(0, 1);
			break;
		}
	}
	wxStringTokenizer tokenizer(profileWithoutName, L" ", wxTOKEN_RET_EMPTY_ALL);
	tokenizer.GetNextToken();
	//time
	if (tokenizer.HasMoreTokens()){
		TimeText->SetTime(STime(wxAtoi(tokenizer.GetNextToken()), Options.GetInt(MoveTimesFrames)));
		tokenizer.GetNextToken();
	}
	//forward backward
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		if (token == L"1"){
			Forward->SetValue(true);
		}
		else{
			Backward->SetValue(true);
		}
		tokenizer.GetNextToken();
	}
	//frames
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		bool displayFrames = DisplayFrames->GetValue();
		bool newDisplayFrames = token == L"1";
		if (displayFrames != newDisplayFrames){
			TabPanel *tab = (TabPanel *)GetParent();
			if (tab->Video->VFF){
				//there are times as true
				ChangeDisplayUnits(!newDisplayFrames);
			}
			else if (displayFrames){
				ChangeDisplayUnits(true);
			}
		}
		DisplayFrames->SetValue(token == L"1");
		tokenizer.GetNextToken();
	}
	//move tag times
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		MoveTagTimes->SetValue(token == L"1");
		tokenizer.GetNextToken();
	}
	//move to start/end time video/audio
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		if (token == L"1"){
			StartVAtime->SetValue(true);
		}
		else{
			EndVAtime->SetValue(true);
		}
		tokenizer.GetNextToken();
	}
	//move to video time
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		MoveToVideoTime->SetValue(token == L"1");
		tokenizer.GetNextToken();
	}
	//move to audio time
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		MoveToAudioTime->SetValue(token == L"1");
		tokenizer.GetNextToken();
	}
	//which lines
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		WhichLines->SetSelection(wxAtoi(token));
		tokenizer.GetNextToken();
	}
	//chosen styles as text
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		Stylestext->SetValue(token);
		tokenizer.GetNextToken();
	}
	//which times
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		WhichTimes->SetSelection(wxAtoi(token));
		tokenizer.GetNextToken();
	}
	//which times
	if (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		EndTimeCorrection->SetSelection(wxAtoi(token));
		tokenizer.GetNextToken();
	}
}

void ShiftTimesWindow::OnAddProfile(wxCommandEvent& event)
{
	wxArrayString profilesNames;
	GetProfilesNames(profilesNames);
	wxSize plSize = ProfilesList->GetClientSize();
	wxPoint plPos = ProfilesList->GetPosition();
	plPos = ClientToScreen(plPos);
	ProfileEdition pe(this, profilesNames, wxPoint(plPos.x, plPos.y + plSize.y));
	if (pe.ShowModal() == wxID_OK){
		bool exist = false;
		wxString profileName = pe.GetProfileName(&exist);
		CreateProfile(profileName, exist);
	}
}

void ShiftTimesWindow::OnRemoveProfile(wxCommandEvent& event)
{
	int selectedProfile = ProfilesList->GetSelection();
	wxSize plSize = ProfilesList->GetClientSize();
	wxPoint plPos = ProfilesList->GetPosition();
	plPos = ClientToScreen(plPos);
	//here it's possible rather it needs info
	if (selectedProfile < 0){
		KaiMessageBox(_("Na liście nie wybrano profilu do usunięcia"), 
			_("Informacja"), wxOK, this, wxPoint(0, plPos.y + plSize.y));
		return;
	}
	wxString profileName = ProfilesList->GetString(selectedProfile);
	int result = KaiMessageBox(wxString::Format(_("Na pewno chcesz usunąć profil o nazwie \"%s\""), profileName), 
		_("Informacja"), wxYES_NO, this, wxPoint(0, plPos.y + plSize.y));
	if (result != wxYES){
		return;
	}
	
	wxArrayString fullProfiles;
	Options.GetTable(SHIFT_TIMES_PROFILES, fullProfiles, L"\f", wxTOKEN_STRTOK);
	wxString newProfiles;
	//make a new profiles
	for (auto profile : fullProfiles){
		wxString profileName1 = profile.BeforeFirst(L':');
		//add olny different profiles then removed
		if (profileName1 != profileName){
			newProfiles << profile << L"\f";
		}
	}
	Options.SetString(SHIFT_TIMES_PROFILES, newProfiles);
	wxArrayString profilesList;
	GetProfilesNames(profilesList);
	ProfilesList->PutArray(&profilesList);
	ProfilesList->SetSelection(-1);
}

void ShiftTimesWindow::OnChangeProfile(wxCommandEvent& event)
{
	int selectedProfile = ProfilesList->GetSelection();
	if (selectedProfile < 0)
		return;
	//no error for now I do not know if it's possible
	wxString profileName = ProfilesList->GetString(selectedProfile);
	SetProfile(profileName);
}

void ShiftTimesWindow::OnEdition(wxCommandEvent& event)
{
	if (LeadIn || ProfilesList->GetSelection() < 0)
		return;

	ProfilesList->SetSelection(-1);
}

bool ShiftTimesWindow::SetFont(const wxFont &font)
{
	wxFont stFont = font;
	stFont.SetPointSize(font.GetPointSize() - 2);

	/*const wxWindowList& siblings = panel->GetChildren();
	for (wxWindowList::compatibility_iterator nodeAfter = siblings.GetFirst();
		nodeAfter;
		nodeAfter = nodeAfter->GetNext()){

		wxWindow *win = nodeAfter->GetData();
		win->SetFont(stFont);
	}*/

	wxWindow::SetFont(stFont);
	//panel->Layout();
	SaveOptions();
	Freeze();
	bool normal = LeadIn == NULL;
	panel->Destroy();
	LeadIn = NULL;
	panel = new wxWindow(this, -1);
	CreateControls(normal);
	Thaw();
	isscrollbar = false;
	wxSizeEvent evt;
	OnSize(evt);
	RefVals();
	TabPanel* cur = (TabPanel*)GetParent();
	cur->BoxSizer3->Layout();
	return true;
}

BEGIN_EVENT_TABLE(ShiftTimesWindow, wxWindow)
EVT_SIZE(ShiftTimesWindow::OnSize)
EVT_COMMAND_SCROLL_THUMBTRACK(5558, ShiftTimesWindow::OnScroll)
EVT_MOUSEWHEEL(ShiftTimesWindow::OnMouseScroll)
END_EVENT_TABLE()
