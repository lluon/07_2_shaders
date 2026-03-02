#include "ofApp.h"
#include "ofGraphicsUtil.h"
using namespace glm;

//--------------------------------------------------------------
static std::string glslLighting(){
    return R"(
        
        struct Light {
            vec3 pos;
            float strength;
            float halfDist;
            float ambient;
            vec3 diffuse;
            vec3 specular;
        };

        struct Material {
            vec3 diffuse;
            vec3 specular;
            float shine;
        };

        struct LightFall {
            vec3 diffuse;
            vec3 specular;
        };

        // In-place addition : a += b

        void addTo(inout LightFall a, in LightFall b){
            a.diffuse += b.diffuse;
            a.specular += b.specular;
        }

        // Compute light components falling on surface

        LightFall computeLightFall(vec3 pos, vec3 N, vec3 eye, in Light lt, in Material mt){
            vec3 lightDist = lt.pos - pos;
            float hh = lt.halfDist * lt.halfDist;
            float atten = lt.strength * hh/(hh + dot(lightDist, lightDist));

            vec3 L = normalize(lightDist);

            // diffuse

            float d = max(dot(N, L), 0.);
            d += lt.ambient;

            //specular

            vec3 V = normalize(eye - pos);
            vec3 H = normalize(L + V);
            float s = pow(max(dot(N, H), 0.), mt.shine);
            LightFall fall;
            fall.diffuse = lt.diffuse * (d * atten);
            fall.specular = lt.specular * (s * atten);
            return fall;
        }

        // Get final color reflected off material

        vec3 lightColor(in LightFall f, in Material mt){
            return f.diffuse * mt.diffuse + f.specular * mt.specular;
        }
    )";
}


//--------------------------------------------------------------
void ofApp::setup(){
    // Setup camera (for 3D rendering)
    cam.setPosition(vec3(0., 0., 2.));
    cam.setNearClip(0.05);
    cam.setFarClip(100.);
    
    // Other setup for 3D rendering
    ofEnableDepthTest();
    ofSetFrameRate(40); // must be set for ofGetTargetFrameRate to work
    
    // Build shader (from GLSL code)
    build(shader, glslLighting() + R"(         
        
        // Vertex program

        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
 
        uniform vec3 eye;
  
        in vec3 position;
        in vec3 vnormal;
        in vec3 vcolor;
        out vec4 fragColor;

      void main(){
 
        vcolor = color;
 
        vec3 pos = (modelMatrix*position).xyz;
        vec3 N = normalize(mat3(modelMatrix)*normal); 
        gl_Position = projectionMatrix * viewMatrix * vec4(pos,1.);
 
        // lights setup

        Light light1;
        light1.pos = vec3(1., 0., 0.);
        light1.strength = 1.;
        light1.halfDist = 1.;
        light1.ambient = 0.2;
        light1.diffuse = vec3(1., 0.5, 0.); // orange
        light1.specular = light1.diffuse;

        Light light2 = light1;
        light2.pos = -light1.pos;
        light2.diffuse = vec3(1., 0., 1.);
        light2.specular = light2.diffuse;

  
        // Material setup
 
        Material mtrl;
        mtrl.diffuse = vec3(1.);
        mtrl.specular = vec3(0.5);
        mtrl.shine = 60.;

        // light fall lights 
 
        LightFall fall = computeLightFall(pos, N, eye, light1, mtrl);
        addTo(fall, computeLightFall(pos, N, eye, light2, mtrl));

        vcolor = lightColor(fall, mtrl); 

        gl_Position = projectionMatrix * viewMatrix * vec4(pos, 1.);
      }
 
 )", R"(
 
  // Fragment program
 
  in vec3 vcolor;
  out vec4 fragColor;
 
  void main(){
    vec3 col = vcolor;
   fragColor = vec4(col, 1.);
  }
 )");
    
    mesh = ofMesh::sphere(1.,32);
    for (auto pos : mesh.getVertices())
         mesh.addColor(ofFloatColor(0, 0, 1));
}

//--------------------------------------------------------------
void ofApp::update(){
	//printf("frame:%u dt:%f, fps:%f (%g)\n", (unsigned)ofGetFrameNum(), ofGetLastFrameTime(), ofGetFrameRate(), ofGetTargetFrameRate());
}

//--------------------------------------------------------------
void ofApp::draw(){
	cam.begin();
	shader.begin();
    shader.setUniform3f("eye", cam.getPosition());
    
    ofSetRandomSeed(58309834);
    
    for(int i=0; i<50; ++i){
        ofPushMatrix();
            ofTranslate(ofRandomUniform<vec3>(-1.,1.));
            ofScale(0.15);
            mesh.draw();
        ofPopMatrix();
    }
    shader.end();
	cam.end();
}


void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
