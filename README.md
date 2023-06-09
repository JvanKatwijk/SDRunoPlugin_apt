
    *EXPERIMENTAL* plugin for decoding apt137 pictures

------------------------------------------------------------------------
------------------------------------------------------------------------

As known, NOAA - polar - satelites continuously transmit photographs of the
area they are flying over in a simple format.
The experimental apt plugin aims at decoding such signals.

Of course, the time the satelite is above horizon is limited, 
and with my experimental antenna, based on what I found in
"http://rfelektronik.se/manuals/Datasheets/DIY%20137MHz%20WX-sat%20V-dipole%20antenna.pdf" it is not easy to get sharp pictures.

To help in testing, I developed - based on the encoder developed
by "Gokberk Yaltirakli" and available under a GNU GPL, and using
the Adalm Pluto - a simple "transmitter" for fake apt 137 pcitures.

Decoding with imput from this "transmitter" seems to work fine.

I'll need to pay more attention to the antenna problem, in order to
be able to receive more - and above all - better pictures from the sky.

The data for photographs is transmitted as analog data,
however, since each line contains black and white data, it
is possible to assign grey values to values received.

The data is first modulated as AM data on a 2400 Hz carrier, the
resulting signal is FM modulated and transmitted.

Most decoders I found are able to handle the signal after FM decoding
took place, that is why the plugin has an option to record datra,
after FM decoding.

-----------------------------------------------------------------------
The plugin
----------------------------------------------------------------------

![overview](/SDRunoPlugin_apt.png?raw=true)

As seen on the picture, the plugin is fairly straightforward, it contains
a single topline with buttons and indicators, and a white field where
the picture is shown.


The controls are

 * the *satelite* selector, there are three satelites,
transmitting on different frequencies, of course you are
free to select another frequency;

 * the *start* button, with the obvious function that after
touching the software will try to decode the incoming signals.
This allows you to wait until a clear signal is shown on the spectrum
display of SDRuno, and then start decoding;

 * the stop button, with the obviosu function,

 * the save button. If decoding is *off*, the save button - when touched - will show a file selection menu and after selecting a file will store the picture
- in the real format - as a bitmap into a file;

 * the *reset* button, with a fairly obvious function;

 * the *reverse* button. Since the satelite may come over from north to south
but also from south to north, the picture can be reversed. Again, this
function will work when deceding is off.

 * the *print* button. Touching this button - with decoding off - will
reprint the picture on the screen. Meant to be used with the  *grey correcting*
slider.

* the *sync* label, green whenever the software believes that a sync 
is reached, red otherwise. If synced, two number are displayed. telling
the number of samples where - according to two methods for syncing -
the current line in the picture starts.

 * the *linenumber*, obviously telling the line number currently
being the botton line of the picture.

 * the *grey slider*, the setting of which influences the mapping from number
to grey values;

 * to the right a small button labeled *dump* is visible, touching this
button will dump  the input, *AFTER THE FM DECODING HAS TAKEN PLACE*, as
a ".wav" file, single channel, with a rate of 11025. That rate - and
single channel - was chosen to be able to use such a file in a separate
decoder.


The picture is limited to app 540 lines.

---------------------------------------------------------------------------
----------------------------------------------------------------------------

Note that the plugin requires the availability of the *libsndfile-1.sll*,
i.e. the dll that takes care of generating the ".wav" file

------------------------------------------------------------------------------
Important note
----------------------------------------------------------------------------

The current experimental version has an issue in the unloading of the plugin.
Usually, using the *stop* button, wait a second or two, and only then tell
SDRuno to unload the plugin is safe


