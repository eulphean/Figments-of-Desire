#pragma once
#include "ofMain.h"
#include "ofxFilterLibrary.h"

class BgMesh {
  public:
    BgMesh() {
      filter = new PerlinPixellationFilter(ofGetWidth(), ofGetHeight(), 40.f);
    }
  
    void setParams(ofParameterGroup params);
    void createBg();
    void update(std::vector<glm::vec2> centroids);
    void updateWithVertices(std::vector<ofMesh> meshes);
    void draw();
  
  private:
    void createMesh();
    glm::vec2 interact(glm::vec2 meshVertex, glm::vec2 centroid, int vIdx);
    
    ofFbo bgImage;
    ofMesh mesh;
    ofMesh meshCopy;
    ofParameterGroup bgParams;
  
    AbstractFilter * filter; 
};
