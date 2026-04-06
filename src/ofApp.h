#pragma once

#include "ofMain.h"

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
    
        // Textures:
        ofImage wallTex;     // wall texure
        ofImage ceilingTex;  // ceiling texure
        ofImage sphereTex;   // sphere texure
        ofImage torusTex;    // torus texture
        ofImage MusicOnTex;  // logo texture
        ofImage floorTex;    // floor texture
        ofImage envirMap;   //chrome
    
        //math claculation
        float PI_calc = 355./113.;
        float oneOverPi = 113./355.;

};
