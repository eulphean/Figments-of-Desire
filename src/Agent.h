#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"

struct AgentProperties {
  ofPoint meshSize; // w, h of the mesh.
  ofPoint meshDimensions; // row, columns of the mesh.
  ofPoint vertexPhysics;
  ofPoint jointPhysics;
  ofPoint textureDimensions; // Use it when we have a texture. 
  float vertexRadius;
  int agentId;
};

class VertexData {
  public:
    VertexData(int idx, int vId) {
      agentId = idx;
      vertexId = std::to_string(idx) + '_' + std::to_string(vId);
    }
  
    int agentId;
    string vertexId;
};


// Subsection body that is torn apart from the actual texture and falls on the ground. 
class Agent {
  public:
    void setup(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void update();
    void draw(bool showSoftBody);
    void clean();
  
    // Behaviors
    void applyBehaviors();
    void setTarget(int x, int y);
    void setRandomForce();
  
  
    // Helpers
    std::shared_ptr<ofxBox2dCircle> getRandomVertex();
    
  private:
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void updateMesh();
  
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
    ofMesh mesh;
    glm::vec2 targetPos;
    bool updateTarget;
    bool applyRandomForce;
  
    // Weights
    float attractWeight;
    float randWeight;
  
    // Health
    float tickleHealth;
    float targetHealth;
  
    // Perception
    int targetPerceptionRad; 
};
