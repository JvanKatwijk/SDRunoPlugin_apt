#include	<sstream>
#include	<unoevent.h>
#include	<iunoplugincontroller.h>
#include	<vector>
#include	<sstream>
#include	<chrono>
#include	<Windows.h>
#include        <mutex>
#include	<atomic>
#include	"SDRunoPlugin_apt.h"
#include	"SDRunoPlugin_aptUi.h"

#include	".\support\fft-complex.h"

#define  _USE_MATH_DEFINES
#include <math.h>

#define	INRATE		192000
#define	OVER_SAMPLING	3
#define	APT_RATE	4160
#define	WORKING_RATE	(OVER_SAMPLING * APT_RATE)
#define	CARRIERFREQ	2400

#define	START_WEDGE	(39 + 47 + 909)
#define	WEDGE_SIZE	45

#define	RfDcAlpha	(1.0 / INRATE)
#define DCRlimit	0.01f

template <typename F, typename T>
constexpr T transl (F value, F f1, F t1, T f2, T t2) {
	return f2 + ((t2 - f2) * (value - f1)) / (t1 - f1);
}

static inline
int     npow2 (int size) {
int     length = 2;
	while (length < size)
	   length *= 2;
	return 2 * length;
}

static inline
std::complex<float> cmul(std::complex<float> x, float y) {
	return std::complex<float>(real(x) * y, imag(x) * y);
}

      
	SDRunoPlugin_apt::
	            SDRunoPlugin_apt (IUnoPluginController& controller) :
	                                   IUnoPlugin (controller),
	                                   m_form (*this, controller),
	                                   m_worker (nullptr),
	                                   inputBuffer (128 * 32768),
	                                   passbandFilter (25,
	                                                   -20000,
	                                                   20000,
	                                                   INRATE),
	                                   Dec48_filter (25, 20000,
	                                                INRATE, 4),
	                                   Dec16_filter (25, 8000,
	                                                INRATE / 4, 3),
	                                   H_filter	(4096, 255),
	                                   H2_filter	(8192, 255),
	                                   theFilter	(61, 0, 3000,
	                                                      WORKING_RATE),
	                                   myfm_pll	(INRATE / 4,
	                                                 0,
	                                                 -2 * CARRIERFREQ,
	                                                  2 * CARRIERFREQ,
	                                                 30000) {

	m_controller	        = &controller;
	running. store (false);
	decoding.store(false);
//	m_controller    -> RegisterStreamProcessor (0, this);
	m_controller	-> RegisterAudioProcessor (0, this);
	m_controller	-> SetDemodulatorType (0,
	                            IUnoPluginController::DemodulatorIQOUT);
	if (m_controller->GetAudioSampleRate(0) != 192000)
	   return;

	int	selectedRate	= 16000;
	int	outRate		= WORKING_RATE;
	convBufferSize		= selectedRate / 20;
	outSize			= outRate / 20;
	mapTable_int. resize (outSize);
	mapTable_float. resize (outSize);
	convBuffer. resize (convBufferSize + 1);
	for (int i = 0;  i < outSize; i ++) {
	   float inVal = float (convBufferSize);
	   mapTable_int [i] = (int)(floor (i * inVal / (float)outSize));
	   mapTable_float [i] = i * (inVal / (float)outSize) - mapTable_int [i];
	}
	convIndex		= 0;

	outfileRate		= 11025;
	outTable_int. resize (outfileRate);
	outTable_float. resize (outfileRate);
	outfileBuffer. resize (selectedRate + 1);
	for (int i = 0; i < outfileRate; i ++) {
	   float inVal = (float)selectedRate;
	   outTable_int [i] = (int)(floor (i * inVal / (float)outfileRate));
	   outTable_float [i] = i * inVal / (float)outfileRate - outTable_int [i];
	}
	outTableIndex	= 0;

	am_carr_ampl	= 0;
	carrierAlpha	= 0.001f;

	
	lineLength	= WORKING_RATE / 2;
	pictureWidth	= APT_RATE / 2;
	bufferLength	= npow2 (WORKING_RATE);
	amplBuffer. resize (bufferLength);
	channelBuffer. resize (m_form. maxLine ());
	for (int i = 0; i < m_form. maxLine (); i ++)
	   channelBuffer. at (i). resize (pictureWidth);
	channelIndex    = 0;
	wedgeBuffer. resize (m_form. maxLine ());
	spectrumFillPointer	= 0;
	mask		= bufferLength - 1;
	amount		= 0;
	npos		= 0;
	synced		= 0;
	startPos	= 0;
	failures	= 0;
	lineno		= 0;
	dumping.store(false);

	RfDC		= std::complex<float> (0, 0);
	m_controller	-> SetCenterFrequency (0, 137912500.0);
//	m_controller	-> SetVfoFrequency (0, 137912500.0);
	m_worker        = new std::thread (&SDRunoPlugin_apt::WorkerFunction,
	                                                               this);
}

	SDRunoPlugin_apt::~SDRunoPlugin_apt () {	
	decoding. store (false);
	running. store (false);
	Sleep(100);
	m_worker->join(); 
	m_controller    -> UnregisterAudioProcessor (0, this);

	delete m_worker;
}

void    SDRunoPlugin_apt::StreamProcessorProcess (channel_t    channel,
	                                          Complex      *buffer,
	                                          int          length,
	                                          bool         &modified) {
	(void)channel; (void)buffer; (void)length;
	modified = false;
}

void    SDRunoPlugin_apt::AudioProcessorProcess (channel_t channel,
	                                         float* buffer,
	                                         int length,
	                                         bool& modified) {
//	Handling IQ input, note that SDRuno interchanges I and Q elements
	if (!modified) {
	   if (decoding. load ()) {
	      for (int i = 0; i < length; i++) {
	         std::complex<float> sample =
	                   std::complex<float> (2 * buffer [2 * i + 1],
	                                        2 * buffer [2 * i]);
		  
	         inputBuffer. putDataIntoBuffer (&sample, 1);
	      }
	   }
	}
}

void	SDRunoPlugin_apt::HandleEvent (const UnoEvent& ev) {
	switch (ev. GetType ()) {
	   case UnoEvent::FrequencyChanged:
	      break;

	   case UnoEvent::CenterFrequencyChanged:
	      break;

	   default:
	      m_form. HandleEvent (ev);
	      break;
	}
}


void	SDRunoPlugin_apt::WorkerFunction () {
//std::vector<std::complex<float>> res (outSize);
std::complex<float> sample;
	running. store (true);
	decoding.store(false);
	m_form. clearScreen ();
	m_form. clearSpectrum ();
	m_form. clearWedge	();
	m_form. setSynced (false);
	while (running. load ()) {
	   if (!decoding. load ()) {
	      Sleep (100);
	      continue;
	   }
	   if (inputBuffer.GetRingBufferReadAvailable() < 1) {
		   Sleep (1);
		   continue;
	   }
	   inputBuffer. getDataFromBuffer (&sample, 1);
	   sample	= passbandFilter. Pass (sample);
	   if (!Dec48_filter. Pass (sample, &sample))
	      continue;

	   if (abs (sample) < 0.001)
	      sample = std::complex<float> (0.001, 0.001);
	   sample = cmul (sample, 1.0 / abs (sample));
//
//	handle the FM
	   RfDC	= cmul (sample - RfDC, RfDcAlpha) + RfDC;
//	limit the maximum DC correction.
	   float rfDcReal = real (RfDC);
	   float rfDcImag = imag (RfDC);
	   if (rfDcReal > +DCRlimit)
	      rfDcReal = +DCRlimit;
	   else
	   if (rfDcReal < -DCRlimit)
	      rfDcReal = -DCRlimit;

	   if (rfDcImag > +DCRlimit)
	      rfDcImag = +DCRlimit;
	   else
	   if (rfDcImag < -DCRlimit)
	      rfDcImag = -DCRlimit;

	   sample -= std::complex<float> (rfDcReal, rfDcImag);

	   myfm_pll. do_pll (sample);
	   float rr	= myfm_pll. getPhaseIncr ();
	   sample	= H2_filter. Pass (20 * rr);
//
//      we now have a 48000 Ss signal, it is a demodulated FM
//      signal with a bandwidth of about 4 Khz
//      In step 1 we decimate to 16000 Ss, then interpolating
//      to 12480 Ss, such that we have 3 samples per element
           if (!Dec16_filter. Pass (sample, &sample))
              continue;
//
	   if (dumping. load ()) {
	      outfileBuffer [outTableIndex ++] = rr;
	      if (outTableIndex >= 16000) {
	         float fileBuffer [11025];
	         for (int j = 0; j < outfileRate; j ++) {
	            int16_t inpBase = outTable_int [j];
	            float   inpRatio = outTable_float [j];
	            fileBuffer [j] = outfileBuffer [inpBase + 1] * inpRatio +
	                              outfileBuffer [inpBase] * (1 - inpRatio);
	         }
	         sf_writef_float (dumpFile_11025, fileBuffer, outfileRate);
	         outfileBuffer [0] = outfileBuffer [16000];
	         outTableIndex = 1;
	      }
	   }
	   convBuffer [convIndex ++] = sample;
	   if (convIndex <  convBufferSize + 1) 
	      continue;
//
//	convert the buffer with samples on a 16000 rate to
//	their final rate
	   for (int j = 0; j < outSize; j ++) {
	      int16_t inpBase = mapTable_int [j];
	      float   inpRatio = mapTable_float [j];
	      std::complex<float> Z =
	                cmul (convBuffer [inpBase + 1], inpRatio) +
	                     cmul (convBuffer [inpBase], (1 - inpRatio));
	      processSample (theFilter. Pass (Z));
	   }
	   convBuffer [0] = convBuffer [convBufferSize];
	   convIndex = 1;
	}
}

void	SDRunoPlugin_apt::processSample (std::complex<float> Z) {

	if (!decoding.load())
		return;

	spectrumBuffer[spectrumFillPointer] = Z;
	spectrumFillPointer ++;
	if (spectrumFillPointer >= 2048) {
	   apt_drawSpectrum ();
	   spectrumFillPointer = 0;
	}

	am_carr_ampl = (1 - carrierAlpha) * am_carr_ampl +
	                                    carrierAlpha * abs (Z);
	float gainLimit	= 0.001f;
	float res = (abs (Z) - am_carr_ampl) /
                     (am_carr_ampl < gainLimit ? gainLimit : am_carr_ampl);
	amplBuffer [npos] = 8 * res;
	npos = (npos + 1) & mask;
	amount ++;
	if (amount < bufferLength) 
	   return;
//
//	searchWidth is half a linelength (including oversampling)
	int searchWidth      = lineLength / 4;
//
//	To avoid false positives, we first compute
//	an offset and validate by looking at the B offset
	int	A_offset = -1, B_offset = -1, C_offset = -1;
//
//	synced should be 3 to assume we are synced
	if (synced < 3) {
	   A_offset = findSync('A', amplBuffer.data(), startPos, searchWidth);

	   if (A_offset < 0) {
	      startPos = (startPos + searchWidth) & mask;
	      amount -= searchWidth;
	      synced	= 0;
	      return;
	   }
//	Validating is by first checking that we see the syncB
	   B_offset = findSync ('B', amplBuffer. data (),
	                             startPos + A_offset + lineLength / 2 - 10,
	                             30);
	   m_form.status("synced =   " + std::to_string(A_offset) + " " + std::to_string (B_offset));

	   if ((B_offset < 0) || (B_offset > 30)) {
	      startPos = (startPos + searchWidth) & mask;
	      amount -= searchWidth;
	      synced	= 0;
	      return;
	   }

	   startPos = (startPos + A_offset + lineLength - 10) & mask;
	   amount -= A_offset + lineLength - 10;
	   synced ++;
	   if (synced >= 3)
	      m_form.setSynced (true);
	   return;
	}
//
//	if here, we are apparently "in sync"
	A_offset		= findStart (amplBuffer. data (),
	                                               startPos, 20);
	B_offset	 	= findSync ('A',  amplBuffer. data (),
	                                               startPos, 20);
	if (B_offset == -1) {
	   failures ++;
	   if (failures > 3) {
	      synced -= 1;
	      amount -= searchWidth;
	      startPos = (startPos + searchWidth) & mask;
	      failures = 0;
		  m_form.setSynced(false);
	      return;
	   }
	   B_offset = 10;
	}
	
	m_form.status(std::to_string(A_offset) + " " + std::to_string(B_offset));
	amount		-= B_offset;
	startPos	= (startPos + B_offset) & mask;
	int inc	= readLine (amplBuffer. data (), startPos, lineno ++);
	amount		-= inc - 10;
	startPos	= (startPos + inc - 10) % mask;
}

//
//	Looking at buffer [pos] and on, we check whether or not
//	a pattern is encoded there.
//	A pattern is SYNC_LENGTH * oversampling samples
static int table_A [] = 
 {-14, -14, -14, 18, 18,
       -14, -14, 18, 18,
       -14, -14, 18, 18,
       -14, -14, 18, 18,
       -14, -14, 18, 18,
       -14, -14, 18, 18,
       -14, -14, 18, 18, -14, -14, -14 };

int	SDRunoPlugin_apt::findStart (int16_t *buffer,
	                                   int pos, int searchLength) {
int  res	= -10;
int	maxRes	= 0;
int	maxIndex = -1;

	for (int k = 0; k < searchLength; k ++) {
	   int res = 0;
	   for (int i = 0; i < sizeof (table_A) / sizeof (int); i ++) {
	      int val = 0;
	      for (int j = 0; j < OVER_SAMPLING; j ++)
	         val += buffer [(pos + k + i * OVER_SAMPLING + j) & mask];
	      res += table_A [i] * val;
	   }
	   if (res > maxRes) {
	      maxRes = res;
	      maxIndex = k;
	   }
	
	}
	return maxIndex;
}
//
int32_t SDRunoPlugin_apt::findSync (char syncer, int16_t *buffer, int pos,
	                                     int searchLength) {

int	pulseLength	= syncer == 'A' ? 4 : 5;
int	maxOffset	= -1;
//	A: 0110 7 times
//	B: 01110 7 times
	pulseLength *= OVER_SAMPLING;

float	bestQ		= 0;
float s1 = 0, s2 = 0, s3 = 0, s4 = 0;

	for (int i = 0; i < searchLength * OVER_SAMPLING; i ++) {
	  bool flag		= true;;
//	walk over the 7 pulses
	   for (int j = 0; j < 7; j ++) {
	      int k;
	      int l;
	      int p = (pos + 2 * OVER_SAMPLING + i + j * pulseLength) & mask;
	      s1 = 0; s2 = 0; s3 = 0; s4 = 0;
	      for (k = 0, l = k + OVER_SAMPLING; k < l; k ++)
	         s1  += buffer [(p + k) & mask];	// should be low
	      for (l = k + OVER_SAMPLING; k < l; k ++)
	         s2  += buffer [(p + k) & mask];	// should be high
	      if (syncer == 'B')
	         k += OVER_SAMPLING;			// skip for 'B'
	      for (l = k + OVER_SAMPLING; k < l; k ++)
	         s3  += buffer [(p + k) & mask];	// should be high
	      for (l = k + OVER_SAMPLING; k < l; k ++)
	         s4  += buffer [(p + k) & mask];	// should be low
 	      if ((s1 > s2) || (s3 <s4)) {
	         flag = false;
	         break;
	      }
	   }
	   if (flag) {
	      if ((s2 + s3) - (s1 + s4) > bestQ) {
	         bestQ = (s2 + s3) - (s1 + s4);
	         maxOffset = i;
	      }
	   }
	}
	return maxOffset;
}

//	we know for sure that there are more samples available
//	in the buffer than needed for a single line
int	SDRunoPlugin_apt::readLine (int16_t *buffer, int pos, int lineno) {
float	max	= 0;
float 	min	= 1000;
float	avg	= 0;
std::vector<float> currentLine (pictureWidth);

	if (lineno >= m_form.maxLine()) {
	   decoding.store(true);
	   return lineLength;
	}

	for (int i = 0; i < pictureWidth; i ++) {
	   int pix = 0;
	   for (int j = 0; j < OVER_SAMPLING; j ++)
	      pix += buffer [(pos + OVER_SAMPLING * i + j) & mask];
	   pix /= OVER_SAMPLING;
	   if (pix > max)
	      max = pix;
	   if (pix < min)
	      min = pix;
	   avg	+= pix;
	}
	avg /= (lineLength / OVER_SAMPLING);

	for (int i = 0; i < lineLength / OVER_SAMPLING; i ++) {
	   float pix = 0;
	   for (int j = 0; j < OVER_SAMPLING; j ++)
	      pix += buffer [(pos + OVER_SAMPLING * i + j) & mask];
	   pix	/= OVER_SAMPLING;
	   channelBuffer [lineno][i] = transl (pix, min, max, 0.0f, 255.0f);
	   currentLine [i] = channelBuffer [lineno][i];
	}
	m_form. showLineNumber (lineno);
	m_form. drawLine (currentLine, lineno);

	wedgeBuffer [lineno] = 0;
	for (int i = START_WEDGE; i < START_WEDGE + WEDGE_SIZE; i ++) {
	   wedgeBuffer [lineno] += channelBuffer [lineno][i];
	   wedgeBuffer [lineno] += channelBuffer [lineno] [i + APT_RATE / 4];
	}
	wedgeBuffer[lineno] /= 2 * WEDGE_SIZE;
	
	if (wedgeBuffer[lineno] < 0) {
	   m_form.status ("dat mag niet");
	   wedgeBuffer [lineno] = 1;
	}

	if ((lineno > 0) && (lineno % 20 == 0)) {
	   m_form. updateImage ();
	   m_form. drawWedge (wedgeBuffer, lineno);
	}
	return lineLength;
}

void	SDRunoPlugin_apt::apt_start	() {
	decoding. store (true);
}

void	SDRunoPlugin_apt::apt_stop	() {
	decoding. store (false);
}

void	SDRunoPlugin_apt::apt_reset		() {
	apt_stop ();
	m_form. clearScreen ();
	amount          = 0;
	npos            = 0;
	synced          = 0;
	startPos        = 0;
	failures        = 0;
	lineno          = 0;
	apt_start ();
}

void	SDRunoPlugin_apt::apt_reverseImage	() {
std::vector<float> currentLine;

	if (lineno == 0)
	   return;
	if (decoding. load ())
	   return;
	if (lineno > m_form.maxLine())
	   lineno = m_form. maxLine();

	m_form. clearScreen ();
	if (lineno & 01)
	   lineno --;
	for (int i = 0; i < lineno / 2; i ++) {
		m_form. show_dumpName(std::to_string (lineno));
	   exchange (i, lineno - 1 - i);
	}
	apt_printImage ();
}

void	SDRunoPlugin_apt::exchange	(int ind_1, int ind_2) {
std::vector<float> currentLine_1 (pictureWidth);
std::vector<float> currentLine_2 (pictureWidth);
	for (int i = 0; i < pictureWidth; i ++) {
	   currentLine_1 [i] = channelBuffer [ind_1][i];
	   currentLine_2 [i] = channelBuffer [ind_2][i];
	}
//
//	we have to mirror the A and B pictures
	for (int i = 0; i < 86; i ++)
	   channelBuffer [ind_2][i] = currentLine_1 [i];
	for (int i = 0; i < 909; i ++)
	   channelBuffer [ind_2][86 + 909 - 1 - i] =
	                                      currentLine_1 [86 + i];
	for (int i = 0; i < 45 + 86; i ++)
	   channelBuffer [ind_2][86 + 909 + i] =
	                                      currentLine_1 [86 + 909 + i];
	for (int i = 0; i < 909; i ++)
	   channelBuffer [ind_2][1040 + 86 + 909 - 1 - i] =
	                                      currentLine_1 [1040 + 86 + i];
	for (int i = 0; i < 45; i ++)
	   channelBuffer [ind_2][2080 - 1 - 45 + i] =
	                                      currentLine_1 [2080 - 46 + i];

	for (int i = 0; i < 86; i ++)
	   channelBuffer [ind_1][i]	= currentLine_2 [i];
	for (int i = 0; i < 909; i ++)
	   channelBuffer [ind_1][86 + 909 - 1 - i] =
	                                      currentLine_2 [86 + i];
	for (int i = 0; i < 45 + 86; i ++)
	   channelBuffer [ind_1][86 + 909 + i] =
	                                      currentLine_2 [86 + 909 + i];
	for (int i = 0; i < 909; i ++)
	   channelBuffer [ind_1][1040 + 86 + 909 - 1 - i] =
	                                      currentLine_2 [1040 + 86 + i];
	for (int i = 0; i < 45; i ++)
	   channelBuffer [ind_1][2080 - 1 - 45 + i] =
	                                      currentLine_2 [2080 - 46 + i];
}
//
void	SDRunoPlugin_apt::apt_printImage	() {
std::vector<float> currentLine (pictureWidth);

	if (lineno == 0)
	   return;

	for (int line = 0; line < lineno; line ++) {
	   for (int i = 0; i < pictureWidth; i ++)
	      currentLine [i] = channelBuffer [line] [i];
	   m_form. drawLine (currentLine, line);
	}
	m_form. updateImage ();
}

void	SDRunoPlugin_apt::apt_setChannel  (const std::string &f, int freq) {
	m_controller	-> SetCenterFrequency (0, (float)freq);
	m_controller	-> SetVfoFrequency (0, (float)freq);
}

void	SDRunoPlugin_apt::apt_drawSpectrum	() {
std::vector<float>  spectrum (1024);

	Fft_transform (spectrumBuffer, 2048, false);
	for (int i = 0; i < 1024; i ++)
	   spectrum [i] = abs (spectrumBuffer [i]);
	m_form. drawSpectrum (spectrum);
}

void	SDRunoPlugin_apt::apt_savePicture () {
	if (decoding. load ())
	   return;

	char	*home	= getenv ("HOMEPATH");
	nana::filebox fb (0, false);
	fb. add_filter ("bitmap file", "*.bmp");
	fb. add_filter ("All Files", "*.*");
	fb. init_path (home);
	auto files = fb();

	if (files. empty ())
	   return;
	int greyDifference	= m_form. get_greySetting ();
	std::string fileName = files. front (). string ();
        m_form. show_dumpName (fileName);
	nana::paint::graphics graph (nana::size (pictureWidth, lineno));
        for (int row = 0; row < lineno; row++) {
           for (int column = 0; column < pictureWidth; column++) {
	      int c = channelBuffer [row][column];
              if (greyDifference >= 0)     // make it lighter
                 c = transl ((float)c, 0.0f, 255.0f, greyDifference, 255);
              else
                 c = transl ((float)c, 0.0f, 255.0f, 0, 255 + greyDifference);
	      graph. set_pixel (column, row, nana::color (c, c, c));
	   }
        }
        graph. save_as_file (fileName. c_str ()); 
}

void	SDRunoPlugin_apt::apt_saveFile	() {
SF_INFO sf_info;

	if (decoding. load ())
	   return;

	if (dumping. load ()) {
	   sf_close (dumpFile_11025);
	   m_form. dumpfileText ("dump");
	   dumpFile_11025 = nullptr;
	   return;
	}
//
//	get a filename
	char	*home	= getenv ("HOMEPATH");
	nana::filebox fb (0, false);
	fb. add_filter ("wav file", "*.wav");
	fb. add_filter ("All Files", "*.*");
	fb. init_path (home);
	auto files	= fb();
	if (files. empty ())
	   return;

	std::string fileName	= files.front (). string ();
	sf_info. samplerate     = 11025;
        sf_info. channels       = 1;
        sf_info. format         = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

        dumpFile_11025		= sf_open (fileName. c_str (),
                                           SFM_WRITE, &sf_info);
        if (dumpFile_11025 != nullptr)  {
	   m_form. dumpfileText ("write");
	}
}


