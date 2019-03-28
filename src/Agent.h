#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxFilterLibrary.h"
#include "Message.h"

struct AgentProperties {
  ofPoint meshSize; // w, h of the mesh.
  ofPoint meshDimensions; // row, columns of the mesh.
  ofPoint vertexPhysics;
  ofPoint jointPhysics;
  ofPoint textureDimensions; // Use it when we have a texture.
  ofPoint meshOrigin; // Derived class populates this. 
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
    void handleBondState();
    bool canBond();
  
    // Enabling behaviors
    void setSeekTarget();
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
  
    // Vertices
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
  
    // Partners
    Agent *getPartner();
    void setPartner(Agent *partner);
  
    // Texture
    void createTexture(ofPoint meshSize);
    ofPoint getTextureSize();
  
    // Pubic iterator to access messages. 
    std::vector<Message>::iterator it;
    std::vector<Message> messages;
  
  protected:
    // Derived class needs to have access to these. 
    int numMessages;
    std::vector<ofColor> palette;
    AbstractFilter * filter;
    
  private:
    void assignMessages(ofPoint meshSize);
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void updateMesh();
  
    // ----------------- Data members -------------------
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
    
    // Partner agent.
    Agent *partner = NULL;
  
    // Mesh.
    ofMesh mesh;
  
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
  
    // Bond timers.
    float curBondCounter = 0; 
    float maxBondCounter; 
  
    // Perception
    int targetPerceptionRad;
    // Max InterAgent joints for each agent
    int maxInterAgentJoints;
  
    // Texture
    ofFbo fbo; 
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
