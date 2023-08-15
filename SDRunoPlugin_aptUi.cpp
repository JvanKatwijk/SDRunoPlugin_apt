#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_apt.h"
#include "SDRunoPlugin_aptUi.h"
#include "SDRunoPlugin_aptForm.h"

// Ui constructor - load the Ui control into a thread
	SDRunoPlugin_aptUi::
	         SDRunoPlugin_aptUi (SDRunoPlugin_apt& parent,
	                               IUnoPluginController& controller) :
	                                            m_parent (parent),
	                                            m_form (nullptr),
	                                            m_controller(controller) {
	m_thread = std::thread (&SDRunoPlugin_aptUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
	SDRunoPlugin_aptUi::~SDRunoPlugin_aptUi () {	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void	SDRunoPlugin_aptUi::ShowUi () {	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_aptForm> (*this, m_controller);
	m_lock.unlock();
	m_form->Run();
}

// Load X from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_aptUi::LoadX () {
	std::string tmp;
	m_controller.GetConfigurationKey("apt.X", tmp);
	if (tmp.empty ()) {
	   return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_aptUi::LoadY () {
	std::string tmp;
	m_controller.GetConfigurationKey ("apt.Y", tmp);
	if (tmp.empty ()) {
	   return -1;
	}
	return stoi(tmp);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void	SDRunoPlugin_aptUi::HandleEvent (const UnoEvent& ev) {
	switch (ev.GetType()) {
	   case UnoEvent::StreamingStarted:
	      break;

	   case UnoEvent::StreamingStopped:
	      break;

	   case UnoEvent::SavingWorkspace:
	      break;

	   case UnoEvent::ClosingDown:
	      FormClosed ();
	      break;

	   default:
	      break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void	SDRunoPlugin_aptUi::FormClosed () {
	m_controller. RequestUnload (&m_parent);
}

void	SDRunoPlugin_aptUi::apt_start		() {
	m_parent. apt_start ();
}

void	SDRunoPlugin_aptUi::apt_stop		() {
	m_parent. apt_stop ();
}

void	SDRunoPlugin_aptUi::apt_reset() {
	m_parent.apt_reset();
}

void	SDRunoPlugin_aptUi::apt_savePicture	() {
	m_parent. apt_savePicture ();
}

void	SDRunoPlugin_aptUi::apt_saveFile	() {
	m_parent. apt_saveFile ();
}

void	SDRunoPlugin_aptUi::apt_reverseImage	() {
	m_parent. apt_reverseImage ();
}

void	SDRunoPlugin_aptUi::apt_printImage() {
	m_parent. apt_printImage();
}

void	SDRunoPlugin_aptUi::apt_setChannel	(const std::string &s,
	                                                 int freq) {
	m_parent. apt_setChannel (s, freq);
}
//

void	SDRunoPlugin_aptUi::updateImage		() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> updateImage ();
}

void	SDRunoPlugin_aptUi::drawWedge		(std::vector<float> &line,
	                                                    int length) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> drawWedge (line, length);
}

void	SDRunoPlugin_aptUi::drawSpectrum	(std::vector<float> &s) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr) 
	   m_form -> drawSpectrum (s);
}

void	SDRunoPlugin_aptUi::drawLine	(std::vector<float> &v, int ln) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr) 
	   m_form -> drawLine (v, ln);
}

void	SDRunoPlugin_aptUi::setSynced	(bool b) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> setSynced (b);
}

void	SDRunoPlugin_aptUi::clearScreen	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> clearScreen ();
}

void	SDRunoPlugin_aptUi::clearSpectrum	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> clearSpectrum ();
}

void	SDRunoPlugin_aptUi::clearWedge		() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> clearWedge ();
}

void	SDRunoPlugin_aptUi::status	(const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
           m_form -> status (s);
}

void	SDRunoPlugin_aptUi::showLineNumber (int ln) {
	std::lock_guard<std::mutex> l(m_lock);
	if (m_form != nullptr)
		m_form->showLineNumber(ln);
}

void	SDRunoPlugin_aptUi::show_dumpName	(const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> show_dumpName (s);
}

void	SDRunoPlugin_aptUi::dumpfileText	(const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
           m_form -> dumpfileText (s);
}

int	SDRunoPlugin_aptUi::get_greySetting	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   return m_form -> get_greySetting ();
	return 0;
}

void    SDRunoPlugin_aptUi::showOffset  (float x) {
std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
           m_form -> showOffset (x);
}


