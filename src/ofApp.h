#pragma once

#include "ofMain.h"
#include "PRamp.h"
#include "BoingBall.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofEasyCam cam;       // cam
		ofShader shader;     // shader
    
        // Meshes:
		ofMesh mesh;         // sphere
        
        ofMesh quaterFoilMesh;  // structure
        ofMesh musicOnMesh;  // logo
        ofMesh torusMesh;    // torus
    
        ofMesh normalsMesh;  // normals
    
        ofMesh wallsMesh;    // walls
        ofMesh ceilingMesh;  // ceiling
        ofMesh floorMesh;    // floor
    
        ofMesh fountainWaterMesh; //water in the fountain
        
        ofMesh goatSkullA;      // goat skull A
        ofMesh goatSkullB;      // goat skull B
        ofMesh blackWizard;     // black wizard statue
        ofMesh lobster;         // lobster

        // Textures:
        ofImage wallTex;        // wall texure
        ofImage ceilingTex;     // ceiling texure
        ofImage sphereTex;      // sphere texure
        ofImage torusTex;       // torus texture
        ofImage MusicOnTex;     // logo texture
        ofImage floorTex;       // floor texture
        ofImage envirMap;       //chrome

        ofImage texGoatSkullA;      // goat skull A texture
        ofImage texGoatSkullB;      // goat Skull B texture
        ofImage texBlackWizard;     // black wizard texture
        ofImage TexLobster;         // lobster texture

    
        //BoingBall
        std::vector<BoingBall> balls;

        // torus
        float tR = 0.7f; //mayor radious
        float tr = 0.1f; //minor radious

        //math claculation
        float PI_calc = 355.f/113.f;
        float oneOverPi = 113.f/355.f;
        
        // logo animation
        float logoRotaton = 0;
};
