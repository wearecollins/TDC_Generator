#include "ofMain.h"
#include "testApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( int argc, char * const argv[] ){
    int width = 800;
    int height = 600;
    
    vector<string> args(argv + 1, argv + argc);
    // from: http://stackoverflow.com/questions/441547/most-efficient-way-to-process-arguments-from-the-command-line-in-c
    
    // Loop over command-line args
    for (vector<string>::iterator i = args.begin(); i != args.end(); ++i) {
        if (*i == "-w") {
            width = ofToInt(*++i);
        } else if (*i == "-h") {
            height = ofToInt(*++i);
        } else if (*i == "-d"){
            pixelDPI = ofToInt(*++i);
        }
    }
    
    ofAppGLFWWindow window;
    window.setNumSamples(8);
    window.setDepthBits(32);
    window.setDoubleBuffering(true);
	ofSetupOpenGL(&window, width,height,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new testApp());

}
