//
//  TargetMesh.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/21/14.
//
//

#pragma once

#include "ofMain.h"
#include "TDCConstants.h"
#include "TypeGrid.h"
#include "TypeOutline.h"

class Container;

// container for main elements
class SubMesh {
public:
    
    void setup( string directory, string name, int numPositions );
    
    // setup: grid
    void        createGrid();
    
    // building stuff
    TypeOutline outline;
    TypeGrid    grid;
    
    // mesh lookups for particles
    ofMesh      gridMesh, outlineMesh;
    
    vector<vector <QuickVertex> > letterGridParticles, letterOutlineParticles;
    
    map<DrawMode, map<GridType,ofMesh> > meshes;
    void        buildMeshes();
    void        buildMesh(DrawMode mode, GridType type );
    
    int         numPositions;
    string      directory;
    string      name;
};

// container for series of related meshes

class TargetMesh {
public:
    
    void setup( string directory, int numPositions );
    
    int getNumMeshes(){ return subMeshes.size(); };
    
    SubMesh & getSubMesh( int index );
    ofMesh & getMesh( int index, DrawMode mode, GridType type );
    
protected:
    
    vector <SubMesh> subMeshes;
    
    int         numPositions;
    string      svgDirectory;
};
