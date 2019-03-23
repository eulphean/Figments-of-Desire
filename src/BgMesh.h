#pragma once
#include "ofMain.h"

class BgMesh {
  public:
    void setParams(ofParameterGroup params);
    void createBg();
    void update(std::vector<glm::vec2> centroids);
    void draw();
  
  private:
    void createMesh();
    void interact(glm::vec2 meshVertex, glm::vec2 centroid, int vIdx);
    
    ofFbo bgImage;
    ofMesh mesh;
    ofMesh meshCopy;
    ofParameterGroup bgParams; 
};
