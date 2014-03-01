#include "testApp.h"
#include "WeatherColors.h"

// GUI events
bool bClear = true;
bool bCapture = false;
bool bRendering = false;
bool bStartedRendering = false;
bool bSave    = false;
bool bReload    = false;

// time to reset
bool bTurnOff = false;
bool bWasOff = false;
bool bExplode = false;

// COLORS
ofColor communicationColor = ofColor(255,125,5);
ofColor communityColor = ofColor(255,216,0);
ofColor envColor = ofColor(59,107,239);

// Particle settings
bool bUseGrid = true;
int mode = 0;
int lastDrawMode = -1;
float bgAlpha = 255.0f;
ofFloatColor particleColor, lastParticleColor;
float hueVariance;

// Type settings
bool bDrawTypeAsOverlay = true;
bool bDrawTypeInSpace = true;
ofFloatColor typeColor;

// Data settings
bool bUseLiveInput      = true;

// Projection settings
bool bUseHomography     = true;
bool bDrawFBO           = true;
bool bDrawPoster        = true;
double scale = 1.0;
double x = 0;
double y = 0;
ofMatrix4x4 matrix;
ofPoint posterPts[4];

// tracking settings
bool bDrawKinect        = false;
bool bDrawTracking        = false;
bool bTracking          = false;

// SURF settings
int posterThresh = 20;

// Poster settings
ofRectangle poster = ofRectangle(0,0,100,400);
ofFloatColor posterColor = ofColor(1.0,0,0);
ofFloatColor posterColorBottom = ofColor(1.0,0,0);
vector<string> posterNames;
string currentPosterName;

// Community settings (background color)
ofPoint currentIntensity = ofPoint(0,0,0);
string currentCondition = "";
float currentHue = ofRandom(0,1.0);
float liveHueTop = ofRandom(0,1.0);
float liveHueBottom = ofRandom(0,1.0);

float liveSat = 0;
float liveBright = 0;

float posterSat = 1.0, posterBright = 1.0;
float lastChanged = 0.0;

// min and max for each value
float maxSaturation = 1.0;

//--------------------------------------------------------------
void testApp::exit(){
    particles.camera.getKinect().close();
}

//--------------------------------------------------------------
void testApp::setup(){
    
    dimensionsX = ofGetWidth();
    dimensionsY = ofGetHeight();
    particles.setup();
    
    // projection setup
    bEditingMask = false;
    maskEditor.setup();
    mask.loadImage("mask.png");
    
    // load SVG for aspect ratio
    posterSrc.load("paper.svg");
    ofPath pz = posterSrc.getPathAt(0);
    pz.setPolyWindingMode(OF_POLY_WINDING_ODD);
    posterPts[0] = pz.getOutline()[0].getVertices()[0];
    posterPts[1] = pz.getOutline()[0].getVertices()[1];
    posterPts[2] = pz.getOutline()[0].getVertices()[2];
    posterPts[3] = pz.getOutline()[0].getVertices()[3];
    
//    ofEnableAntiAliasing();
    
    toSave.allocate(ofGetWidth(), ofGetHeight() );//, GL_RGBA32F, 8);
    toSavePoster.setUseTexture(false);
    toSavePoster.allocate(7200, 10800 );//, GL_RGBA32F, 8);
    toSavePosterPing.setUseTexture(false);
//    toSavePosterPing.allocate(7200, 10800 );
    
    pixelDPI = (toSavePoster.getWidth()*2.0)/toSave.getWidth();
    
    toSave.begin();
    ofClear(0,0,0,0);
    toSave.end();
    
    toSavePoster.begin();
    ofClear(0,0,0,0);
    toSavePoster.end();
    
//    toSavePosterPing.begin();
//    ofClear(0,0,0,0);
//    toSavePosterPing.end();
    // load optional type overlay
    
    map<string, TargetMesh>::iterator it = particles.meshes.begin();
    
    currentPosterName = it->first;
    
    bool bUseSurf = false;
    
    for (it; it != particles.meshes.end(); ++it){
        posterNames.push_back(it->first);
        type[it->first] = ofxSVG();
        type[it->first].load("meshes/" + it->first +"/sources/" + it->first +".svg");
        
        int n = type[it->first].getNumPath();
        
        for (int i=0; i<n; i++){
            type[it->first].getPathAt(i).setUseShapeColor(false);
        }
        
        if ( bUseSurf ){
            //setup surfers
            surfers[it->first] = ofPtr<ofxSurf>(new ofxSurf());
            surfImages[it->first].loadAndScale( "surf/" + it->first + ".png");
            surfers[it->first]->setSource(surfImages[it->first].getPixelsRef());
        }
    }
    
    lastDrawMode = 0;
    drawMode = DRAW_POINTS;
    ofDisableDepthTest();
    //glPointSize(.5f);
    
    // GUI
    gui = new ofxUITabBar(10, 10, ofGetWidth(), ofGetHeight());
    gui->setVisible(false);
    gui->setColorBack(ofColor(0,0,0,0));
    
    ofxUISuperCanvas * guiDrawing = new ofxUISuperCanvas("DRAWING",0,0,ofGetWidth()-200, ofGetHeight());
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
    guis.back()->addSlider("Particle Home attraction", 0.0, 1.0, &particles.home);
    guis.back()->addSlider("Particle Color: R", 0.0, 1.0, &particleColor.r);
    guis.back()->addSlider("Particle Color: G", 0.0, 1.0, &particleColor.g);
    guis.back()->addSlider("Particle Color: B", 0.0, 1.0, &particleColor.b);
    guis.back()->addSlider("Particle Color: Hue Randomization", 0.0, 1.0, &hueVariance);
    guis.back()->addSlider("Particle Density", 0.0, 1.0, &particles.density);
    guis.back()->addSlider("Point size", 0.01, 100.0f, &particles.pointSize);
    guis.back()->addSlider("Point randomization", 0.0, 10.0, &particles.pointRandomization);
    gui->addCanvas(guis.back());
    
    ofxUISuperCanvas * guiTypeOverlay = new ofxUISuperCanvas("TYPE",0,0,ofGetWidth()-200, ofGetHeight());
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
    
    ofxUISuperCanvas * guiMovement = new ofxUISuperCanvas("MOVEMENT",0,0,ofGetWidth()-200, ofGetHeight());
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
    
    ofxUISuperCanvas * guiPoster = new ofxUISuperCanvas("POSTER",0,0,ofGetWidth()-200, ofGetHeight());
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
    guis.back()->addSlider("Poster Color: S", 0.0, 1.0, &posterSat);
    guis.back()->addSlider("Poster Color: B", 0.0, 1.0, &posterBright);
    guis.back()->addIntSlider("Dectection thresh", 0, 100, &posterThresh);
    guis.back()->addDoubleSlider("scale", 0.0, 4.0, &scale);
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiEvents = new ofxUISuperCanvas("EVENTS",0,0,ofGetWidth()-200, ofGetHeight());
    guis.push_back(guiEvents);
    guis.back()->setColorBack(ofColor(0,0,0,0));
    guis.back()->setName("EVENTS");
    guis.back()->addToggle("Save Frame", &bCapture);
    guis.back()->addToggle("Start Highres Render", &bRendering);
    guis.back()->addToggle("Save Settings", &bSave);
    guis.back()->addToggle("Reload Settings", &bReload);
    guis.back()->addSpacer();
    guis.back()->addSpacer();
    guis.back()->addToggle("Use Homography", &bUseHomography);
    guis.back()->addToggle("Draw Render Buffer", &bDrawFBO);
    guis.back()->addToggle("Draw Poster", &bDrawPoster);
    guis.back()->addToggle("Turn Off", &bTurnOff);
    
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiKinect = new ofxUISuperCanvas("KINECT",0,0,ofGetWidth()-200, ofGetHeight() * 2);
    guis.push_back(guiKinect);
    guis.back()->setColorBack(ofColor(0,0,0,0));
    guis.back()->setName("KINECT");
    guis.back()->addIntSlider("Near thresh", 0.0, 10000.0, &particles.camera.near );
    guis.back()->addIntSlider("Far thresh", 0.0, 10000.0, &particles.camera.far );
    guis.back()->addSpacer();
    guis.back()->addToggle("Draw Kinect", &bDrawKinect);
    guis.back()->addToggle("Draw Sensing", &bDrawTracking);

    guis.back()->addToggle("Flip Horiz", &particles.camera.bFlipHoriz);
    guis.back()->addToggle("Flip Vert", &particles.camera.bFlipVert);
    guis.back()->addToggle("Flip LR + TB", &particles.camera.bFlipAxes);
    guis.back()->addSlider("Threshold", 0, 255, &particles.camera.thresh );
    guis.back()->addSlider("Skew Colors", -1.0, 1.0, &particles.camera.cameraSkew);
    guis.back()->add2DPad("Crop: TL", ofPoint(-1600,1600), ofPoint(-1600,1600), &particles.camera.quad[0], 160, 120, 0, 0);
    guis.back()->add2DPad("Crop: TR", ofPoint(-1600,1600), ofPoint(-1600,1600), &particles.camera.quad[1], 160, 120, 200, 0);
    guis.back()->add2DPad("Crop: BL", ofPoint(0,640), ofPoint(0,480), &particles.camera.quad[2], 160, 120, 0, 140);
    guis.back()->add2DPad("Crop: BR", ofPoint(0,640), ofPoint(0,480), &particles.camera.quad[3], 160, 120, 200, 140);
    guis.back()->addSpacer();
    
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiTracking = new ofxUISuperCanvas("COLOR TRACKING",0,0,ofGetWidth()-200, ofGetHeight());
    guis.push_back(guiTracking);
    guis.back()->setColorBack(ofColor(0,0,0,0));
    guis.back()->setName("COLOR TRACKING");
    guis.back()->addIntSlider("Edit which color", 0, 2, &particles.camera.tracker.currentColor );
    guis.back()->addSlider("Threshold", 0, 255, &particles.camera.tracker.threshold );
    guis.back()->addSlider("Min Radius", 0, 255, &particles.camera.tracker.minRad );
    guis.back()->addSlider("Max Radius", 0, 255, &particles.camera.tracker.maxRad );
    guis.back()->addToggle("Use RGB", &particles.camera.tracker.bUseRGB);
    
    
    guis.back()->addSpacer();
    guis.back()->addToggle("Draw Tracker", &particles.camera.tracker.bDraw);
    guis.back()->addToggle("Draw Blobs", &particles.camera.tracker.bDrawBlobs);
    guis.back()->addToggle("Enable Color Selection", &particles.camera.tracker.bChoose);
    guis.back()->add2DPad("ROI: XY", ofPoint(0,640), ofPoint(0,480), &particles.camera.tracker.rectXY, 160, 120, 0, 140);
    guis.back()->add2DPad("ROI: WH", ofPoint(0,640), ofPoint(0,480), &particles.camera.tracker.rectWH, 160, 120, 200, 140);
    
    gui->addCanvas(guis.back());
    ofAddListener(guis.back()->newGUIEvent, this, &testApp::onGui);
    
    ofxUISuperCanvas * guiData = new ofxUISuperCanvas("DATA",0,0,ofGetWidth()-200, ofGetHeight());
    guis.push_back(guiData);
    guis.back()->setColorBack(ofColor(0,0,0,0));
    guis.back()->setName("DATA");
    guis.back()->addSlider("Local Weight", 0.0, 1.0, &particles.dataObject.eiWeight );
    guis.back()->addSlider("Env Weight", 0.0, 1.0, &particles.dataObject.elWeight );
    guis.back()->addSlider("Lang Weight", 0.0, 1.0, &particles.dataObject.langWeight );
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
    setupDataBar();
    setupDataBarRetina();
    
    // randomize
    randomize();
}

//--------------------------------------------------------------
void testApp::randomize(){
    drawMode = ofRandom(DRAW_SHAPES);
    particles.setBehavior( (MovementType) ofRandom(MOVE_BUMP) );
    currentHue = ofRandom(.5);
    
    bUseGrid = floor(ofRandom(2));
    hueVariance = ofRandom(1.0);
    particleColor.r = ofRandom(1.0);
    particleColor.g = ofRandom(1.0);
    particleColor.b = ofRandom(1.0);
    lastParticleColor = ofColor(0,0,0,0);
}

//--------------------------------------------------------------
void testApp::update(){
    if ( bTurnOff ){
        maxSaturation *= .75;
        particles.home *= .75;
    } else {
        maxSaturation = maxSaturation * .95 + .05;
        if ( maxSaturation >= .9){
            if ( bExplode ){
                bExplode = false;
                particles.explode();
            }
            particles.home = particles.home * .9 + .1;
        }
    }
    if ( !bTurnOff && bWasOff ){
        bExplode = true;
    }
    bWasOff = bTurnOff;
    
    // CHECK FOR DRAW MODE + PARTICLE UPDATES
    if ( lastDrawMode != drawMode || particles.getDrawMode() != ((DrawMode) drawMode) ){
        lastDrawMode = drawMode;
        particles.setDrawMode( (DrawMode) drawMode);
        //((ofxUILabel*)gui->getActiveCanvas()->getWidget("Drawmode Label"))->setLabel(particles.getDrawModeString());
    }
    if ( lastParticleColor != particleColor ){ 
        particles.setColor(particleColor, hueVariance);
        lastParticleColor = particleColor;
    }
    
    // IMAGE + SETTINGS SAVING
    if ( bSave ){
        gui->saveSettings("settings/", "ui-");
        bSave = false;
    }
    if ( bReload ){
        
        gui->loadSettings("settings/", "ui-");
        bReload = false;
    }
    
    // DATA OBJECT: UPDATE WEIGHTS
    particles.dataObject.eiWeight   = (float) particles.camera.tracker.getNumberByColor(0) / 3.0;
    particles.dataObject.elWeight   = (float) particles.camera.tracker.getNumberByColor(1) / 3.0;
    particles.dataObject.langWeight = (float) particles.camera.tracker.getNumberByColor(2) / 3.0;
    
    // DATA OBJECT: TIME/DATE
    
    /*float timeMappedSunset = particles.dataObject.time > .6 ? ofMap(particles.dataObject.time, .6, 1.0, 1.0, 0) : ofMap(particles.dataObject.time, 0.0, .6, 0, 1.0);
    float timeMappedMidday = particles.dataObject.time > .5 ? ofMap(particles.dataObject.time, .5, 1.0, 1.0, 0.0) : ofMap(particles.dataObject.time, 0,1.0, 0, 1.0);
    float yesterday = particles.dataObject.date - 1.0/365.0f;
    
    if ( bUseLiveInput ){
        particleColor.setHue( particles.dataObject.date  );
        particleColor.setSaturation( timeMappedSunset );
        particleColor.setBrightness( timeMappedMidday );
    }*/
    
    // yes OK
    
    posterMesh.setVertex(0, posterPts[0]);
    posterMesh.setVertex(1, posterPts[1]);
    posterMesh.setVertex(2, posterPts[2]);
    posterMesh.setVertex(3, posterPts[3]);
    
    // DATA OBJECT: COMMUNICATION
    
    if ( bUseLiveInput ){
        particles.density = particles.density * .9 + (fmin(1.0,.3 + particles.dataObject.langWeight)) * .1;
    }
    
    // CLAP TO RANDOMIZE
    if ( particles.dataObject.environmentImmediate > .2 && ofGetElapsedTimef() - lastChanged > 2.0 ){
        randomize();
        lastChanged = ofGetElapsedTimef();
    }
    
    // DATA OBJECT: ENVIRONMENT
    
    particles.dataObject.elString = "Temp: "+ofToString(particles.dataObject.environmentLocal * 100.0);// + "\nCondition: "+currentCondition;
    particles.dataObject.eiString = "Sound: "+ofToString(particles.dataObject.environmentImmediate * 100.0);
    
    if ( bUseLiveInput ){
        Behavior * b = particles.getSettingsBehavior();
        //b->intensity.x = particles.dataObject.environmentImmediate * particles.dataObject.eiWeight * 500.0;
        //b->intensity.y = particles.dataObject.environmentLocal * particles.dataObject.elWeight * 500.0;
        //b->intensity.z = particles.dataObject.language * particles.dataObject.langWeight * 500.0;
        
        ofPoint intense = currentIntensity;
        intense *= (50. * particles.dataObject.elWeight);
        intense.x = intense.x * .5 + particles.dataObject.getWeightedEL() * 50.;
        intense.y = intense.y * .5 + particles.dataObject.getWeightedEL() * 50.;
        intense.z = intense.z * .5 + particles.dataObject.getWeightedEL() * 50.;
        b->intensity.set( intense );
        b->timeFactor = particles.dataObject.getWeightedEL() / 1000.0;
        
        // UPDATE GUI BASED ON DATA OBJECT
        ((ofxUISlider *)guis[2]->getWidget("intensityX"))->setValue(b->intensity.x);
        ((ofxUISlider *)guis[2]->getWidget("intensityY"))->setValue(b->intensity.y);
        ((ofxUISlider *)guis[2]->getWidget("intensityZ"))->setValue(b->intensity.z);
        
        liveHueTop = liveHueTop * .9 + (currentHue + ofMap(particles.dataObject.environmentImmediate * particles.dataObject.eiWeight, 0.0, 1.0, -currentHue * .75, currentHue, true));
        
        liveHueBottom = liveHueBottom * .9 + (currentHue + ofMap(particles.dataObject.environmentImmediate * particles.dataObject.eiWeight, 0.0, 1.0, -currentHue * .5, currentHue * .5, true));
        
        posterColor.setHue(liveHueTop);
        posterColorBottom.setHue(liveHueBottom);
        
        liveSat = (liveSat * .9 + particles.dataObject.eiWeight * .1) * maxSaturation;
        
        posterColor.setSaturation( liveSat );
        posterColorBottom.setSaturation( liveSat );
        posterColor.setBrightness( liveSat );
        posterColorBottom.setBrightness( liveSat );
        
        posterMesh.setColor(0, posterColor );
        posterMesh.setColor(1, posterColor );
        posterMesh.setColor(2, posterColorBottom );
        posterMesh.setColor(3, posterColorBottom );
        
    } else {
        posterColor.setSaturation(posterSat * maxSaturation);
        posterColor.setBrightness(posterBright * maxSaturation);
        
        posterColorBottom.setBrightness(posterBright * maxSaturation);
        posterColorBottom.setSaturation(posterSat * maxSaturation);
        
        posterMesh.setColor(0, posterColor );
        posterMesh.setColor(1, posterColor );
        posterMesh.setColor(2, posterColor );
        posterMesh.setColor(3, posterColor );
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
    
    // set stupid data objects
    
    ((ofxUILabel *)envGui->getWidget("data"))->setLabel(particles.dataObject.elString);
    ((ofxUILabel *)commGui->getWidget("data"))->setLabel(particles.dataObject.langString);
    ((ofxUILabel *)collabGui->getWidget("data"))->setLabel(particles.dataObject.eiString);
    
    // UPDATE: PARTICLES
    
    particles.setUseGrid(bUseGrid);
    particles.update();
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
    
    // HOMOGRAPHY + MASKING
    
	maskEditor.update();
    maskEditor.findHomography(posterPts, maskEditor.dst, matrix);
    if ( particles.getCurrentBehavior() != NULL && particles.getCurrentBehavior()->getName() == "flocking" )
        particles.getCurrentBehavior()->homography = matrix;
}

//--------------------------------------------------------------
void testApp::draw(){
    
    toSave.begin();
    ofDisableDepthTest();
    ofPushMatrix();
    if ( bClear ) ofClear(posterColor.r,posterColor.g, posterColor.b,0);
    else renderBackground( bUseHomography );
    renderParticles( bUseHomography );
    ofPopMatrix();
    toSave.end();
    
    if ( bCapture ){
        bCapture = false;
        renderPoster(false);
        renderPoster();
    } else if ( bRendering ){
        renderPoster(false);
    }
    
    ofPushMatrix(); {
        if ( bUseHomography){
            
        } else {
            ofTranslate(poster.x, poster.y);
            ofScale(scale, scale);
        }
        //if (!bDrawFBO ){
            ofDisableDepthTest();
        
            //???
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            ofPushMatrix(); {
                if ( bUseHomography ) ofMultMatrix(matrix);
                posterMesh.draw();
            } ofPopMatrix();
            //renderParticles();
            ofPushMatrix(); {
                ofFill();
                ofSetColor(255);
                if ( bUseHomography && (particles.getCurrentBehavior() != NULL && particles.getCurrentBehavior()->getName() == "flocking" )){
                    ofMultMatrix(matrix);
                }
                if ( bDrawPoster ){
                    toSave.draw(0, 0);
                } else {
                    renderParticles(false);
                }
            } ofPopMatrix();
            ofPushMatrix(); {
                if ( bUseHomography ) ofMultMatrix(matrix);
                if ( particles.getCurrentBehavior() != NULL ) drawDataBar( !bUseHomography );
                if ( bDrawTracking ) ofEllipse(particles.camera.lastPoint, 10, 10);
            } ofPopMatrix();
        //}
    } ofPopMatrix();
    if ( bUseHomography ) mask.draw(0,0);
    
    if ( bDrawKinect ){
        ofSetColor(255);
        //particles.camera.getKinect().draw(10, 10, 320, 240);
        particles.camera.draw(10, 250, 320, 240);
        particles.camera.getKinect().drawDepth (330, 250, 320, 240);
//        map<string, ofPtr<ofxSurf> >::iterator it = surfers.begin();
//        int i=0;
//        for (it; it != surfers.end(); ++it){
//            it->second->draw(640,480 + 320 * i);
//            i++;
//        }
    }
    
    if ( bEditingMask ){
        maskEditor.draw();
    } else {
        particles.camera.tracker.draw();
    }
}

//--------------------------------------------------------------
void testApp::renderBackground( bool bHomography ){
    ofPushMatrix();
    ofEnableAlphaBlending();
    if ( bHomography && (particles.getCurrentBehavior() == NULL || particles.getCurrentBehavior()->getName() != "flocking" ) ){
        ofMultMatrix(matrix);
    }
    if ( bClear){
        ofDisableDepthTest();
        posterMesh.draw();
    } else {
        posterColor.a = bgAlpha  / 255.0f;
        posterColorBottom.a = bgAlpha / 255.0f;
        
        posterMesh.setColor(0, posterColor );
        posterMesh.setColor(1, posterColor );
        posterMesh.setColor(2, posterColorBottom );
        posterMesh.setColor(3, posterColorBottom );
        
        posterMesh.draw();
        
        posterColor.a = 1.0;
        posterColorBottom.a = 1.0;
        
        posterMesh.setColor(0, posterColor );
        posterMesh.setColor(1, posterColor );
        posterMesh.setColor(2, posterColorBottom );
        posterMesh.setColor(3, posterColorBottom );
    }
    ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::renderPoster( bool bSave ){
    if ( !bSave ){
        float oldSize = particles.pointSize;
        
        particles.pointSize *= pixelDPI;
        particles.getCurrentBehavior()->pointSize = particles.pointSize;
        particles.getCurrentBehavior()->screen.x  = 2.0;
        particles.getCurrentBehavior()->screen.y  = 2.0;
        
        toSavePoster.begin(); {
            ofPushMatrix();
            ofScale(pixelDPI, pixelDPI);
            if ( bClear || (!bStartedRendering && bRendering) ){
                ofClear(0,0,0,255);
                posterMesh.draw();
            }
            renderBackground( false );
            renderParticles( false );
            ofPopMatrix();
            
            ofPushMatrix();
            ofSetColor(255);
            //drawDataBarRetina( true );
            ofPopMatrix();
            
        } toSavePoster.end();
        
        if ( bRendering ) bStartedRendering = true;
        particles.pointSize = oldSize;
        particles.getCurrentBehavior()->pointSize = particles.pointSize;
        particles.getCurrentBehavior()->screen.x  = 1.0;
        particles.getCurrentBehavior()->screen.y  = 2.0;
    }
    
    if ( bSave ){
        if ( bRendering ){
//            toSavePosterPing.begin();
//                ofClear(0,0,0,0);
//                ofDisableDepthTest();
//                ofPushMatrix();
//                ofScale(pixelDPI, pixelDPI);
//                posterColor.a = 1.0;
//                posterColorBottom.a = 1.0;
//                
//                posterMesh.setColor(0, posterColor );
//                posterMesh.setColor(1, posterColor );
//                posterMesh.setColor(2, posterColorBottom );
//                posterMesh.setColor(3, posterColorBottom );
//                posterMesh.draw();
//                ofPopMatrix();
//                toSavePoster.draw(0,0);
//            toSavePosterPing.end();
//            toSavePosterPing.readToPixels(pix);
        } else {
        }
        string name = ofToString(ofGetFrameNum())+"_" + ofGetTimestampString();
        toSavePoster.readToPixels(pix);
        ofImage screen;
        screen.setUseTexture(false);
        screen.setFromPixels(pix);
        screen.saveImage("captures/cap_"+ name+ ".png");
        bRendering = false;
        bStartedRendering = false;
        
        toSavePoster.begin(); {
            ofPushMatrix();
            ofScale(pixelDPI, pixelDPI);
            ofClear(0,0,0,255);
            posterMesh.draw();
            ofPopMatrix();
        } toSavePoster.end();
        
        toSavePoster.readToPixels(pix);
        
        screen.setFromPixels(pix);
        screen.saveImage("backgrounds/cap_"+ name+ ".png");
    }
}

//--------------------------------------------------------------
void testApp::renderParticles( bool bHomography ){
    ofPushMatrix();
    if ( bHomography  && (particles.getCurrentBehavior() == NULL || particles.getCurrentBehavior()->getName() != "flocking" )){
        ofMultMatrix(matrix);
    }
    
    //    static float scale = fmax( ofGetWidth()/1280.0f, ofGetHeight()/720.0f );
    //    ofScale(scale, scale);
    ofPushStyle();
    ofEnableAlphaBlending();
    ofEnableBlendMode((ofBlendMode) mode);
    if (bDrawTypeInSpace){
        ofEnableDepthTest();
        ofPushMatrix();
        ofSetColor(typeColor);
        ofTranslate(0,0,1);
        type[currentPosterName].draw();
        ofPopMatrix();
    } else {
        ofDisableDepthTest();
    }
    particles.draw();
    ofDisableDepthTest();
    if (bDrawTypeAsOverlay){
        ofSetColor(typeColor);
        type[currentPosterName].draw();
    }
    ofPopStyle();
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
    } else if ( key == 'c'){
        particles.camera.tracker.bDraw = !particles.camera.tracker.bDraw;
    } else if ( key == 'r' ){
        if ( particles.getCurrentBehavior() != NULL ) particles.getCurrentBehavior()->reload();
    } else if ( key == 's' ){
        mask.loadImage("mask.png");
    } else if (key == 'S'){
        bCapture = true;
    } else if ( key == 'R'){
        bTurnOff = !bTurnOff;
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
        ((ofxUISlider *)guis[2]->getWidget("intensityX"))->setValue(ofToFloat(m.value) * 100.0f);
    } else if ( m.name == "intensityy"){
        if ( b != NULL ){
            b->intensity.y = ofToFloat(m.value) * 100.0f;
            ((ofxUISlider *)guis[2]->getWidget("intensityY"))->setValue(ofToFloat(m.value) * 100.0f);
        }
    } else if ( m.name == "rate"){
        if ( b != NULL ){
            b->timeFactor = ofToFloat(m.value) * 0.01;
            ((ofxUISlider *)guis[2]->getWidget("rate"))->setValue(ofToFloat(m.value) * 10.0f);
        }
    } else if ( m.name == "mix"){
        if ( b != NULL ){
            b->mix = ofToFloat(m.value);
            ((ofxUISlider *)guis[2]->getWidget("mix"))->setValue(ofToFloat(m.value));
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
        currentIntensity = weather().getIntensity( ofToInt(m.value) );
        currentCondition = weather().getCondition( ofToInt(m.value) );
    }
}

//--------------------------------------------------------------
void testApp::drawDataBar( bool bBottom ){
    ofPushStyle();
//    ofSetColor(255,150);
//    ofRect(0,465,400,50);
    ofPushMatrix();
    if (!bBottom) ofTranslate(0,465 );
    else ofTranslate(0,500);

    ofSetColor(envColor);
    ofSetColor(communityColor);
    ofSetColor(communicationColor);
//    envGui->draw();
//    commGui->draw();
//    collabGui->draw();
    ofPopMatrix();
    ofPopStyle();
}
//--------------------------------------------------------------
void testApp::drawDataBarRetina( bool bBottom ){
    ofPushStyle();
    //    ofSetColor(255,150);
    //    ofRect(0,465,400,50);
    ofPushMatrix();
    if (!bBottom) ofTranslate(0,465 * pixelDPI);
    else ofTranslate(0,500 * pixelDPI);
    envGuiRetina->draw();
    commGuiRetina->draw();
    collabGuiRetina->draw();
    ofPopMatrix();
    ofPopStyle();
}


//--------------------------------------------------------------
void testApp::setupDataBarRetina(){
    if ( envGuiRetina == NULL ){
        envGuiRetina = new ofxUICanvas(0,0,100* pixelDPI,50* pixelDPI);
        envGuiRetina->disableAppEventCallbacks();
        envGuiRetina->setWidgetFontSize(OFX_UI_FONT_SMALL);
        envGuiRetina->enableAppUpdateCallback();
        envGuiRetina->setGlobalSliderHeight(5.0* pixelDPI);
        envGuiRetina->setWidgetSpacing(4* pixelDPI);
        
        commGuiRetina = new ofxUICanvas(100* pixelDPI,0,100* pixelDPI,50* pixelDPI);
        commGuiRetina->setWidgetFontSize(OFX_UI_FONT_SMALL);
        commGuiRetina->disableAppEventCallbacks();
        commGuiRetina->enableAppUpdateCallback();
        commGuiRetina->setGlobalSliderHeight(5.0* pixelDPI);
        commGuiRetina->setWidgetSpacing(4* pixelDPI);
        
        collabGuiRetina = new ofxUICanvas(200* pixelDPI,0,100* pixelDPI,50* pixelDPI);
        collabGuiRetina->setWidgetFontSize(OFX_UI_FONT_SMALL);
        collabGuiRetina->disableAppEventCallbacks();
        collabGuiRetina->enableAppUpdateCallback();
        collabGuiRetina->setGlobalSliderHeight(5.0* pixelDPI);
        collabGuiRetina->setWidgetSpacing(4* pixelDPI);
        
        static bool bLoadedFonts = false;
        if ( !bLoadedFonts ){
            bLoadedFonts = commGuiRetina->setFont("fonts/monaco.ttf");
            commGuiRetina->setFontSize(OFX_UI_FONT_SMALL, 3* pixelDPI);
            commGuiRetina->setFontSize(OFX_UI_FONT_MEDIUM, 5* pixelDPI);
            commGuiRetina->setFontSize(OFX_UI_FONT_LARGE, 4* pixelDPI);
            commGuiRetina->addLabel("Communication");
            
            bLoadedFonts = collabGuiRetina->setFont("fonts/monaco.ttf");
            collabGuiRetina->setFontSize(OFX_UI_FONT_SMALL, 3* pixelDPI);
            collabGuiRetina->setFontSize(OFX_UI_FONT_MEDIUM, 5* pixelDPI);
            collabGuiRetina->setFontSize(OFX_UI_FONT_LARGE, 4* pixelDPI);
            collabGuiRetina->addLabel("Community");
            
            
            bLoadedFonts = envGuiRetina->setFont("fonts/monaco.ttf");
            envGuiRetina->setFontSize(OFX_UI_FONT_SMALL, 3* pixelDPI);
            envGuiRetina->setFontSize(OFX_UI_FONT_MEDIUM, 5* pixelDPI);
            envGuiRetina->setFontSize(OFX_UI_FONT_LARGE, 4* pixelDPI);
            envGuiRetina->addLabel("Environment");
        }
        
        //            envGuiRetina->addLabel("Environment");
        envGuiRetina->addSlider("Influence", 0,1.0, &particles.dataObject.elWeight);
        envGuiRetina->addLabel("data", "Temp: 100", OFX_UI_FONT_SMALL)->setAutoSize(false);
        //            commGuiRetina->addLabel("Communication");
        commGuiRetina->addSlider("Influence", 0,1.0, &particles.dataObject.langWeight);
        commGuiRetina->addLabel("data", "Trending Topics", OFX_UI_FONT_SMALL)->setAutoSize(false);
        //cout << "FONT LOADED? "<<bFontLoaded << " LOADED?"<<endl;
        //            collabGuiRetina->addLabel("Community");
        collabGuiRetina->addSlider("Influence", 0,1.0, &particles.dataObject.eiWeight);
        collabGuiRetina->addLabel("data", "Local Sound Level: 0", OFX_UI_FONT_SMALL)->setAutoSize(false);
    }
}

//--------------------------------------------------------------
void testApp::setupDataBar(){
    if ( envGui == NULL ){
        envGui = new ofxUICanvas(0,0,100,50);
        envGui->disableAppEventCallbacks();
        envGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
        envGui->enableAppUpdateCallback();
        envGui->setGlobalSliderHeight(5.0);
        envGui->setWidgetSpacing(4);
        
        commGui = new ofxUICanvas(100,0,100,50);
        commGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
        commGui->disableAppEventCallbacks();
        commGui->enableAppUpdateCallback();
        commGui->setGlobalSliderHeight(5.0);
        commGui->setWidgetSpacing(4);
        
        collabGui = new ofxUICanvas(200,0,100,50);
        collabGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
        collabGui->disableAppEventCallbacks();
        collabGui->enableAppUpdateCallback();
        collabGui->setGlobalSliderHeight(5.0);
        collabGui->setWidgetSpacing(4);
        
        static bool bLoadedFonts = false;
        if ( !bLoadedFonts ){
            bLoadedFonts = commGui->setFont("fonts/monaco.ttf");
            commGui->setFontSize(OFX_UI_FONT_SMALL, 3);
            commGui->setFontSize(OFX_UI_FONT_MEDIUM, 5);
            commGui->setFontSize(OFX_UI_FONT_LARGE, 4);
            commGui->addLabel("Communication");
            
            bLoadedFonts = collabGui->setFont("fonts/monaco.ttf");
            collabGui->setFontSize(OFX_UI_FONT_SMALL, 3);
            collabGui->setFontSize(OFX_UI_FONT_MEDIUM, 5);
            collabGui->setFontSize(OFX_UI_FONT_LARGE, 4);
            collabGui->addLabel("Community");
            
            
            bLoadedFonts = envGui->setFont("fonts/monaco.ttf");
            envGui->setFontSize(OFX_UI_FONT_SMALL, 3);
            envGui->setFontSize(OFX_UI_FONT_MEDIUM, 5);
            envGui->setFontSize(OFX_UI_FONT_LARGE, 4);
            envGui->addLabel("Environment");
        }
        
        //            envGui->addLabel("Environment");
        envGui->addSlider("Influence", 0,1.0, &particles.dataObject.elWeight);
        envGui->addLabel("data", "Temp: 100", OFX_UI_FONT_SMALL)->setAutoSize(false);
        //            commGui->addLabel("Communication");
        commGui->addSlider("Influence", 0,1.0, &particles.dataObject.langWeight);
        commGui->addLabel("data", "Trending Topics", OFX_UI_FONT_SMALL)->setAutoSize(false);
        //cout << "FONT LOADED? "<<bFontLoaded << " LOADED?"<<endl;
        //            collabGui->addLabel("Community");
        collabGui->addSlider("Influence", 0,1.0, &particles.dataObject.eiWeight);
        collabGui->addLabel("data", "Local Sound Level: 0", OFX_UI_FONT_SMALL)->setAutoSize(false);
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
