//
//  TargetMesh.cpp
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/21/14.
//
//

#include "TargetMesh.h"

int numLetters = 0;

#pragma mark TargetMesh

//-------------------------------------------------------------------------------------------
void TargetMesh::setup( string directory, int num ){
    svgDirectory = directory;
    numPositions = num;
    
    // open directory
    ofDirectory dir;
    int numFiles = dir.listDir( svgDirectory +"/sources" );
    
    // build sets of meshes for each file in dir
    for (int i=0; i<numFiles; i++){
        subMeshes.push_back( SubMesh());
        string name = dir.getFile(i).getBaseName();
        cout <<"loading"<<name;
        subMeshes.back().setup( svgDirectory, name, numPositions);
    }
}

//-------------------------------------------------------------------------------------------
SubMesh & TargetMesh::getSubMesh( int index ){
    if ( index >= 0 && index < subMeshes.size()){
        return subMeshes[index];
    }
    ofLogWarning()<<"NO MESH AT THIS INDEX " + svgDirectory;
    static SubMesh dummyMesh;
    return dummyMesh;
}

//-------------------------------------------------------------------------------------------
ofMesh & TargetMesh::getMesh( int index, DrawMode mode, GridType type ){
    return subMeshes[index].meshes[mode][type];
}

#pragma mark SubMesh

//-------------------------------------------------------------------------------------------
void SubMesh::setup( string dir, string n, int num ){
    directory   = dir;
    name        = n;
    numPositions = num;
    
    // make sure main dir exists
    ofDirectory dirTest ( directory + "/" + name );
    if ( !dirTest.exists() ) dirTest.create();
    
    // build grids
    createGrid();
    
    // build position lookups
    
    ofMesh t_gridMesh     = grid.getMesh();
    ofMesh t_outlineMesh  = outline.getMesh();
    
    ofColor color(255);
    
    // set up letter containers
    for ( int i=0; i< numLetters; i++){
        letterGridParticles.push_back(vector<QuickVertex>());
        letterOutlineParticles.push_back(vector<QuickVertex>());
    }
    
// TODO: ADD XML LOAD HERE!!!!
    
    ofxXmlSettings gridParticleSettings;
    bool bLoaded = gridParticleSettings.load( dir + "/" + name + "/settings/grid_particles.xml" );
    
    if ( !bLoaded ){
        // build randomized grid
        for (int i=0; i<numPositions; i++){
            int index = (int) ofRandom(0, t_gridMesh.getNumVertices());
            int tries = 0;
            while ( grid.isOccupied(index) && tries < 100){
                index = (int) ofRandom(0, t_gridMesh.getNumVertices());
                tries++;
            }
            int letter = grid.getLetterByParticle( index );
            
            ofVec3f pos = t_gridMesh.getVertex(index);
            
            //setup letter-based lookup
            
            QuickVertex qv;
            qv.pos      = pos;
            qv.index    = i;
            qv.bInterior = grid.isParticleInterior(index);
            
            letterGridParticles[letter].push_back(qv);
            
            // setup mesh-based lookup
            gridMesh.addVertex( pos );
            gridMesh.addColor( color );
            
            grid.occupyIndex(index);
            
            gridParticleSettings.addTag("particle");
            gridParticleSettings.pushTag("particle", i); {
                gridParticleSettings.addValue("x", pos.x );
                gridParticleSettings.addValue("y", pos.y );
                gridParticleSettings.addValue("z", pos.z );
                
                gridParticleSettings.addValue("grid_index", index);
                gridParticleSettings.addValue("letter", letter);
                gridParticleSettings.addValue("isInterior", qv.bInterior);
            } gridParticleSettings.popTag();
        }
        gridParticleSettings.save( dir + "/" + name + "/settings/grid_particles.xml" );
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
                
                QuickVertex qv;
                qv.pos      = pos;
                qv.index    = index;
                qv.bInterior = gridParticleSettings.getValue("isInterior", false);
                letterGridParticles[letter].push_back(qv);
                
                gridMesh.addVertex( pos );
                gridMesh.addColor( color );
                gridMesh.addIndex( index );
                
            } gridParticleSettings.popTag();
        }
        cout << "LOADED "<<n<<" PARTICLES"<<endl;
    }
    
    ofxXmlSettings outlineParticleSettings;
    bLoaded = outlineParticleSettings.load( dir + "/" + name + "/settings/outline_particles.xml");
    
    if ( !bLoaded ){
        
        // add lookups for outline
        for (int i=0; i<7000; i++){
            int index = (int) ofRandom(0, t_outlineMesh.getNumVertices());
            while ( outline.isOccupied(index) && !outline.isFull() ){
                index = (int) ofRandom(0, t_outlineMesh.getNumVertices());
            }
            int letter = outline.getLetterByParticle( index );
            
            ofVec3f pos = t_outlineMesh.getVertex(index);
            
            // setup letter-based lookups
            
            QuickVertex qv;
            qv.pos      = pos;
            qv.index    = i;
            qv.bInterior = outline.isParticleInterior(index);
            letterOutlineParticles[letter].push_back(qv);
            
            // setup mesh-based lookup
            outlineMesh.addVertex( pos );
            outlineMesh.addColor( color );
            outlineMesh.addIndex( i );
            
            outline.occupyIndex(index);
            
            outlineParticleSettings.addTag("particle");
            outlineParticleSettings.pushTag("particle", i); {
                outlineParticleSettings.addValue("x", pos.x );
                outlineParticleSettings.addValue("y", pos.y );
                outlineParticleSettings.addValue("z", pos.z );
                
                outlineParticleSettings.addValue("grid_index", index);
                outlineParticleSettings.addValue("letter", letter);
                outlineParticleSettings.addValue("isInterior", qv.bInterior);
            } outlineParticleSettings.popTag();
        }
        outlineParticleSettings.save(dir + "/" + name + "/settings/outline_particles.xml");
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
                
                QuickVertex qv;
                qv.pos      = pos;
                qv.index    = index;
                qv.bInterior = outlineParticleSettings.getValue("isInterior", false);
                letterOutlineParticles[letter].push_back(qv);
                
                outlineMesh.addVertex( pos );
                outlineMesh.addColor( color );
                outlineMesh.addIndex( index );
                
            } outlineParticleSettings.popTag();
        }
    }
    
    buildMeshes();
}

//-------------------------------------------------------------------------------------------
void SubMesh::createGrid(){
    ofMesh test;
    cout<<"SVG "<<directory + "/sources/" + name + ".svg"<<endl;
    test.load( directory + "/" + name + "/meshes/mesh_2_0" );
    
    ofxXmlSettings gridOutlineSettings;
    bool bLoaded2 = gridOutlineSettings.load( directory +"/" + name + "/settings/type_grid_settings.xml");
    
    // need to build main grid
    if ( test.getNumVertices() == 0 ){
        grid.load( directory + "/sources/" + name +".svg", directory + "/meshes" );
        outline.load( directory + "/sources/" + name +".svg", directory + "/meshes"  );
        numLetters = outline.getNumLetters();
        gridOutlineSettings.addValue("numLetter", numLetters);
        gridOutlineSettings.save( directory + "/" + name + "/settings/type_grid_settings.xml");
    } else {
        numLetters = gridOutlineSettings.getValue("numLetter", numLetters);
    }
}

//-------------------------------------------------------------------------------------------
void SubMesh::buildMeshes(){
    
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
void SubMesh::buildMesh(DrawMode mode, GridType type ){
    meshes[ mode ][ type ] = ofVboMesh();
    
    ofMesh * mesh = &meshes[ mode ][ type ];
    vector<vector <QuickVertex> > * letterParticles = type == GRID_POINTS ? &letterGridParticles : &letterOutlineParticles;
    
    // FIRST: try to load this mesh
    string folder = directory + "/" + name + "/meshes/";
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
    ofMesh   * currentMesh;
    
    // add vertices and colors
    switch (type) {
        case GRID_POINTS:
            typeMesh = &grid;
            currentMesh = &gridMesh;
            for ( int i=0; i< gridMesh.getNumVertices(); i++){
                meshes[ mode ][ type ].addVertex(gridMesh.getVertex(i));
                meshes[ mode ][ type ].addColor(gridMesh.getColor(i));
            }
            break;
            
        case GRID_OUTLINE:
            typeMesh = &outline;
            currentMesh = &outlineMesh;
            for ( int i=0; i< outlineMesh.getNumVertices(); i++){
                meshes[ mode ][ type ].addVertex(outlineMesh.getVertex(i));
                meshes[ mode ][ type ].addColor(outlineMesh.getColor(i));
            }
            break;
            
        default:
            break;
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
            for (int j=0; j<numPositions; j++){
                
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
            
            for( int i=0; i < currentMesh->getNumVertices(); i++ )
            {
                ofVec3f pos = currentMesh->getVertex(i);
                int letterIndex = typeMesh->getLetterByParticle( typeMesh->getParticleIndex( pos ) );
                mesh->addIndex(attachIndex);
                
                int index = (int) ofRandom((*letterParticles)[letterIndex].size());
                QuickVertex pt = (*letterParticles)[letterIndex][index];
                while (pt.pos == pos ) {
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
            
            for( int i=0; i < currentMesh->getNumVertices(); i++ )
            {
                mesh->addIndex(currentMesh->getIndex(i));
                i++;
            }
            break;
            
        case DRAW_SHAPES:
            mesh->setMode(OF_PRIMITIVE_TRIANGLES);
            // to-do: triangulation!
            mesh->clearIndices();
            
            // method 1: randomize
            for( int i=0; i < currentMesh->getNumVertices(); i++ )
            {
                ofVec3f p1 = currentMesh->getVertex(i);
                int ind1, ind2, ind3;
                ind1 = ind2 = ind3 = -1;
                
                ind1 = currentMesh->getIndex(i);
                
                ofVec3f p2;
                ofVec3f p3;
                
                bool pointGood = false;
                
                for( int j=0; j < currentMesh->getNumVertices(); j++ )
                {
                    p2 = currentMesh->getVertex(j);
                    if ( p2 != p1 ){
                        if ( p1.distance( p2 ) < 10 ){
                            ind2 = currentMesh->getIndex(j);
                            pointGood = true;
                            break;
                        }
                    }
                }
                
                if ( !pointGood ) continue;
                
                pointGood = false;
                
                for( int k=0; k < currentMesh->getNumVertices(); k++ )
                {
                    p3 = currentMesh->getVertex(k);
                    
                    if ( p3 != p1 && p3 != p2 ){
                        if ( p2.distance( p3 ) < 20 ){
                            ind3 = currentMesh->getIndex(k);
                            pointGood = true;
                            break;
                        }
                    }
                }
                
                if ( !pointGood ) continue;
                
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