#include "testApp.h"

int serialRead = 0;
bool bSerial = true;

//--------------------------------------------------------------
void testApp::setup(){
    ofSetFrameRate(60);
    spacebrew.setAutoReconnect();
    // print!
    spacebrew.addPublish("print", "boolean");
    spacebrew.connect("localhost", "button", "a button you dummy");
    
    ofSerial s;
    s.listDevices();
    arduino.connect("/dev/tty.usbmodem1411", 57600);
	
	// listen for EInitialized notification. this indicates that
	// the arduino is ready to receive commands and it is safe to
	// call setupArduino()
	ofAddListener(arduino.EInitialized, this, &testApp::setupArduino);
	bSetupArduino	= false;	// flag so we setup arduino when its ready, you don't need to touch this :)
}

//--------------------------------------------------------------
void testApp::update(){
    
	updateArduino();
    
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(serialRead * 255);
    
    ofDrawBitmapString(ofToString(serialRead), 20,20);
}

//--------------------------------------------------------------
void testApp::onMessage(Spacebrew::Message & m ){
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if ( key == 's' ){
        bSerial = !bSerial;
    } else if ( key == 'p' ){
        spacebrew.sendBoolean("print", true);
        serialRead = !serialRead;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ }

//--------------------------------------------------------------
void testApp::setupArduino(const int & version) {
	
	// remove listener because we don't need it anymore
	ofRemoveListener(arduino.EInitialized, this, &testApp::setupArduino);
    
    // it is now safe to send commands to the Arduino
    bSetupArduino = true;
    
    // print firmware name and version to the console
    ofLogNotice() << arduino.getFirmwareName();
    ofLogNotice() << "firmata v" << arduino.getMajorFirmwareVersion() << "." << arduino.getMinorFirmwareVersion();
    
    // Note: pins A0 - A5 can be used as digital input and output.
    // Refer to them as pins 14 - 19 if using StandardFirmata from Arduino 1.0.
    // If using Arduino 0022 or older, then use 16 - 21.
    // Firmata pin numbering changed in version 2.3 (which is included in Arduino 1.0)
    
    // set pins D2 and A5 to digital input
    arduino.sendDigitalPinMode(2, ARD_INPUT);
    arduino.sendDigitalPinMode(19, ARD_INPUT);  // pin 21 if using StandardFirmata from Arduino 0022 or older
        // Listen for changes on the digital and analog pins
    ofAddListener(arduino.EDigitalPinChanged, this, &testApp::digitalPinChanged);
}

//--------------------------------------------------------------
void testApp::updateArduino(){
    
	// update the arduino, get any data or messages.
    // the call to ard.update() is required
	arduino.update();
    
}

// digital pin event handler, called whenever a digital pin value has changed
// note: if an analog pin has been set as a digital pin, it will be handled
// by the digitalPinChanged function rather than the analogPinChanged function.

//--------------------------------------------------------------
void testApp::digitalPinChanged(const int & pinNum) {
    // do something with the digital input. here we're simply going to print the pin number and
    // value to the screen each time it changes
    buttonState = "digital pin: " + ofToString(pinNum) + " = " + ofToString(arduino.getDigital(pinNum));
    if ( !bSerial ) return;
    serialRead = arduino.getDigital(pinNum);
    if ( serialRead == 1 && pinNum == 2){
        if ( bSerial ) spacebrew.sendBoolean("print", true);
    }
}
