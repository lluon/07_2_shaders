#include "ofApp.h"
#include "ofGraphicsUtil.h"
using namespace glm;

//--------------------------------------------------------------
// Quaterfoil structure formula
float getQuaterfoilRadious(float t, float r_base){
    float x = std::cos(t);
    float y = std::sin(t);
    float ax = std::abs(x);
    float ay = std::abs(y);
    
    float r = 0.65f; //inner corner
    
    if (ax > 0.88f || ay > 0.88f) {
        r = 1.0f;
    }//Arms
    else if (ax > 0.55f && ay > 0.55f){
        r = 0.75f;
    }
    return r_base * r;
}

//--------------------------------------------------------------
// graphics_02 section 5.4.1 normals visualizer

void addNormalLines(ofMesh & dst, const ofMesh & src, float len = 0.1, ofFloatColor col = ofFloatColor(1, 0, 0)) {
    if (!src.getNormals().size()) return;
    dst.setMode(OF_PRIMITIVE_LINES);
    for (int i = 0; i < src.getVertices().size(); ++i) {
        auto p = src.getVertices()[i];
        auto N = src.getNormals()[i];
        dst.addVertex(p);
        dst.addVertex(p + N * len);
        for (int k = 0; k < 2; ++k) dst.addColor(col);
    }
}

//--------------------------------------------------------------
// numerical normals generator

void calcNormals(ofMesh& m) {
    m.clearNormals();
    
    if(m.getNumIndices()){
        for (int i = 0; i < m.getNumVertices(); i++) m.addNormal(vec3(0));
        for (int i = 0; i < m.getNumIndices(); i += 3) {
            int i1 = m.getIndex(i); int i2 = m.getIndex(i + 1); int i3 = m.getIndex(i + 2);
            vec3 v1 = m.getVertex(i1); vec3 v2 = m.getVertex(i2); vec3 v3 = m.getVertex(i3);
            vec3 n = normalize(cross(v2 - v1, v3 - v1));
            m.getNormals()[i1] += n; m.getNormals()[i2] += n; m.getNormals()[i3] += n;
        }
        // Dr Putnam add on
        for (auto & n : m.getNormals()) n = normalize(n);
    } else { // non-indexed mesh (just triangles)
        for (int i = 0; i < m.getNumVertices(); i += 3) {
            vec3 v1 = m.getVertex(i+0); vec3 v2 = m.getVertex(i+1); vec3 v3 = m.getVertex(i+2);
            vec3 n = normalize(cross(v2 - v1, v3 - v1));
            for(int k=0; k<3; ++k) m.addNormal(n);
        }
    }
}

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

            //LightFall    
            LightFall fall;
            fall.diffuse = lt.diffuse * (d * atten);
            fall.specular = lt.specular * (s * atten);
            return fall;
        }
    
        // Get final color reflected off material
            vec3 lightColor(in LightFall f, in Material mt){
            return f.diffuse * mt.diffuse + f.specular * mt.specular;
        }
        
        // chromeEffect
        vec3 chromeEffect (vec3 rayDir, sampler2D envirMap, float u_oneOverPi){
            float tetha = atan(-rayDir.x,-rayDir.z);
            float phi = asin(rayDir.y);
            vec2 uv = vec2(0.5 + 0.5 * tetha * u_oneOverPi, 0.5 - phi * u_oneOverPi);
            return texture(envirMap,uv).rgb;
        }
        // reflection calc
        vec3 calcReflection(vec3 I, vec3 N, in sampler2D envirMap, float u_oneOverPi){
            vec3  rayDir = reflect(I,N);
            return chromeEffect(rayDir, envirMap, u_oneOverPi);
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
    ofEnableDepthTest(); // test fragments against depth buffer (for opaque geometry)
    ofEnableNormalizedTexCoords(); // use [0,1] range for UVs
    ofDisableArbTex(); // disable use of legacy rect textures
    ofSetFrameRate(60); // must be set for ofGetTargetFrameRate to work
    
    // Load All Textures
    if(!wallTex.load("walls.jpg")) std::cout << "is not loading"<< std::endl;
    ceilingTex.load("ceiling.jpeg");
    sphereTex.load("sphere.jpeg");
    torusTex.load("car_underside_ed1.png");
    MusicOnTex.load("gem.jpeg");
    floorTex.load("floor.jpeg");
    envirMap.load("envirMap.jpg");
    
    // Wrap settings for floor repetition
    floorTex.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    torusTex.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
        
    // Build shader (from GLSL code)
    build(shader, R"(        
        // Vertex program
        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
   
        in vec4 position;
        in vec3 normal;
        in vec3 color;
 
        in vec2 texcoord;
        
        out vec3 vposition;
        out vec3 vnormal;
        out vec3 vcolor;
 
        out vec2 vtexcoord;
 
      void main(){
  
        vcolor = color;
 
        vtexcoord = texcoord;
 
        vnormal = mat3(modelMatrix) * normal; 
        vposition = (modelMatrix*position).xyz;
        gl_Position = projectionMatrix * viewMatrix * vec4(vposition, 1.);
    }
 )", glslLighting() + R"(
 
   // Fragment program (7,2,3)
     uniform vec3 eye;
 
    uniform sampler2D tex;
    uniform sampler2D envirMap; //chrome fx
    uniform float reflectivity; //chrome fx
    uniform float u_oneOverPi;  //chrome fx
     
     in vec3 vposition;
     in vec3 vnormal;
     in vec3 vcolor;
 
    in vec2 vtexcoord;
  
     out vec4 fragColor;
  
    void main(){
        // declare pos and N
        vec3 pos = vposition;
        vec3 N = normalize(vnormal);
 
         // lights setup
         Light light1;
         light1.pos = vec3(1., 0., 0.);
         light1.strength = 1.;
         light1.halfDist = 5.;
         light1.ambient = 0.2;
         light1.diffuse = vec3(1., 0.5, 0.); // orange
         light1.specular = light1.diffuse;
 
         Light light2 = light1;
         light2.pos = -light1.pos;
         light2.diffuse = vec3(1., 1., 1.);
         light2.specular = light2.diffuse;
    
         // Material setup  
         Material mtrl;
         mtrl.diffuse = texture(tex,vtexcoord).rgb;
         mtrl.specular = vec3(0.5);
         mtrl.shine = 60.;
 
         // light fall lights 
         LightFall fall = computeLightFall(pos, N, eye, light1, mtrl);
         addTo(fall, computeLightFall(pos, N, eye, light2, mtrl));
  
         // pass the calculated light color to the fragment shader CHANGE (7,2,3)
         vec3 col = lightColor(fall, mtrl); 
         
         // crome fx
         vec3 I = normalize(vposition - eye);
         vec3 reflCol = calcReflection(I, N, envirMap, u_oneOverPi);
         col = mix(col, reflCol, reflectivity);
 
         // moved (7,2,3)
         fragColor = vec4(col, 1.);
    }
 
 )");
    
    mesh = ofMesh::sphere(0.5, 16);
    
    // my torus
    int Nx = 50, Ny = 50;
    torusMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    for(int j=0; j < Ny; ++j){
        for(int i=0; i < Nx; ++i){
            auto uv= vec2 (i,j)/(vec2(Nx,Ny)-1.f);
            torusMesh.addVertex(vec3 (uv,0.));
            torusMesh.addTexCoord(uv *vec2(8.,4.));
            // initial plane vertices
            if(j<(Ny-1)&&i<(Nx-1)){
                auto k = j*Nx + i;
                torusMesh.addIndex(k); //left-bottom triangle
                torusMesh.addIndex(k+1);
                torusMesh.addIndex(k+Nx);
                torusMesh.addIndex(k+Nx);//right-top triangle
                torusMesh.addIndex(k+1);
                torusMesh.addIndex(k+Nx+1);
            }
        }
    }

    
    for ( auto & p : torusMesh.getVertices()){
        auto t = 2. * PI_calc * p.x;
        auto l = 2. * PI_calc * (1 - p.y);
        p = vec3 (
                  (tr * std::sin(l) + tR) * std::cos(t),
                  (tr * std::sin(l) + tR) * std::sin(t),
                  tr * std::cos(l)
                  );
        p = {p.x,p.z,-p.y}; // rotate 90 around x
    }
    calcNormals(torusMesh);
    
    // my logo (musicOn)
    musicOnMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    float ms = 0.001f;
    musicOnMesh.addVertex(vec3(-300, 0, 0) * ms);    //0
    musicOnMesh.addVertex(vec3(-300, 500, 0) * ms);  //1
    musicOnMesh.addVertex(vec3(0, 200, 0) * ms);     //2
    musicOnMesh.addVertex(vec3(300, 500, 0) * ms);   //3
    musicOnMesh.addVertex(vec3(300, -500, 0) * ms);  //4
    musicOnMesh.addVertex(vec3(0, 0, 200) * ms);     //5
    musicOnMesh.addVertex(vec3(0, 0, -200) * ms);    //6
    
    // musicOn UVs
    musicOnMesh.addTexCoord(vec2(0.0, 0.5)); //0
    musicOnMesh.addTexCoord(vec2(0.0, 1.0)); //1
    musicOnMesh.addTexCoord(vec2(0.5, 0.7)); //2
    musicOnMesh.addTexCoord(vec2(1.0, 1.0)); //3
    musicOnMesh.addTexCoord(vec2(1.0, 0.0)); //4
    musicOnMesh.addTexCoord(vec2(0.5, 0.5)); //5
    musicOnMesh.addTexCoord(vec2(0.5, 0.5)); //6
    
    // musicOn faces
    musicOnMesh.addTriangle(5, 1, 0);
    musicOnMesh.addTriangle(5, 2, 1);
    musicOnMesh.addTriangle(5, 3, 2);
    musicOnMesh.addTriangle(5, 4, 3);
    musicOnMesh.addTriangle(5, 0, 4);
    musicOnMesh.addTriangle(6, 0, 1);
    musicOnMesh.addTriangle(6, 1, 2);
    musicOnMesh.addTriangle(6, 2, 3);
    musicOnMesh.addTriangle(6, 3, 4);
    musicOnMesh.addTriangle(6, 4, 0);
    
    musicOnMesh.flatNormals(); // duplicate vertices and remove indices (normals appear wrong, so dr Putnam generate them ourself)
    calcNormals(musicOnMesh);
    
    
    Nx = 400; // resolution
    int Ny_wall = 40;
    float r_base = 2.6;
    float h = 1.5;
    float textureRepeatScale = 0.8f;
    
    wallTex.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    
    // Quaterfoil walls
    wallsMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    float c_Arch_Lenght = 0.f;
    vec3 lastWall_p = vec3(getQuaterfoilRadious(0, r_base), 0, 0);
    
    for (int i = 0; i< Nx; ++i){
        float t = 2. * PI_calc * (float)i / (Nx -1);
        float r = getQuaterfoilRadious(t, r_base);
        vec3 p_base = vec3 (r * std::cos(t), 0, -r * std::sin(t));// y = up
        
        if (i > 0) c_Arch_Lenght += length(p_base - lastWall_p);
        lastWall_p = p_base;
        
        for (int j =0; j < Ny_wall; ++j){
            float normY = (float)j / (Ny_wall -1);
            float py = h * (2.f * normY -1.f);
            
            wallsMesh.addVertex(vec3(p_base.x, py, p_base.z));
            
            // U texture
            wallsMesh.addTexCoord(vec2(c_Arch_Lenght * textureRepeatScale, py * textureRepeatScale));
            
            if (i < Nx - 1 && j < Ny_wall -1){
                int curr = i *Ny_wall + j;
                int next = (i + 1) *Ny_wall + j;
                
                wallsMesh.addIndex(curr);
                wallsMesh.addIndex(curr + 1);
                wallsMesh.addIndex(next);
                
                wallsMesh.addIndex(next);
                wallsMesh.addIndex(curr + 1);
                wallsMesh.addIndex(next + 1);
            }
        }
    }
    calcNormals(wallsMesh);
    
    
    // Quaterfoil ceiling
    ceilingMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    float h_top = h;
    vec3 c_tip = vec3(0, 0, h_top + 0.5);
    c_tip = {c_tip.x, c_tip.z, c_tip.y};
    ceilingMesh.addVertex(c_tip);
    ceilingMesh.addTexCoord(vec2(0.5, 0.5));
    
    for(int i=0; i<Nx; ++i){
        float t = 2. * PI_calc * (float) i / (Nx - 1);
        
        float r = getQuaterfoilRadious(t, r_base);
        
        vec3 p = vec3( r * std::cos(t), r * std::sin(t), h_top);
        p = {p.x, p.z, p.y};
        ceilingMesh.addVertex(p);
        ceilingMesh.addTexCoord(vec2((float) i/Nx, 1.));
        if (i < Nx - 1) ceilingMesh.addTriangle(0, i + 1, i +2);
    }
    calcNormals(ceilingMesh);
    
    // Quaterfoil floor
    floorMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    float h_bottom = -h;
    
//    f_center ={f_center.x,f_center.z,f_center.y};
    floorMesh.addVertex(vec3 (0, h_bottom, 0));
    floorMesh.addTexCoord(vec2(0, 0));
    
    for (int i = 0; i < Nx; ++i){
        float t = 2. * PI_calc * (float) i / (Nx - 1);
        float r = getQuaterfoilRadious(t, r_base);
        
        float px = r * std::cos(t);
        float pz = r * std::sin(t);
        
        floorMesh.addVertex(vec3(px, h_bottom, pz));
        floorMesh.addTexCoord(vec2(px * textureRepeatScale, pz * textureRepeatScale));
        if (i < Nx - 1) floorMesh.addTriangle(0, i + 2, i + 1);
    };
    calcNormals(floorMesh);
    
    ofSetRandomSeed(58309834);
    for (int i = 0; i < 50; i++){
        BoingBall b;
        b.setup((float)i);
        balls.push_back(b);
    }
    
}




//--------------------------------------------------------------
void ofApp::update(){
    
    float dt = ofGetLastFrameTime();
    
    for (auto& b : balls){
        b.update(dt, tR, tr);
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    cam.begin();
    
    
    shader.begin();
    shader.setUniform3f("eye", cam.getPosition());
    shader.setUniform1f("u_oneOverPi", oneOverPi);
    shader.setUniformTexture("envirMap", envirMap, 1);
    
    //floor texture
    shader.setUniform1f("reflectivity", 0.f);
    floorTex.getTexture().bind(0);
    floorMesh.draw();
    floorTex.getTexture().unbind(0);
    
    //shader.end(); cam.end(); return;/// lance line to preview
    
    //wall texture
    wallTex.getTexture().bind(0);
        wallsMesh.draw();
    wallTex.getTexture().unbind(0);
    
    //ceiling texture
    ceilingTex.getTexture().bind(0);
        ceilingMesh.draw();
    ceilingTex.getTexture().unbind(0);
    
    ofPushMatrix(); // LOGO
    ofTranslate(0, -0.2, 0);
    MusicOnTex.getTexture().bind(0);
        musicOnMesh.draw();
    MusicOnTex.getTexture().unbind(0);
    ofPopMatrix();
    
    ofPushMatrix(); // TORUS
    ofTranslate(0., -1.4, 0.);
    ofRotateDeg(0, 1, 1, 0);
    torusTex.getTexture().bind(0);
        torusMesh.draw();
    torusTex.getTexture().unbind(0);
    ofPopMatrix();
    
    shader.setUniform1f("reflectivity", 0.4f);
    sphereTex.getTexture().bind(0);
    
    
    for (auto& b: balls){
        b.draw(mesh);
    }
    
    sphereTex.getTexture().unbind(0);
    
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
