#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

using namespace nana;

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
//#define formWidth (297)
#define formWidth (1100)
#define formHeight (760)
#define	aptWidth	(formWidth - 60)
#define	aptHeight	(formHeight - 200)

#define	spectrumWidth	(formWidth / 2 - 40)
#define	wedgeWidth	(formWidth / 2 - 40)
#define	spectrumHeight	60
#define	wedgeHeight	60
class SDRunoPlugin_aptUi;

class SDRunoPlugin_aptForm : public nana::form {
public:

		SDRunoPlugin_aptForm	(SDRunoPlugin_aptUi& parent,
	                                 IUnoPluginController& controller);		
		~SDRunoPlugin_aptForm	();
	void	Run			();
//
//	going up
	void	apt_start		();
	void	apt_stop		();
	void	apt_reset		();
	void	apt_savePicture		();
	void	apt_reverseImage	();
	void	apt_printImage		();
	void	apt_setChannel		(const std::string &);
	int	pictureWidth		() {return aptWidth;}
	int	pictureHeight		() {return aptHeight;}
//
//	coming down
	void	updateImage		();
	void	showWedge		(std::vector<float> &, int);
	void	drawSpectrum		(std::vector<float> &);
	void	drawLine		(std::vector<float> &line, int lineno);
	void	setSynced		(bool b);
	void	clearScreen		();
	void	clearSpectrum		();
	void	status			(const std::string &);

	void	drawWedge		(std::vector<float> &, int);
	void	clearWedge		();
	void	showLineNumber		(int);
	void	show_dumpName		(const std::string &);

	void	handle_greyCorrector	(int);
	int	get_greySetting		();
private:

	void Setup			();

// The following is to set up the panel graphic to look
// like a standard SDRuno panel
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	nana::picture close_button {*this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button {*this, nana::rectangle(0, 0, 20, 15) };

	// Uncomment the following 4 lines if you want a SETT button and panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();

	// TODO: Now add your UI controls here
//
	nana::combox	channels	{*this, nana::rectangle
	                                         (40, 40, 80, 20) };
	nana::button	startButton	{*this, nana::rectangle
	                                         (130, 40, 50, 20) };
	nana::button	stopButton	{*this, nana::rectangle
	                                         (190, 40, 50, 20) };
	nana::button	savePictureButton{ *this, nana::rectangle
	                                         (250, 40, 50, 20) };
	nana::button	resetButton	{*this, nana::rectangle
	                                         (310, 40, 50, 20) };
	nana::button	reverseButton	{*this, nana::rectangle
	                                         (370, 40, 50, 20) };
	nana::button	printButton	{*this, nana::rectangle
	                                         (430, 40, 50, 20)};
	nana::label	syncedLabel	{*this, nana::rectangle
	                                         (490, 40, 40, 20)};
	nana::label	statusLabel	{*this, nana::rectangle
	                                         (540, 40, 120, 20) };
	nana::label	lineNumber	{*this, nana::rectangle
	                                         (650, 40, 50, 20) };
	nana::slider	greyCorrector	{*this, nana::rectangle 
	                                         (710, 40, 120, 20) };
	nana::label	greyLabel	{*this, nana::rectangle
	                                         (840, 40, 50, 20) };
	nana::label	dumpName	{*this, nana::rectangle
	                                         (900, 40, 100, 20) };
        nana::label	copyRightLabel	{*this, nana::rectangle
	                                         (1010, 40, 20, 20)};

	nana::label     imageLabel	{*this, nana::rectangle
	                                         (30, 80, aptWidth, aptHeight)};
        drawing         *aptContainer;
	nana::label     spectrumLabel	{*this, nana::rectangle
	                                         (30, 
	                                          80 + aptHeight, 
	                                          spectrumWidth,
	                                          spectrumHeight)};
        drawing         *aptSpectrum;
	nana::label     wedgeLabel	{*this, nana::rectangle
	                                         (30 + spectrumWidth,
	                                          80 + aptHeight, 
	                                          wedgeWidth,
	                                          wedgeHeight)};
        drawing         *wedgeContainer;

	SDRunoPlugin_aptUi	& m_parent;
	IUnoPluginController	& m_controller;
	nana::color mapColor (uint8_t c);
	int		greyDifference;

};


