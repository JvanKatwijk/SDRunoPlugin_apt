
    Experimental plugin for decoding apt137 pictures

------------------------------------------------------------------------
This plugin is "under development" and needs more work
------------------------------------------------------------------------

As known, NOAA - polar - satelites continuously transmit photographs of the
area trey are flying over.
The experimental apt plugin aims at decoding such signals.

-----------------------------------------------------------------------
The plugin
----------------------------------------------------------------------

The plugin itself is simple, there is a top line with controls,
a larger white area where the received data is shown if possible,
and a small bottom area where some parameters of the signal are made visible

The controls are

 * the satelite selector, there are three satelites, transmitting on difefrent frequencies, 
 * the start button, with the obvious function that after touching the software will try to decode the incoming signals,

 * the stop button, with the obviosu function,

 * the save button. If the decoding is off, the save button - when touched - will show a file selection menu and after selecting a file will store the picture
- in the real format - as a bitmap into a file;

 * the reset button.

 * the reverse button. Since the satelite may come over from north to south
but also from south to north, the picture can be reversed. Again, this
function will work when deceding is off.

 * the print button. Touching this button - with decoding off - will
reprint the picture on the scren. Meant to be used with the grey correcting
slider.

 * the grey slider, the setting of which influences the mapping from number
to grey values;

Furthermore, there is a sync label, colouring green if the software is able
to synchronize with the transmitted lines of the picture, with to its
right side a small text. If synced, the text gives two numbers, usually
around 10, if not synced it will tell so.

