#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxFilterLibrary.h"

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
    void draw(bool debug, bool showTexture);
  
    // Clean the agent
    void clean(ofxBox2d &box2d);
  
    // Behaviors
    void applyBehaviors();
    void handleSeek();
    void handleRepulsion();
    void handleStretch();
    void handleTickle();
  
    // Enabling behaviors
    void setSeekTarget(glm::vec2 target);
    void setTickle(float weight);
    void setStretch(float weight);
    void setRepulsionTarget(Agent *target, int targetAgentId);
    
    // Filters
    void nextFilter();
  
    // Helpers
    std::shared_ptr<ofxBox2dCircle> getRandomVertex();
    glm::vec2 getCentroid();
    ofMesh& getMesh();
    int getMaxInterAgentJoints();
  
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
  
    // ----------------- Data members -------------------
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
    ofMesh mesh;
  
    // Agent texture.
    const int maxSlots = 8; // Number of slots.
    ofFbo fbo;
    vector<AbstractFilter *>  _filters;
    int _currentFilter = 0;
  
    // Partner agent
    Agent *partner = NULL;
  
    // Seek
    glm::vec2 seekTargetPos;
    float seekWeight;
    bool applySeek;
  
    // Tickle.
    float tickleWeight;
    bool applyTickle;
    float curTickleCounter;
    float maxTickleCounter;
  
    // Stretch out.
    float stretchWeight;
    bool applyStretch;
    float curStretchCounter;
    float maxStretchCounter;
  
    // Repulsion.
    bool applyRepulsion;
    float repulsionWeight;
    float curRepulsionCounter;
    float maxRepulsionCounter;
  
    // Perception
    int targetPerceptionRad;
    // Max InterAgent joints for each agent
    int maxInterAgentJoints;
  
    // ------------------- Desires ----------------------
    // THINK WHAT THE DESIRES ARE
};

// Data Structure to hold a pointer to the agent instance
// to which this vertex belongs to.
class VertexData {
  public:
    VertexData(Agent *ptr) {
      agent = ptr;
    }
  
    Agent * agent;
};
