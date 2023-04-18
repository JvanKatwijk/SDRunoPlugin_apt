#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_aptForm.h"

// Forward reference
class SDRunoPlugin_apt;

class SDRunoPlugin_aptUi {
public:

		SDRunoPlugin_aptUi	(SDRunoPlugin_apt& parent,
	                                 IUnoPluginController& controller);
		~SDRunoPlugin_aptUi	();

	void	HandleEvent	(const UnoEvent& evt);
	void	FormClosed	();
	void	ShowUi		();
	int	LoadX		();
	int	LoadY		();
//	going up
	void	apt_start		();
	void	apt_stop		();
	void	apt_reset();
	void	apt_savePicture		();
	void	apt_saveFile		();
	void	apt_reverseImage	();
	void	apt_printImage		();
	void	apt_setChannel		(const std::string &, int);

//	coming down
	void	updateImage		();
	void	drawSpectrum		(std::vector<float> &);
	void	drawWedge		(std::vector<float> &, int);
	void	drawLine		(std::vector<float> &, int);
	void	setSynced		(bool);
	void	clearScreen		();
	void	clearSpectrum		();
	void	clearWedge		();
	void	status			(const std::string &);
	void	showLineNumber		(int);
	void	show_dumpName		(const std::string &);
	int	maxLine			() {return aptHeight;}
	int	get_greySetting		();
	void	dumpfileText		(const std::string &);
//
private:
	
	SDRunoPlugin_apt & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_aptForm> m_form;
	bool m_started;
	std::mutex m_lock;
	IUnoPluginController & m_controller;
};
