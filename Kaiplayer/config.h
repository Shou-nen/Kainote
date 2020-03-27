//  Copyright (c) 2016, Marcin Drob

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


#pragma once

//#include <wx/textfile.h>
#include <wx/tokenzr.h>
//#include <wx/colour.h>
//#include <wx/string.h>
//#include <wx/image.h>
#include <map>
#include <vector>
#include <algorithm>
#include "Styles.h"
#include <wx/utils.h> 
#include "EnumFactory.h"
#include <wx/wx.h>
#include "LogHandler.h"
#undef wxBITMAP_PNG


#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef MID
#define MID(a,b,c) MAX((a),MIN((b),(c)))
#endif

//Pami�taj, zmiana ostatniej audio opcji wymaga te� zmiany przy szukaniu w zapisie
//Dont change enumeration config and colors from 1 to last, zero for non exist trash
#define CFG(CG) \
	CG(AUDIO_AUTO_COMMIT,=1)\
	CG(AUDIO_AUTO_FOCUS,)\
	CG(AUDIO_AUTO_SCROLL,)\
	CG(AUDIO_BOX_HEIGHT,)\
	CG(AUDIO_CACHE_FILES_LIMIT,)\
	CG(AUDIO_DELAY,)\
	CG(AUDIO_DRAW_KEYFRAMES,)\
	CG(AUDIO_DRAW_SECONDARY_LINES,)\
	CG(AUDIO_DRAW_SELECTION_BACKGROUND,)\
	CG(AUDIO_DRAW_TIME_CURSOR,)\
	CG(AUDIO_DRAW_VIDEO_POSITION,)\
	CG(AUDIO_GRAB_TIMES_ON_SELECT,)\
	CG(AUDIO_HORIZONTAL_ZOOM,)\
	CG(AUDIO_INACTIVE_LINES_DISPLAY_MODE,)\
	CG(AUDIO_KARAOKE,)\
	CG(AUDIO_KARAOKE_MOVE_ON_CLICK,)\
	CG(AUDIO_KARAOKE_SPLIT_MODE,)\
	CG(AUDIO_LEAD_IN_VALUE,)\
	CG(AUDIO_LEAD_OUT_VALUE,)\
	CG(AUDIO_LINE_BOUNDARIES_THICKNESS,)\
	CG(AUDIO_LINK,)\
	CG(AUDIO_LOCK_SCROLL_ON_CURSOR,)\
	CG(AUDIO_MARK_PLAY_TIME,)\
	CG(AUDIO_MERGE_EVERY_N_WITH_SYLLABLE,)\
	CG(AUDIO_NEXT_LINE_ON_COMMIT,)\
	CG(AUDIO_RAM_CACHE,)\
	CG(AUDIO_SNAP_TO_KEYFRAMES,)\
	CG(AUDIO_SNAP_TO_OTHER_LINES,)\
	CG(AUDIO_SPECTRUM_ON,)\
	CG(AUDIO_SPECTRUM_NON_LINEAR_ON,)\
	CG(AUDIO_START_DRAG_SENSITIVITY,)\
	CG(AUDIO_VERTICAL_ZOOM,)\
	CG(AUDIO_VOLUME,)\
	CG(AUDIO_WHEEL_DEFAULT_TO_ZOOM,)\
	CG(ACCEPTED_AUDIO_STREAM,)\
	CG(ASS_PROPERTIES_TITLE,)\
	CG(ASS_PROPERTIES_SCRIPT,)\
	CG(ASS_PROPERTIES_TRANSLATION,)\
	CG(ASS_PROPERTIES_EDITING,)\
	CG(ASS_PROPERTIES_TIMING,)\
	CG(ASS_PROPERTIES_UPDATE,)\
	CG(ASS_PROPERTIES_TITLE_ON,)\
	CG(ASS_PROPERTIES_SCRIPT_ON,)\
	CG(ASS_PROPERTIES_TRANSLATION_ON,)\
	CG(ASS_PROPERTIES_EDITING_ON,)\
	CG(ASS_PROPERTIES_TIMING_ON,)\
	CG(ASS_PROPERTIES_UPDATE_ON,)\
	CG(ASS_PROPERTIES_ASK_FOR_CHANGE,)\
	CG(AUDIO_RECENT_FILES,)\
	CG(AUTOMATION_LOADING_METHOD,)\
	CG(AUTOMATION_OLD_SCRIPTS_COMPATIBILITY,)\
	CG(AUTOMATION_RECENT_FILES,)\
	CG(AUTOMATION_SCRIPT_EDITOR,)\
	CG(AUTOMATION_TRACE_LEVEL,)\
	CG(AUTO_MOVE_TAGS_FROM_ORIGINAL,)\
	CG(AUTOSAVE_MAX_FILES,)\
	CG(AUTO_SELECT_LINES_FROM_LAST_TAB,)\
	CG(COLORPICKER_RECENT_COLORS,)\
	CG(COLORPICKER_SWITCH_CLICKS,)\
	CG(CONVERT_ASS_TAGS_TO_INSERT_IN_LINE,)\
	CG(CONVERT_FPS,)\
	CG(CONVERT_FPS_FROM_VIDEO,)\
	CG(CONVERT_NEW_END_TIMES,)\
	CG(CONVERT_RESOLUTION_WIDTH,)\
	CG(CONVERT_RESOLUTION_HEIGHT,)\
	CG(CONVERT_SHOW_SETTINGS,)\
	CG(CONVERT_STYLE,)\
	CG(CONVERT_STYLE_CATALOG,)\
	CG(CONVERT_TIME_PER_CHARACTER,)\
	CG(COPY_COLLUMS_SELECTIONS,)\
	CG(DICTIONARY_LANGUAGE,)\
	CG(DISABLE_LIVE_VIDEO_EDITING,)\
	CG(DONT_ASK_FOR_BAD_RESOLUTION,)\
	CG(EDITBOX_SUGGESTIONS_ON_DOUBLE_CLICK,)\
	CG(EDITBOX_TIMES_TO_FRAMES_SWITCH,)\
	CG(EDITOR_ON,)\
	CG(FIND_IN_SUBS_FILTERS_RECENT,)\
	CG(FIND_IN_SUBS_PATHS_RECENT,)\
	CG(FIND_REPLACE_STYLES,)\
	CG(FIND_RECENT_FINDS,)\
	CG(FIND_REPLACE_OPTIONS,)\
	CG(FONT_COLLECTOR_ACTION,)\
	CG(FontCollectorDirectory,)\
	CG(FontCollectorFromMKV,)\
	CG(FontCollectorUseSubsDirectory,)\
	CG(FFMS2VideoSeeking,)\
	CG(GridChangeActiveOnSelection,)\
	CG(GridFontName,)\
	CG(GridFontSize,)\
	CG(GridAddToFilter,)\
	CG(GridFilterAfterLoad,)\
	CG(GridFilterBy,)\
	CG(GridFilterInverted,)\
	CG(GridFilterStyles,)\
	CG(GridHideCollums,)\
	CG(GridHideTags,)\
	CG(GridIgnoreFiltering,)\
	CG(GridLoadSortedSubs,)\
	CG(GridSaveAfterCharacterCount,)\
	CG(GridTagsSwapChar,)\
	CG(InsertEndOffset,)\
	CG(InsertStartOffset,)\
	CG(KEYFRAMES_RECENT,)\
	CG(LAST_SESSION_CONFIG,)\
	CG(MoveTimesByTime,)\
	CG(MoveTimesLoadSetTabOptions,)\
	CG(MoveTimesCorrectEndTimes,)\
	CG(MoveTimesForward,)\
	CG(MoveTimesFrames,)\
	CG(MoveTimesOn,)\
	CG(MoveTimesOptions,)\
	CG(MoveTimesWhichLines,)\
	CG(MoveTimesWhichTimes,)\
	CG(MoveTimesStyles,)\
	CG(MoveTimesTime,)\
	CG(MoveVideoToActiveLine,)\
	CG(NoNewLineAfterTimesEdition,)\
	CG(OpenSubsInNewCard,)\
	CG(OpenVideoAtActiveLine,)\
	CG(PasteCollumnsSelection,)\
	CG(PlayAfterSelection,)\
	CG(PostprocessorEnabling,)\
	CG(PostprocessorKeyframeBeforeStart,)\
	CG(PostprocessorKeyframeAfterStart,)\
	CG(PostprocessorKeyframeBeforeEnd,)\
	CG(PostprocessorKeyframeAfterEnd,)\
	CG(PostprocessorLeadIn,)\
	CG(PostprocessorLeadOut,)\
	CG(PostprocessorThresholdStart,)\
	CG(PostprocessorThresholdEnd,)\
	CG(PreviewText,)\
	CG(PROGRAM_FONT,)\
	CG(PROGRAM_FONT_SIZE,)\
	CG(ProgramLanguage,)\
	CG(ProgramTheme,)\
	CG(ReplaceRecent,)\
	CG(SelectionsRecent,)\
	CG(SelectionsOptions,)\
	CG(SelectVisibleLineAfterFullscreen,)\
	CG(SHIFT_TIMES_PROFILES,)\
	CG(SpellcheckerOn,)\
	CG(StyleEditFilterText,)\
	CG(StyleFilterTextOn,)\
	CG(StyleManagerPosition,)\
	CG(StyleManagerDetachEditor,)\
	CG(SubsAutonaming,)\
	CG(SubsComparisonType,)\
	CG(SubsComparisonStyles,)\
	CG(SubsRecent,)\
	CG(TAB_TEXT_MAX_CHARS,)\
	CG(TEXT_EDITOR_HIDE_STATUS_BAR,)\
	CG(TEXT_EDITOR_CHANGE_QUOTES,)\
	CG(TEXT_EDITOR_TAG_LIST_OPTIONS,)\
	CG(TextFieldAllowNumpadHotkeys,)\
	CG(TlModeShowOriginal,)\
	CG(TL_MODE_HIDE_ORIGINAL_ON_VIDEO,)\
	CG(ToolbarIDs,)\
	CG(ToolbarAlignment,)\
	CG(UpdaterCheckIntensity,)\
	CG(UpdaterCheckOptions,)\
	CG(UpdaterCheckForStable,)\
	CG(UpdaterLastCheck,)\
	CG(VideoFullskreenOnStart,)\
	CG(VideoIndex,)\
	CG(VideoPauseOnClick,)\
	CG(VideoProgressBar,)\
	CG(VideoRecent,)\
	CG(VideoVolume,)\
	CG(VideoWindowSize,)\
	CG(VisualWarningsOff,)\
	CG(VSFILTER_INSTANCE,)\
	CG(WindowMaximized,)\
	CG(WindowPosition,)\
	CG(WindowSize,)\
	CG(EditboxTagButtons,)\
	CG(EditboxTagButton1,)\
	CG(EditboxTagButton2,)\
	CG(EditboxTagButton3,)\
	CG(EditboxTagButton4,)\
	CG(EditboxTagButton5,)\
	CG(EditboxTagButton6,)\
	CG(EditboxTagButton7,)\
	CG(EditboxTagButton8,)\
	CG(EditboxTagButton9,)\
	CG(EditboxTagButton10,)\
	CG(EditboxTagButton11,)\
	CG(EditboxTagButton12,)\
	CG(EditboxTagButton13,)\
	CG(EditboxTagButton14,)\
	CG(EditboxTagButton15,)\
	CG(EditboxTagButton16,)\
	CG(EditboxTagButton17,)\
	CG(EditboxTagButton18,)\
	CG(EditboxTagButton19,)\
	CG(EditboxTagButton20,)\
	//if you write here a new enum then change configSize below after colors

DECLARE_ENUM(CONFIG, CFG)

#define CLR(CR) \
	CR(WindowBackground,=1)\
	CR(WindowBackgroundInactive,)\
	CR(WindowText,)\
	CR(WindowTextInactive,)\
	CR(WindowBorder,)\
	CR(WindowBorderInactive,)\
	CR(WindowBorderBackground,)\
	CR(WindowBorderBackgroundInactive,)\
	CR(WindowHeaderText,)\
	CR(WindowHeaderTextInactive,)\
	CR(WindowHoverHeaderElement,)\
	CR(WindowPushedHeaderElement,)\
	CR(WindowHoverCloseButton,)\
	CR(WindowPushedCloseButton,)\
	CR(WindowWarningElements,)\
	CR(GridText,)\
	CR(GridBackground,)\
	CR(GridDialogue,)\
	CR(GridComment,)\
	CR(GridSelection,)\
	CR(GridVisibleOnVideo,)\
	CR(GridCollisions,)\
	CR(GridLines,)\
	CR(GridActiveLine,)\
	CR(GridHeader,)\
	CR(GridHeaderText,)\
	CR(GridLabelNormal,)\
	CR(GridLabelModified,)\
	CR(GridLabelSaved,)\
	CR(GridLabelDoubtful,)\
	CR(GridSpellchecker,)\
	CR(GridComparisonOutline,)\
	CR(GridComparisonBackgroundNotMatch,)\
	CR(GridComparisonBackgroundMatch,)\
	CR(GridComparisonCommentBackgroundNotMatch,)\
	CR(GridComparisonCommentBackgroundMatch,)\
	CR(EditorText,)\
	CR(EditorTagNames,)\
	CR(EditorTagValues,)\
	CR(EditorCurlyBraces,)\
	CR(EditorTagOperators,)\
	CR(EditorTemplateVariables,)\
	CR(EditorTemplateCodeMarks,)\
	CR(EditorTemplateFunctions,)\
	CR(EditorTemplateKeywords,)\
	CR(EditorTemplateStrings,)\
	CR(EditorPhraseSearch,)\
	CR(EditorBracesBackground,)\
	CR(EditorBackground,)\
	CR(EditorSelection,)\
	CR(EditorSelectionNoFocus,)\
	CR(EditorBorder,)\
	CR(EditorBorderOnFocus,)\
	CR(EditorSpellchecker,)\
	CR(AudioBackground,)\
	CR(AudioLineBoundaryStart,)\
	CR(AudioLineBoundaryEnd,)\
	CR(AudioLineBoundaryMark,)\
	CR(AudioLineBoundaryInactiveLine,)\
	CR(AudioPlayCursor,)\
	CR(AudioSecondsBoundaries,)\
	CR(AudioKeyframes,)\
	CR(AudioSyllableBoundaries,)\
	CR(AudioSyllableText,)\
	CR(AudioSelectionBackground,)\
	CR(AudioSelectionBackgroundModified,)\
	CR(AudioInactiveLinesBackground,)\
	CR(AudioWaveform,)\
	CR(AudioWaveformInactive,)\
	CR(AudioWaveformModified,)\
	CR(AudioWaveformSelected,)\
	CR(AudioSpectrumBackground,)\
	CR(AudioSpectrumEcho,)\
	CR(AudioSpectrumInner,)\
	CR(TextFieldBackground,)\
	CR(TextFieldBorder,)\
	CR(TextFieldBorderOnFocus,)\
	CR(TextFieldSelection,)\
	CR(TextFieldSelectionNoFocus,)\
	CR(ButtonBackground,)\
	CR(ButtonBackgroundHover,)\
	CR(ButtonBackgroundPushed,)\
	CR(ButtonBackgroundOnFocus,)\
	CR(ButtonBorder,)\
	CR(ButtonBorderHover,)\
	CR(ButtonBorderPushed,)\
	CR(ButtonBorderOnFocus,)\
	CR(ButtonBorderInactive,)\
	CR(TogglebuttonBackgroundToggled,)\
	CR(TogglebuttonBorderToggled,)\
	CR(ScrollbarBackground,)\
	CR(ScrollbarScroll,)\
	CR(ScrollbarScrollHover,)\
	CR(ScrollbarScrollPushed,)\
	CR(StaticboxBorder,)\
	CR(StaticListBorder,)\
	CR(StaticListBackground,)\
	CR(StaticListSelection,)\
	CR(StaticListBackgroundHeadline,)\
	CR(StaticListTextHeadline,)\
	CR(StatusBarBorder,)\
	CR(MenuBarBackground1,)\
	CR(MenuBarBackground2,)\
	CR(MenuBarBorderSelection,)\
	CR(MENU_BAR_BACKGROUND_HOVER,)\
	CR(MenuBarBackgroundSelection,)\
	CR(MenuBackground,)\
	CR(MenuBorderSelection,)\
	CR(MenuBackgroundSelection,)\
	CR(TabsBarBackground1,)\
	CR(TabsBarBackground2,)\
	CR(TabsBorderActive,)\
	CR(TabsBorderInactive,)\
	CR(TabsBackgroundActive,)\
	CR(TabsBackgroundInactive,)\
	CR(TabsBackgroundInactiveHover,)\
	CR(TabsBackgroundSecondWindow,)\
	CR(TabsTextActive,)\
	CR(TabsTextInactive,)\
	CR(TabsCloseHover,)\
	CR(TabsBarArrow,)\
	CR(TabsBarArrowBackground,)\
	CR(TabsBarArrowBackgroundHover,)\
	CR(SliderPathBackground,)\
	CR(SliderPathBorder,)\
	CR(SliderBorder,)\
	CR(SliderBorderHover,)\
	CR(SliderBorderPushed,)\
	CR(SliderBackground,)\
	CR(SliderBackgroundHover,)\
	CR(SliderBackgroundPushed,)\
	CR(WINDOW_RESIZER_DOTS,)\
	CR(FIND_RESULT_FILENAME_FOREGROUND,)\
	CR(FIND_RESULT_FILENAME_BACKGROUND,)\
	CR(FIND_RESULT_FOUND_PHRASE_FOREGROUND,)\
	CR(FIND_RESULT_FOUND_PHRASE_BACKGROUND,)\
	CR(StylePreviewColor1,)\
	CR(StylePreviewColor2,)\
	//if you write here a new enum then change colorsSize below
DECLARE_ENUM(COLOR, CLR)

typedef void csri_rend;

class config
{
private:
	static const unsigned int configSize = EditboxTagButton20 + 1;
	wxString stringConfig[configSize];
	static const unsigned int colorsSize = StylePreviewColor2 + 1;
	wxColour colors[colorsSize];
	bool isClosing = false;
	std::map<int, wxFont*> programFonts;
	std::map<wxString, wxString> Languages;
	csri_rend *vsfilter = NULL;
	wxColour defaultColour;
	void InitLanguagesTable();
	bool hasCrashed = false;
public:
	std::vector<Styles*> assstore;
	wxString progname;
	//actual style catalog --- path to program exe
	wxString actualStyleDir, pathfull, configPath;
	wxArrayString dirs;
	bool AudioOpts;

	void SetClosing(){ isClosing = true; }
	bool GetClosing(){ return isClosing; }

	const wxString &GetString(CONFIG opt);
	bool GetBool(CONFIG opt);
	const wxColour &GetColour(COLOR opt);
	AssColor GetColor(COLOR opt);
	int GetInt(CONFIG opt);
	float GetFloat(CONFIG opt);
	void GetTable(CONFIG opt, wxArrayString &tbl, wxString split = L"|", int mode = 4);
	void GetIntTable(CONFIG opt, wxArrayInt &tbl, wxString split = L"|", int mode = 4);
	void GetCoords(CONFIG opt, int* coordx, int* coordy);

	void SetString(CONFIG opt, const wxString &sopt);
	void SetBool(CONFIG opt, bool bopt);
	void SetColour(COLOR opt, wxColour &copt);
	void SetColor(COLOR opt, AssColor &copt);
	void SetInt(CONFIG opt, int iopt);
	void SetFloat(CONFIG opt, float fopt);
	void SetTable(CONFIG opt, wxArrayString &iopt, wxString split = L"|");
	void SetIntTable(CONFIG opt, wxArrayInt &iopt, wxString split = L"|");
	void SetCoords(CONFIG opt, int coordx, int coordy);
	void GetRawOptions(wxString &options, bool Audio = false);
	void AddStyle(Styles *styl);
	void ChangeStyle(Styles *styl, int i);
	Styles *GetStyle(int i, const wxString &name = L"", Styles* styl = NULL);
	int FindStyle(const wxString &name, int *multiplication = NULL);
	void DelStyle(int i);
	int StoreSize();
	void CatchValsLabs(const wxString &rawoptions);
	bool SetRawOptions(const wxString &textconfig);
	int LoadOptions();
	void LoadColors(const wxString &themeName = L"");
	//if you want to use defaultOptions alocate table for configSize
	//use with AudioConfig
	void LoadDefaultConfig(wxString * defaultOptions = NULL);
	void LoadDefaultColors(bool dark = true, wxColour *table = NULL);
	//if you want to use defaultOptions alocate table for configSize
	//use with AudioConfig
	void LoadDefaultAudioConfig(wxString * defaultOptions = NULL);
	void LoadMissingColours(const wxString &path);
	bool LoadAudioOpts();
	void ResetDefault();
	void SaveAudioOpts();
	void SaveOptions(bool cfg = true, bool style = true, bool crashed = false);
	void SaveColors(const wxString &path = L"");
	void LoadStyles(const wxString &katalog);
	void clearstyles();
	void Sortstyles();
	void SetHexColor(const wxString &nameAndColor);
	//if failed returns symbol
	const wxString & FindLanguage(const wxString & symbol);
	//main value is 10 offset from 10
	wxFont *GetFont(int offset = 0);
	void FontsClear();
	//wxString GetStringColor(unsigned int);
	wxString GetStringColor(size_t optionName);
	wxString GetReleaseDate();
	csri_rend *GetVSFilter();
	void ChangeVsfilter();
	void GetVSFiltersList(wxArrayString &filters);
	bool HasCrashed(){ return hasCrashed; }
	config();
	~config();
};
bool sortfunc(Styles *styl1, Styles *styl2);
//formating here works like this, 
//first digit - digits before dot, second digit - digits after dot, for example 5.3f;
wxString getfloat(float num, const wxString &format = L"5.3f", bool Truncate = true);
wxBitmap CreateBitmapFromPngResource(const wxString& t_name);
wxBitmap *CreateBitmapPointerFromPngResource(const wxString& t_name);
wxImage CreateImageFromPngResource(const wxString& t_name);
#define wxBITMAP_PNG(x) CreateBitmapFromPngResource(x)
#define PTR_BITMAP_PNG(x) CreateBitmapPointerFromPngResource(x)
void MoveToMousePosition(wxWindow *win);
wxString MakePolishPlural(int num, const wxString &normal, const wxString &plural24, const wxString &pluralRest);
BOOL __stdcall MonitorEnumProc1(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
wxRect GetMonitorRect(int wmonitor, std::vector<tagRECT> *MonitorRects, const wxPoint &position, bool workArea);
static const wxString emptyString;
bool IsNumber(const wxString &txt);

#ifdef _M_IX86
void SetThreadName(DWORD id, const char *name);
#else
void SetThreadName(size_t id, const char *name);
#endif

enum{
	ASS = 1,
	SRT,
	TMP,
	MDVD,
	MPL2,
	FRAME = 10
};

extern config Options;

