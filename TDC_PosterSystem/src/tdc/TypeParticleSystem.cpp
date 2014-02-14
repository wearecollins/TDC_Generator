//
//  TypeParticleSystem.cpp
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/6/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "TypeParticleSystem.h"


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
    
    // mouse interaction
    lastMass = 10.0f;
    meshUpdatingFrames = 0;
    bNeedToChangeMesh = false;
    bUseGrid = true;
    
    // behavior
    
    behaviors[ MOVE_NONE ] = NULL;
    behaviors[ MOVE_NOISE ] = new Noise();
    behaviors[ MOVE_WARP ] = new Warp();
    behaviors[ MOVE_FLOCK ] = NULL;
    behaviors[ MOVE_BUMP ] = new BumpMap();
    behaviors[ MOVE_BUMP ]->setup(NULL);
    
    //MOVE_GRAVITY,
    
    // null till we builds them
    currentMesh = NULL;
    
    // containers
    currentMeshBuffer = new ofVboMesh();
    
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
    
    // lez do it
    startThread();
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::threadedFunction(){
    static bool bIsSetup = false;
    
    bMeshIsUpdated = false;
    
    if (!bIsSetup){
        // setup grid
        grid.load(svgFile);
        outline.load(svgFile);
        
        ofMesh t_gridMesh     = grid.getMesh();
        ofMesh t_outlineMesh  = outline.getMesh();
        
        //drawMesh.setupIndicesAuto();
        
        ofColor color;
        color.setHue( ofMap( ofGetHours(), 0, 24, 0, 360.0f ));
        color.setSaturation(180.0f);
        
        int hour = ofGetHours();
        
#pragma mark TODO: ATTACH TO API!
        
        if ( hour < 9 || hour > 18 ){
            color.setBrightness(100);
        } else if ( hour > 9 && hour < 6 ){
            int diff = 12 - hour;
            color.setBrightness( ofMap(diff, 0, 6, 360, 100));
        }
        
        // set up letter containers
        for ( int i=0; i< grid.getNumLetters(); i++){
            letterGridParticles.push_back(vector<QuickVertex>());
            letterOutlineParticles.push_back(vector<QuickVertex>());
        }
        
        // build 10000 particles for grid
        for (int i=0; i<10000; i++){
            int index = (int) ofRandom(0, t_gridMesh.getNumVertices());
            while ( grid.isOccupied(index)){
                index = (int) ofRandom(0, t_gridMesh.getNumVertices());
            }
            int letter = grid.getLetterByParticle( index );
            
            QuickVertex qv;
            qv.pos      = t_gridMesh.getVertex(index);
            qv.index    = i;
            
            letterGridParticles[letter].push_back(qv);
            
            grid.occupyIndex(index);
            TypeParticle * t = new TypeParticle(t_gridMesh.getVertex(index));
            t->velocity = ofVec2f(ofRandom(-100,100));
            t->mass = 50.0f;
            t->damping  = .9f;
            
            addParticle(t);
            bufferGridMesh.addVertex(t_gridMesh.getVertex(index));
            ofColor localColor = color;
            color.setHue( color.getHue() + sin(i) * 50.0f );
            bufferGridMesh.addColor( localColor );
        }
        _particlesGrid = _particles;
        clear();
        
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
            
            letterOutlineParticles[letter].push_back(qv);
            
            outline.occupyIndex(index);
            TypeParticle * t = new TypeParticle(t_outlineMesh.getVertex(index));
            //t->velocity = ofVec2f(ofRandom(-100,100));
            t->mass = 50.0f;
            t->damping  = .9f;
            
            addParticle(t);
            bufferOutlineMesh.addVertex(t_outlineMesh.getVertex(index));
            ofColor localColor = color;
            color.setHue( color.getHue() + sin(i) * 50.0f );
            bufferOutlineMesh.addColor( localColor );
        }
        _particlesOutline = _particles;
        clear();
        
        lastDrawMode = DRAW_NULL;
        drawMode = DRAW_POINTS;
        
        buildMeshes();
        
        bIsSetup = true;
    }
    
    while (isThreadRunning()){
        lock();
        
        // some behaviors need everybody
        if ( currentBehavior != NULL ){
            currentBehavior->updateAll(&_particles);
        }
        
        ofxLabFlexParticleSystem::update();
        
        bMeshIsUpdated = false;
        
        // FIRST!
        if ( bNeedToChangeMesh ){
            if ( bUseGrid ){
                currentTypeMesh = &grid;
                currentLetterParticles = &letterGridParticles;
                _particles = _particlesGrid;
            } else {
                currentTypeMesh = &outline;
                currentLetterParticles = &letterOutlineParticles;
                _particles = _particlesOutline;
            }
            currentMeshBuffer = &meshes[ drawMode ][ bUseGrid ? GRID_POINTS : GRID_OUTLINE ];
            
            if ( behaviors[ MOVE_FLOCK ]  != NULL ) ((Flocking*) behaviors[ MOVE_FLOCK ])->setLetters( &_particles );
        }
        
        // update mesh based on positionsIterator it;
        int i=0;
        
        for( it = _particles.begin(); it != _particles.end(); ++it )
        {
            TypeParticle * p = (TypeParticle*)(it->second);
            
            if ( currentBehavior != NULL ){
                currentBehavior->update(p);
            }
            
            /*if( moveType == MOVE_NOISE ){
                
            } else if ( moveType == MOVE_FLOCK ){
                
            } else if ( moveType == MOVE_GRAVITY ){
                (*p).velocity += gravity;
            } else if ( moveType == MOVE_WARP ){
                //(*p) += ofVec2f( ofSignedNoise(p->seedPosition.x + ofGetElapsedTimef() * .1), ofSignedNoise(p->seedPosition.x + ofGetElapsedTimef() * .2) * 50.0f);
                //(*p) += ofVec2f( 0, sin(p->seedPosition.x + ofGetElapsedTimeMillis() * .2) * 10.0);
                (*p) += ofVec2f( 0, ofSignedNoise((p->seedPosition.x / ofGetWidth() * 10.0f) + ofGetElapsedTimeMillis() * .001) * 50.0);
            }*/
            
            currentMeshBuffer->setVertex(i, *it->second);
            i++;
        }
        
        bMeshIsUpdated = true;
        bNeedToChangeMesh = false;
        meshUpdatingFrames = 0;
        lastDrawMode = drawMode;
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
    }
    //gridMesh = bufferGridMesh;
    //outlineMesh = bufferOutlineMesh;
    currentBehavior = behaviors[ moveType ];
    unlock();
    
    /*if ( bUseGrid ){
        currentMesh = &gridMesh;
    } else {
        currentMesh = &outlineMesh;
    }*/
    
    currentMesh = currentMeshBuffer;
    
    if ( currentMesh && currentMesh->getNumVertices() > 0 ){
        if ( currentBehavior != NULL ){
            currentBehavior->beginDraw();
        }
        currentMesh->draw();
        
        if ( currentBehavior != NULL ){
            currentBehavior->endDraw();
        }
    }
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::draw()
{
    if ( currentMesh && currentMesh->getNumVertices() > 0 ){
        if ( currentBehavior == NULL || (currentBehavior->getName() != "flocking" && currentBehavior->getName() != "bumpmap")) currentMesh->draw();
        if ( currentBehavior != NULL ){
            currentBehavior->draw();
        }
    }
    meshUpdatingFrames++;
    if ( meshUpdatingFrames > 3 ){
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
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::buildMesh(DrawMode mode, GridType type ){
    meshes[ mode ][ type ] = ofVboMesh();

    ofMesh * mesh = &meshes[ mode ][ type ];
    vector<vector <QuickVertex> > * letterParticles = type == GRID_POINTS ? &letterGridParticles : &letterOutlineParticles;
    
    // FIRST: try to load this mesh
    string folder = "meshes/";
    string file = folder + "mesh_" + ofToString( mode ) + "_" + ofToString( type );
    
    mesh->load( file );
    
    bool bLoad = mesh->getVertices().size() > 0;
    
    if ( bLoad ){
        ofLogWarning()<<"Load success "<<mode<<":"<<type;
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
    
    switch (mode) {
        case DRAW_LINES:
            mesh->setMode(OF_PRIMITIVE_LINES);
            
            mesh->clearIndices();
            
            // method 1: randomize
            if ( type == GRID_OUTLINE ){
                for (int j=0; j<letterParticles->size(); j++){
                    for (int k=0; k<(*letterParticles)[j].size(); k++){
                        mesh->addIndex((*letterParticles)[j][k].index);
                        int ind = floor(ofRandom((*letterParticles)[j].size()));
                        mesh->addIndex((*letterParticles)[j][ind].index);
                    }
                }
            } else {
                // method 2: attach to anywhere inside letter
                
                for (int j=0; j<letterParticles->size(); j++){
                    QuickVertex qv = (*letterParticles)[j][ofRandom((*letterParticles)[j].size())];
                    attach.push_back(qv.index);
                }
                
                for( it = _particlesGrid.begin(); it != _particlesGrid.end(); ++it )
                {
                    int letterIndex = typeMesh->getLetterByParticle(typeMesh->getParticleIndex(((TypeParticle*)it->second)->getStart()));
                    mesh->addIndex(attach[letterIndex]);
                    mesh->addIndex(i);
                    
                    i++;
                }
            }
            
            break;
            
        case DRAW_LINES_RANDOMIZED:
            mesh->setMode(OF_PRIMITIVE_LINES);
            
            mesh->clearIndices();
            
        {
            
            vector<vector <QuickVertex> >  tempLetters;
            for ( int j=0; j<letterParticles->size(); j++){
                tempLetters.push_back( vector<QuickVertex>() );
                for ( int k=0; k<(*letterParticles)[j].size(); k++ ){
                    tempLetters[j].push_back( (*letterParticles)[j][k]);
                }
            }
            
            for( it = particlePtr.begin(); it != particlePtr.end(); ++it )
            {
                int letterIndex = typeMesh->getLetterByParticle(typeMesh->getParticleIndex(((TypeParticle*)it->second)->getStart()));
                mesh->addIndex( i );
                
                int index = (int) ofRandom(tempLetters[letterIndex].size());
                QuickVertex pt = tempLetters[letterIndex][index];
                while (pt.pos == *it->second) {
                    index = (int) ofRandom(tempLetters[letterIndex].size());
                    pt = tempLetters[letterIndex][index];
                }
                
                mesh->addIndex(pt.index);
                tempLetters[letterIndex].erase(tempLetters[letterIndex].begin() + index);
                
                i++;
            }
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
            mesh->setMode(OF_PRIMITIVE_POINTS);
            mesh->clearIndices();
            for( it = particlePtr.begin(); it != particlePtr.end(); ++it )
            {
                mesh->addIndex(i);
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
                mesh->addIndex(i);
                Iterator it2;
                int j=0;
                for( it2 = particlePtr.begin(); it2 != particlePtr.end(); ++it2 )
                {
                    if ( it2 != it ){
                        if ( it->second->distance( *it2->second ) < 10 ){
                            mesh->addIndex(j);
                            break;
                        }
                    }
                    j++;
                }
                
                j=0;
                
                Iterator it3;
                for( it3 = particlePtr.begin(); it3 != particlePtr.end(); ++it3 )
                {
                    if ( it3 != it && it3 != it2 ){
                        if ( it->second->distance( *it3->second ) < 20 ){
                            mesh->addIndex(j);
                            break;
                        }
                    }
                    j++;
                }
                i++;
            }
            break;
            
        default:
            break;
    }
    
    // save that shit
    mesh->save(file, false);
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setUseGrid( bool useGrid ){
    bool last = bUseGrid;
    bUseGrid = useGrid;
    if ( bUseGrid != last ){
        bNeedToChangeMesh = true;
    }
}

//-------------------------------------------------------------------------------------------
Behavior * TypeParticleSystem::getCurrentBehavior(){
    return currentBehavior;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::setBehavior( MovementType type ){
    moveType = type;
}

//-------------------------------------------------------------------------------------------
void TypeParticleSystem::mouseMoved( int x, int y ){
    if (!bMeshIsUpdated) return;
    
    ofVec3f mp = ofVec3f(x,y,0);
    float mouseMass = (lastMass * .9 + mp.distance(lastMouse) * .8);
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