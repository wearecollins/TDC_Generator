#include "testApp.h"

bool bClear = true;
bool bCapture = false;
bool bSave    = false;
bool bReload    = false;
bool bUseGrid = true;
bool bDrawTypeAsOverlay = true;
bool bDrawTypeInSpace = true;

bool bUseLiveInput      = true;

// main layout vars
int mode = 0;
int lastDrawMode = -1;
float bgAlpha = 255.0f;
double scale = 1.0;

// particle settings
ofFloatColor particleColor, lastParticleColor;
float        hueVariance;

// positioning
double x = 0;
double y = 0;

// type color
//float typeColor = 0;

// poster
ofRectangle poster = ofRectangle(0,0,100,400);
ofFloatColor posterColor = ofColor(1.0,0,0);
ofFloatColor posterColorBottom = ofColor(1.0,0,0);

ofFloatColor typeColor;

//--------------------------------------------------------------
void testApp::setup(){
    particles.setup();
    toSave.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);//, 6);
    toSavePoster.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);//, 6);
    
    toSave.begin();
    ofClear(0,0,0,0);
    toSave.end();
    
    toSavePoster.begin();
    ofClear(0,0,0,0);
    toSavePoster.end();
    
    // load optional type overlay
    
    // TO DO FIX ME OH GOD!
    
    //type.load("Type4_003.svg");
    
    int n = type.getNumPath();
    
    for (int i=0; i<n; i++){
        type.getPathAt(i).setUseShapeColor(false);
    }
    
    lastDrawMode = 0;
    drawMode = DRAW_POINTS;
    ofDisableDepthTest();
    //glPointSize(.5f);
    
    // GUI
    gui = new ofxUITabBar(10, 10, ofGetWidth(), ofGetHeight());
    gui->setVisible(false);
    
    ofxUISuperCanvas * guiDrawing = new ofxUISuperCanvas("DRAWING",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiDrawing);
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    guis.back()->setName("DRAWING");
    guis.back()->addLabel("Drawmode Label", "Draw Points");
    guis.back()->addIntSlider("Draw Mode", 0, DRAW_SHAPES + 1, &drawMode);
    guis.back()->addSpacer();
    guis.back()->addToggle("Use grid or outline", &bUseGrid);
    guis.back()->addSpacer();
    guis.back()->addIntSlider("Blend Mode", 0, OF_BLENDMODE_SCREEN, &mode);
    guis.back()->addToggle("Auto Clear Background", &bClear);
    guis.back()->addSlider("Background Alpha Clear", 0.0, 255.0f, &bgAlpha);
    guis.back()->addSpacer();
    guis.back()->addSlider("Particle Color: R", 0.0, 1.0, &particleColor.r);
    guis.back()->addSlider("Particle Color: G", 0.0, 1.0, &particleColor.g);
    guis.back()->addSlider("Particle Color: B", 0.0, 1.0, &particleColor.b);
    guis.back()->addSlider("Particle Color: Hue Randomization", 0.0, 1.0, &hueVariance);
    gui->addCanvas(guis.back());
    
    ofxUISuperCanvas * guiTypeOverlay = new ofxUISuperCanvas("TYPE",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiTypeOverlay);
    guis.back()->setName("TYPE");
    guis.back()->addToggle("Draw type overlay", &bDrawTypeAsOverlay);
    guis.back()->addToggle("Draw type in space", &bDrawTypeInSpace);
    guis.back()->addDoubleSlider("scale", 0.0, 1.0, &particles.scale);
    guis.back()->addDoubleSlider("x", -1.0, 1.0, &x);
    guis.back()->addDoubleSlider("y", -1.0, 1.0, &y);
    guis.back()->addSlider("type color r", 0, 1.0, &typeColor.r);
    guis.back()->addSlider("type color g", 0, 1.0, &typeColor.g);
    guis.back()->addSlider("type color b", 0, 1.0, &typeColor.b);
    gui->addCanvas(guis.back());
    
    ofxUISuperCanvas * guiMovement = new ofxUISuperCanvas("MOVEMENT",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiMovement);
    guis.back()->setName("MOVEMENT");
    guis.back()->addIntSlider("Movement Type", 0, MOVE_PUSH, MOVE_NONE);
    // this should be separate panels for each behavior
    guis.back()->addSlider("intensityX", 0.0, 100.0, 10.0f);
    guis.back()->addSlider("intensityY", 0.0, 100.0, 50.0f);
    guis.back()->addSlider("intensityZ", 0.0, 100.0, 50.0f);
    guis.back()->addSlider("rate", 0.0, 10.0, 10.0);
    guis.back()->addSlider("mix", 0.0, 1.0, .5);
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiPoster = new ofxUISuperCanvas("POSTER",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiPoster);
    guis.back()->setName("POSTER");
    guis.back()->addSlider("Poster X", 0.0, ofGetWidth(), &poster.x);
    guis.back()->addSlider("Poster Y", 0.0, ofGetWidth(), &poster.y);
    guis.back()->addSlider("Poster Width", 0.0, ofGetWidth(), &poster.width);
    guis.back()->addSlider("Poster Height", 0.0, ofGetWidth(), &poster.height);
    guis.back()->addSlider("Poster Color: R", 0.0, 1.0, &posterColor.r);
    guis.back()->addSlider("Poster Color: G", 0.0, 1.0, &posterColor.g);
    guis.back()->addSlider("Poster Color: B", 0.0, 1.0, &posterColor.b);
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiEvents = new ofxUISuperCanvas("EVENTS",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiEvents);
    guis.back()->setName("EVENTS");
    guis.back()->addToggle("Save Frame", &bCapture);
    guis.back()->addToggle("Save Settings", &bSave);
    guis.back()->addToggle("Reload Settings", &bReload);
    guis.back()->addToggle("Use Inputs", &bUseLiveInput);
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    gui->setTriggerWidgetsUponLoad(true);
    gui->loadSettings("settings/", "ui-");
    
    // SPACEBREW
    setupSpacebrew();
    
    // build poster mesh
    posterMesh.load("poster");
    if ( posterMesh.getNumVertices() == 0){
        posterMesh.addVertex(ofVec3f(0,0));
        posterMesh.addVertex(ofVec3f(ofGetWidth(),0));
        posterMesh.addVertex(ofVec3f(ofGetWidth(),ofGetHeight()));
        posterMesh.addVertex(ofVec3f(0,ofGetHeight()));
        
        posterMesh.addColor(ofFloatColor(1.0,1.0,1.0));
        posterMesh.addColor(ofFloatColor(1.0,1.0,1.0));
        posterMesh.addColor(ofFloatColor(1.0,1.0,1.0));
        posterMesh.addColor(ofFloatColor(1.0,1.0,1.0));
        
        posterMesh.addIndex(0); posterMesh.addIndex(1); posterMesh.addIndex(3);
        posterMesh.addIndex(1); posterMesh.addIndex(2); posterMesh.addIndex(3);
        posterMesh.save("poster");
    }
}

//--------------------------------------------------------------
void testApp::update(){
    if ( lastDrawMode != drawMode || particles.getDrawMode() != ((DrawMode) drawMode) ){
        lastDrawMode = drawMode;
        particles.setDrawMode( (DrawMode) drawMode);
        //((ofxUILabel*)gui->getActiveCanvas()->getWidget("Drawmode Label"))->setLabel(particles.getDrawModeString());
    }
    if ( lastParticleColor != particleColor ){ 
        particles.setColor(particleColor, hueVariance / 10.0f);
        lastParticleColor = particleColor;
    }
    if ( bCapture ){
        bCapture = false;
        
        toSavePoster.readToPixels(pix);
        ofImage save; save.setFromPixels(pix);
        save.saveImage("cap_"+ofToString(ofGetFrameNum())+"_" + ofGetTimestampString() + ".png");
    }
    if ( bSave ){
        gui->saveSettings("settings/", "ui-");
        bSave = false;
    }
    if ( bReload ){
        
        gui->loadSettings("settings/", "ui-");
        bReload = false;
    }
    
    // DATA OBJECT: TIME/DATE
    
    float timeMappedSunset = particles.dataObject.time > .6 ? ofMap(particles.dataObject.time, .6, 1.0, 1.0, 0) : ofMap(particles.dataObject.time, 0.0, .6, 0, 1.0);
    float timeMappedMidday = particles.dataObject.time > .5 ? ofMap(particles.dataObject.time, .5, 1.0, 1.0, 0.0) : ofMap(particles.dataObject.time, 0,1.0, 0, 1.0);
    float yesterday = particles.dataObject.date - 1.0/365.0f;
    //osterColor.setHue( yesterday * 1.0 * (1.0-particles.dataObject.time) + particles.dataObject.date * 1.0 * particles.dataObject.time);
    
    if ( bUseLiveInput ){
        posterColorBottom.setHue( yesterday * (1.0-particles.dataObject.time) + particles.dataObject.date * 1.0 * particles.dataObject.time );
        posterColorBottom.setSaturation( timeMappedSunset );
        posterColorBottom.setBrightness( timeMappedMidday );

        posterColor.setHue( particles.dataObject.date  );
        posterColor.setSaturation( timeMappedSunset );
        posterColor.setBrightness( timeMappedMidday );
    }
    
    posterMesh.setVertex(0, ofVec2f(poster.x, poster.y));
    posterMesh.setVertex(1, ofVec2f(poster.x + poster.width, poster.y));
    posterMesh.setVertex(2, ofVec2f(poster.x + poster.width, poster.y + poster.height));
    posterMesh.setVertex(3, ofVec2f(poster.x, poster.y + poster.height));
    
    posterMesh.setColor(0, posterColor );
    posterMesh.setColor(1, posterColor );
    posterMesh.setColor(2, posterColor );
    posterMesh.setColor(3, posterColor );
    
    ofLogVerbose() << yesterday << ":" << particles.dataObject.date << ":" << timeMappedSunset << ":" << timeMappedMidday <<":"<<particles.dataObject.time<<endl;
    
    // DATA OBJECT: ENVIRONMENT
    if ( bUseLiveInput ){
        Behavior * b = particles.getSettingsBehavior();
        b->intensity.x = particles.dataObject.environmentImmediate * 100.0;
        b->intensity.y = particles.dataObject.environmentLocal * 100.0;
        b->intensity.z = particles.dataObject.environmentGlobal * 100.0;
        
        // UPDATE GUI BASED ON DATA OBJECT
        ((ofxUISlider *)guis[2]->getWidget("intensityX"))->setValue(particles.dataObject.environmentImmediate * 100.0);
        ((ofxUISlider *)guis[2]->getWidget("intensityY"))->setValue(particles.dataObject.environmentLocal * 100.0);
        ((ofxUISlider *)guis[2]->getWidget("intensityZ"))->setValue(particles.dataObject.environmentGlobal * 100.0);
    }

    particles.setUseGrid(bUseGrid);
    particles.update();
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
    
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0);
    toSave.begin();
    ofPushMatrix();
    
//    static float scale = fmax( ofGetWidth()/1280.0f, ofGetHeight()/720.0f );
//    ofScale(scale, scale);
    
    if ( bClear ) ofClear(0,0,0,0);
    else {
        ofSetColor(0,0,0,bgAlpha);
        ofRect(0,0, toSave.getWidth(), toSave.getHeight());
        ofSetColor(255,255);
    }
    ofEnableAlphaBlending();
    ofEnableBlendMode((ofBlendMode) mode);
    ofTranslate( ofGetWidth() / 2.0 - (ofGetWidth() / 2.0 * (1- x)), - ofGetHeight() / 2.0 + (ofGetHeight() / 2.0 * (1-y)));
    if (bDrawTypeInSpace){
        ofEnableDepthTest();
        ofPushMatrix();
        ofSetColor(typeColor);
        ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
        ofScale(particles.scale, particles.scale);
        ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
        type.draw();
        ofPopMatrix();
    }
    particles.draw();
    ofDisableDepthTest();
    if (bDrawTypeAsOverlay){
        ofSetColor(typeColor);
        ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
        ofScale(particles.scale, particles.scale);
        ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
        type.draw();
    }
    ofPopMatrix();
    toSave.end();
    
    toSavePoster.begin(); {
        ofClear(0,0,0,255);
        ofPushMatrix(); {
            //ofTranslate( ofGetWidth() / 2.0 - (ofGetWidth() / 2.0 * (1- x)), - ofGetHeight() / 2.0 + (ofGetHeight() / 2.0 * (1-y)));
            posterMesh.draw();
        } ofPopMatrix();
    
        ofSetColor(255);
        toSave.draw(0, 0);
        
    } toSavePoster.end();
    
    toSavePoster.draw(0,0);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if ( key == 'g' ){
        gui->toggleVisible();
        if ( gui->isVisible() ){
            ofShowCursor();
        } else {
            ofHideCursor();
            for (int i=0; i<guis.size(); i++){
                guis[i]->setVisible(false);
            }
        }
    } else if ( key == 'f' ){
        ofToggleFullscreen();
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

void testApp::onGui( ofxUIEventArgs & e ){
    Behavior * b = particles.getSettingsBehavior();
    if ( e.getName() == "intensityX" ){
        if ( b != NULL ){
            b->intensity.x = e.getSlider()->getValue();
        }
    } else if ( e.getName() == "intensityY" ){
        if ( b != NULL ){
            b->intensity.y = e.getSlider()->getValue();
        }
    } else if ( e.getName() == "intensityZ" ){
        if ( b != NULL ){
            b->intensity.z = e.getSlider()->getValue();
        }
    } else if ( e.getName() == "rate" ){
        if ( b != NULL ){
            b->timeFactor = e.getSlider()->getValue() / 1000.0;
        }
    } else if ( e.getName() == "Movement Type" ){
        ofxUIIntSlider * s = (ofxUIIntSlider *) e.getSlider();
        particles.setBehavior( (MovementType) s->getValue() );
    } else if ( e.getName() == "mix" ){
        if ( b != NULL ){
            b->mix = e.getSlider()->getValue();
        }
    }
}

//--------------------------------------------------------------
void testApp::setupSpacebrew(){
    // settings
    string server = "sandbox.spacebrew.cc";
    string name = "TDC Output";
    string description = "TDC Output";
    
    ofxXmlSettings spacebrewSettings;
    bool bLoaded = spacebrewSettings.load("settings/spacebrew.xml");
    if ( bLoaded ){
        spacebrewSettings.pushTag("settings"); {
            server = spacebrewSettings.getValue("server", server);
            name = spacebrewSettings.getValue("name", name);
            description = spacebrewSettings.getValue("description", description);
        } spacebrewSettings.popTag();
    }
    
    // basix
    spacebrew.setAutoReconnect();
    
    // interaction settings
    spacebrew.addSubscribe("intensityx", "float_normalized");
    spacebrew.addSubscribe("intensityy", "float_normalized");
    spacebrew.addSubscribe("intensityz", "float_normalized");
    spacebrew.addSubscribe("rate", "float_normalized");
    spacebrew.addSubscribe("mix", "float_normalized");
    
    // inputs
    spacebrew.addSubscribe("environmentimmediate", "float_normalized");
    spacebrew.addSubscribe("environmentlocal", "float_normalized");
    spacebrew.addSubscribe("environmentglobal", "float_normalized");
    spacebrew.addSubscribe("language", "float_normalized");
    
    // specific stuff...
    spacebrew.addSubscribe("weather", "range");
    spacebrew.addSubscribe("condition", "range");
    
    
    spacebrew.connect(server, name, description);
    
    Spacebrew::addListener(this, spacebrew);
}

//--------------------------------------------------------------
void testApp::onMessage( Spacebrew::Message & m ){
    Behavior * b = particles.getSettingsBehavior();
    m.value = m.value.substr(1,m.value.length()-1);
    if ( m.name == "intensityx" ){
        if ( b != NULL ){
            b->intensity.x = ofToFloat(m.value) * 100.0f;
        }
        ((ofxUISlider *)gui->getWidget("intensityX"))->setValue(ofToFloat(m.value) * 100.0f);
    } else if ( m.name == "intensityy"){
        if ( b != NULL ){
            b->intensity.y = ofToFloat(m.value) * 100.0f;
            ((ofxUISlider *)gui->getWidget("intensityY"))->setValue(ofToFloat(m.value) * 100.0f);
        }
    } else if ( m.name == "rate"){
        if ( b != NULL ){
            b->timeFactor = ofToFloat(m.value) * 0.01;
            ((ofxUISlider *)gui->getWidget("rate"))->setValue(ofToFloat(m.value) * 10.0f);
        }
    } else if ( m.name == "mix"){
        if ( b != NULL ){
            b->mix = ofToFloat(m.value);
            ((ofxUISlider *)gui->getWidget("mix"))->setValue(ofToFloat(m.value));
        }
    } else if ( m.name == "environmentimmediate" ){
        particles.dataObject.environmentImmediate = ofToFloat(m.value);
    } else if ( m.name == "environmentlocal" ){
        particles.dataObject.environmentLocal = ofToFloat(m.value);
    } else if ( m.name == "environmentglobal" ){
        particles.dataObject.environmentGlobal = ofToFloat(m.value);
    } else if ( m.name == "language" ){
        particles.dataObject.language = ofToFloat(m.value);
    } else if ( m.name == "weather" ){
        particles.dataObject.environmentLocal = ofToFloat(m.value) / 100.0f;
    } else if ( m.name == "condition" ){
        //particles.dataObject.language = ofToFloat(m.value);
        // ?
        
        //particleColor.setHue(particles.dataObject.environmentLocal);
    }
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
