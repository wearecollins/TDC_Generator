#include "testApp.h"

bool bClear = true;
int mode = 0;

//--------------------------------------------------------------
void testApp::setup(){
    particles.setup("TDC_Type_Sketching.svg");
    toSave.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 6);
    
    toSave.begin();
    ofClear(0,0,0,0);
    toSave.end();
    
//    ofEnableBlendMode(OF_BLENDMODE_ADD);
    drawMode = 0;
    ofDisableDepthTest();
    glPointSize(2.0f);
}

//--------------------------------------------------------------
void testApp::update(){
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
    toSave.end();
    
    ofSetColor(255);
    toSave.draw(0, 0);
    
    ofDrawBitmapString("Press ' ' to save image\nPress 'c' to toggle background clearing\nPress '+' to change draw modes\nPress 'm' to change blend modes", 20, 40 );
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
        //type.resetColor();
    } else if ( key == 'm' ){
        mode++;
        if ( mode > OF_BLENDMODE_ADD ){
            mode = 0;
        }
    } else if ( key == '=' && ofGetKeyPressed(OF_KEY_SHIFT)){
        drawMode++;
        if ( drawMode > TypeParticleSystem::DRAW_SHAPES ){
            drawMode = 0;
        }
        particles.setDrawMode( (TypeParticleSystem::DrawMode) drawMode);
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
