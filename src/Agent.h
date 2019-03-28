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
  ofPoint meshOrigin; // Derived class populates this. 
  float vertexRadius;
};

// Subsection body that is torn apart from the actual texture and falls on the ground. 
class Agent {
  public:
    void setup(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void update();
  
    // Each agent must override this method to be able to draw itself.
    virtual void drawAgent(bool debug, bool showTexture) {
      cout << "Base class.. Should never be called." << endl;
    }
  
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
  
    // Vertices
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
  
    // Partners
    Agent *getPartner();
    void setPartner(Agent *partner);
  
  protected:
    ofMesh mesh;
    void draw(bool debug, bool showTexture);
    
  private:
    void createMesh(AgentProperties softBodyProperties);
    void createSoftBody(ofxBox2d &box2d, AgentProperties softBodyProperties);
    void updateMesh();
  
    // ----------------- Data members -------------------
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
    
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
