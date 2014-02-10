#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    flipSensor.setup();
    
    // ugh
    ofxXmlSettings settings; settings.load("settings.xml");
    settings.pushTag("Widget", 2);
    float max = settings.getValue("Value", 10000);
    settings.popTag();
    
    gui = new ofxUICanvas("Settings");
    gui->setDimensions(ofGetWidth()/2.0f, ofGetHeight());
    gui->addLabel("NEAR THRESHOLD");
    gui->addSlider("near", 0., max, 0.);
    
    gui->addSpacer();
    gui->addLabel("FAR THRESHOLD");
    gui->addSlider("far", 0., max, 0.);
    
    gui->addSpacer();
    gui->addLabel("THRESHOLD MAX");
    gui->addSlider("max", 0., 10000.0, 0.);
    gui->setVisible(false);
    
    ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);
    
    gui->loadSettings("settings.xml");
    
    ofBackground(0);
}

//--------------------------------------------------------------
void testApp::update(){
    flipSensor.update();
}

//--------------------------------------------------------------
void testApp::draw(){
    flipSensor.draw();
}
//--------------------------------------------------------------
void testApp::guiEvent(ofxUIEventArgs &e){
    if ( e.getName() == "near"){
        flipSensor.setNear(e.getSlider()->getValue());
    } else if ( e.getName() == "far"){
        flipSensor.setFar(e.getSlider()->getValue());
    } else if ( e.getName() == "max"){
        vector<ofxUIWidget*> widgets = gui->getWidgetsOfType(OFX_UI_WIDGET_SLIDER_H);
        for (int i=0; i<widgets.size(); i++){
            if ( widgets[i]->getName() != "max" && widgets[i]->getName() != "FPS" ){
                ((ofxUISlider*)widgets[i])->setMax(e.getSlider()->getValue());
            }
        }
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if ( key == 'g'){
        gui->toggleVisible();
    } else if ( key == 's' ){
        gui->saveSettings("settings.xml");
        cout << "sav'd" << endl;
    } else if ( key == 't' ){
        flipSensor.startDetecting();
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    flipSensor.mouse(x,y);
}

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
void testApp::dragEvent(ofDragInfo dragInfo){}
