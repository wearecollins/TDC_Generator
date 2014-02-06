#include "testApp.h"

bool bClear = true;
int mode = 0;

//--------------------------------------------------------------
void testApp::setup(){
    type.load("TDC_Type_Sketching.svg");
    toSave.allocate(ofGetWidth(), ofGetHeight());
    
    toSave.begin();
    ofClear(0,0,0,0);
    toSave.end();
    
//    ofEnableBlendMode(OF_BLENDMODE_ADD);
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0);
    toSave.begin();
    if ( bClear ) ofClear(0,0,0,0);
    ofEnableAlphaBlending();
    ofEnableBlendMode((ofBlendMode) mode);
    type.draw();
    toSave.end();
    
    ofSetColor(255);
    toSave.draw(0, 0);
    
    ofDrawBitmapString("Press ' ' to save image\nPress 'c' to toggle background clearing\nPress 'r' to reset colors\nPress 'm' to change blend modes", 20, 40 );
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if ( key == ' '){
        toSave.readToPixels(pix);
        ofImage save; save.setFromPixels(pix);
        save.saveImage("cap_"+ofToString(ofGetFrameNum())+"_" + ofGetTimestampString() + ".png");
    } else if ( key == 'c' ){
        bClear = !bClear;
    } else if ( key == 'r' ){
        type.resetColor();
    } else if ( key == 'm' ){
        mode++;
        if ( mode > 5 ){
            mode = 0;
        }
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    type.mouseMoved(x,y);
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
