#foraging sensor
Code to drive arduino-based sensors that send data to the foraging map.

##how to use
In order to work on a home network, the arduino sketch needs to be able to access your home network. In the sketch, there is a section marked "THINGS THAT NEED CUSTOMIZATION." These include network name , network password, and network security type (lines 34, 35, and 36, respectively). Simply change these lines to match your network's settings. The network security type options are included inside the sketch.