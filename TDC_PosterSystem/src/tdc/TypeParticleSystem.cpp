//
//  TypeParticleSystem.cpp
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "TypeParticleSystem.h"

ofMesh test;

//-------------------------------------------------------------------------------------------
TypeParticleSystem::~TypeParticleSystem(){
    waitForThread();
    stopThread();
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setup( string file ){
    
    // setup ourselves based on window size
    setupSquare(ofVec2f(ofGetWidth(), ofGetHeight()));
    setOption(ofxLabFlexParticleSystem::HORIZONTAL_WRAP, false);
    setOption(ofxLabFlexParticleSystem::VERTICAL_WRAP, false);
    setOption( ofxLabFlexParticleSystem::DETECT_COLLISIONS, false);
    
    svgFile = file;
    
    ofDirectory svgSettingsFolder("settings/"+svgFile);
    if ( !svgSettingsFolder.exists() ) svgSettingsFolder.create();
    
    // mouse interaction
    lastMass = 10.0f;
    meshUpdatingFrames = 0;
    bNeedToChangeMesh = false;
    bUseGrid = true;
    scale = 1.0f;
    
    // behavior
    
    behaviors[ MOVE_NONE ] = new Behavior();
    behaviors[ MOVE_NOISE ] = new Noise();
    behaviors[ MOVE_WARP ] = new Warp();
    behaviors[ MOVE_FLOCK ] = NULL;
    behaviors[ MOVE_BUMP ] = new BumpMap();
    behaviors[ MOVE_PUSH ] = new CameraWarp();
    
    camera.setup();
    behaviors[ MOVE_BUMP ]->setup(NULL);
    behaviors[ MOVE_BUMP ]->camera = &camera;
    behaviors[ MOVE_PUSH ]->setup(NULL);
    behaviors[ MOVE_PUSH ]->camera = &camera;
    
    // setup data object
    dataObject.setup();
    
    //MOVE_GRAVITY,
    
    // null till we builds them
    currentMesh = new ofMesh();
    
    // containers
    currentMeshBuffer = new ofMesh();
    
    if ( bUseGrid ){
        currentTypeMesh = &grid;
        currentLetterParticles = &letterGridParticles;
    } else {
        currentTypeMesh = &outline;
        currentLetterParticles = &letterOutlineParticles;
    }
    
    moveType = MOVE_FLOCK;
    
    currentBehavior = behaviors[ moveType ];
    gravity  = ofVec2f(0,4.0f);
    
    // test mesh to decide to start from scratch or not
    test.load("meshes/mesh_2_0");
    test.setMode(OF_PRIMITIVE_TRIANGLES);
    
    
    // lez do it
    startThread();
}

/********************************************************************************************
  __  __    _    ___ _   _   _____ _   _ ____  _____    _    ____
 |  \/  |  / \  |_ _| \ | | |_   _| | | |  _ \| ____|  / \  |  _ \
 | |\/| | / _ \  | ||  \| |   | | | |_| | |_) |  _|   / _ \ | | | |
 | |  | |/ ___ \ | || |\  |   | | |  _  |  _ <| |___ / ___ \| |_| |
 |_|  |_/_/   \_\___|_| \_|   |_| |_| |_|_| \_\_____/_/   \_\____/
 
 ********************************************************************************************/

void TypeParticleSystem::threadedFunction(){
    static bool bIsSetup = false;
    
    bMeshIsUpdated = false;
    
    if (!bIsSetup){
        // setup grid
        ofxXmlSettings gridParticleSettings;
        bool bLoaded = gridParticleSettings.load("settings/"+svgFile+"/grid_particles.xml");
        
        ofxXmlSettings gridOutlineSettings;
        bool bLoaded2 = gridOutlineSettings.load("settings/"+svgFile+"/type_grid_settings.xml");
        int numLetters = 0;
        
        if (!bLoaded2 || !bLoaded || test.getNumVertices() == 0){
            grid.load(svgFile);
            outline.load(svgFile);
            numLetters = outline.getNumLetters();
            gridOutlineSettings.addValue("numLetter", numLetters);
            gridOutlineSettings.save("settings/"+svgFile+"/type_grid_settings.xml");
        } else {
            numLetters = gridOutlineSettings.getValue("numLetter", numLetters);
        }
        
        ofMesh t_gridMesh     = grid.getMesh();
        ofMesh t_outlineMesh  = outline.getMesh();
        
        //drawMesh.setupIndicesAuto();
        
#pragma mark TODO: ATTACH TO API!
        
        ofColor color;
        color.setHue( ofMap( ofGetHours(), 0, 24, 0, 360.0f ));
        color.setSaturation(180.0f);
        
        int hour = ofGetHours();
        
        if ( hour < 9 || hour > 18 ){
            color.setBrightness(100);
        } else if ( hour > 9 && hour < 6 ){
            int diff = 12 - hour;
            color.setBrightness( ofMap(diff, 0, 6, 360, 100));
        }
        
        // set up letter containers
        for ( int i=0; i< numLetters; i++){
            letterGridParticles.push_back(vector<QuickVertex>());
            letterOutlineParticles.push_back(vector<QuickVertex>());
        }
        
        if ( !bLoaded ){
            
            // build 10000 particles for grid
            for (int i=0; i<7000; i++){
                int index = (int) ofRandom(0, t_gridMesh.getNumVertices());
                int tries = 0;
                while ( grid.isOccupied(index) && tries < 100){
                    index = (int) ofRandom(0, t_gridMesh.getNumVertices());
                    tries++;
                }
                int letter = grid.getLetterByParticle( index );
                
                QuickVertex qv;
                qv.pos      = t_gridMesh.getVertex(index);
                qv.index    = i;
                qv.bInterior = grid.isParticleInterior(index);
                
                letterGridParticles[letter].push_back(qv);
                
                grid.occupyIndex(index);
                TypeParticle * t = new TypeParticle(t_gridMesh.getVertex(index));
                t->velocity = ofVec3f(ofRandom(-100,100),0,0);
                t->mass = 50.0f;
                t->damping  = .9f;
                t->index = i;
                
                addParticle(t);
                bufferGridMesh.addVertex(t_gridMesh.getVertex(index));
                ofColor localColor = color;
                color.setHue( color.getHue() + sin(i) * 50.0f );
                bufferGridMesh.addColor( localColor );
                
                gridParticleSettings.addTag("particle");
                gridParticleSettings.pushTag("particle", i); {
                    gridParticleSettings.addValue("x", t->x );
                    gridParticleSettings.addValue("y", t->y );
                    gridParticleSettings.addValue("z", t->z );
                    
                    gridParticleSettings.addValue("grid_index", index);
                    gridParticleSettings.addValue("letter", letter);
                    gridParticleSettings.addValue("isInterior", qv.bInterior);
                    
                    gridParticleSettings.addValue("vel_x", t->velocity.x);
                    gridParticleSettings.addValue("vel_y", t->velocity.y);
                    gridParticleSettings.addValue("mass", t->mass);
                    gridParticleSettings.addValue("damping", t->damping);
                    gridParticleSettings.addValue("index", t->index);
                    
                    gridParticleSettings.addValue("r", color.r );
                    gridParticleSettings.addValue("g", color.g );
                    gridParticleSettings.addValue("b", color.b );
                } gridParticleSettings.popTag();
            }
            gridParticleSettings.save("settings/"+svgFile+"/grid_particles.xml");
        } else {
            int n = gridParticleSettings.getNumTags("particle");
            for (int i=0; i<n; i++){
                gridParticleSettings.pushTag("particle", i); {
                    ofVec3f pos;
                    pos.x = gridParticleSettings.getValue("x", pos.x );
                    pos.y = gridParticleSettings.getValue("y", pos.y );
                    pos.z = gridParticleSettings.getValue("z", pos.z );
                    
                    int index = gridParticleSettings.getValue("grid_index", 0);
                    int letter = gridParticleSettings.getValue("letter", 0);
                    
                    TypeParticle * t = new TypeParticle(pos);
                    
                    t->velocity.x = gridParticleSettings.getValue("vel_x", t->velocity.x);
                    t->velocity.y= gridParticleSettings.getValue("vel_y", t->velocity.y);
                    
                    t->mass = gridParticleSettings.getValue("mass", t->mass);
                    t->damping = gridParticleSettings.getValue("damping", t->damping);
                    t->index = gridParticleSettings.getValue("index", 0);
                    
                    addParticle(t);
                    bufferGridMesh.addVertex(pos);
                    ofColor localColor = color;
                    color.setHue( color.getHue() + sin(i) * 50.0f );
                    bufferGridMesh.addColor( localColor );
                    
                    QuickVertex qv;
                    qv.pos      = pos;
                    qv.index    = t->index;
                    qv.bInterior = gridParticleSettings.getValue("isInterior", false);
                    letterGridParticles[letter].push_back(qv);
                    
                    //gridParticleSettings.addValue("r", color.r );
                    //gridParticleSettings.addValue("g", color.g );
                    //gridParticleSettings.addValue("b", color.b );
                } gridParticleSettings.popTag();
            }
            cout << "LOADED "<<n<<" PARTICLES"<<endl;
        }
        _particlesGrid = _particles;
        _particles.clear();
        
        ofxXmlSettings outlineParticleSettings;
        bLoaded = outlineParticleSettings.load("settings/"+svgFile+"/outline_particles.xml");
        
        if ( !bLoaded ){
            
            // add 8000 particles for outline
            for (int i=0; i<7000; i++){
                int index = (int) ofRandom(0, t_outlineMesh.getNumVertices());
                while ( outline.isOccupied(index) && !outline.isFull() ){
                    index = (int) ofRandom(0, t_outlineMesh.getNumVertices());
                }
                int letter = outline.getLetterByParticle( index );
                
                QuickVertex qv;
                qv.pos      = t_outlineMesh.getVertex(index);
                qv.index    = i;
                qv.bInterior = outline.isParticleInterior(index);
                
                letterOutlineParticles[letter].push_back(qv);
                
                outline.occupyIndex(index);
                TypeParticle * t = new TypeParticle(t_outlineMesh.getVertex(index));
                //t->velocity = ofVec2f(ofRandom(-100,100));
                t->mass = 50.0f;
                t->damping  = .9f;
                t->index = i;
                
                addParticle(t);
                bufferOutlineMesh.addVertex(t_outlineMesh.getVertex(index));
                ofColor localColor = color;
                color.setHue( color.getHue() + sin(i) * 50.0f );
                bufferOutlineMesh.addColor( localColor );
                
                outlineParticleSettings.addTag("particle");
                outlineParticleSettings.pushTag("particle", i); {
                    outlineParticleSettings.addValue("x", t->x );
                    outlineParticleSettings.addValue("y", t->y );
                    outlineParticleSettings.addValue("z", t->z );
                    
                    outlineParticleSettings.addValue("grid_index", index);
                    outlineParticleSettings.addValue("letter", letter);
                    outlineParticleSettings.addValue("isInterior", qv.bInterior);
                    
                    outlineParticleSettings.addValue("vel_x", t->velocity.x);
                    outlineParticleSettings.addValue("vel_y", t->velocity.y);
                    outlineParticleSettings.addValue("mass", t->mass);
                    outlineParticleSettings.addValue("damping", t->damping);
                    outlineParticleSettings.addValue("index", t->index);
                    
                    outlineParticleSettings.addValue("r", color.r );
                    outlineParticleSettings.addValue("g", color.g );
                    outlineParticleSettings.addValue("b", color.b );
                } outlineParticleSettings.popTag();
            }
            outlineParticleSettings.save("settings/"+svgFile+"/outline_particles.xml");
        } else {
            int n = outlineParticleSettings.getNumTags("particle");
            for (int i=0; i<n; i++){
                outlineParticleSettings.pushTag("particle", i); {
                    ofVec3f pos;
                    pos.x = outlineParticleSettings.getValue("x", pos.x );
                    pos.y = outlineParticleSettings.getValue("y", pos.y );
                    pos.z = outlineParticleSettings.getValue("z", pos.z );
                    
                    int index = outlineParticleSettings.getValue("grid_index", 0);
                    int letter = outlineParticleSettings.getValue("letter", 0);
                    
                    TypeParticle * t = new TypeParticle(pos);
                    
                    t->velocity.x = outlineParticleSettings.getValue("vel_x", t->velocity.x);
                    t->velocity.y= outlineParticleSettings.getValue("vel_y", t->velocity.y);
                    
                    t->mass = outlineParticleSettings.getValue("mass", t->mass);
                    t->damping = outlineParticleSettings.getValue("damping", t->damping);
                    t->index = outlineParticleSettings.getValue("index", 0);
                    
                    addParticle(t);
                    bufferOutlineMesh.addVertex(pos);
                    ofColor localColor = color;
                    color.setHue( color.getHue() + sin(i) * 50.0f );
                    bufferOutlineMesh.addColor( localColor );
                    
                    QuickVertex qv;
                    qv.pos      = pos;
                    qv.index    = t->index;
                    qv.bInterior = outlineParticleSettings.getValue("isInterior", false);
                    letterOutlineParticles[letter].push_back(qv);
                    
                    //outlineParticleSettings.addValue("r", color.r );
                    //outlineParticleSettings.addValue("g", color.g );
                    //outlineParticleSettings.addValue("b", color.b );
                } outlineParticleSettings.popTag();
            }
            cout << "LOADED "<<n<<" PARTICLES"<<endl;
        }
        _particlesOutline = _particles;
        _particles.clear();
        
        lastDrawMode = DRAW_NULL;
        drawMode = DRAW_POINTS;
        
        buildMeshes();
        
        currentMeshBuffer->append( meshes[ drawMode ][ bUseGrid ? GRID_POINTS : GRID_OUTLINE ]);
        
        bIsSetup = true;
    }
    
    while (isThreadRunning()){
        
        // some behaviors need everybody
        if ( currentBehavior != NULL ){
            currentBehavior->copySettings( behaviors[ MOVE_NONE ]);
            currentBehavior->updateAll(&_particles);
        }
        
        // FIRST!
        if ( bNeedToChangeMesh || lastDrawMode != drawMode ){
            
            lock();
            
            if ( bUseGrid ){
                _particles = _particlesGrid;
            } else {
                _particles = _particlesOutline;
            }
            
            
            //TODO: THIS SHOULD BE JUST RESETTING TARGET VERTICES!!!!
            
            currentMeshBuffer->clear();
            currentMeshBuffer->append( meshes[ drawMode ][ bUseGrid ? GRID_POINTS : GRID_OUTLINE ]);
            currentMeshBuffer->setMode( meshes[ drawMode ][ bUseGrid ? GRID_POINTS : GRID_OUTLINE ].getMode());
            lastDrawMode = drawMode;
            
            if ( behaviors[ MOVE_FLOCK ]  != NULL ) ((Flocking*) behaviors[ MOVE_FLOCK ])->setLetters( &_particles );
            unlock();
        }
        
        // Colors
        if ( bNeedToChangeColor ){
            lock();
            ofFloatColor currentColor = particleColor;
            float currentVariance = colorVariance;
            
            for (int i=0; i<currentMeshBuffer->getNumColors(); i++){
                ofFloatColor localColor = currentColor;
                currentColor.setHue( localColor.getHue() + ofRandom(-currentVariance,currentVariance));
                currentMeshBuffer->setColor(i, localColor);
            }
            bNeedToChangeColor = false;
            if ( behaviors[ MOVE_FLOCK ] != NULL ) behaviors[ MOVE_FLOCK ]->copyMesh(currentMesh);
            unlock();
        }
        
        lock();
        ofxLabFlexParticleSystem::update();
        
        bMeshIsUpdated = false;
        
        // update mesh based on positionsIterator it;
        
        for( it = _particles.begin(); it != _particles.end(); ++it )
        {
            TypeParticle * p = (TypeParticle*)(it->second);
            
            if ( currentBehavior != NULL ){
                currentBehavior->update(p);
            }
            currentMeshBuffer->setVertex(p->index, *p);
        }
        
        bMeshIsUpdated = true;
        bNeedToChangeMesh = false;
        meshUpdatingFrames = 0;
        unlock();
        
        sleep(16.6667); // run at ~60fps
    }
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::update(){
    // leave the mesh alone until it's done
    if (!bMeshIsUpdated) return;
    
    lock();
    
    if ( currentMesh && currentMesh->getNumVertices() > 0 && behaviors[ MOVE_FLOCK ] == NULL){
        behaviors[ MOVE_FLOCK ] = new Flocking(&_particles);
        behaviors[ MOVE_FLOCK ]->copyMesh(currentMesh);
    }
    //gridMesh = bufferGridMesh;
    //outlineMesh = bufferOutlineMesh;
    currentMesh->clear();
    currentMesh->append(*currentMeshBuffer);
    currentMesh->setMode(currentMeshBuffer->getMode());
    currentBehavior = behaviors[ moveType ];
    unlock();
    
    /*if ( bUseGrid ){
        currentMesh = &gridMesh;
    } else {
        currentMesh = &outlineMesh;
    }*/
    
    // update stuff based on grid
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::draw()
{
    if ( currentMesh && currentMesh->getNumVertices() > 0 ){
        
        if ( currentBehavior == NULL ){
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
            ofScale(scale, scale);
            ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
            currentMesh->draw();
            ofPopMatrix();
        } else {
            ofSetColor(255);
            if ( currentMesh && currentMesh->getNumVertices() > 0 ){
                if ( currentBehavior->getName() != "flocking" ){
                    ofPushMatrix();
                    ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
                    ofScale(scale, scale);
                    ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
                }
                if ( currentBehavior != NULL ){
                    currentBehavior->scale = scale;
                    currentBehavior->beginDraw();
                }
                if ( currentBehavior->getName() != "flocking" ) currentMesh->draw();
                
                if ( currentBehavior != NULL ){
                    currentBehavior->endDraw();
                }
                if ( currentBehavior->getName() != "flocking" ){
                    ofPopMatrix();
                }
            }
        }
        
        if ( currentBehavior != NULL ){
            currentBehavior->draw();
        }
    }
    meshUpdatingFrames++;
    if ( meshUpdatingFrames > 10 ){
        ofSetColor(0,50);
        ofRect(0,0,ofGetWidth(), ofGetHeight());
        ofSetColor(255);
        string str = "LOADING";
        int n = abs(sin(ofGetElapsedTimeMillis() * .01)) * 4;
        for ( int i=0; i<n; i++){
            str += ".";
        }
        
        ofDrawBitmapString(str, ofGetWidth()/2.0, ofGetHeight()/2.0);
    }
}


//-------------------------------------------------------------------------------------------
TypeParticleSystem::DrawMode TypeParticleSystem::getDrawMode(){
    return drawMode;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setDrawMode( DrawMode mode )
{
    // ignoring if updating
    if (!bMeshIsUpdated) return;
    lastDrawMode = drawMode;
    drawMode = mode;
}

//-------------------------------------------------------------------------------------------
string TypeParticleSystem::getDrawModeString(){
    switch (drawMode) {
        case DRAW_NULL:
            return "NULL";
            break;
            
        case DRAW_POINTS:
            return "Points";
            break;
            
        case DRAW_LINES:
            return "Lines to letter point";
            break;
            
        case DRAW_LINES_RANDOMIZED:
            return "Lines Randomized";
            break;
            
        case DRAW_LINES_ARBITARY:
            return "Lines to point in space";
            break;
            
        case DRAW_SHAPES:
            return "Randomized shapes";
            break;
            
        default:
            return "Undefined";
            break;
    }
}

/********************************************************************************************
  __  __ ______  _____ _    _    _____ ______ _   _
 |  \/  |  ____|/ ____| |  | |  / ____|  ____| \ | |
 | \  / | |__  | (___ | |__| | | |  __| |__  |  \| |
 | |\/| |  __|  \___ \|  __  | | | |_ |  __| | . ` |
 | |  | | |____ ____) | |  | | | |__| | |____| |\  |
 |_|  |_|______|_____/|_|  |_|  \_____|______|_| \_|
 
 ********************************************************************************************/

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::buildMeshes(){
    /*
    DRAW_POINTS = 0,
    DRAW_LINES,
    DRAW_LINES_RANDOMIZED,
    DRAW_LINES_ARBITARY,
    DRAW_SHAPES
    */
    
    buildMesh(DRAW_POINTS, GRID_POINTS);
    buildMesh(DRAW_POINTS, GRID_OUTLINE);
    
    buildMesh(DRAW_LINES, GRID_POINTS);
    buildMesh(DRAW_LINES, GRID_OUTLINE);
    
    buildMesh(DRAW_LINES_RANDOMIZED, GRID_POINTS);
    buildMesh(DRAW_LINES_RANDOMIZED, GRID_OUTLINE);
    
    buildMesh(DRAW_LINES_ARBITARY, GRID_POINTS);
    buildMesh(DRAW_LINES_ARBITARY, GRID_OUTLINE);
    
    buildMesh(DRAW_SHAPES, GRID_POINTS);
    buildMesh(DRAW_SHAPES, GRID_OUTLINE);
    
    cout << "done building"<<endl;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::buildMesh(DrawMode mode, GridType type ){
    meshes[ mode ][ type ] = ofVboMesh();
    
    ofMesh * mesh = &meshes[ mode ][ type ];
    vector<vector <QuickVertex> > * letterParticles = type == GRID_POINTS ? &letterGridParticles : &letterOutlineParticles;
    
    // FIRST: try to load this mesh
    string folder = "meshes/" + svgFile.substr(0,svgFile.length()-4)+"/";
    string file = folder + "mesh_" + ofToString( mode ) + "_" + ofToString( type );
    
    mesh->load( file );
    
    bool bLoad = mesh->getVertices().size() > 0;
    
    if ( bLoad ){
        ofLogWarning()<<"Load success "<<mode<<":"<<type;
        
        switch (mode) {
            case DRAW_LINES:
                mesh->setMode(OF_PRIMITIVE_LINES);
                break;
                
            case DRAW_LINES_RANDOMIZED:
                mesh->setMode(OF_PRIMITIVE_LINES);
                break;
                
            case DRAW_LINES_ARBITARY:
                mesh->setMode(OF_PRIMITIVE_LINES);
                break;
                
            case DRAW_POINTS:
                mesh->setMode(OF_PRIMITIVE_POINTS);
                break;
                
            case DRAW_SHAPES:
                mesh->setMode(OF_PRIMITIVE_TRIANGLES);
                break;
                
            default:
                break;
        }
        
        return;
    }
    
    TypeMesh * typeMesh;
    
    // add vertices and colors
    switch (type) {
        case GRID_POINTS:
            typeMesh = &grid;
            for ( int i=0; i< bufferGridMesh.getNumVertices(); i++){
                meshes[ mode ][ type ].addVertex(bufferGridMesh.getVertex(i));
                meshes[ mode ][ type ].addColor(bufferGridMesh.getColor(i));
            }
            break;
            
        case GRID_OUTLINE:
            typeMesh = &outline;
            for ( int i=0; i< bufferOutlineMesh.getNumVertices(); i++){
                meshes[ mode ][ type ].addVertex(bufferOutlineMesh.getVertex(i));
                meshes[ mode ][ type ].addColor(bufferOutlineMesh.getColor(i));
            }
            break;
            
        default:
            break;
    }
    
    Container particlePtr;
    if ( type == GRID_OUTLINE ){
        particlePtr = _particlesOutline;
    } else {
        particlePtr = _particlesGrid;
    }
    
    
    int i=0;
    vector<int> attach;
    mesh->clearIndices();
    
    switch (mode) {
        case DRAW_LINES:
            mesh->setMode(OF_PRIMITIVE_LINES);
            mesh->clearIndices();
            
            // method 1: randomize
            //if ( type == GRID_OUTLINE ){
                for (int j=0; j<letterParticles->size(); j++){
                    
                    vector <QuickVertex> quickVerts = letterParticles->at(j);
                    
                    for (int k=0; k<(*letterParticles)[j].size(); k++){
                        mesh->addIndex((*letterParticles)[j][k].index);
                        
                        int ind = floor(ofRandom(quickVerts.size()));
                        mesh->addIndex(quickVerts[ind].index);
                        quickVerts.erase( quickVerts.begin() + ind );
                        if ( quickVerts.size() == 0 ) break;
                    }
                }
            /*} else {
                // method 2: attach to anywhere inside letter
                
                attach.clear();
                i = 0;
                
                for (int j=0; j<letterParticles->size(); j++){
                    QuickVertex qv = letterParticles->at(j).at(ofRandom(letterParticles->at(j).size()));
                    attach.push_back(qv.index);
                }
                
                for( it = _particlesGrid.begin(); it != _particlesGrid.end(); ++it )
                {
                    int letterIndex = typeMesh->getLetterByParticle(typeMesh->getParticleIndex(((TypeParticle*)it->second)->getStart()));
                    mesh->addIndex(attach[letterIndex]);
                    mesh->addIndex(i);
                    
                    i++;
                }
            }*/
            
            break;
            
        case DRAW_LINES_RANDOMIZED:
            mesh->setMode(OF_PRIMITIVE_LINES);
            
        {
            
            vector<vector <QuickVertex> >  tempLettersExt;
            vector<vector <QuickVertex> >  tempLettersInt;
            
            for ( int j=0; j<letterParticles->size(); j++){
                tempLettersExt.push_back( vector<QuickVertex>() );
                tempLettersInt.push_back( vector<QuickVertex>() );
                for ( int k=0; k<(*letterParticles)[j].size(); k++ ){
                    if ( (*letterParticles)[j][k].bInterior ){
                        tempLettersInt[j].push_back( (*letterParticles)[j][k]);
                    } else {
                        tempLettersExt[j].push_back( (*letterParticles)[j][k]);
                    }
                }
            }
            
            for (int j=0; j<tempLettersExt.size(); j++){
                for (int k=0; k<tempLettersExt[j].size(); k++){
                    mesh->addIndex( tempLettersExt[j][k].index );
                    if ( tempLettersInt[j].size() > 0 ){
                        
                        int min = 0;
                        float minDist = 10000;
                        
                        for ( int l=0; l<tempLettersInt[j].size(); l++){
                            float dist = tempLettersExt[j][k].pos.distance(tempLettersInt[j][l].pos);
                            if ( dist < minDist ){
                                min = l;
                                minDist = dist;
                            }
                        }
                        
                        mesh->addIndex( tempLettersInt[j][min].index );
                    } else {
                        int min = 0;
                        float minDist = 10000;
                        
                        for ( int l=0; l<tempLettersExt[j].size(); l++){
                            float dist = tempLettersExt[j][k].pos.distance(tempLettersExt[j][l].pos);
                            if ( dist < minDist && dist > 2 ){
                                min = l;
                                minDist = dist;
                            }
                        }
                        
                        mesh->addIndex( tempLettersExt[j][min].index );
                    }
                }
            }
            
            /*for( it = particlePtr.begin(); it != particlePtr.end(); ++it )
            {
                TypeParticle* p = (TypeParticle*)it->second;
                int letterIndex = typeMesh->getLetterByParticle(typeMesh->getParticleIndex(p->getStart()));
                mesh->addIndex( p->index );
                
                int index = (int) ofRandom(tempLettersExt[letterIndex].size());
                int tries = 100;
                
                QuickVertex pt = tempLettersExt[letterIndex][index];
                while (pt.pos == *it->second) {
                    index = (int) ofRandom(tempLettersExt[letterIndex].size());
                    pt = tempLettersExt[letterIndex][index];
                    tries--;
                    if (tries <= 0) continue;
                }
                
                mesh->addIndex(pt.index);
                tempLettersExt[letterIndex].erase(tempLettersExt[letterIndex].begin() + index);
            }*/
        }
            break;
            
        case DRAW_LINES_ARBITARY:
            mesh->setMode(OF_PRIMITIVE_LINES);
            
            mesh->clearIndices();
            
        {
            
            int lpIndex = ofRandom(letterParticles->size());
            QuickVertex qv = (*letterParticles)[lpIndex][ofRandom((*letterParticles)[lpIndex].size())];
            int attachIndex = qv.index;
            ofColor color = mesh->getColor(attachIndex);
            color.a = 0;
            mesh->setColor(attachIndex, color);
            
            for( it = particlePtr.begin(); it != particlePtr.end(); ++it )
            {
                int letterIndex = typeMesh->getLetterByParticle(typeMesh->getParticleIndex(((TypeParticle*)it->second)->getStart()));
                mesh->addIndex(attachIndex);
                
                int index = (int) ofRandom((*letterParticles)[letterIndex].size());
                QuickVertex pt = (*letterParticles)[letterIndex][index];
                while (pt.pos == *it->second) {
                    index = (int) ofRandom((*letterParticles)[letterIndex].size());
                    pt = (*letterParticles)[letterIndex][index];
                }
                
                mesh->addIndex(pt.index);
                
                i++;
            }
        }
            break;
            
            
        case DRAW_POINTS:
            cout << "draw points "<<endl;
            mesh->setMode(OF_PRIMITIVE_POINTS);
            mesh->clearIndices();
            for( it = particlePtr.begin(); it != particlePtr.end(); ++it )
            {
                TypeParticle* p = (TypeParticle*)it->second;
                mesh->addIndex(p->index);
                i++;
            }
            break;
            
        case DRAW_SHAPES:
            mesh->setMode(OF_PRIMITIVE_TRIANGLES);
            // to-do: triangulation!
            mesh->clearIndices();
            
            // method 1: randomize
            for( it = particlePtr.begin(); it != particlePtr.end(); ++it )
            {
                int ind1, ind2, ind3;
                ind1 = ind2 = ind3 = -1;
                
                TypeParticle* p = (TypeParticle*)it->second;
                if ( p == NULL ) continue;
                ind1 = p->index;
                
                TypeParticle* p2;
                TypeParticle* p3;
                
                
                Iterator it2;
                for( it2 = particlePtr.begin(); it2 != particlePtr.end(); ++it2 )
                {
                    if ( it2 != it ){
                        p2 = (TypeParticle*)it2->second;
                        if ( p->distance( *p2 ) < 10 ){
                            ind2 = p2->index;
                            break;
                        }
                    }
                }
                
                if ( p2 == NULL ) continue;
                
                Iterator it3;
                for( it3 = particlePtr.begin(); it3 != particlePtr.end(); ++it3 )
                {
                    if ( it3 != it && it3 != it2 ){
                        p3 = (TypeParticle*)it3->second;
                        if ( p2->distance( *p3 ) < 20 ){
                            ind3 = p3->index;
                            break;
                        }
                    }
                }
                if ( p3 == NULL ) continue;
                
                if ( ind1 != -1 && ind2 != -1 && ind3 != -1 ){
                    mesh->addIndex(ind1);
                    mesh->addIndex(ind2);
                    mesh->addIndex(ind3);
                }

            }
            break;
            
        default:
            break;
    }
    
    // save that shit
    mesh->save(file, false);
}


/********************************************************************************************
  ____  _____ _____ _____ ___ _   _  ____ ____
 / ___|| ____|_   _|_   _|_ _| \ | |/ ___/ ___|
 \___ \|  _|   | |   | |  | ||  \| | |  _\___ \
  ___) | |___  | |   | |  | || |\  | |_| |___) |
 |____/|_____| |_|   |_| |___|_| \_|\____|____/
 
 ********************************************************************************************/

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setColor( ofFloatColor color, float variance ){
    particleColor = color;
    colorVariance = variance;
    
    bNeedToChangeColor = true;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setUseGrid( bool useGrid ){
    bool last = bUseGrid;
    bUseGrid = useGrid;
    if ( bUseGrid != last ){
        bNeedToChangeMesh = true;
    }
}

#pragma mark behaviors
/********************************************************************************************
  ____  ______ _    _     __      _______ ____  _____   _____
 |  _ \|  ____| |  | |   /\ \    / /_   _/ __ \|  __ \ / ____|
 | |_) | |__  | |__| |  /  \ \  / /  | || |  | | |__) | (___
 |  _ <|  __| |  __  | / /\ \ \/ /   | || |  | |  _  / \___ \
 | |_) | |____| |  | |/ ____ \  /   _| || |__| | | \ \ ____) |
 |____/|______|_|  |_/_/    \_\/   |_____\____/|_|  \_\_____/

********************************************************************************************/

//-------------------------------------------------------------------------------------------
Behavior * TypeParticleSystem::getCurrentBehavior(){
    return currentBehavior;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setBehavior( MovementType type ){
    moveType = type;
}

//-------------------------------------------------------------------------------------------
Behavior *  TypeParticleSystem::getSettingsBehavior(){
    return behaviors[ MOVE_NONE ];
}


#pragma mark interaction
/********************************************************************************************
 ___ _   _ _____ _____ ____      _    ____ _____ ___ ___  _   _
 |_ _| \ | |_   _| ____|  _ \    / \  / ___|_   _|_ _/ _ \| \ | |
  | ||  \| | | | |  _| | |_) |  / _ \| |     | |  | | | | |  \| |
  | || |\  | | | | |___|  _ <  / ___ \ |___  | |  | | |_| | |\  |
 |___|_| \_| |_| |_____|_| \_\/_/   \_\____| |_| |___\___/|_| \_|
 
 ********************************************************************************************/

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::mouseMoved( int x, int y ){
    if (!bMeshIsUpdated) return;
    
    ofVec3f mp = ofVec3f(x,y,0);
    float mouseMass = (lastMass * .9 + mp.distance(lastMouse) * .3);
    lastMouse = mp;
    lastMass = mouseMass;
    
    lock();
    if ( currentBehavior != NULL ){
        currentBehavior->setMouse( lastMouse );
    }
    
    for( it = _particles.begin(); it != _particles.end(); ++it )
    {
        TypeParticle * p = (TypeParticle *) it->second;
        float distance = ofDist(x, y, p->x, p->y);
        
        if( distance > 100 ) {
            continue;
        }
        
        // ofVec2f - ofVec2f
        ofVec3f diff = mp -( *p );
        
        diff.normalize();
        
        // http://www.grantchronicles.com/astro09.htm
        // this is a really lose interpretation.. like not very close
        float force = mouseMass * p->mass / MAX(1, distance);
        float accel = force / p->mass;
        
        p->acceleration += diff * accel;
    }
    unlock();
}