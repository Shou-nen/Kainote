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

#include "SubsGrid.h"


#include <wx/intl.h>
#include <wx/string.h>
#include "Utils.h"
#include <wx/clipbrd.h>
#include "KainoteMain.h"
#include "Hotkeys.h"
#include "OpennWrite.h"
#include "TLDialog.h"
#include "MKVWrap.h"
#include "Stylelistbox.h"
#include "Menu.h"
#include <wx/regex.h>
#include "KaiMessageBox.h"
#include "SubsGridFiltering.h"
#include "SubsGridDialogs.h"

SubsGrid::SubsGrid(wxWindow* parent, KainoteFrame* kfparent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	:SubsGridWindow(parent, id, pos, size, style)
{
	Kai = kfparent;
	ignoreFiltered = Options.GetBool(GridIgnoreFiltering);
	//jak już wszystko będzie działało to można wywalić albo dać if(!autofilter)
	//Options.SetInt(GridFilterBy, 0);
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt){
		MenuItem *item = (MenuItem*)evt.GetClientData();
		int id = item->id;
		if (id > 5000 && id < 5555){
			int id5000 = (id - 5000);
			if (visibleColumns & id5000){ visibleColumns ^= id5000; }
			else{ visibleColumns |= id5000; }
			SpellErrors.clear();
			Options.SetInt(GridHideCollums, visibleColumns);
			RefreshColumns();
		}
		else if (id == GRID_FILTER_INVERTED){
			Options.SetBool(GridFilterInverted, !Options.GetBool(GridFilterInverted));
		}
		else if (id >= FilterByStyles && id <= FilterByUntranslated){
			int filterBy = Options.GetInt(GridFilterBy);
			int addToFilterBy = pow(2, (id - FilterByStyles));
			if (item->check){
				filterBy |= addToFilterBy;
			}
			else{
				filterBy ^= addToFilterBy;
			}
			Options.SetInt(GridFilterBy, filterBy);
		}//styles checking
		else if (id == 4448){
			int filterBy = Options.GetInt(GridFilterBy);
			if (!filterStyles.size() && (filterBy & FILTER_BY_STYLES))
				filterBy ^= FILTER_BY_STYLES;

			wxString &name = item->label;
			bool found = false;
			for (int i = 0; i < filterStyles.size(); i++){
				if (filterStyles[i] == name){
					if (!item->check){ filterStyles.RemoveAt(i); found = true; }
					break;
				}
			}
			if (!found && item->check){ filterStyles.Add(name); }
			Options.SetTable(GridFilterStyles, filterStyles, L",");
			if (filterStyles.size() > 0 && !(filterBy & FILTER_BY_STYLES)){
				Options.SetInt(GridFilterBy, filterBy | FILTER_BY_STYLES);
				Menu *parentMenu = NULL;
				MenuItem * parentItem = Menu::FindItemGlobally(FilterByStyles, &parentMenu);
				if (parentItem){
					parentItem->Check(true);
					if (parentMenu)
						parentMenu->RefreshMenu();
				}
			}
			else if (filterStyles.size() < 1 && (filterBy & FILTER_BY_STYLES)){
				Options.SetInt(GridFilterBy, filterBy ^ FILTER_BY_STYLES);
				Menu *parentMenu = NULL;
				MenuItem * parentItem = Menu::FindItemGlobally(FilterByStyles, &parentMenu);
				if (parentItem){
					parentItem->Check(false);
					if (parentMenu)
						parentMenu->RefreshMenu();
				}
			}
		}
		else if (id == GRID_FILTER_DO_NOT_RESET){
			Options.SetBool(GridAddToFilter, item->check);
		}
		else if (id == GRID_FILTER_AFTER_SUBS_LOAD){
			Options.SetBool(GridFilterAfterLoad, item->check);
		}
		else if (id == GRID_FILTER_IGNORE_IN_ACTIONS){
			Options.SetBool(GridIgnoreFiltering, item->check);
			ignoreFiltered = item->check;
		}
	}, ID_CHECK_EVENT);
}

SubsGrid::~SubsGrid()
{
}

void SubsGrid::ContextMenu(const wxPoint &pos)
{
	VideoCtrl *VB = ((TabPanel*)GetParent())->Video;
	VB->blockpaint = true;
	file->GetSelections(selections);
	int sels = selections.GetCount();
	Menu *menu = new Menu(GRID_HOTKEY);
	menu->SetMaxVisible(35);
	Menu *hidemenu = new Menu(GRID_HOTKEY);
	Menu *filterMenu = new Menu(GRID_HOTKEY);
	//hide submenu
	hidemenu->SetAccMenu(GRID_HIDE_LAYER, _("Ukryj warstwę"), _("Ukryj warstwę"), subsFormat < SRT, ITEM_CHECK)->Check((visibleColumns & LAYER) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_START, _("Ukryj czas początkowy"), _("Ukryj czas początkowy"), true, ITEM_CHECK)->Check((visibleColumns & START) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_END, _("Ukryj czas końcowy"), _("Ukryj czas końcowy"), subsFormat != TMP, ITEM_CHECK)->Check((visibleColumns & END) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_ACTOR, _("Ukryj aktora"), _("Ukryj aktora"), subsFormat < SRT, ITEM_CHECK)->Check((visibleColumns & ACTOR) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_STYLE, _("Ukryj styl"), _("Ukryj styl"), subsFormat < SRT, ITEM_CHECK)->Check((visibleColumns & STYLE) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_MARGINL, _("Ukryj lewy margines"), _("Ukryj lewy margines"), subsFormat < SRT, ITEM_CHECK)->Check((visibleColumns & MARGINL) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_MARGINR, _("Ukryj prawy margines"), _("Ukryj prawy margines"), subsFormat < SRT, ITEM_CHECK)->Check((visibleColumns & MARGINR) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_MARGINV, _("Ukryj pionowy margines"), _("Ukryj pionowy margines"), subsFormat < SRT, ITEM_CHECK)->Check((visibleColumns & MARGINV) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_EFFECT, _("Ukryj efekt"), _("Ukryj efekt"), subsFormat < SRT, ITEM_CHECK)->Check((visibleColumns & EFFECT) != 0);
	hidemenu->SetAccMenu(GRID_HIDE_CPS, _("Ukryj znaki na sekundę"), _("Ukryj znaki na sekundę"), true, ITEM_CHECK)->Check((visibleColumns & CPS) != 0);

	//styles menu
	Menu *stylesMenu = new Menu();
	std::vector<Styles*> *styles = file->GetStyleTable();
	wxArrayString optionsFilterStyles;
	Options.GetTable(GridFilterStyles, optionsFilterStyles, L",");
	filterStyles.clear();
	for (int i = 0; i < StylesSize(); i++){
		MenuItem * styleItem = stylesMenu->Append(4448, (*styles)[i]->Name, L"", true, NULL, NULL, ITEM_CHECK);
		if (optionsFilterStyles.Index((*styles)[i]->Name) != -1){ styleItem->Check(); filterStyles.Add((*styles)[i]->Name); }
	}
	//filter submenu
	int filterBy = Options.GetInt(GridFilterBy);
	bool isASS = subsFormat == ASS;
	filterMenu->SetAccMenu(GRID_FILTER_AFTER_SUBS_LOAD, _("Filtruj po wczytaniu napisów"), _("Nie obejmuje zaznaczonych linii"), isASS, ITEM_CHECK)->Check(Options.GetBool(GridFilterAfterLoad));
	filterMenu->SetAccMenu(GRID_FILTER_INVERTED, _("Filtrowanie odwrócone"), _("Filtrowanie odwrócone"), true, ITEM_CHECK)->Check(Options.GetBool(GridFilterInverted));
	filterMenu->SetAccMenu(GRID_FILTER_DO_NOT_RESET, _("Nie resetuj wcześniejszego filtrowania"), _("Nie resetuj wcześniejszego filtrowania"), true, ITEM_CHECK)->Check(Options.GetBool(GridAddToFilter));
	MenuItem *Item = new MenuItem(FilterByStyles, _("Ukryj linie ze stylami"), _("Ukryj linie ze stylami"), isASS, NULL, stylesMenu, ITEM_CHECK);
	filterMenu->SetAccMenu(Item, Item->label)->Check(filterStyles.size() > 0);
	filterMenu->SetAccMenu(FilterBySelections, _("Ukryj zaznaczone linie"), _("Ukryj zaznaczone linie"), sels > 0, ITEM_CHECK)->Check(filterBy & FILTER_BY_SELECTIONS && sels > 0);
	filterMenu->SetAccMenu(FilterByDialogues, _("Ukryj komentarze"), _("Ukryj komentarze"), isASS, ITEM_CHECK)->Check((filterBy & FILTER_BY_DIALOGUES) != 0);
	filterMenu->SetAccMenu(FilterByDoubtful, _("Pokaż niepewne"), _("Pokaż niepewne"), hasTLMode, ITEM_CHECK)->Check(filterBy & FILTER_BY_DOUBTFUL && hasTLMode);
	filterMenu->SetAccMenu(FilterByUntranslated, _("Pokaż nieprzetłumaczone"), _("Pokaż nieprzetłumaczone"), hasTLMode, ITEM_CHECK)->Check(filterBy & FILTER_BY_UNTRANSLATED && hasTLMode);
	filterMenu->SetAccMenu(GRID_FILTER, _("Filtruj"), _("Filtruj"));
	filterMenu->SetAccMenu(FilterByNothing, _("Wyłącz filtrowanie"), _("Wyłącz filtrowanie"))->Enable(isFiltered);

	bool isEnabled;
	isEnabled = (sels > 0);
	menu->SetAccMenu(InsertBefore, _("Wstaw &przed"))->Enable(isEnabled);
	menu->SetAccMenu(InsertAfter, _("Wstaw p&o"))->Enable(isEnabled);
	isEnabled = (isEnabled && Kai->GetTab()->Video->GetState() != None);
	menu->SetAccMenu(InsertBeforeVideo, _("Wstaw przed z &czasem wideo"))->Enable(isEnabled);
	menu->SetAccMenu(InsertAfterVideo, _("Wstaw po z c&zasem wideo"))->Enable(isEnabled);
	menu->SetAccMenu(InsertBeforeWithVideoFrame, _("Wstaw przed z czasem klatki wideo"))->Enable(isEnabled);
	menu->SetAccMenu(InsertAfterWithVideoFrame, _("Wstaw po z czasem klatki wideo"))->Enable(isEnabled);
	isEnabled = (sels > 0);
	menu->SetAccMenu(Duplicate, _("&Duplikuj linie"))->Enable(isEnabled);
	isEnabled = (sels == 2);
	menu->SetAccMenu(Swap, _("Za&mień"))->Enable(isEnabled);
	isEnabled = (sels >= 2 && sels <= 20);
	menu->SetAccMenu(Join, _("Złącz &linijki"))->Enable(isEnabled);
	isEnabled = (sels >= 2 && sels <= 50);
	menu->SetAccMenu(JoinToFirst, _("Złącz linijki zostaw pierwszą"))->Enable(isEnabled);
	menu->SetAccMenu(JoinToLast, _("Złącz linijki zostaw ostatnią"))->Enable(isEnabled);
	isEnabled = (sels > 0);
	menu->SetAccMenu(ContinousPrevious, _("Ustaw czasy jako ciągłe (poprzednia linijka)"))->Enable(isEnabled);
	menu->SetAccMenu(ContinousNext, _("Ustaw czasy jako ciągłe (następna linijka)"))->Enable(isEnabled);
	menu->SetAccMenu(Copy, _("Kopiuj\tCtrl-C"))->Enable(isEnabled);
	menu->SetAccMenu(Cut, _("Wytnij\tCtrl-X"))->Enable(isEnabled);
	menu->SetAccMenu(Paste, _("Wklej\tCtrl-V"));
	menu->SetAccMenu(CopyCollumns, _("Kopiuj kolumny"))->Enable(isEnabled);
	menu->SetAccMenu(PasteCollumns, _("Wklej kolumny"));
	menu->Append(4444, _("Ukryj kolumny"), hidemenu);
	menu->SetAccMenu(HideSelected, _("Ukryj zaznaczone linijki"))->Enable(sels > 0);
	menu->Append(4445, _("Filtrowanie"), filterMenu);
	menu->SetAccMenu(GRID_FILTER_IGNORE_IN_ACTIONS, _("Ignoruj filtrowanie przy akcjach"), "", true, ITEM_CHECK)->Check(ignoreFiltered); 
	menu->SetAccMenu(GRID_TREE_MAKE, _("Stwórz drzewko"))->Enable(sels > 0);
	menu->SetAccMenu(ShowPreview, _("Pokaż podgląd napisów"))->Enable(Notebook::GetTabs()->Size() > 1 && !preview);
	menu->SetAccMenu(NewFPS, _("Ustaw nowy FPS"));
	menu->SetAccMenu(FPSFromVideo, _("Ustaw FPS z wideo"))->Enable(Notebook::GetTab()->Video->GetState() != None && sels == 2);
	menu->SetAccMenu(PasteTranslation, _("Wklej tekst tłumaczenia"))->Enable(subsFormat < SRT && ((TabPanel*)GetParent())->SubsPath != "");
	menu->SetAccMenu(TranslationDialog, _("Okno przesuwania dialogów"))->Enable(showOriginal);
	menu->AppendSeparator();

	menu->SetAccMenu(RemoveText, _("Usuń tekst"))->Enable(isEnabled);
	menu->SetAccMenu(Remove, _("Usuń"))->Enable(isEnabled);
	menu->AppendSeparator();
	menu->SetAccMenu(FontCollectorID, _("Kolekcjoner czcionek"))->Enable(subsFormat < SRT);
	menu->SetAccMenu(SubsFromMKV, _("Wczytaj napisy z pliku MKV"))->Enable(Kai->GetTab()->VideoName.EndsWith(".mkv"));

	int Modifiers = 0;
	int id = menu->GetPopupMenuSelection(pos, this, &Modifiers);

	if (id < 0){ goto done; }

	if (Modifiers == wxMOD_SHIFT){
		if (id <= 5000){ goto done; }
		Hkeys.OnMapHkey(id, L"", this, GRID_HOTKEY);
		goto done;
	}
	OnAccelerator(wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, id));
done:
	delete menu;
	VB->blockpaint = false;
}


void SubsGrid::ContextMenuTree(const wxPoint &pos, int treeLine)
{
	int sels = file->SelectionsSize();
	Menu *menu = new Menu();
	menu->SetAccMenu(6789, _("Dodaj linie"))->Enable(sels > 0);
	menu->SetAccMenu(6790, _("Kopiuj drzewko"));
	menu->SetAccMenu(6791, _("Zmień opis"));
	menu->SetAccMenu(6792, _("Usuń"));
	int id = menu->GetPopupMenuSelection(pos, this);
	switch (id)
	{
	case 6789:
		TreeAddLines(treeLine);
		break;
	case 6790:
		TreeCopy(treeLine);
		break;
	case 6791:
		TreeChangeName(treeLine);
		break;
	case 6792:
		TreeRemove(treeLine);
		break;

	default:
		break;
	}

	delete menu;
}

void SubsGrid::OnInsertBefore()
{
	SaveSelections(true);
	int rw = currentLine;
	Dialogue *dialog = CopyDialogue(rw, false);
	dialog->Text = L"";
	dialog->TextTl = L"";
	dialog->End = dialog->Start;
	Dialogue *previousDialogue = GetDialogueWithOffset(rw, -1);
	if (previousDialogue && previousDialogue->End > dialog->Start){
		dialog->Start = previousDialogue->End;
	}
	else{ dialog->Start.Change(-4000); }
	markedLine = currentLine;
	InsertRows(rw, 1, dialog, false, true);
}

void SubsGrid::OnInsertAfter()
{
	SaveSelections(true);
	int rw = currentLine;
	Dialogue *dialog = CopyDialogue(rw, false);
	dialog->Text = L"";
	dialog->TextTl = L"";
	dialog->Start = dialog->End;
	Dialogue *nextDialogue = GetDialogueWithOffset(rw, +1);
	if (nextDialogue && nextDialogue->Start > dialog->End){
		dialog->End = nextDialogue->Start;
	}
	else{ dialog->End.Change(4000); }
	currentLine = markedLine = rw + 1;
	InsertRows(rw + 1, 1, dialog, false, true);
}

void SubsGrid::OnDuplicate()
{
	SaveSelections(true);
	int rw = selections[0];
	int rw1 = rw;
	size_t i = 0;
	std::vector<Dialogue *> dupl;
	while (i < selections.GetCount()){
		if (rw1 == selections[i]){ 
			Dialogue *dial = file->CopyDialogue(rw1, false);
			dupl.push_back(dial);
			i++; rw1++; 
		}
		else if (*GetDialogue(rw1)->isVisible){ 
			break; 
		}
		else
			rw1++;
	}

	if (dupl.size() > 0){
		InsertRows(rw1, dupl);
		dupl.clear();
	}
	file->InsertSelections(rw1, rw1 + i - 1, false, true);
	SetModified(GRID_DUPLICATE, true, false, rw1);
	Refresh(false);
}


void SubsGrid::OnJoin(wxCommandEvent &event)
{
	SaveSelections(true);
	long startTl = 0, endTl = 0, startOrg = 0, endOrg = 0;
	Edit->TextEdit->GetSelection(&startTl, &endTl);
	Edit->TextEditOrig->GetSelection(&startOrg, &endOrg);
	wxString ntext;
	wxString ntltext;
	wxString en1;
	int idd = event.GetId();
	if (idd == JoinWithPrevious){
		size_t prevLine = GetKeyFromPosition(currentLine, -1);
		if (currentLine <= prevLine){ return; }
		selections.Clear();
		selections.Add(prevLine);
		selections.Add(currentLine);
		en1 = L" ";
	}
	else if (idd == JoinWithNext){
		size_t nextLine = GetKeyFromPosition(currentLine, -1);
		if (currentLine >= nextLine){ return; }
		selections.Clear();
		selections.Add(currentLine);
		selections.Add(nextLine);
		en1 = L" ";
	}
	else{ en1 = L"\\N"; }


	Dialogue *dialc = file->CopyDialogue(selections[0]);
	currentLine = selections[0];
	int start = INT_MAX, end = 0;
	for (size_t i = 0; i < selections.size(); i++)
	{
		wxString en = (i == 0) ? L"" : en1;
		Dialogue *dial = GetDialogue(selections[i]);
		if (dial->Start.mstime < start){ start = dial->Start.mstime; }
		if (dial->End.mstime > end){ end = dial->End.mstime; }
		if (ntext == L""){ ntext = dial->Text; }
		else if (dial->Text != L""){ ntext << en << dial->Text; }
		if (ntltext == L""){ ntltext = dial->TextTl; }
		else if (dial->TextTl != L""){ ntltext << en << dial->TextTl; }
	}

	DeleteRow(selections[1], selections[selections.size() - 1] - selections[1] + 1);
	dialc->Start.NewTime(start);
	dialc->End.NewTime(end);
	dialc->Text = ntext;
	dialc->TextTl = ntltext;
	file->edited = true;
	SpellErrors.clear();
	SetModified((idd == JoinWithPrevious) ? GRID_JOIN_WITH_PREVIOUS :
		(idd == JoinWithNext) ? GRID_JOIN_WITH_NEXT : GRID_JOIN_LINES);
	RefreshColumns();
	Edit->TextEditOrig->SetSelection(startOrg, endOrg);
	Edit->TextEdit->SetSelection(startTl, endTl);
}

void SubsGrid::OnJoinToFirst(int id)
{
	SaveSelections(true);
	Dialogue *dialc = file->CopyDialogue(selections[0]);
	Dialogue *ldial = GetDialogue(selections[selections.size() - 1]);
	dialc->End = ldial->End;

	if (id == JoinToLast){
		dialc->Text = ldial->Text;
		dialc->TextTl = ldial->TextTl;
	}
	currentLine = selections[0];
	DeleteRow(selections[1], selections[selections.size() - 1] - selections[1] + 1);

	file->InsertSelection(selections[0]);
	SpellErrors.clear();
	SetModified((id == JoinToLast) ? GRID_JOIN_TO_LAST : GRID_JOIN_TO_FIRST);
	RefreshColumns();
}


void SubsGrid::OnPaste(int id)
{
	size_t row = FirstSelection();
	if (row == -1){ wxBell(); return; }
	SaveSelections(id != PasteCollumns);
	int collumns = 0;
	if (id == PasteCollumns){
		int numCollumns = (hasTLMode) ? 11 : 10;
		wxString pasteText = (hasTLMode) ? _("Tekst do oryginału") : _("Tekst");
		wxString arr[11] = { _("Warstwa"), _("Czas początkowy"), _("Czas końcowy"), _("Aktor"), _("Styl"), _("Margines lewy"), _("Margines prawy"), _("Margines pionowy"), _("Efekt"), pasteText, _("Tekst do tłumaczenia") };
		int vals[11] = { LAYER, START, END, ACTOR, STYLE, MARGINL, MARGINR, MARGINV, EFFECT, TXT, TXTTL };
		Stylelistbox slx(this, false, numCollumns, arr);
		int PasteCollumnsSelections = Options.GetInt(PasteCollumnsSelection);
		for (int j = 0; j < numCollumns; j++){
			if (PasteCollumnsSelections & vals[j]){
				Item * checkBox = slx.CheckListBox->GetItem(j, 0);
				if (checkBox)
					checkBox->modified = true;
			}
		}
		if (slx.ShowModal() == wxID_OK)
		{
			for (size_t v = 0; v < slx.CheckListBox->GetCount(); v++)
			{
				if (slx.CheckListBox->GetItem(v, 0)->modified){
					collumns |= vals[v];
				}
			}
			Options.SetInt(PasteCollumnsSelection, collumns);
			Options.SaveOptions();
		}
		else{ return; }

	}
	Freeze();
	wxString whatpaste;
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			whatpaste = data.GetText();
		}
		wxTheClipboard->Close();
		if (whatpaste == L""){ Thaw(); return; }
	}
	wxStringTokenizer wpaste(whatpaste, L"\n", wxTOKEN_STRTOK);
	int cttkns = wpaste.CountTokens();
	int rws = (id == PasteCollumns) ? 0 : row;
	std::vector<Dialogue*> tmpdial;
	wxString token;
	wxString tmptoken;
	int startline = rws;
	bool hasTree = false;
	while (wpaste.HasMoreTokens())
	{
		Dialogue *newdial = NULL;
		token = (tmptoken.empty()) ? wpaste.NextToken().Trim(false).Trim() : tmptoken;
		if (IsNumber(token)){
			token.Empty();
			while (wpaste.HasMoreTokens()){
				tmptoken = wpaste.NextToken().Trim(false).Trim();
				if (IsNumber(tmptoken)){ break; }
				token += L"\r\n" + tmptoken;
			}

		}
		newdial = new Dialogue(token);
		if (!newdial){ continue; }
		newdial->ChangeDialogueState(1);
		//TODO przetestować czy wstawia poprawnie w text tłumaczenia
		if (collumns & TXTTL){
			newdial->TextTl = newdial->Text;
		}
		if (newdial->Format != subsFormat){ newdial->Convert(subsFormat); }
		if (newdial->NonDialogue){ newdial->NonDialogue = false; newdial->IsComment = false; }
		if (id == Paste){
			if (newdial->treeState == TREE_DESCRIPTION)
				hasTree = true;
			else if (!hasTree && newdial->treeState > TREE_DESCRIPTION){
				newdial->treeState = 0;
				newdial->isVisible = VISIBLE;
			}
			else if (hasTree)
				hasTree = true;

			tmpdial.push_back(newdial);
		}
		else{
			if (rws < (int)selections.GetCount()/* && selarr[rws] < GetCount()*/){
				ChangeCell(collumns, selections[rws], newdial);
			}
			delete newdial;
		}
		rws++;
	}

	if (tmpdial.size()>0){
		InsertRows(row, tmpdial, true);
		file->InsertSelections(startline, rws - 1);
	}
	scrollPosition += cttkns;
	SetModified((id == Paste) ? GRID_PASTE : GRID_PASTE_COLLUMNS, true, false, FirstSelection());
	Thaw();
	RefreshColumns();
}

void SubsGrid::CopyRows(int id)
{
	int cols = 0;
	if (id == CopyCollumns){
		wxString arr[] = { _("Warstwa"), _("Czas początkowy"), _("Czas końcowy"), _("Aktor"), _("Styl"), _("Margines lewy"), _("Margines prawy"), _("Margines pionowy"), _("Efekt"), _("Tekst"), _("Tekst bez tagów") };
		int vals[] = { LAYER, START, END, ACTOR, STYLE, MARGINL, MARGINR, MARGINV, EFFECT, TXT, TXTTL };
		Stylelistbox slx(this, false, 11, arr);
		int PasteCollumnsSelections = Options.GetInt(CopyCollumnsSelection);
		for (int j = 0; j < 11; j++){
			if (PasteCollumnsSelections & vals[j]){
				Item * checkBox = slx.CheckListBox->GetItem(j, 0);
				if (checkBox)
					checkBox->modified = true;
			}
		}
		if (slx.ShowModal() == wxID_OK)
		{
			for (size_t v = 0; v < slx.CheckListBox->GetCount(); v++)
			{

				if (slx.CheckListBox->GetItem(v, 0)->modified){
					cols |= vals[v];
				}
			}
			Options.SetInt(CopyCollumnsSelection, cols);
			Options.SaveOptions();
		}
		else{ return; }

	}
	wxString whatcopy;
	for (size_t i = 0; i < selections.GetCount(); i++)
	{
		if (id != CopyCollumns){
			//tłumaczenie ma pierwszeństwo w kopiowaniu
			if (subsFormat == SRT)
				whatcopy << (selections[i] + 1) << L"\r\n";

			Dialogue *dial = GetDialogue(selections[i]);
			dial->GetRaw(&whatcopy, hasTLMode && dial->TextTl != L"");
		}
		else{
			whatcopy << GetDialogue(selections[i])->GetCols(cols, hasTLMode && GetDialogue(selections[i])->TextTl != L"");
		}
	}
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(whatcopy));
		wxTheClipboard->Close();
		wxTheClipboard->Flush();
	}
}

void SubsGrid::OnInsertBeforeVideo(bool frameTime)
{
	SaveSelections(true);
	int rw = currentLine;
	file->EraseSelection(rw);
	Dialogue *dialog = CopyDialogue(rw, false);
	if (!frameTime){
		dialog->Text = L"";
		dialog->TextTl = L"";
	}
	int time = Kai->GetTab()->Video->GetFrameTime();
	dialog->Start.NewTime(time);
	dialog->End.NewTime(frameTime ? Kai->GetTab()->Video->GetFrameTime(false) : time + 4000);
	markedLine = currentLine;
	InsertRows(rw, 1, dialog, false, true);
}

void SubsGrid::OnInsertAfterVideo(bool frameTime)
{
	SaveSelections(true);
	int rw = currentLine;
	file->EraseSelection(rw);
	Dialogue *dialog = CopyDialogue(rw, false);
	if (!frameTime){
		dialog->Text = L"";
		dialog->TextTl = L"";
	}
	int time = Kai->GetTab()->Video->GetFrameTime();
	dialog->Start.NewTime(time);
	dialog->End.NewTime(frameTime ? Kai->GetTab()->Video->GetFrameTime(false) : time + 4000);
	currentLine = markedLine = rw + 1;
	InsertRows(rw + 1, 1, dialog, false, true);
}


void SubsGrid::OnAccelerator(wxCommandEvent &event)
{
	int id = event.GetId();
	VideoCtrl *vb = Kai->GetTab()->Video;
	file->GetSelections(selections);
	int sels = selections.GetCount();
	bool hasVideo = vb->GetState() != None;
	switch (id){
	case PlayPause: if (vb->IsShown()){ vb->Pause(); } break;
	case Plus5Second: vb->Seek(vb->Tell() + 5000); break;
	case Minus5Second: vb->Seek(vb->Tell() - 5000); break;
	case InsertBeforeVideo:
	case InsertBeforeWithVideoFrame:
		if (sels > 0 && hasVideo) OnInsertBeforeVideo(id == InsertBeforeWithVideoFrame); break;
	case InsertAfterVideo:
	case InsertAfterWithVideoFrame:
		if (sels > 0 && hasVideo) OnInsertAfterVideo(id == InsertAfterWithVideoFrame); break;
	case InsertBefore: if (sels > 0) OnInsertBefore(); break;
	case InsertAfter: if (sels > 0) OnInsertAfter(); break;
	case Duplicate: if (sels > 0) OnDuplicate(); break;
	case Copy:
	case CopyCollumns: if (sels > 0) CopyRows(id); break;
	case Cut: if (sels > 0) CopyRows(id); DeleteRows(); break;
	case Paste:
	case PasteCollumns: if (sels > 0) OnPaste(id); break;
	case Remove: if (sels > 0) DeleteRows(); break;
	case RemoveText: if (sels > 0) DeleteText(); break;
	case ContinousPrevious:
	case ContinousNext: if (sels > 0) OnMakeContinous(id); break;
	case Swap: if (sels == 2){ SwapRows(selections[0], selections[1], true); } break;
	case FPSFromVideo: if (hasVideo && sels == 2){ OnSetFPSFromVideo(); } break;
	case Join: if (sels > 1 && sels <= 20){ OnJoin(event); } break;
	case JoinToFirst:
	case JoinToLast: if (sels > 1 && sels <= 50){ OnJoinToFirst(id); } break;
	case HideSelected:
	{
		SubsGridFiltering filter(this, currentLine);
		filter.HideSelections();
		isFiltered = true;
		break;
	}
	case GRID_TREE_MAKE:
	{
		SubsGridFiltering filter(this, currentLine);
		filter.MakeTree();
		break;
	}
	case GRID_FILTER:
	case FilterByNothing:
		Filter(id); break;
	case PasteTranslation: if (subsFormat < SRT && ((TabPanel*)GetParent())->SubsPath != L""){ OnPasteTextTl(); } break;
	case SubsFromMKV: if (Kai->GetTab()->VideoName.EndsWith(L".mkv")){ OnMkvSubs(event); } break;
	case NewFPS: OnSetNewFPS(); break;
	case ShowPreview:
		if (Notebook::GetTabs()->Size()>1 && !preview)
			OnShowPreview();
		break;
	case GRID_HIDE_LAYER:
	case GRID_HIDE_START:
	case GRID_HIDE_END:
	case GRID_HIDE_ACTOR:
	case GRID_HIDE_STYLE:
	case GRID_HIDE_MARGINL:
	case GRID_HIDE_MARGINR:
	case GRID_HIDE_MARGINV:
	case GRID_HIDE_EFFECT:
	case GRID_HIDE_CPS:
	{
		int id5000 = (id - 5000);
		visibleColumns ^= id5000;
		SpellErrors.clear();
		Options.SetInt(GridHideCollums, visibleColumns);
		RefreshColumns();
		break;
	}
	case GRID_FILTER_INVERTED:
		Options.SetBool(GridFilterInverted, !Options.GetBool(GridFilterInverted));
		break;
	case FilterByStyles:
	case FilterBySelections:
	case FilterByDialogues:
	case FilterByDoubtful:
	case FilterByUntranslated:
	{
		int filterBy = Options.GetInt(GridFilterBy);
		int addToFilterBy = pow(2, (id - FilterByStyles));
		filterBy ^= addToFilterBy;
		Options.SetInt(GridFilterBy, filterBy);
		break;
	}//styles checking
	case GRID_FILTER_DO_NOT_RESET:
		Options.SetBool(GridAddToFilter, !Options.GetBool(GridAddToFilter));
		break;
	case GRID_FILTER_AFTER_SUBS_LOAD:
		Options.SetBool(GridFilterAfterLoad, !Options.GetBool(GridFilterAfterLoad));
		break;
	case GRID_FILTER_IGNORE_IN_ACTIONS:
		ignoreFiltered = !Options.GetBool(GridIgnoreFiltering);
		Options.SetBool(GridIgnoreFiltering, ignoreFiltered);
		break;
	default:
		break;
	}

	if (id == TranslationDialog && showOriginal){
		static TLDialog *tld = new TLDialog(this, this);
		tld->Show();
	}
	else if (id > 6000){
		Kai->OnMenuSelected(event);
	}
	else if (id > 600 && id < 1700 && Edit->ABox){
		Edit->ABox->GetEventHandler()->AddPendingEvent(event);
	}
}


void SubsGrid::OnPasteTextTl()
{
	wxFileDialog *FileDialog1 = new wxFileDialog(this, _("Wybierz plik napisów"), Kai->GetTab()->SubsPath.BeforeLast(L'\\'), L"", _("Pliki napisów (*.ass),(*.srt),(*.sub),(*.txt)|*.ass;*.srt;*.sub;*.txt"), wxFD_OPEN | wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, L"wxFileDialog");
	if (FileDialog1->ShowModal() == wxID_OK){
		OpenWrite op;
		wxString pathh = FileDialog1->GetPath();
		wxString txt;
		if (!op.FileOpen(pathh, &txt)){ return; }
		wxString ext = pathh.AfterLast(L'.');
		int iline = 0;

		if (ext == L"srt"){
			//wxString dbg;
			wxStringTokenizer tokenizer(txt, L"\n", wxTOKEN_STRTOK);
			tokenizer.GetNextToken();
			wxString text1;
			while (tokenizer.HasMoreTokens())
			{
				wxString text = tokenizer.GetNextToken().Trim();
				if (IsNumber(text)){
					if (text1 != L""){
						Dialogue diall = Dialogue(text1.Trim());
						if (iline < GetCount()){
							diall.Convert(subsFormat);
							CopyDialogue(iline)->TextTl = diall.Text;
						}
						else{
							diall.Convert(subsFormat);
							diall.Start.NewTime(0);
							diall.End.NewTime(0);
							diall.Style = GetSInfo(L"TLMode Style");
							diall.TextTl = diall.Text;
							diall.Text = L"";
							AddLine(diall.Copy());
						}
						//todo write here skipping 
						iline++; 
						text1 = L"";
					}
				}
				else{ text1 << text << L"\r\n"; }

			}
		}
		else{

			wxStringTokenizer tokenizer(txt, L"\n", wxTOKEN_STRTOK);
			while (tokenizer.HasMoreTokens())
			{
				wxString token = tokenizer.GetNextToken();
				if (!(ext == L"ass" && !token.StartsWith(L"Dialogue"))){
					Dialogue diall = Dialogue(token);
					if (iline < GetCount()){
						diall.Convert(subsFormat);
						CopyDialogue(iline)->TextTl = diall.Text;
					}
					else{
						diall.Convert(subsFormat);
						diall.Start.NewTime(0);
						diall.End.NewTime(0);
						diall.Style = GetSInfo(L"TLMode Style");
						diall.TextTl = diall.Text;
						diall.Text = L"";
						AddLine(diall.Copy());
					}
					iline++;
				}
			}
		}



		Edit->SetTlMode(true);
		SetTlMode(true);
		AddSInfo(L"TLMode Showtl", L"Yes");
		showOriginal = true;
		//Edit->SetIt(Edit->ebrow);
		SetModified(GRID_PASTE_TRANSLATION);
		Refresh(false);
	}
	FileDialog1->Destroy();
}

void SubsGrid::MoveTextTL(char mode)
{
	file->GetSelections(selections);
	if (selections.GetCount() < 1 || !showOriginal || !hasTLMode) return;
	SaveSelections(true);

	//key
	int firstSelected = selections[0];
	//use for offsets
	int numSelected = 1;
	if (selections.GetCount() > 1){
		numSelected = selections[1] - firstSelected;
	}

	if (mode < 3){// w górę ^
		//tryb 2 gdzie dodaje puste linijki a tekst pl pozostaje bez zmian
		if (mode == 2){
			Dialogue *insertDial = GetDialogue(firstSelected)->Copy();
			insertDial->Text = L"";
			InsertRows(firstSelected, numSelected, insertDial);
		}
		file->InsertSelection(firstSelected);
		for (int i = firstSelected; i < GetCount(); i++)
		{
			Dialogue *dial = GetDialogue(i);
			if (!dial->isVisible)
				continue;

			Dialogue *nextDial = GetDialogueWithOffset(i, 1);
			Dialogue *lastDial = GetDialogueWithOffset(i, numSelected);
			if (i < firstSelected + numSelected){
				//tryb1 gdzie łączy wszystkie nachodzące linijki w jedną
				if (mode == 1){
					if (nextDial){
						wxString mid = (GetDialogue(firstSelected)->TextTl != L"" && nextDial->TextTl != L"") ? L"\\N" : L"";
						CopyDialogue(firstSelected)->TextTl << mid << nextDial->TextTl;
						if (i != firstSelected && lastDial){ CopyDialogue(i)->TextTl = lastDial->TextTl; }
					}
				}
				else if (lastDial){
					CopyDialogue(i)->TextTl = lastDial->TextTl;
				}
			}
			else if (lastDial){
				CopyDialogue(i)->TextTl = lastDial->TextTl;
			}
			else if (dial->Text != L""){ numSelected--; }

		}

		if (numSelected > 0){
			DeleteRow(GetCount() - numSelected, numSelected);
		}

	}
	else{//w dół v
		int oldgc = GetCount();
		Dialogue diall;
		diall.End.NewTime(0);
		diall.Style = GetSInfo(L"TLMode Style");
		for (int i = 0; i < numSelected; i++)
		{
			AddLine(diall.Copy());
		}

		bool onlyo = true;
		//sel.insert(first+mrow);
		for (int i = GetCount() - 1; i >= firstSelected; i--)
		{

			if (i < firstSelected + numSelected){
				if (mode == 3){
					CopyDialogue(i)->TextTl = L"";
				}
				else if (mode == 4 || mode == 5){
					if (mode == 4){
						if (onlyo){ CopyDialogue(firstSelected + numSelected)->Start = GetDialogue(firstSelected)->Start; onlyo = false; }
						CopyDialogue(firstSelected + numSelected)->Text->Prepend(GetDialogue(i)->Text + L"\\N"); numSelected--;
					}
					DeleteRow(i);
				}
			}
			else{
				CopyDialogue(i)->TextTl = GetDialogue(i - numSelected)->TextTl;
			}


		}

	}
	SetModified(GRID_TRANSLATION_TEXT_MOVE, true, false, firstSelected);
	Refresh(false);

}


void SubsGrid::OnMkvSubs(wxCommandEvent &event)
{
	int idd = event.GetId();
	if (IsModified()){
		int wbutton = KaiMessageBox(_("Zapisać plik przed wczytaniem napisów z MKV?"),
			_("Potwierdzenie"), wxICON_QUESTION | wxYES_NO | wxCANCEL, this);
		if (wbutton == wxYES){ Kai->Save(false); }
		else if (wbutton == wxCANCEL){ return; }
	}
	TabPanel *tab = Kai->GetTab();
	wxString mkvpath = (idd == SubsFromMKV) ? tab->VideoPath : event.GetString();

	MatroskaWrapper mw;
	if (!mw.Open(mkvpath, false)){ return; }
	int isgood = (int)mw.GetSubtitles(this);
	mw.Close();

	if (isgood){
		if (hasTLMode){ Edit->SetTlMode(false); hasTLMode = false; showOriginal = false; Kai->Menubar->Enable(SaveTranslation, false); }
		SetSubsFormat();
		wxString ext = (subsFormat < SRT) ? L"ass" : L"srt";
		if (subsFormat < SRT){ Edit->TlMode->Enable(); }
		else{ Edit->TlMode->Enable(false); }

		tab->SubsPath = mkvpath.BeforeLast(L'.') + L"." + ext;
		tab->SubsName = tab->SubsPath.AfterLast(L'\\');
		//Kai->SetRecent();
		Kai->UpdateToolbar();
		Edit->RefreshStyle(true);

		Kai->Label();
		LoadStyleCatalog();
		if (tab->Video->GetState() != None){
			tab->Video->OpenSubs(SaveText(), true, true);
			if (!isgood){ KaiMessageBox(_("Otwieranie napisów nie powiodło się"), _("Uwaga")); }
			if (tab->Video->GetState() == Paused){ tab->Video->Render(); }
		}

		if (!tab->editor&&!tab->Video->isFullscreen){ Kai->HideEditor(); }
		tab->ShiftTimes->Contents();
		file->InsertSelection(currentLine);
		RefreshColumns();
		Edit->HideControls();
		if (StyleStore::HasStore() && subsFormat == ASS){ StyleStore::Get()->LoadAssStyles(); }
		Kai->SetSubsResolution(!Options.GetBool(DontAskForBadResolution));
	}

}



void SubsGrid::ResizeSubs(float xnsize, float ynsize, bool stretch)
{
	float val = xnsize;
	float val1 = ynsize;
	float valFscx = 1.f;
	float vectorXScale = xnsize;
	int resizeScale = 0;
	if (ynsize != xnsize){
		if (ynsize > xnsize){
			resizeScale = (stretch) ? 1 : 0;
			valFscx = (stretch) ? (ynsize / xnsize) : 1.f;
		}
		else{
			val = ynsize;
			val1 = xnsize;
			resizeScale = (stretch) ? 1 : 0;
			valFscx = (stretch) ? (xnsize / ynsize) : 1.f;
		}
		if (stretch){ vectorXScale /= valFscx; }
	}


	for (int i = 0; i < StylesSize(); i++){
		Styles *resized = file->CopyStyle(i);
		int ml = wxAtoi(resized->MarginL);
		ml *= xnsize;
		resized->MarginL = L"";
		resized->MarginL << ml;
		int mr = wxAtoi(resized->MarginR);
		mr *= xnsize;
		resized->MarginR = L"";
		resized->MarginR << mr;
		int mv = wxAtoi(resized->MarginV);
		mv *= ynsize;
		resized->MarginV = L"";
		resized->MarginV << mv;
		if (resizeScale == 1){
			double fscx = 100;
			resized->ScaleX.ToCDouble(&fscx);
			fscx *= valFscx;
			resized->ScaleX = getfloat(fscx);
		}
		double fs = 0;
		resized->Fontsize.ToCDouble(&fs);
		//TODO: sprawdzić czy nie ma jakiegoś przypadku, gdzie ta wartość będzie musiała przyjąć val1
		fs *= val/*1*/;
		resized->Fontsize = getfloat(fs);
		double ol = 0;
		resized->Outline.ToCDouble(&ol);
		ol *= val;
		resized->Outline = getfloat(ol);
		double sh = 0;
		resized->Shadow.ToCDouble(&sh);
		sh *= val;
		resized->Shadow = getfloat(sh);
		double fsp = 0;
		resized->Spacing.ToCDouble(&fsp);
		fsp *= val;
		resized->Spacing = getfloat(fsp);
	}

	wxString tags[] = { L"pos", L"move", L"bord", L"shad", L"org", L"fsp", L"fscx", L"fs", L"clip", L"iclip", L"p", L"xbord", L"ybord", L"xshad", L"yshad" };
	for (int i = 0; i < file->GetCount(); i++){
		//zaczniemy od najłatwiejszego, marginesy

		Dialogue *diall = file->GetDialogue(i);
		if (diall->IsComment){ continue; }
		diall = diall->Copy(false, false);
		bool marginChanged = false;
		bool textChanged = false;
		if (diall->MarginL){ diall->MarginL *= xnsize; marginChanged = true; }
		if (diall->MarginR){ diall->MarginR *= xnsize; marginChanged = true; }
		if (diall->MarginV){ diall->MarginV *= ynsize; marginChanged = true; }

		wxString &txt = diall->GetText();
		/*long long replaceMismatch = 0;*/
		size_t pos = 0;

		diall->ParseTags(tags, 15, false);
		ParseData *pdata = diall->parseData;
		if (!pdata){ continue; }
		size_t tagsSize = pdata->tags.size();
		//if(tagsSize < 1){continue;}
		//pętla tagów
		int j = tagsSize - 1;
		while (j >= 0){
			TagData *tag = pdata->tags[j--];
			size_t tagValueLen = tag->value.Len();
			pos = tag->startTextPos;
			double tagValue = 0.0;
			int ii = 0;
			wxString resizedTag;
			if (tag->tagName != L"fsp" && tag->tagName.EndsWith(L'p') && tag->value.find(L'm') != -1){
				int mPos = tag->value.find(L'm');
				resizedTag = tag->value.Left(mPos) + L"m ";
				wxStringTokenizer tknzr(tag->value.AfterFirst(L'm'), L" ", wxTOKEN_STRTOK);
				
				float xscale = (tag->tagName == L"p") ? vectorXScale : xnsize;

				while (tknzr.HasMoreTokens()){
					wxString tkn = tknzr.NextToken();
					if (tkn != L"m" && tkn != L"l" && tkn != L"b" && tkn != L"s" && tkn != L"c"){
						wxString lastC;
						if (tkn.EndsWith(L"c")){
							tkn.RemoveLast(1);
							lastC = L"c";
						}
						if (tkn.ToCDouble(&tagValue)){
							tagValue *= (ii % 2 == 0) ? xscale : ynsize;
							resizedTag << getfloat(tagValue) << lastC << L" ";
						}
						else{
							wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw tagu '%s'"), i + 1, tkn, tag->tagName);
							resizedTag << tkn << lastC << L" ";
						}

						ii++;
					}
					else{
						resizedTag << tkn << L" ";
					}
				}
				resizedTag.Trim();
			}
			else if (tag->multiValue){
				wxStringTokenizer tknzr(tag->value, L",", wxTOKEN_STRTOK);
				while (tknzr.HasMoreTokens()){
					wxString tkn = tknzr.NextToken();
					tkn.Trim();
					tkn.Trim(false);

					if (ii < 4 && tkn.ToCDouble(&tagValue))
					{
						tagValue *= (ii % 2 == 0) ? xnsize : ynsize;
						resizedTag << getfloat(tagValue) << L",";
						ii++;
					}
					else
					{
						if (ii < 4){
							wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw tagu '%s'"), i + 1, tkn, tag->tagName);
						}
						resizedTag << tkn << L",";
						ii++;
					}
				}
				resizedTag = resizedTag.BeforeLast(L',');

			}
			else if (tag->tagName != L"p"){
				if (tag->value.ToCDouble(&tagValue)){
					tagValue *= (tag->tagName == L"fscx") ? valFscx :
						(tag->tagName == L"fs") ? val1 :
						(tag->tagName.StartsWith(L'x')) ? xnsize :
						(tag->tagName.StartsWith(L'y')) ? ynsize : val;
					resizedTag = getfloat(tagValue);
				}
				else{
					wxLogMessage(_("W linii %i nie można przeskalować wartości '%s'\nw tagu '%s'"), i + 1, tag->value, tag->tagName);
					resizedTag = tag->value;
				}
			}
			else{
				continue;
			}
			/*replaceMismatch += (tagValueLen - resizedTag.Len());*/
			txt.replace(pos, tagValueLen, resizedTag);
			textChanged = true;
		}

		if (marginChanged || textChanged){
			if (textChanged){
				if (SpellErrors.size() >= (size_t)i) SpellErrors[i].clear();
			}
			file->SetDialogue(i, diall);
			diall->ClearParse();
		}
		else{
			delete diall;
		}

	}

	//Refresh(false);
}

void SubsGrid::OnMakeContinous(int idd)
{
	if (selections.size() < 0){ wxBell(); return; }
	if (idd == ContinousPrevious){

		/*int diff=GetDial(fs)->End.mstime - GetDial(fs-1)->Start.mstime;*/
		for (size_t i = 0; i < selections.size(); i++)
		{
			if (selections[i] < 1){ continue; }
			CopyDialogue(selections[i])->Start = GetDialogue(selections[i] - 1)->End;
		}
	}
	else
	{
		int dialsize = GetCount() - 1;
		for (size_t i = 0; i < selections.size(); i++)
		{
			if (selections[i] >= dialsize){ continue; }
			CopyDialogue(selections[i])->End = GetDialogue(selections[i] + 1)->Start;
		}
	}
	SetModified(GRID_MAKE_LINES_CONTINUES);
	Refresh(false);
}

void SubsGrid::OnShowPreview()
{
	if (CG1 == this || CG2 == this){
		ShowSecondComparedLine(currentLine, true);
	}
	else{
		ShowPreviewWindow(NULL, this, currentLine, currentLine - scrollPosition);
	}
}

void SubsGrid::ConnectAcc(int id)
{
	Connect(id, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&SubsGrid::OnAccelerator);
}

void SubsGrid::OnSetFPSFromVideo()
{
	if (selections.size() != 2){ return; }
	Dialogue *first = GetDialogue(selections[0]);
	Dialogue *second = GetDialogue(selections[1]);
	int firstTime = first->Start.mstime;
	int secondTime = second->Start.mstime;
	int videoTime = Notebook::GetTab()->Video->Tell();
	float diffVideo = (videoTime - secondTime);
	float diffLines = (secondTime - firstTime);

	for (int i = 0; i < GetCount(); i++){
		Dialogue *dialc = CopyDialogue(i);
		dialc->Start.Change(diffVideo *((dialc->Start.mstime - firstTime) / diffLines));
		dialc->End.Change(diffVideo *((dialc->End.mstime - firstTime) / diffLines));
	}
	SetModified(GRID_SET_VIDEO_FPS);
	if (subsFormat > TMP){ RefreshColumns(START | END); }
	else{ Refresh(false); }
}

void SubsGrid::OnSetNewFPS()
{
	FPSDialog nfps(this);
	if (nfps.ShowModal() == 1){
		double sub = nfps.ofps / nfps.nfps;

		for (int i = 0; i < GetCount(); i++){
			Dialogue *dialc = CopyDialogue(i);
			dialc->Start.NewTime(dialc->Start.mstime*sub);
			dialc->End.NewTime(dialc->End.mstime*sub);
		}
		SetModified(GRID_SET_CUSTOM_FPS);
		if (subsFormat > TMP){ RefreshColumns(START | END); }
		else{ Refresh(false); }
	}
}

class SwapPropertiesDialog :public KaiDialog
{
public:
	SwapPropertiesDialog(wxWindow *parent)
		:KaiDialog(parent, -1, _("Potwierdzenie"))
	{
		DialogSizer *main = new DialogSizer(wxVERTICAL);
		const int numFields = 6;
		wxString fieldNames[numFields] = { _("Tytuł"), _("Autor"), _("Tłumaczenie"), _("Korekta"), _("Timing"), _("Edycja") };
		CONFIG fieldOnValues[numFields] = { ASSPropertiesTitleOn, ASSPropertiesScriptOn, ASSPropertiesTranslationOn,
			ASSPropertiesEditingOn, ASSPropertiesTimingOn, ASSPropertiesUpdateOn };
		for (int i = 0; i < numFields; i++){
			fields[i] = new KaiCheckBox(this, -1, fieldNames[i]);
			fields[i]->SetValue(Options.GetBool(fieldOnValues[i]));
			main->Add(fields[i], 0, wxEXPAND | wxALL, 3);
		}
		wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
		MappedButton *Ok = new MappedButton(this, wxID_OK, L"OK");
		MappedButton *Cancel = new MappedButton(this, wxID_CANCEL, _("Anuluj"));
		MappedButton *TurnOf = new MappedButton(this, 19921, _("Wyłącz potwierdzenie"));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt){
			Options.SetBool(ASSPropertiesAskForChange, false);
			Options.SaveOptions(true, false);
			EndModal(19921);
		}, 19921);
		buttons->Add(Ok, 1, wxALL, 4);
		buttons->Add(Cancel, 1, wxALL, 4);
		buttons->Add(TurnOf, 0, wxALL, 4);
		main->Add(buttons);
		SetSizerAndFit(main);
		CenterOnParent();
	}
	virtual ~SwapPropertiesDialog(){};
	void SaveValues(){
		const int numFields = 6;
		CONFIG fieldOnValues[numFields] = { ASSPropertiesTitleOn, ASSPropertiesScriptOn, ASSPropertiesTranslationOn,
			ASSPropertiesEditingOn, ASSPropertiesTimingOn, ASSPropertiesUpdateOn };
		for (int i = 0; i < numFields; i++){
			Options.SetBool(fieldOnValues[i], fields[i]->GetValue());
		}
	}
private:
	KaiCheckBox *fields[6];
};

bool SubsGrid::SwapAssProperties()
{
	const int numFields = 6;
	CONFIG fieldOnValues[numFields] = { ASSPropertiesTitleOn, ASSPropertiesScriptOn, ASSPropertiesTranslationOn,
		ASSPropertiesEditingOn, ASSPropertiesTimingOn, ASSPropertiesUpdateOn };
	CONFIG fieldValues[numFields] = { ASSPropertiesTitle, ASSPropertiesScript, ASSPropertiesTranslation,
		ASSPropertiesEditing, ASSPropertiesTiming, ASSPropertiesUpdate };
	wxString fieldNames[numFields] = { L"Title", L"Original Script", L"Original Translation",
		L"Original Editing", L"Original Timing", L"Script Updated By" };
	if (Options.GetBool(ASSPropertiesAskForChange)){
		bool hasSomethingToChange = false;
		for (int i = 0; i < numFields; i++){
			if (Options.GetBool(fieldOnValues[i])){
				if (GetSInfo(fieldNames[i]) != Options.GetString(fieldValues[i])){
					hasSomethingToChange = true;
					break;
				}
			}
		}
		if (!hasSomethingToChange){ return false; }
		SwapPropertiesDialog SPD(Kai);
		int id = SPD.ShowModal();
		if (id == wxID_OK){
			SPD.SaveValues();
		}
		else if (id == 19921){
			return false;
		}
		else{
			return true;
		}
	}

	for (int i = 0; i < numFields; i++){
		if (Options.GetBool(fieldOnValues[i])){
			AddSInfo(fieldNames[i], Options.GetString(fieldValues[i]));
		}
	}
	return false;
}

void SubsGrid::Filter(int id)
{
	SubsGridFiltering filter((SubsGrid*)this, currentLine);
	const wxString & styles = Options.GetString(GridFilterStyles);
	if (!styles.empty()){
		int filterBy = Options.GetInt(GridFilterBy);
		Options.SetInt(GridFilterBy, filterBy | FILTER_BY_STYLES);
	}
	if (id != FilterByNothing){ isFiltered = true; }
	else{ Options.SetInt(GridFilterBy, 0); }
	filter.Filter();
}

void SubsGrid::TreeAddLines(int treeLine)
{
	file->GetSelections(selections);
	if (selections.GetCount() < 1) return;

	int keystart = treeLine;
	//need collect selected dialogues before and after tree
	//in tree all dialogues are deselected
	std::vector<Dialogue*> beforeTreeLines;
	std::vector<Dialogue*> afterTreeLines;
	int beforeLinesDiff = 0;
	Dialogue *dialwithstate = file->GetDialogue(keystart+1);
	bool closed = (dialwithstate->treeState == TREE_CLOSED);

	for (int i = 0; i < selections.GetCount(); i++){
		int sel = selections[i];
		Dialogue *dial = file->GetDialogue(sel);
		//we must deselect lines from this tree;
		if (sel >= keystart && !(!dial->treeState || (dial->treeState == TREE_DESCRIPTION && sel != keystart)))
			file->EraseSelection(sel);
		else if (sel > keystart){//adding after lines
			dial = CopyDialogue(sel);
			dial->treeState = closed ? TREE_CLOSED : TREE_OPENED;
			if (closed)
				dial->isVisible = NOT_VISIBLE;
			afterTreeLines.push_back(dial);
		}
		else if (sel < keystart){//adding before lines
			dial = CopyDialogue(sel);
			dial->treeState = closed ? TREE_CLOSED : TREE_OPENED;
			if (closed)
				dial->isVisible = NOT_VISIBLE;
			beforeTreeLines.push_back(dial);
			beforeLinesDiff++;
		}

	}
	//Delete selected rows
	file->DeleteSelectedDialogues();
	//decrease keystart by num of selected lines before of tree
	keystart -= beforeLinesDiff;
	//insert before lines on tree start 
	if (beforeTreeLines.size()){
		InsertRows(keystart + 1, beforeTreeLines, false);
	}
	//insert after lines need to find end of tree
	if (afterTreeLines.size()){
		for (int i = keystart; i < file->GetCount(); i++){
			Dialogue *dial = file->GetDialogue(i);
			if ((!dial->treeState || (dial->treeState == TREE_DESCRIPTION && i != keystart))){
				InsertRows(i, afterTreeLines, false);
				break;
			}
		}
	}
	//if something changed set modified and stuff
	if (beforeTreeLines.size() || afterTreeLines.size()){
		SaveSelections();
		file->ClearSelections();
		SpellErrors.clear();
		//file->ReloadVisibleDialogues();
		SetModified(TREE_ADD_LINES, true, false, keystart + 1);
	}
	else{
		file->InsertSelection(currentLine);
	}
	Refresh(false);
}

void SubsGrid::TreeCopy(int treeLine)
{
	wxString whattocopy;
	int keystart = treeLine;
	for (int i = keystart; i < file->GetCount(); i++){
		Dialogue *dial = file->GetDialogue(i);
		if (!dial->treeState || (dial->treeState == TREE_DESCRIPTION && i != keystart))
			break;
		dial->GetRaw(&whattocopy, hasTLMode && dial->TextTl != L"");
	}
	if(wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(whattocopy));
		wxTheClipboard->Close();
		wxTheClipboard->Flush();
	}
}

void SubsGrid::TreeChangeName(int treeLine)
{
	Dialogue *dial = file->GetDialogue(treeLine);
	TreeDialog td(this, dial->Text);
	if (td.ShowModal() == wxID_OK){
		file->CopyDialogue(treeLine)->Text = td.GetDescription();
		SetModified(TREE_SET_DESCRIPTION);
		Refresh(false);
	}
}

void SubsGrid::TreeRemove(int treeLine)
{
	int keystart = treeLine;
	int keyend = keystart;
	//tree changing need to be save to history instead of visibility
	for (int i = keystart; i < file->GetCount(); i++){
		Dialogue *dial = file->GetDialogue(i);
		if (!dial->treeState || (dial->treeState == TREE_DESCRIPTION && i != keystart)){
			keyend = i - 1;
			break;
		}
		dial = file->CopyDialogue(i);
		dial->treeState = 0;
		if (!dial->isVisible)
			dial->isVisible = VISIBLE;
	}
	DeleteRow(treeLine, 1);
	SetModified(TREE_REMOVE);
	Refresh(false);
}

void SubsGrid::RefreshSubsOnVideo(int newActiveLineKey, bool scroll)
{
	file->ClearSelections();
	SpellErrors.clear();
	int corrected = -1;
	int newActiveLine = file->FindVisibleKey(newActiveLineKey, &corrected);
	if (corrected >= 0){
		newActiveLineKey = corrected;
	}
	//newActiveLine = MID(0, newActiveLine, GetCount()-1);
	if (currentLine != newActiveLine){
		currentLine = newActiveLine;
		if (scroll){ ScrollTo(newActiveLine, true); }
	}
	file->InsertSelection(newActiveLine);
	Edit->SetLine(newActiveLine);
	if (Comparison && (Options.GetInt(SubsComparisonType) & COMPARE_BY_VISIBLE)){ SubsComparison(); }
	VideoCtrl *vb = ((TabPanel*)GetParent())->Video;
	if (vb->GetState() != None){
		vb->OpenSubs(GetVisible());
		vb->Render();
	}
}

void SubsGrid::LoadStyleCatalog()
{
	if (subsFormat != ASS){ return; }
	const wxString &catalog = GetSInfo(L"Last Style Storage");

	if (catalog.empty()){ return; }
	for (size_t i = 0; i < Options.dirs.size(); i++){
		if (catalog == Options.dirs[i]){
			Options.LoadStyles(catalog);
			if (StyleStore::HasStore()){
				StyleStore *ss = StyleStore::Get();
				ss->Store->SetSelection(0, true);
				int chc = ss->catalogList->FindString(Options.actualStyleDir);
				ss->catalogList->SetSelection(chc);
			}
		}
	}

}

BEGIN_EVENT_TABLE(SubsGrid, SubsGridBase)
EVT_MENU(Cut, SubsGrid::OnAccelerator)
EVT_MENU(Copy, SubsGrid::OnAccelerator)
EVT_MENU(Paste, SubsGrid::OnAccelerator)
END_EVENT_TABLE()