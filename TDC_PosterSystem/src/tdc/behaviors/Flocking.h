//
//  Flocking.h
//  TDC_PosterSystem
//
//  Created by Brett Renfer on 2/12/14.
//
//

#pragma once

#include "Behavior.h"

// this should be namespaced like woah

class Flocking : public Behavior {
public:
    
    Flocking() : Behavior(){
        name = "flocking";
        bNeedToRefreshAttract = false;
        maxParticles = 5000;
    }
    
    Flocking( ofxLabFlexParticleSystem::Container * particles ) : Behavior(){
        name = "flocking";
        setup(particles);
        mix = 0.0;
        intensity.set(1,1);
        bNeedToRefreshAttract = false;
        maxParticles = 5000;
    }
    
    float maxspeed;
    float maxforce;
    
    void setLetters( ofxLabFlexParticleSystem::Container * particles ){
        cout << "SET LETTERS"<<endl;
        ofxLabFlexParticleSystem::Iterator it = particles->begin();
        for (int x = 0; x < textureRes; x++){
            for (int y = 0; y < textureRes; y++){
                if ( it == particles->end() || it->first >= particles->size() ) break;
                
                int i = textureRes * y + x;
                
                livepos[i*3 + 0] = it->second->x / (float) dimensionsX;
                livepos[i*3 + 1] = it->second->y / (float) dimensionsY;
                livepos[i*3 + 2] = 0.0;
                it++;
                if ( it == particles->end()) break;
            }
        }
        bNeedToRefreshAttract = true;
    }
    
    void setup( ofxLabFlexParticleSystem::Container * particles ){
        bNeedToReload = false;
        maxspeed = 3;
        maxforce = 0.05;
        maxParticles = 4000;        // setup shaders
        string shadersFolder;
        shadersFolder="shaders";
        
        timeStep = 0.005f;
        particleSize = 30.0f;
        
        // Loading the Shaders
        updatePos.load("",shadersFolder+"/posUpdate.frag");// shader for updating the texture that store the particles position on RG channels
        updateVel.load("",shadersFolder+"/velUpdate.frag");// shader for updating the texture that store the particles velocity on RG channels
        
        updateRender.setGeometryInputType(GL_POINTS);
        updateRender.setGeometryOutputType(GL_TRIANGLES);
        updateRender.setGeometryOutputCount(36);
        updateRender.load(shadersFolder+"/render.vert","","shaders/render.geom");
        
        numParticles = fmin(maxParticles, particles->size());
        
        livepos = new float[numParticles*3];
        
        // Seting the textures where the information ( position and velocity ) will be
        textureRes = (int)sqrt((float)numParticles);
        numParticles = textureRes * textureRes;
        
        // 1. Making arrays of float pixels with position information
        ofxLabFlexParticleSystem::Iterator it = particles->begin();
        
        float * pos = new float[numParticles*3];
        for (int x = 0; x < textureRes; x++){
            for (int y = 0; y < textureRes; y++){
                int i = textureRes * y + x;
                
                pos[i*3 + 0] = it->second->x / (float) dimensionsX;
                pos[i*3 + 1] = it->second->y / (float) dimensionsY;
                pos[i*3 + 2] = 0.0;
                ++it;
            }
        }
        // Load this information in to the FBOÔøΩs texture
        posPingPong.allocate(textureRes, textureRes,GL_RGB32F);
        posPingPong.src->getTextureReference().loadData(pos, textureRes, textureRes, GL_RGB);
        posPingPong.dst->getTextureReference().loadData(pos, textureRes, textureRes, GL_RGB);
        
        // load into attract zone
        attractFbo.allocate(textureRes, textureRes,GL_RGB32F);
        attractFbo.getTextureReference().loadData(pos, textureRes, textureRes, GL_RGB);
        
        delete [] pos;    // Delete the array
        
        
        // 2. Making arrays of float pixels with velocity information and the load it to a texture
        it = particles->begin();
        float * vel = new float[numParticles*3];
        for (int i = 0; i < numParticles; i++){
            vel[i*3 + 0] = ofRandom(-.01,.01);//it->second->velocity.x;
            vel[i*3 + 1] = ofRandom(-.01,.01);//it->second->velocity.y;
            vel[i*3 + 2] = 1.0;
        }
        // Load this information in to the FBOÔøΩs texture
        velPingPong.allocate(textureRes, textureRes,GL_RGB32F);
        velPingPong.src->getTextureReference().loadData(vel, textureRes, textureRes, GL_RGB);
        velPingPong.dst->getTextureReference().loadData(vel, textureRes, textureRes, GL_RGB);
        delete [] vel; // Delete the array
        
        // Allocate the final 
        renderFBO.allocate(dimensionsX, dimensionsY, GL_RGBA);//32F);
        renderFBO.begin();
        ofClear(0, 0, 0, 0);
        renderFBO.end();
        
        mesh.setMode(OF_PRIMITIVE_POINTS);
        for(int x = 0; x < textureRes; x++){
            for(int y = 0; y < textureRes; y++){
                mesh.addVertex(ofVec3f(x,y));
                mesh.addTexCoord(ofVec2f(x, y));
                mesh.addColor(ofFloatColor(1,1,1,1));
            }
        }
    }
    
    void copyMesh( ofMesh * m ){
        if ( m->getNumColors() < mesh.getNumColors() ){
            ofLogError()<<"Not enough colors";
            return;
        }
        for (int i=0; i<mesh.getNumVertices(); i++){
            mesh.setColor( i, m->getColor(i));
        }
    }
    
    void beginDraw(){
        if ( bNeedToReload){
            cout <<"refresh"<<endl;
            bNeedToReload = false;
            updateRender.unload();
            updateRender.setGeometryInputType(GL_POINTS);
            updateRender.setGeometryOutputType(GL_TRIANGLES);
            updateRender.setGeometryOutputCount(36);
            updateRender.load("shaders/render.vert","","shaders/render.geom");
        }
        if ( bNeedToRefreshAttract ){
            cout <<"refresh"<<endl;
            bNeedToRefreshAttract = false;
            // load into attract zone
            attractFbo.getTextureReference().loadData(livepos, textureRes, textureRes, GL_RGB);
        }
        
        // Load this information in to the FBOÔøΩs texture
        //posPingPong.src->getTextureReference().loadData(livepos, textureRes, textureRes, GL_RGB);
        
        velPingPong.dst->begin();
        ofClear(0);
        updateVel.begin();
        updateVel.setUniform1i("posWidth", velPingPong.src->getWidth());
        updateVel.setUniform1i("posHeight", velPingPong.src->getWidth());
        updateVel.setUniformTexture("backbuffer", velPingPong.src->getTextureReference(), 0);   // passing the previus velocity information
        updateVel.setUniformTexture("posData", posPingPong.src->getTextureReference(), 1);      // passing the position information
        updateVel.setUniformTexture("attractData", attractFbo.getTextureReference(), 2);        // much attractive data
        
        //updateVel.setUniform1i("resolution", (int)textureRes);
        updateVel.setUniform2f("screen", (float)dimensionsX, (float)dimensionsY);
        updateVel.setUniform2f("mouse", mouse.x, mouse.y);
        updateVel.setUniform1f("timestep", (float)timeStep);
        updateVel.setUniform1f("maxspeed", intensity.x / 5000.0f);
        updateVel.setUniform1f("maxforce", intensity.y / 5000.0f);
        updateVel.setUniform1f("attractMix", mix);
        updateVel.setUniform1f("scale", scale );
        
        // draw the source velocity texture to be updated
        velPingPong.src->draw(0, 0);
        
        updateVel.end();
        velPingPong.dst->end();
        
        velPingPong.swap();
        
        
        // Positions PingPong
        //
        // With the velocity calculated updates the position
        //
        posPingPong.dst->begin();
        ofClear(0);
        updatePos.begin();
        updatePos.setUniform1i("posWidth", velPingPong.src->getWidth());
        updatePos.setUniform1i("posHeight", velPingPong.src->getWidth());
        updatePos.setUniformTexture("prevPosData", posPingPong.src->getTextureReference(), 0); // Previus position
        updatePos.setUniformTexture("velData", velPingPong.src->getTextureReference(), 1);  // Velocity
        updatePos.setUniform1f("timestep",(float) timeStep );
        updatePos.setUniform1f("scale", scale );
        
        // draw the source position texture to be updated
        posPingPong.src->draw(0, 0);
        
        updatePos.end();
        posPingPong.dst->end();
        
        posPingPong.swap();
        
        
        // Rendering
        //
        // 1.   Sending this vertex to the Vertex Shader.
        //      Each one itÔøΩs draw exactly on the position that match where itÔøΩs stored on both vel and pos textures
        //      So on the Vertex Shader (thatÔøΩs is first at the pipeline) can search for it information and move it
        //      to it right position.
        //
        /*renderFBO.begin();
        
        ofPushMatrix();
        ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
        ofScale(scale, scale);
        ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
        ofClear(0,0,0,0);
        updateRender.begin();
        updateRender.setUniformTexture("posTex", posPingPong.dst->getTextureReference(), 0);
        updateRender.setUniform1i("resolution", (float)textureRes);
        updateRender.setUniform2f("screen", (float)2.0, (float)2.0);
        updateRender.setUniform1f("scale", scale );
        //updateRender.setUniform1f("size", (float)particleSize);
        
        ofPushStyle();
        ofSetColor(255);
        
        mesh.draw();
        
        ofPopStyle();
        
        updateRender.end();
        ofPopMatrix();
        renderFBO.end();*/
        
        // reset mouse to junk
        mouse.set(-1,-1);
    }
    
    void endDraw(){
    }
    
    
    void updateAll( ofxLabFlexParticleSystem::Container * c ){
        /*ofxLabFlexParticleSystem::Iterator it = c->begin();
        for (int x = 0; x < textureRes; x++){
            for (int y = 0; y < textureRes; y++){
                int i = textureRes * y + x;
                
                livepos[i*3 + 0] = it->second->x / (float) ofGetWidth();
                livepos[i*3 + 1] = it->second->y / (float) ofGetHeight();
                livepos[i*3 + 2] = 0.0;
                ++it;
            }
        }*/
    }
    
    void draw(){
        ofSetColor(255);
//        renderFBO.draw(0,0);
        ofPushMatrix();
//            ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
//            ofScale(scale, scale);
//            ofTranslate(-ofGetWidth()/2.0, -ofGetHeight()/2.0);
            ofVec3f trans = ofGetCurrentMatrix(OF_MATRIX_MODELVIEW).getTranslation();
            //trans *= scale;
            updateRender.begin();
            updateRender.setUniformTexture("posTex", posPingPong.dst->getTextureReference(), 0);
            updateRender.setUniform1i("resolution", (float)textureRes);
            updateRender.setUniform2f("screen", (float)2.0, (float)2.0);
            updateRender.setUniform2f("screenPos", (float) trans.x / (dimensionsX), (float) trans.y / (dimensionsY) );
            updateRender.setUniform1f("scale", scale );
            updateRender.setUniformMatrix4f("homography", homography);
        
            updateRender.setUniform1f("pointSize", pointSize);
            updateRender.setUniform1f("pointRandomization", pointRandomization);
            //updateRender.setUniform1f("size", (float)particleSize);
        
            ofPushStyle();
            ofSetColor(255);
            
            mesh.draw();
            
            ofPopStyle();
            
            updateRender.end();
        ofPopMatrix();
    }
    
    void reload(){
        cout << "reload?"<<endl;
        bNeedToReload = true;
    }
    
protected:
    bool bNeedToReload;
    bool bNeedToRefreshAttract;
    float * livepos;
    
    int maxParticles;
    
    ofShader    updatePos;
    ofShader    updateVel;
    ofShader    updateRender; // do we need this one?
    
    float       timeStep;
    float       particleSize;
    ofFbo       renderFBO;
    
    pingPongBuffer posPingPong;
    pingPongBuffer velPingPong;
    ofFbo       attractFbo;
    
    ofVboMesh       mesh;
    
    int     width, height;
    int     numParticles;
    int     textureRes;
};
