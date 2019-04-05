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
    void setup(ofxBox2d &box2d, AgentProperties softBodyProperties, string fileName);
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
  
    // Helpers
    std::shared_ptr<ofxBox2dCircle> getRandomVertex();
    glm::vec2 getCentroid();
    ofMesh& getMesh();
    int getMaxInterAgentJoints();
  
    // Vertices
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
  
    // Texture
    void createTexture(ofPoint meshSize);
    ofPoint getTextureSize();
  
    // Pubic iterator to access messages. 
    std::vector<Message>::iterator curMsg;
    std::vector<Message> messages;
  
    // Agent's partner
    Agent *partner = NULL;
  
  protected:
    // Derived class needs to have access to these. 
    int numBogusMessages;
    std::vector<ofColor> palette;
    AbstractFilter * filter;
    ofTrueTypeFont font;
    
  private:
    void readFile(string fileName);
    void assignMessages(ofPoint meshSize);
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void updateMesh();
  
    // ----------------- Data members -------------------
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
  
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
  
    // Messages for this agent.
    std::vector<string> textMsgs; 
};

// Data Structure to hold a pointer to the agent instance
// to which this vertex belongs to.
class VertexData {
  public:
    VertexData(Agent *ptr, bool interAgent = false) {
      agent = ptr;
      interAgentVertex = interAgent;
    }
  
    Agent * agent;
    bool interAgentVertex;
};
