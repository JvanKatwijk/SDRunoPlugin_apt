#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "SDRunoPlugin_aptForm.h"
#include "SDRunoPlugin_aptUi.h"
#include "resource.h"
#include <io.h>
#include <shlobj.h>

#define	noaa15	"NOAA-15"
#define	noaa18	"NOAA-18"
#define	noaa19	"NOAA-19"

struct {
	const std::string name;
	int frequency;
} freqTable [] = {
	{ noaa15, 137620000},
	{ noaa18, 137912500},
	{ noaa19, 137100000},
	{ "", 137500000}
};

float	xBuf		[aptHeight * aptWidth];
float	spectrumBuffer	[spectrumHeight * spectrumWidth];
float	wedgeBuffer	[spectrumHeight * spectrumWidth];

template <typename F, typename T>
constexpr T transl (F value, F f1, F t1, T f2, T t2) {
        return f2 + ((t2 - f2) * (value - f1)) / (t1 - f1);
}

//	Form constructor with handles to parent and uno controller
//	- launches form Setup
	SDRunoPlugin_aptForm::
	           SDRunoPlugin_aptForm (SDRunoPlugin_aptUi& parent,
	                                 IUnoPluginController& controller) :
	nana::form(nana::API::make_center(formWidth, formHeight),
		nana::appearance(false, true, false, false, true, false, false)),
	m_parent(parent),
	m_controller(controller) {
	Setup();
}

// Form deconstructor
	SDRunoPlugin_aptForm::~SDRunoPlugin_aptForm () {
	aptContainer	-> clear ();
	aptSpectrum	-> clear ();
	wedgeContainer	-> clear ();
	delete aptContainer;
	delete aptSpectrum;
	delete wedgeContainer;
}

// Start Form and start Nana UI processing
void	SDRunoPlugin_aptForm::Run	() {
	show();
	nana::exec();
}

// Create the initial plugin form
void	SDRunoPlugin_aptForm::Setup () {
	// This first section is all related to the background and border
	// it shouldn't need to be changed
	nana::paint::image img_border;
	nana::paint::image img_inner;
	HMODULE hModule = NULL;
	HRSRC rc_border = NULL;
	HRSRC rc_inner = NULL;
	HRSRC rc_close = NULL;
	HRSRC rc_close_over = NULL;
	HRSRC rc_min = NULL;
	HRSRC rc_min_over = NULL;
	HRSRC rc_bar = NULL;
	HRSRC rc_sett = NULL;
	HRSRC rc_sett_over = NULL;
	HBITMAP bm_border = NULL;
	HBITMAP bm_inner = NULL;
	HBITMAP bm_close = NULL;
	HBITMAP bm_close_over = NULL;
	HBITMAP bm_min = NULL;
	HBITMAP bm_min_over = NULL;
	HBITMAP bm_bar = NULL;
	HBITMAP bm_sett = NULL;
	HBITMAP bm_sett_over = NULL;
	BITMAPINFO bmInfo_border = { 0 };
	BITMAPINFO bmInfo_inner = { 0 };
	BITMAPINFO bmInfo_close = { 0 };
	BITMAPINFO bmInfo_close_over = { 0 };
	BITMAPINFO bmInfo_min = { 0 };
	BITMAPINFO bmInfo_min_over = { 0 };
	BITMAPINFO bmInfo_bar = { 0 };
	BITMAPINFO bmInfo_sett = { 0 };
	BITMAPINFO bmInfo_sett_over = { 0 };
	BITMAPFILEHEADER borderHeader = { 0 };
	BITMAPFILEHEADER innerHeader = { 0 };
	BITMAPFILEHEADER closeHeader = { 0 };
	BITMAPFILEHEADER closeoverHeader = { 0 };
	BITMAPFILEHEADER minHeader = { 0 };
	BITMAPFILEHEADER minoverHeader = { 0 };
	BITMAPFILEHEADER barHeader = { 0 };
	BITMAPFILEHEADER settHeader = { 0 };
	BITMAPFILEHEADER settoverHeader = { 0 };
	HDC hdc = NULL;
	BYTE* borderPixels = NULL;
	BYTE* innerPixels = NULL;
	BYTE* closePixels = NULL;
	BYTE* closeoverPixels = NULL;
	BYTE* minPixels = NULL;
	BYTE* minoverPixels = NULL;
	BYTE* barPixels = NULL;
	BYTE* barfocusedPixels = NULL;
	BYTE* settPixels = NULL;
	BYTE* settoverPixels = NULL;
	const unsigned int rawDataOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	hModule = GetModuleHandle(L"SDRunoPlugin_apt");
	hdc = GetDC(NULL);
	rc_border = FindResource(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), RT_BITMAP);
	rc_inner = FindResource(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), RT_BITMAP);
	rc_close = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE), RT_BITMAP);
	rc_close_over = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), RT_BITMAP);
	rc_min = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN), RT_BITMAP);
	rc_min_over = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), RT_BITMAP);
	rc_bar = FindResource(hModule, MAKEINTRESOURCE(IDB_HEADER), RT_BITMAP);
	rc_sett = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT), RT_BITMAP);
	rc_sett_over = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), RT_BITMAP);
	bm_border = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_inner = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_bar = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_HEADER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bmInfo_border.bmiHeader.biSize = sizeof(bmInfo_border.bmiHeader);
	bmInfo_inner.bmiHeader.biSize = sizeof(bmInfo_inner.bmiHeader);
	bmInfo_close.bmiHeader.biSize = sizeof(bmInfo_close.bmiHeader);
	bmInfo_close_over.bmiHeader.biSize = sizeof(bmInfo_close_over.bmiHeader);
	bmInfo_min.bmiHeader.biSize = sizeof(bmInfo_min.bmiHeader);
	bmInfo_min_over.bmiHeader.biSize = sizeof(bmInfo_min_over.bmiHeader);
	bmInfo_bar.bmiHeader.biSize = sizeof(bmInfo_bar.bmiHeader);
	bmInfo_sett.bmiHeader.biSize = sizeof(bmInfo_sett.bmiHeader);
	bmInfo_sett_over.bmiHeader.biSize = sizeof(bmInfo_sett_over.bmiHeader);
	GetDIBits(hdc, bm_border, 0, 0, NULL, &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, 0, NULL, &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, 0, NULL, &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, 0, NULL, &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, 0, NULL, &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, 0, NULL, &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, 0, NULL, &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, 0, NULL, &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, 0, NULL, &bmInfo_sett_over, DIB_RGB_COLORS);
	bmInfo_border.bmiHeader.biCompression = BI_RGB;
	bmInfo_inner.bmiHeader.biCompression = BI_RGB;
	bmInfo_close.bmiHeader.biCompression = BI_RGB;
	bmInfo_close_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_min.bmiHeader.biCompression = BI_RGB;
	bmInfo_min_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_bar.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett_over.bmiHeader.biCompression = BI_RGB;
	borderHeader.bfOffBits = rawDataOffset;
	borderHeader.bfSize = bmInfo_border.bmiHeader.biSizeImage;
	borderHeader.bfType = 0x4D42;
	innerHeader.bfOffBits = rawDataOffset;
	innerHeader.bfSize = bmInfo_inner.bmiHeader.biSizeImage;
	innerHeader.bfType = 0x4D42;
	closeHeader.bfOffBits = rawDataOffset;
	closeHeader.bfSize = bmInfo_close.bmiHeader.biSizeImage;
	closeHeader.bfType = 0x4D42;
	closeoverHeader.bfOffBits = rawDataOffset;
	closeoverHeader.bfSize = bmInfo_close_over.bmiHeader.biSizeImage;
	closeoverHeader.bfType = 0x4D42;
	minHeader.bfOffBits = rawDataOffset;
	minHeader.bfSize = bmInfo_min.bmiHeader.biSizeImage;
	minHeader.bfType = 0x4D42;
	minoverHeader.bfOffBits = rawDataOffset;
	minoverHeader.bfSize = bmInfo_min_over.bmiHeader.biSizeImage;
	minoverHeader.bfType = 0x4D42;
	barHeader.bfOffBits = rawDataOffset;
	barHeader.bfSize = bmInfo_bar.bmiHeader.biSizeImage;
	barHeader.bfType = 0x4D42;
	settHeader.bfOffBits = rawDataOffset;
	settHeader.bfSize = bmInfo_sett.bmiHeader.biSizeImage;
	settHeader.bfType = 0x4D42;
	settoverHeader.bfOffBits = rawDataOffset;
	settoverHeader.bfSize = bmInfo_sett_over.bmiHeader.biSizeImage;
	settoverHeader.bfType = 0x4D42;
	borderPixels = new BYTE[bmInfo_border.bmiHeader.biSizeImage + rawDataOffset];
	innerPixels = new BYTE[bmInfo_inner.bmiHeader.biSizeImage + rawDataOffset];
	closePixels = new BYTE[bmInfo_close.bmiHeader.biSizeImage + rawDataOffset];
	closeoverPixels = new BYTE[bmInfo_close_over.bmiHeader.biSizeImage + rawDataOffset];
	minPixels = new BYTE[bmInfo_min.bmiHeader.biSizeImage + rawDataOffset];
	minoverPixels = new BYTE[bmInfo_min_over.bmiHeader.biSizeImage + rawDataOffset];
	barPixels = new BYTE[bmInfo_bar.bmiHeader.biSizeImage + rawDataOffset];
	settPixels = new BYTE[bmInfo_sett.bmiHeader.biSizeImage + rawDataOffset];
	settoverPixels = new BYTE[bmInfo_sett_over.bmiHeader.biSizeImage + rawDataOffset];
	*(BITMAPFILEHEADER*)borderPixels = borderHeader;
	*(BITMAPINFO*)(borderPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_border;
	*(BITMAPFILEHEADER*)innerPixels = innerHeader;
	*(BITMAPINFO*)(innerPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_inner;
	*(BITMAPFILEHEADER*)closePixels = closeHeader;
	*(BITMAPINFO*)(closePixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close;
	*(BITMAPFILEHEADER*)closeoverPixels = closeoverHeader;
	*(BITMAPINFO*)(closeoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close_over;
	*(BITMAPFILEHEADER*)minPixels = minHeader;
	*(BITMAPINFO*)(minPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min;
	*(BITMAPFILEHEADER*)minoverPixels = minoverHeader;
	*(BITMAPINFO*)(minoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min_over;
	*(BITMAPFILEHEADER*)barPixels = barHeader;
	*(BITMAPINFO*)(barPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_bar;
	*(BITMAPFILEHEADER*)settPixels = settHeader;
	*(BITMAPINFO*)(settPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett;
	*(BITMAPFILEHEADER*)settoverPixels = settoverHeader;
	*(BITMAPINFO*)(settoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett_over;
	GetDIBits(hdc, bm_border, 0, bmInfo_border.bmiHeader.biHeight, (LPVOID)(borderPixels + rawDataOffset), &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, bmInfo_inner.bmiHeader.biHeight, (LPVOID)(innerPixels + rawDataOffset), &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, bmInfo_close.bmiHeader.biHeight, (LPVOID)(closePixels + rawDataOffset), &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, bmInfo_close_over.bmiHeader.biHeight, (LPVOID)(closeoverPixels + rawDataOffset), &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, bmInfo_min.bmiHeader.biHeight, (LPVOID)(minPixels + rawDataOffset), &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, bmInfo_min_over.bmiHeader.biHeight, (LPVOID)(minoverPixels + rawDataOffset), &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, bmInfo_bar.bmiHeader.biHeight, (LPVOID)(barPixels + rawDataOffset), &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, bmInfo_sett.bmiHeader.biHeight, (LPVOID)(settPixels + rawDataOffset), &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, bmInfo_sett_over.bmiHeader.biHeight, (LPVOID)(settoverPixels + rawDataOffset), &bmInfo_sett_over, DIB_RGB_COLORS);
	img_border.open(borderPixels, bmInfo_border.bmiHeader.biSizeImage);
	img_inner.open(innerPixels, bmInfo_inner.bmiHeader.biSizeImage);
	img_close_normal.open(closePixels, bmInfo_close.bmiHeader.biSizeImage);
	img_close_down.open(closeoverPixels, bmInfo_close_over.bmiHeader.biSizeImage);
	img_min_normal.open(minPixels, bmInfo_min.bmiHeader.biSizeImage);
	img_min_down.open(minoverPixels, bmInfo_min_over.bmiHeader.biSizeImage);
	img_header.open(barPixels, bmInfo_bar.bmiHeader.biSizeImage);
	img_sett_normal.open(settPixels, bmInfo_sett.bmiHeader.biSizeImage);
	img_sett_down.open(settoverPixels, bmInfo_sett_over.bmiHeader.biSizeImage);
	ReleaseDC(NULL, hdc);
	bg_border.load(img_border, nana::rectangle(0, 0, 590, 340));
	bg_border.stretchable(0, 0, 0, 0);
	bg_border.transparent(true);
	bg_inner.load(img_inner, nana::rectangle(0, 0, 582, 299));
	bg_inner.stretchable(sideBorderWidth, 0, sideBorderWidth, bottomBarHeight);
	bg_inner.transparent(false);

	// TODO: Form code starts here

	// Load X and Y location for the form from the ini file (if exists)
	int posX = m_parent.LoadX();
	int posY = m_parent.LoadY();
	move	(posX, posY);

	// This code sets the plugin size, title and what to do when the X is pressed
	size (nana::size(formWidth, formHeight));
	caption ("SDRuno Plugin apt");
	events().destroy([&] { m_parent.FormClosed(); });

	//Initialize header bar
	header_bar.size(nana::size(122, 20));
	header_bar.load(img_header, nana::rectangle(0, 0, 122, 20));
	header_bar.stretchable(0, 0, 0, 0);
	header_bar.move(nana::point((formWidth / 2) - 61, 5));
	header_bar.transparent(true);

	//Initial header text 
	title_bar_label.size(nana::size(65, 12));
	title_bar_label.move(nana::point((formWidth / 2) - 5, 9));
	title_bar_label.format(true);
	title_bar_label.caption("< bold size = 6 color = 0x000000 font = \"Verdana\">apt decoder</>");
	title_bar_label.text_align(nana::align::center, nana::align_v::center);
	title_bar_label.fgcolor(nana::color_rgb(0x000000));
	title_bar_label.transparent(true);

//Iniitialize drag_label
	form_drag_label.move(nana::point(0, 0));
	form_drag_label.transparent(true);

	//Initialize dragger and set target to form, and trigger to drag_label 
	form_dragger.target(*this);
	form_dragger.trigger(form_drag_label);

//Initialise the "Minimize button"
	min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15));
	min_button.bgcolor(nana::color_rgb (0x000000));
	min_button.move(nana::point(formWidth - 51, 9));
	min_button.transparent(true);
	min_button.events().mouse_down([&] { min_button.load(img_min_down, nana::rectangle(0, 0, 20, 15)); });
	min_button.events().mouse_up([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15)); nana::API::zoom_window(this->handle(), false); });
	min_button.events().mouse_leave([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15)); });

	//Initialise the "Close button"
	close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15));
	close_button.bgcolor(nana::color_rgb(0x000000));
	close_button.move(nana::point(formWidth - 26, 9));
	close_button.transparent(true);
	close_button.events().mouse_down([&] { close_button.load(img_close_down, nana::rectangle(0, 0, 20, 15)); });
	close_button.events().mouse_up([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); close(); });
	close_button.events().mouse_leave([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); });

	//Uncomment the following block of code to Initialise the "Setting button"
	sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15));
	sett_button.bgcolor(nana::color_rgb(0x000000));
	sett_button.move(nana::point(10, 9));
	sett_button.events().mouse_down([&] { sett_button.load(img_sett_down, nana::rectangle(0, 0, 40, 15)); });
	sett_button.events().mouse_up([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); SettingsButton_Click(); });
	sett_button.events().mouse_leave([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); });
	sett_button.tooltip("Show settings window");
	sett_button.transparent(true);

// TODO: Extra Form code goes here
//
//      we draw the picture on an label with a size of aptWidth x aptHeight
	aptContainer = new drawing (imageLabel);
	aptContainer -> draw ([&](paint::graphics& graph) {
	        for (int i = 0; i < aptHeight; i ++) {
	           for (int j = 0; j < aptWidth; j ++) {
	              float res = xBuf [i * aptWidth + j];
	              graph.set_pixel (j, i, mapColor (res));
	          }
	        }
	       });
//      we draw the picture on an label with a size of aptWidth x aptHeight
//
	aptSpectrum = new drawing (spectrumLabel);
	aptSpectrum -> draw ([&](paint::graphics& graph) {
	        for (int i = 0; i < spectrumHeight; i ++) {
	           for (int j = 0; j < spectrumWidth; j ++) {
	              float res = spectrumBuffer [i * spectrumWidth + j];
	              graph. set_pixel (j, i, res >= 128 ?
	                                         nana::colors::white :
	                                         nana::colors::black);
	          }
	        }
	       });

	wedgeContainer = new drawing (wedgeLabel);
	wedgeContainer -> draw ([&](paint::graphics& graph) {
	        for (int i = 0; i < wedgeHeight; i ++) {
	           for (int j = 0; j < wedgeWidth; j ++) {
	              float res = wedgeBuffer [i * wedgeWidth + j];
	              graph. set_pixel (j, i, res >= 128 ?
	                                         nana::colors::white :
	                                         nana::colors::black);
	          }
	        }
	       });
//	coloring the labels
	copyRightLabel. transparent (true);
	copyRightLabel. fgcolor (nana::colors::white);
	copyRightLabel. caption ("\xa9");
	copyRightLabel. tooltip ("Created by Jan van Katwijk, all rights reserved\n");

	statusLabel. transparent (true);
	statusLabel. fgcolor (nana::colors::white);
	statusLabel. tooltip ("shows - if synced - offsets of A sync and B sync, if not synced the sync error");

	syncedLabel. bgcolor (nana::colors::red);
	syncedLabel. tooltip ("Colors green if synced, red otherwise");

	dumpName. transparent (true);
	dumpName. fgcolor (nana::colors::white);

	channels. push_back (noaa15);
	channels. push_back (noaa18);
	channels. push_back (noaa19);
	channels. option (0);
	channels. events (). selected ([&] (const nana::arg_combox &ar_cbx)
		{apt_setChannel (ar_cbx. widget. caption ());});
	channels. tooltip ("predefined settings for the common noaa satelites");

	startButton. caption ("start");
	startButton. events (). click ([&] () { apt_start ();});
	startButton. tooltip ("attempt to decode incoming data stream starts after touching this button");

	stopButton. caption ("stop");
	stopButton. events (). click ([&] () { apt_stop ();} );
	stopButton. tooltip ("decoding of incoming data stream will stop after touching this button");

	lineNumber.transparent(true);
	lineNumber.fgcolor(nana::colors::white);
	lineNumber. tooltip ("Number of current line in picture");

	greyLabel. transparent (true);
	greyLabel. fgcolor (nana::colors::white);
	greyLabel. caption ("0");
	greyDifference	= 50;
	greyLabel. tooltip ("adjustment value for grey intensity");
	

	resetButton.caption ("reset");
	resetButton.events().click([&]() {apt_reset(); });
	resetButton. tooltip ("reset the decoding process");

	savePictureButton. caption ("save");
	savePictureButton. events (). click ([&] () { apt_saveFile ();});
	savePictureButton. tooltip ("save the current picture - in the official format - to a file as bitmap");

	reverseButton. caption ("Reverse");
	reverseButton. events (). click ([&] () { apt_reverseImage ();});
	reverseButton. tooltip ("If the satelite crosses from south to north, the picture will be reversed, with this button you can reverse the picture, but only if decoding is stopped");

	printButton. caption ("Print");
	printButton. events (). click ([&] () { apt_printImage ();});
	printButton. tooltip ("reprint the current picture on the screen, used after setting different grey value");

	greyCorrector. maximum (100);
	greyCorrector. value (50);
	greyCorrector. events (). value_changed ([&]
	                              (const nana::arg_slider &s) {
	                                 handle_greyCorrector (greyCorrector. value ());});
	greyCorrector. tooltip ("adjustment for the range of grey values");

	dumpfileButton. caption ("dump");
	dumpfileButton. events (). click ([&] () { apt_saveFile ();});
	dumpfileButton. tooltip ("save the current input after FM decoding in 11025 Ss format");

	delete[] borderPixels;
	delete[] innerPixels;
	delete[] closePixels;
	delete[] closeoverPixels;
	delete[] minPixels;
	delete[] minoverPixels;
	delete[] barPixels;
	delete[] barfocusedPixels;
	delete[] settPixels;
	delete[] settoverPixels;
}

void SDRunoPlugin_aptForm::SettingsButton_Click () {
	// TODO: Insert code here to show settings panel
}

//
//	signals to "up"
void	SDRunoPlugin_aptForm::apt_start	() {
	m_parent. apt_start ();
}

void	SDRunoPlugin_aptForm::apt_stop	() {
	m_parent. apt_stop ();
}

void	SDRunoPlugin_aptForm::apt_reset() {
	m_parent.apt_reset ();
} 

void	SDRunoPlugin_aptForm::apt_savePicture	() {
	m_parent. apt_savePicture ();
}

void	SDRunoPlugin_aptForm::apt_reverseImage	() {
	m_parent. apt_reverseImage ();
}

void	SDRunoPlugin_aptForm::apt_printImage	() {
	m_parent. apt_printImage ();
}

void	SDRunoPlugin_aptForm::apt_setChannel	(const std::string &s) {
	for (int i = 0; freqTable [i]. name != ""; i ++)
	   if (freqTable [i]. name == s) {
	      m_parent. apt_setChannel (s, freqTable [i]. frequency);
	      return;
	   }
}
//
//	"commands coming down"
void	SDRunoPlugin_aptForm::showWedge		(std::vector<float> &, int) {
}

void    SDRunoPlugin_aptForm::drawLine  (std::vector<float> &line, int lineno) {
float v;
float	max	= 0;
float	min	= 10000;

	for (int i = 0; i < line. size (); i ++) {
	   if (line [i] > max)
	      max = line [i];
	   else
	   if (line [i] < min)
	      min = line [i];
	}
	for (int i = 0; i < line. size (); i += 2) {
	   if (greyDifference >= 0)	// make it lighter
	      v	= transl (line [i], min, max, greyDifference, 255);
	   else
	      v	= transl (line [i], min, max, 0, 255 + greyDifference);
	   
		xBuf[lineno * aptWidth + i / 2] = v;
	}
}

void	SDRunoPlugin_aptForm::setSynced		(bool b) {
	syncedLabel. bgcolor (b? nana::colors::green : nana::colors::red);
}

void    SDRunoPlugin_aptForm::updateImage       () {
	aptContainer -> update ();
}

void	SDRunoPlugin_aptForm::clearScreen	() {
	for (int X = 0; X < aptWidth; X ++)
	   for (int Y = 0; Y < aptHeight; Y ++)
	      xBuf [Y * aptWidth + X] = 255;
	updateImage ();
}

void	SDRunoPlugin_aptForm::drawSpectrum (std::vector<float> &v) {
float	max = 1;
	if (v. size () < spectrumWidth)
	   for (int i = 0; i < spectrumWidth; i ++)
	      v. push_back (0);
	for (int i = 0; i < spectrumHeight; i ++)
	   for (int j = 0; j < spectrumWidth; j++)
	      spectrumBuffer [i * spectrumWidth + j] = 255;
	for (int i = 0; i < spectrumWidth; i ++) 
	   if (v [i] > max)
	      max = v [i];
	max += 0.5;
	for (int i = 1; i < spectrumWidth - 1; i++) {
	   int y_index = spectrumHeight -  v[i] / max * spectrumHeight - 0.5;
	   spectrumBuffer [y_index * spectrumWidth + i] = 0;
	   if (y_index == 0)
	      spectrumBuffer [(y_index + 1) * spectrumWidth + i] = 0;
	   else
	      spectrumBuffer [(y_index - 1) * spectrumWidth + i] = 0;
	   spectrumBuffer [(y_index - 1) * spectrumWidth + i - 1] = 0;
	   spectrumBuffer [(y_index - 1) * spectrumWidth + i + 1] = 0;
	}
	aptSpectrum -> update ();
}

void	SDRunoPlugin_aptForm::clearSpectrum () {
	for (int X = 0; X < spectrumWidth; X ++)
	   for (int Y = 0; Y < spectrumHeight; Y ++)
	      spectrumBuffer [Y * spectrumWidth + X] = 255;
	aptSpectrum -> update ();
}

void	SDRunoPlugin_aptForm::drawWedge (std::vector<float> &v, int l) {
float	max = 0;
	if (l < wedgeWidth)
	   for (int i = l; i < wedgeWidth; i ++)
	      v [i] = 0;
	for (int i = 0; i < wedgeHeight; i ++)
	   for (int j = 0; j < wedgeWidth; j++)
	      wedgeBuffer [i * wedgeWidth + j] = 255;
	for (int i = 0; i < wedgeWidth; i ++) {
		v[i] += 0.1;

	   if (v [i] > max)
	      max = v [i];
	}
	max += 0.5;
	for (int i = 1; i < wedgeWidth - 1; i++) {
	   int y_index = wedgeHeight - v [i] / max * wedgeHeight;
	   if (y_index < 0) {
	      y_index = wedgeHeight / 2;
	   }
	   if (y_index >= wedgeHeight) {
	      y_index = wedgeHeight / 2;
	   }
	
	   wedgeBuffer [y_index * wedgeWidth + i] = 0;
	   if (y_index == 0)
	      wedgeBuffer [(y_index + 1) * wedgeWidth + i] = 0;
	   else
	      wedgeBuffer [(y_index - 1) * wedgeWidth + i] = 0;
	   wedgeBuffer [(y_index) * wedgeWidth + i - 1] = 0;
	   wedgeBuffer [(y_index) * wedgeWidth + i + 1] = 0;
	}
	wedgeContainer -> update ();
}

void	SDRunoPlugin_aptForm::clearWedge () {
	for (int X = 0; X < wedgeWidth; X ++)
	   for (int Y = 0; Y < wedgeHeight; Y ++)
	      wedgeBuffer [Y * wedgeWidth + X] = 255;
	wedgeContainer -> update ();
}

void	SDRunoPlugin_aptForm::status	(const std::string &s) {
	statusLabel. caption (s);
}

nana::colors greyTable [] {
	nana::colors::black,
	nana::colors::dark_grey,
	nana::colors::dark_slate_grey,
	nana::colors::dim_grey,
	nana::colors::grey,
	nana::colors::light_grey,
	nana::colors::light_slate_grey,
	nana::colors::slate_grey,
	nana::colors::white
};
int table [] = {0, 75, 112, 155, 180, 255};

nana::color	SDRunoPlugin_aptForm::mapColor (uint8_t c) {
	return nana::color (c, c, c);
}

void	SDRunoPlugin_aptForm::showLineNumber	(int l) {
	lineNumber.caption (std::to_string (l));
}

void	SDRunoPlugin_aptForm::show_dumpName	(const std::string &s) {
	dumpName. caption (s);
}

void	SDRunoPlugin_aptForm::handle_greyCorrector (int v) {
	greyLabel. caption (std::to_string (v - 50));
	greyDifference	= v - 50;
}

int	SDRunoPlugin_aptForm::get_greySetting	() {
	return greyDifference;
}

void	SDRunoPlugin_aptForm::apt_saveFile	() {
	m_parent. apt_saveFile ();
}

void	SDRunoPlugin_aptForm::dumpfileText	(const std::string &s) {
	dumpfileButton. caption (s);
}

