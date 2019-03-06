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
    void draw(bool debug);
    void clean();
  
    // Behaviors
    void applyBehaviors();
    void handleAttraction();
    void handleRepulsion();
    void handleTickle();
  
    // Enabling behaviors
    void setAttractionTarget(glm::vec2 target);
    void setRandomForce();
    void setRepulsionTarget(glm::vec2 target);
  
    // Helpers
    std::shared_ptr<ofxBox2dCircle> getRandomVertex();
    glm::vec2 getCentroid();
  
    // Color dimension of this agent.
    std::vector<ofColor> colorSlots;
    // Core colors.
    std::array<ofColor, 5> colors = { ofColor::maroon, ofColor::red, ofColor::green, ofColor::yellow, ofColor::white};
    
  private:
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void updateMesh();
  
    // Colors
    void populateSlots();
    void createTexture(ofPoint meshSize);
  
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
    ofMesh mesh;
  
    // Target position
    glm::vec2 attractTargetPos;
    glm::vec2 repelTargetPos;
  
    bool applyRandomForce;
    bool attractTarget;
    bool repelTarget;
  
    // Weights
    float attractWeight;
    float randWeight;
  
    // Health
    float tickleHealth;
    float targetHealth;
  
    // Perception
    int targetPerceptionRad;
  
    const int maxSlots = 5; // Number of slots.
    ofFbo fbo;
};
