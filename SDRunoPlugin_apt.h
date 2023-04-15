#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>

#include "SDRunoPlugin_aptUi.h"
//
//	plugin specifics
#include	"ringbuffer.h"
#include	".\support\fir-filters.h"
#include	".\support\fft-filters.h"
#include	".\support\pllC.h"
#include	<stdio.h>

class SDRunoPlugin_apt : public IUnoPlugin,
	                          public IUnoStreamProcessor,
	                          public IUnoAudioProcessor {
public:
	
		SDRunoPlugin_apt	(IUnoPluginController& controller);
	virtual ~SDRunoPlugin_apt	();

virtual
	const char* GetPluginName() const override {
	   return "SDRuno apt Plugin";
	}

	// IUnoPlugin
virtual
	void	HandleEvent (const UnoEvent& ev) override;
//
//	coming from the GUI
	void	apt_start		();
	void	apt_stop		();
	void	apt_reset		();
	void	apt_savePicture		();
	void	apt_reverseImage	();
	void	apt_printImage		();
	void	apt_setChannel		(const std::string &, int);
private:
	bandpassFIR		passbandFilter;
	decimatingFIR		DecimatingF;
	fftFilterHilbert        Hilbert1;
	fftFilterHilbert        Hilbert2;
	lowpassFIR              theFilter;
	pllC			myfm_pll;

	int                     convBufferSize;
	int                     convIndex;
	int                     outSize;
	std::vector<int>        mapTable_int;
	std::vector<float>      mapTable_float;
	std::vector<float>        convBuffer;

	std::mutex	        m_lock;
	SDRunoPlugin_aptUi	m_form;
	std::mutex		locker;
	IUnoPluginController    *m_controller;
	void		        WorkerFunction		();
	std::thread*	        m_worker;
	RingBuffer<float>	inputBuffer;
	std::atomic<bool> 	running;
	int			selectedFrequency;
	int			centerFrequency;
	void	                StreamProcessorProcess (channel_t    channel,
	                               Complex* buffer,
	                               int          length,
	                               bool& modified);
	void	                AudioProcessorProcess (channel_t channel,
	                               float* buffer,
		                       int length,
		                       bool& modified);
	void	                process (std::complex<float> z);
	void	                processSample (std::complex<float>);
	void			processLine	();
	int			findStart	(int16_t *, int, int);
	int			findSync	(char, int16_t *, int, int);
	int			readLine	(int16_t *, int, int);
	void			exchange	(int, int);
	void			apt_drawSpectrum	();
	void			setPixel	(int, int, float);
	float			fm_afc;
	float			fm_cvt;
	float			K_FM;
	float			fmDcAlpha;
	float			am_carr_ampl;
	float			carrierAlpha;
	uint32_t		mask;
	int			amount;
	std::atomic<bool>	decoding;
	int			lineLength;
	int			pictureWidth;
	int			bufferLength;
	std::vector<int16_t>	amplBuffer;
	std::vector<std::vector<int16_t>> channelBuffer;
	std::vector<float>	wedgeBuffer;
	int			channelIndex;
	int			npos;
	std::complex<float>	spectrumBuffer [2048];
	int			spectrumFillPointer;

	int			synced;
	int			startPos;
	int			failures;
	int			lineno;
};
