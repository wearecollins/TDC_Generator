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
        
        maxspeed = 3;
        maxforce = 0.05;
    }
    
    void updateAll( ofxLabFlexParticleSystem::Container * c ){
        static ofxLabFlexParticleSystem::Iterator it = c->begin();
        ofxLabFlexParticleSystem::Iterator begin = c->begin();
        
        if ( std::distance( begin, it) >= c->size() ){
            it = c->begin();
        }
        ofxLabFlexParticleSystem::Iterator next = it;
        std::advance( next, 100 );
        if ( std::distance( begin, next) >= c->size() ) next = c->end();
        
        for( it; it != next; ++it )
        {
            it->second->damping = 1.0;
            it->second->radius = 10.0;
            flock( it->second, c );
        }
    }
    
    void update( TypeParticle * p ){
    }
    
protected:
    
    float maxforce;    // Maximum steering force
    float maxspeed;    // Maximum speed
    
    void flock( ofxLabFlexParticle * p, ofxLabFlexParticleSystem::Container * boids) {
        ofVec2f sep = separate(p, boids);   // Separation
        ofVec2f ali = align(p, boids);      // Alignment
        ofVec2f coh = cohesion(p, boids);   // Cohesion
        // Arbitrarily weight these forces
        sep *= (1.5);
        ali *= (1.0);
        coh *= (1.0);
        // Add the force vectors to acceleration
        applyForce(p, sep);
        applyForce(p, ali);
        applyForce(p, coh);
    }
    
    // Separation
    // Method checks for nearby boids and steers away
    ofVec2f separate (ofxLabFlexParticle * p, ofxLabFlexParticleSystem::Container * boids ) {
        float desiredseparation = 25.0f;
        ofVec2f steer = ofVec2f(0,0);
        int count = 0;
        // For every boid in the system, check if it's too close
        static ofxLabFlexParticleSystem::Iterator sepIt;
        for( sepIt = boids->begin(); sepIt != boids->end(); ++sepIt )
        {
            ofxLabFlexParticle * other = sepIt->second;
            float d = p->distance( *other );
            // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
            if ((d > 0) && (d < desiredseparation)) {
                // Calculate vector pointing away from neighbor
                ofVec2f diff = *p - *other;
                diff.normalize();
                diff /= d;        // Weight by distance
                steer += (diff);
                count++;            // Keep track of how many
            }
        }
        // Average -- divide by how many
        if (count > 0) {
            steer /= ((float)count);
        }
        
        // As long as the vector is greater than 0
        if (steer.length() > 0) {
            // Implement Reynolds: Steering = Desired - Velocity
            steer.normalize();
            steer *= (maxspeed);
            steer /= (p->velocity);
            steer.limit(maxforce);
        }
        return steer;
    }
    
    // Alignment
    // For every nearby boid in the system, calculate the average velocity
    ofVec2f align (ofxLabFlexParticle * p, ofxLabFlexParticleSystem::Container * boids) {
        float neighbordist = 50;
        ofVec2f sum = ofVec2f(0,0);
        int count = 0;
        static ofxLabFlexParticleSystem::Iterator alignIt;
        for( alignIt = boids->begin(); alignIt != boids->end(); ++alignIt )
        {
            ofxLabFlexParticle * other = alignIt->second;
            float d = p->distance( *other );
            if ((d > 0) && (d < neighbordist)) {
                sum += (other->velocity);
                count++;
            }
        }
        if (count > 0) {
            sum /= ((float)count);
            sum.normalize();
            sum *= (maxspeed);
            ofVec2f steer = sum - p->velocity;
            steer.limit(maxforce);
            return steer;
        } else {
            return ofVec2f(0,0);
        }
    }
    
    // Cohesion
    // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
    ofVec2f cohesion (ofxLabFlexParticle * p, ofxLabFlexParticleSystem::Container * boids) {
        float neighbordist = 50;
        ofVec2f sum = ofVec2f(0,0);   // Start with empty vector to accumulate all locations
        int count = 0;
        static ofxLabFlexParticleSystem::Iterator cohIt;
        for( cohIt = boids->begin(); cohIt != boids->end(); ++cohIt )
        {
            ofxLabFlexParticle * other = cohIt->second;
            float d = p->distance( *other );
            if ((d > 0) && (d < neighbordist)) {
                sum += (*other); // Add location
                count++;
            }
        }
        if (count > 0) {
            sum /= (count);
            return seek(p, sum);  // Steer towards the location
        } else {
            return ofVec2f(0,0);
        }
    }
    
    void applyForce(ofxLabFlexParticle * p, ofVec2f force) {
        // We could add mass here if we want A = F / M
        p->acceleration += (force);
    }
    
    ofVec2f seek(ofxLabFlexParticle * p, ofVec2f target) {
        ofVec2f desired = target - *p;  // A vector pointing from the location to the target
        // Normalize desired and scale to maximum speed
        desired.normalize();
        desired *= (maxspeed);
        // Steering = Desired minus Velocity
        ofVec2f steer = desired - p->velocity;
        steer.limit(maxforce);  // Limit to maximum steering force
        return steer;
    }
};
