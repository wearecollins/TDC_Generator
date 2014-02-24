#include "testApp.h"

bool bClear = true;
bool bCapture = false;
bool bSave    = false;
bool bReload    = false;
bool bUseGrid = true;
bool bDrawTypeAsOverlay = true;
bool bDrawTypeInSpace = true;

bool bUseLiveInput      = true;
bool bUseHomography     = true;
bool bDrawFBO           = true;

bool bDrawKinect        = false;
bool bTracking          = false;

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

// detection
int posterThresh = 20;

// type color
//float typeColor = 0;

// poster
ofRectangle poster = ofRectangle(0,0,100,400);
ofFloatColor posterColor = ofColor(1.0,0,0);
ofFloatColor posterColorBottom = ofColor(1.0,0,0);
vector<string> posterNames;
string currentPosterName;
ofMatrix4x4 matrix;
ofPoint posterPts[4];

ofFloatColor typeColor;

//--------------------------------------------------------------
void testApp::setup(){
    particles.setup();
    
    toSave.begin();
    ofClear(0,0,0,0);
    toSave.end();
    
    toSavePoster.begin();
    ofClear(0,0,0,0);
    toSavePoster.end();
    
    // projection setup
    bEditingMask = false;
    maskEditor.setup();
    
    // load SVG for aspect ratio
    posterSrc.load("paper.svg");
    ofPath pz = posterSrc.getPathAt(0);
    pz.setPolyWindingMode(OF_POLY_WINDING_ODD);
    posterPts[0] = pz.getOutline()[0].getVertices()[0];
    posterPts[1] = pz.getOutline()[0].getVertices()[1];
    posterPts[2] = pz.getOutline()[0].getVertices()[2];
    posterPts[3] = pz.getOutline()[0].getVertices()[3];
    
    toSave.allocate(400, 600, GL_RGBA);//, 6);
    toSavePoster.allocate(400, 600, GL_RGBA);//, 6);
    
    // load optional type overlay
    
    map<string, TargetMesh>::iterator it = particles.meshes.begin();
    
    currentPosterName = it->first;
    
    for (it; it != particles.meshes.end(); ++it){
        posterNames.push_back(it->first);
        type[it->first] = ofxSVG();
        type[it->first].load("meshes/" + it->first +"/sources/" + it->first +".svg");
        
        int n = type[it->first].getNumPath();
        
        for (int i=0; i<n; i++){
            type[it->first].getPathAt(i).setUseShapeColor(false);
        }
        
        //setup surfers
        surfers[it->first] = ofPtr<ofxSurf>(new ofxSurf());
        surfImages[it->first].loadAndScale( "surf/" + it->first + ".png");
        surfers[it->first]->setSource(surfImages[it->first].getPixelsRef());
    }
    
    lastDrawMode = 0;
    drawMode = DRAW_POINTS;
    ofDisableDepthTest();
    //glPointSize(.5f);
    
    // GUI
    gui = new ofxUITabBar(10, 10, ofGetWidth(), ofGetHeight());
    gui->setVisible(false);
    gui->setColorBack(ofColor(0,0,0,0));
    
    ofxUISuperCanvas * guiDrawing = new ofxUISuperCanvas("DRAWING",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiDrawing);
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    guis.back()->setColorBack(ofColor(0,0,0,0));
    
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
    guis.back()->setColorBack(ofColor(0,0,0,0));
    guis.back()->setName("TYPE");
    guis.back()->addToggle("Draw type overlay", &bDrawTypeAsOverlay);
    guis.back()->addToggle("Draw type in space", &bDrawTypeInSpace);
    guis.back()->addDoubleSlider("scale", 0.0, 1.0, &particles.scale);
    guis.back()->addDoubleSlider("x", -1.0, 1.0, &x);
    guis.back()->addDoubleSlider("y", -1.0, 1.0, &y);
    guis.back()->addSlider("type color r", 0, 1.0, &typeColor.r);
    guis.back()->addSlider("type color g", 0, 1.0, &typeColor.g);
    guis.back()->addSlider("type color b", 0, 1.0, &typeColor.b);
    guis.back()->addSlider("type color a", 0, 1.0, &typeColor.a);
    
    guis.back()->addSpacer();
    guis.back()->addLabel("Poster Label", "Select Poster");
    guis.back()->addRadio("Poster", posterNames);
    
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiMovement = new ofxUISuperCanvas("MOVEMENT",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiMovement);
    guis.back()->setColorBack(ofColor(0,0,0,0));
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
    guis.back()->setColorBack(ofColor(0,0,0,0));
    guis.back()->setName("POSTER");
    guis.back()->addSlider("Poster X", 0.0, ofGetWidth(), &poster.x);
    guis.back()->addSlider("Poster Y", 0.0, ofGetWidth(), &poster.y);
    guis.back()->addSlider("Poster Width", 0.0, ofGetWidth(), &poster.width);
    guis.back()->addSlider("Poster Height", 0.0, ofGetWidth(), &poster.height);
    guis.back()->addSlider("Poster Color: R", 0.0, 1.0, &posterColor.r);
    guis.back()->addSlider("Poster Color: G", 0.0, 1.0, &posterColor.g);
    guis.back()->addSlider("Poster Color: B", 0.0, 1.0, &posterColor.b);
    guis.back()->addIntSlider("Dectection thresh", 0, 100, &posterThresh);
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiEvents = new ofxUISuperCanvas("EVENTS",0,0,ofGetWidth()-100, ofGetHeight());
    guis.push_back(guiEvents);
    guis.back()->setColorBack(ofColor(0,0,0,0));
    guis.back()->setName("EVENTS");
    guis.back()->addToggle("Save Frame", &bCapture);
    guis.back()->addToggle("Save Settings", &bSave);
    guis.back()->addToggle("Reload Settings", &bReload);
    guis.back()->addSpacer();
    guis.back()->addSpacer();
    guis.back()->addToggle("Use Inputs", &bUseLiveInput);
    guis.back()->addToggle("Use Homography", &bUseHomography);
    guis.back()->addToggle("Draw Kinect", &bDrawKinect);
    guis.back()->addToggle("Draw Render Buffer", &bDrawFBO);
    
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
    // CHECK FOR DRAW MODE + PARTICLE UPDATES
    if ( lastDrawMode != drawMode || particles.getDrawMode() != ((DrawMode) drawMode) ){
        lastDrawMode = drawMode;
        particles.setDrawMode( (DrawMode) drawMode);
        //((ofxUILabel*)gui->getActiveCanvas()->getWidget("Drawmode Label"))->setLabel(particles.getDrawModeString());
    }
    if ( lastParticleColor != particleColor ){ 
        particles.setColor(particleColor, hueVariance / 10.0f);
        lastParticleColor = particleColor;
    }
    
    // IMAGE + SETTINGS SAVING
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
    
    posterMesh.setVertex(0, posterPts[0]);
    posterMesh.setVertex(1, posterPts[1]);
    posterMesh.setVertex(2, posterPts[2]);
    posterMesh.setVertex(3, posterPts[3]);
    
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

    // UPDATE: SURF
    if ( bTracking ){
        map<string, ofPtr<ofxSurf> >::iterator it = surfers.begin();
        if ( particles.camera.getKinect().isFrameNew() && particles.camera.getKinect().getPixelsRef().getWidth() > 0 ){
            //camera.loadAndScale( particles.camera.colorPixels );
            for (it; it != surfers.end(); ++it){
                it->second->detect( particles.camera.getKinect().getPixelsRef() );
                //it->second->draw(640,480 + colorSmall.height * i);
                //                cout << surfers[i]->getMatches().size() << " : "<< i << endl;
                // i need to figure this out!
                if ( it->second->getMatches().size() < posterThresh && it->first != currentPosterName ){
                    cout << "we're on poster "<< it->first << endl;
                    currentPosterName = it->first;
                    particles.setMesh( currentPosterName );
                }
            }
        }
    }
    // UPDATE: PARTICLES
    
    particles.setUseGrid(bUseGrid);
    particles.update();
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
    
    // HOMOGRAPHY + MASKING
    
	maskEditor.update();
    maskEditor.findHomography(posterPts, maskEditor.dst, matrix);
}

//--------------------------------------------------------------
void testApp::draw(){
    toSave.begin();
    if ( bClear) ofClear(0,0,0,0);
    else {
        ofSetColor(0,0,0,bgAlpha);
        ofRect(0,0, toSave.getWidth(), toSave.getHeight());
        ofSetColor(255,255);
    }
    renderParticles();
    toSave.end();
    
    toSavePoster.begin(); {
        ofClear(0,0,0,255);
        posterMesh.draw();
    
        ofSetColor(255);
        toSave.draw(0, 0);
        
    } toSavePoster.end();
    
    ofPushMatrix();
    if ( bUseHomography) ofMultMatrix(matrix);
    if (!bDrawFBO){
        posterMesh.draw();
        renderParticles();
    } else {
        toSavePoster.draw(0,0);
    }
    ofPopMatrix();
    
    if ( bDrawKinect ){
        ofSetColor(255);
        particles.camera.getKinect().draw(10, 10, 320, 240);
        map<string, ofPtr<ofxSurf> >::iterator it = surfers.begin();
        int i=0;
        for (it; it != surfers.end(); ++it){
            it->second->draw(640,480 + 320 * i);
            i++;
        }
    }
    
    if ( bEditingMask ){
        maskEditor.draw();
    }
}


//--------------------------------------------------------------
void testApp::renderParticles(){
    ofPushMatrix();
    
    //    static float scale = fmax( ofGetWidth()/1280.0f, ofGetHeight()/720.0f );
    //    ofScale(scale, scale);
    
    ofEnableAlphaBlending();
    ofEnableBlendMode((ofBlendMode) mode);
    if (bDrawTypeInSpace){
        ofEnableDepthTest();
        ofPushMatrix();
        ofSetColor(typeColor);
        type[currentPosterName].draw();
        ofPopMatrix();
    }
    if (bDrawTypeAsOverlay){
        ofSetColor(typeColor);
        type[currentPosterName].draw();
    }
    particles.draw();
    ofDisableDepthTest();
    
    ofPopMatrix();
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
    } else if(key == ' ' && bEditingMask){
		maskEditor.clear();
	} else if ( key == 'm' ){
        bEditingMask = !bEditingMask;
        if (bEditingMask){
            ofShowCursor();
        } else if ( !gui->isVisible()){
            ofHideCursor();
        }
    } else if ( key == 't'){
        bTracking = true;
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
    } else if ( e.getName() == "Poster" ){
        particles.setMesh( posterNames[((ofxUIRadio*)e.widget)->getValue()] );
        currentPosterName = posterNames[((ofxUIRadio*)e.widget)->getValue()];
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
