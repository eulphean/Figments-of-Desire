#include "SuperAgent.h"

void SuperAgent::setup(Agent *agent1, Agent *agent2, std::shared_ptr<ofxBox2dJoint> joint) {
  agentA = agent1;
  agentB = agent2;
  joints.push_back(joint);
}

void SuperAgent::update(ofxBox2d &box2d, std::vector<std::shared_ptr<ofSoundPlayer>> &sounds, int maxJointForce, bool &enableSound) {
  // Max Force based on which the joint breaks.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j) {
    auto force = j->getReactionForce(ofGetElapsedTimef());
    if (abs(force.length()) > maxJointForce) {
      auto data = (SoundData *) j -> joint -> GetUserData();
      // Disable breaking sound for a bit,
      if (enableSound) {
//        sounds[data->breakIdx]->play();
        // Trigger a Midi note (Instance is the same)
        int note = ofRandom(0, 127);
        Midi::instance().sendBondBreakMidi(note);
      }
      
      box2d.getWorld()->DestroyJoint(j->joint);
      // Trigger the break sound here.
      
      return true;
    } else {
      return false;
    }
  });
  
  if (joints.size() == 0) {
    agentA -> setPartner(NULL);
    agentB -> setPartner(NULL);
    shouldRemove = true;
  }
}

void SuperAgent::draw() {
  for (auto j : joints) {
    ofPushStyle();
      ofSetColor(ofColor::red);
      ofSetLineWidth(3);
      j->draw();
    ofPopStyle();
  }
}

// Check if super body already exists. 
bool SuperAgent::contains(Agent *agent1, Agent *agent2) {
  if (agent1 == agentA) {
    if (agent2 == agentB) {
      return true;
    } else {
      return false;
    }
  } else if (agent2 == agentB) {
    if (agent1 == agentA) {
      return true;
    } else {
      return false;
    }
  }
}

void SuperAgent::clean(ofxBox2d &box2d) {
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
    box2d.getWorld()->DestroyJoint(j->joint);
    return true;
  });
  
  joints.clear();
}
