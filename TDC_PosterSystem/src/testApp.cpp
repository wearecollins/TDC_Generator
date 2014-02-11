#include "testApp.h"

bool bClear = true;
bool bCapture = false;
bool bSave    = false;
bool bUseGrid = true;
bool bDrawTypeAsOverlay = true;

int mode = 0;
int lastDrawMode = -1;

//--------------------------------------------------------------
void testApp::setup(){
    particles.setup("TDC_Type_Sketching.svg");
    toSave.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 6);
    
    toSave.begin();
    ofClear(0,0,0,0);
    toSave.end();
    
    // load optional type overlay
    
    type.load("TDC_Type_Sketching.svg");
    lastDrawMode = 0;
    drawMode = TypeParticleSystem::DRAW_POINTS;
    ofDisableDepthTest();
    glPointSize(2.0f);
    
    // GUI
    gui = new ofxUISuperCanvas("---- SETTINGS ----", 10, 10, 200, ofGetHeight());
    gui->setVisible(false);
    gui->addSpacer();
    gui->addSpacer();
    gui->addLabel("DRAWING");
    gui->addLabel("Drawmode Label", "Draw Points");
    gui->addIntSlider("Draw Mode", 0, TypeParticleSystem::DRAW_SHAPES + 1, &drawMode);
    gui->addSpacer();
    gui->addToggle("Use grid or outline", &bUseGrid);
    gui->addSpacer();
    gui->addIntSlider("Blend Mode", 0, OF_BLENDMODE_SCREEN, &mode);
    gui->addToggle("Auto Clear Background", &bClear);
    gui->addToggle("Draw type overlay", &bDrawTypeAsOverlay);
    gui->addSpacer();
    
    gui->addLabel("MOVEMENT");
    gui->addSpacer();
    gui->addLabel("EVENTS");
    gui->addToggle("Save Frame", &bCapture);
    gui->addToggle("Save Settings", &bSave);
    gui->loadSettings("settings.xml");
}

//--------------------------------------------------------------
void testApp::update(){
    if ( lastDrawMode != drawMode ){
        lastDrawMode = drawMode;
        particles.setDrawMode( (TypeParticleSystem::DrawMode) drawMode);
        ((ofxUILabel*)gui->getWidget("Drawmode Label"))->setLabel(particles.getDrawModeString());
    }
    if ( bCapture ){
        bCapture = false;
        
        toSave.readToPixels(pix);
        ofImage save; save.setFromPixels(pix);
        save.saveImage("cap_"+ofToString(ofGetFrameNum())+"_" + ofGetTimestampString() + ".png");
    }
    if ( bSave ){
        gui->saveSettings("settings.xml");
        bSave = false;
    }
    
    particles.setUseGrid(bUseGrid);
    particles.update();
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0);
    toSave.begin();
    if ( bClear ) ofClear(0,0,0,0);
    else {
        ofSetColor(0,0,0,5);
        ofRect(0,0, toSave.getWidth(), toSave.getHeight());
        ofSetColor(255,255);
    }
    ofEnableAlphaBlending();
    ofEnableBlendMode((ofBlendMode) mode);
    particles.draw();
    if (bDrawTypeAsOverlay){
        type.draw();
    }
    toSave.end();
    
    ofSetColor(255);
    toSave.draw(0, 0);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if ( key == 'g' ){
        gui->toggleVisible();
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    //type.mouseMoved(x,y);
    particles.mouseMoved(x,y);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
