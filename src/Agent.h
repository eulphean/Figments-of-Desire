#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxFilterLibrary.h"
#include "ofxKsmrFragmentFx.h"

struct AgentProperties {
  ofPoint meshSize; // w, h of the mesh.
  ofPoint meshDimensions; // row, columns of the mesh.
  ofPoint vertexPhysics;
  ofPoint jointPhysics;
  ofPoint textureDimensions; // Use it when we have a texture. 
  float vertexRadius;
};

// Subsection body that is torn apart from the actual texture and falls on the ground. 
class Agent {
  public:
    void setup(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void update();
    void draw(bool debug);
  
    // Clean the agent
    void clean(ofxBox2d &box2d);
  
    // Behaviors
    void applyBehaviors();
    void handleAttraction();
    void handleRepulsion();
    void handleTickle();
  
    // Enabling behaviors
    void setAttractionTarget(glm::vec2 target);
    void setRandomForce();
    void setRepulsionTarget(Agent *target, int targetAgentId);
  
    // Filters
    void nextFilter();
  
    // Helpers
    std::shared_ptr<ofxBox2dCircle> getRandomVertex();
    glm::vec2 getCentroid();
    ofMesh& getMesh();
  
    // Color dimension of this agent.
    std::vector<ofColor> colorSlots;
    // Core colors.
    std::array<ofColor, 8> colors = { ofColor::fromHex(0x5D3DFF), ofColor::fromHex(0xF48327), ofColor::fromHex(0xF72E57), ofColor::fromHex(0x7CC934),
            ofColor::fromHex(0x2D3D30), ofColor::fromHex(0xFF6B6C), ofColor::fromHex(0xFFC145), ofColor::fromHex(0x064789) };
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
  
    // Partners
    Agent *getPartner();
    void setPartner(Agent *partner);
  
    // Texture mutation
    void mutateTexture();
    
  private:
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void updateMesh();
  
    // Colors
    void populateSlots();
    void createTexture(ofPoint meshSize);
  
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
    ofMesh mesh;
  
    // Target position
    glm::vec2 attractTargetPos;
  
    bool repelTarget;
    Agent *repelTargetAgent = NULL;
    long repelIntervalTimer;
    long repelTotalTimer;
  
    bool applyRandomForce;
    bool attractTarget;
  
  
    // Agent Id that you are repelling from. 
    int repelTargetAgentId = -1;
  
    // Weights
    float attractWeight;
    float randWeight;
  
    // Health
    float tickleHealth;
    float targetHealth;
  
    // Perception
    int targetPerceptionRad;
  
    const int maxSlots = 8; // Number of slots.
    ofFbo fbo;
  
    // Partner agent
    Agent *partner = NULL;
  
    // Texture
    vector<AbstractFilter *>  _filters;
    int _currentFilter = 0;
};

// Data Structure to hold a pointer to the agent instance 
// to which this vertex
// belongs to.
class VertexData {
  public:
    VertexData(Agent *ptr) {
      agent = ptr;
    }
  
    Agent * agent;
};
