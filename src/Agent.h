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

enum DesireState {
  LOW,
  HIGH
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
    void handleRepulsion();
    void handleRepelCorners();
    void handleAttraction();
    void handleStretch();
    void handleVertexBehaviors();
    void handleTickle();
  
    // Enabling behaviors
    void setTickle(float weight);
    void setStretch(float weight);
  
    // Helpers
    glm::vec2 getCentroid();
    ofMesh& getMesh();
    int getMaxInterAgentJoints();
    void setDesireState(DesireState state);
  
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
  
    // Desires. 
    float desireRadius;
    DesireState desireState;

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
    void assignIndices(AgentProperties agentProps);
  
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
  
    // Stretch out.
    float stretchWeight;
    bool applyStretch;
  
    // Repulsion.
    bool applyRepulsion;
    float repulsionWeight;
  
    // Repel corners
    bool repelCorners; 
  
    // Attraction.
    bool applyAttraction;
    float attractionWeight; 
  
    // Max InterAgent joints for each agent
    int maxInterAgentJoints;
  
    // Texture
    ofFbo fbo;
  
    // Messages for this agent.
    std::vector<string> textMsgs;
  
    // Figment's corner indices
    int cornerIndices[4];
    vector<int> boundaryIndices;
};

// Data Structure to hold a pointer to the agent instance
// to which this vertex belongs to.
class VertexData {
  public:
    VertexData(Agent *ptr) {
      agent = ptr;
      applyRepulsion = false;
    }
  
    Agent * agent;
    bool applyRepulsion;
};
