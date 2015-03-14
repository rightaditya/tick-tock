# Circuit #

We were originally going to use an op-amp, but because we only need three states (nothing, empty bed, occupied bed) we can use a more crude system.  We will use the weight sensors - which are simply variable resistors - as voltage dividers, similar to the circuit shown in figure 32 [here](http://soundlab.cs.princeton.edu/learning/tutorials/sensors/node17.html#opamp_RtoV) but without the op-amp.  I tested some values using one of the chairs in the lab and a resistor value of 500kOhms seems best - it gives a few mV for no load, 1-2V for just the chair, and about 4V for the chair with me in it.  These ranges are quite clearly separable.

# Code #

I've made a C function that returns whether or not the alarm should be active, given lower and upper thresholds (which are determined by calibration).  The code is currently in the wttest project.