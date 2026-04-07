#pragma once

#include "ofMain.h"
#include "PRamp.h"

#include <cmath> // abs, fmod

//bouncing ball as fountain spill
class BoingBall{
    public:
    PRamp aniX, aniY, aniCol, aniRot, aniSize;
    glm::vec3 pos = glm::vec3(0,0,0);
    glm::vec3 vel = glm::vec3(0,0,0);// jump velocity
    glm::vec3 rOffset;

    float rScale;
    float floorLevel = -1.4f;
    
    void setup(float offSet) {
        // Set rates of periodic ramps/oscillators
        //aniX.period(9.);
        //aniY.period(3.2);
        aniRot.period(4.);
        aniSize.freq(2.);
        
        float deSync = fmod(offSet * 0.618, 1);// offset desynchronization based on modulo
        rScale = 0.04 +fmod(offSet * 0.333, 0.05);// random scale
        
        //aniX.phase(deSync); // initialize phase
        //aniY.phase(deSync);
        aniRot.phase(deSync);
        
        reset();

    }
        void reset(){
            pos = glm::vec3(0, floorLevel, 0); //start at floor center
            float PI_calc =  355./113.;
            float angle = ofRandom(0, 2. * PI_calc);
            
            float speed = 0.005; // lower horizontal speed
            
            vel.x = std::cos(angle) * speed; // vel horizontal spread
            vel.z = std::sin(angle) * speed;
            vel.y = ofRandom(0.045, 0.065);     // high spill
        }
        
    
    void update(float dt, float tR,float tr) {
        
        aniRot.update(dt);
        aniSize.update(dt);
        
        float gravity = 0.0014;
        vel.y -= gravity;   //gravity pull
        pos +=vel;          //update vel position
        
        if (pos.y < floorLevel){
            reset();
        }
    }
    
    void draw (ofMesh& mesh){
        ofPushMatrix();
            ofTranslate(pos);
            ofRotateDeg(aniRot.phase()* 360, 1., 0.8, 1.);
            float s = rScale + aniSize.para() * (rScale * 0.4); // Scaling the logo
            ofScale(s);
            ofSetColor(255);
                mesh.draw();
        ofPopMatrix();
    }
};
