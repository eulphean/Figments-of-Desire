#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "Agent.h"

// Subsection body that is torn apart from the actual texture and falls on the ground.
// The entire thing acts like one unique bond now. 
class SuperAgent {
  public:
    void setup(Agent *agentA, Agent *agentB, std::shared_ptr<ofxBox2dJoint>);
    void update(ofxBox2d &box2d, int maxJointForce);
    void draw();
    bool contains(Agent *agentA, Agent *agentB);
    void clean(ofxBox2d &box2d);
  
    Agent *agentA;
    Agent *agentB;
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints;  // These are interAgent joints.
    bool shouldRemove = false;
};


// Should come in the update function of the super agents actually. 
//  // Joint destruction based on a predetermined force between agents.
//  ofRemove(interAgentJoints, [&](std::shared_ptr<ofxBox2dJoint> c){
//      auto f = c->getReactionForce(ofGetElapsedTimef());
//      if (f.length() > maxJointForce) {
//        box2d.getWorld()->DestroyJoint(c->joint);
//        return true;
//      }
//
//      return false;
//  });
