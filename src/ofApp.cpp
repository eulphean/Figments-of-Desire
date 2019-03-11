#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  // Setup OSC
  receiver.setup(PORT);
  ofHideCursor();
  
  //ofBackground(ofColor::fromHex(0x293241, 1.0));
  ofBackground(ofColor::fromHex(0x2E2F2D));
  ofSetCircleResolution(20);
  ofDisableArbTex();
  ofEnableSmoothing();
  ofEnableAlphaBlending();
  
  box2d.init();
  box2d.setGravity(0, 0.7);
  box2d.setFPS(60);
  box2d.enableEvents();
  box2d.registerGrabbing(); // Enable grabbing the circles.
  
  ofAddListener(box2d.contactStartEvents, this, &ofApp::contactStart);
  ofAddListener(box2d.contactEndEvents, this, &ofApp::contactEnd);
  
  // Setup gui.
  setupGui();
  
  hideGui = false;
  debug = false;
  
  // Boundaries
  bounds.x = 20; bounds.y = -100;
  bounds.width = ofGetWidth() - bounds.x * 2; bounds.height = ofGetHeight() - 20 + (-1) * bounds.y;
  box2d.createBounds(bounds);
  
  // Load sounds
  ofDirectory directory("sfx/");
  directory.allowExt("mp3");
  for (auto file: directory)
  {
      auto sound = std::make_shared<ofSoundPlayer>();
      sound->load(file);
      sound->setMultiPlay(true);
      sound->setLoop(false);
      sounds.push_back(sound);
  }
  
  // Setup fft.
  fft.setup();
  fft.setNormalize(true);
  
  enableSound = true;
  
  serial.setup("/dev/cu.usbmodem1411", 9600);
}

void ofApp::contactStart(ofxBox2dContactArgs &e) {
  
}

// Joint creation sequence.
void ofApp::contactEnd(ofxBox2dContactArgs &e) {
  if (agents.size() > 0) {
    if(e.a != NULL && e.b != NULL) {
      if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle
          && e.a->GetBody() && e.b->GetBody()) {
        // Extract Agent pointers.
        Agent* agentA = reinterpret_cast<VertexData*>(e.a->GetBody()->GetUserData())->agent;
        Agent* agentB = reinterpret_cast<VertexData*>(e.b->GetBody()->GetUserData())->agent;

        // Really long routine to evaluate if two vertices belonging to two different agents
        // can actually bond with each other or not. Take a look at the conditions under which
        // this bonding actually happens.
        if (agentA != agentB) {
          evaluateBonding(e.a->GetBody(), e.b->GetBody(), agentA, agentB);
        }
      }
    }
  }
}

//--------------------------------------------------------------
void ofApp::update(){
  box2d.update();
  fft.update();
  processOsc();
  
  // Check for a clap.
  // Heard a clap loud enough
  if (fft.getMidVal() > 1.0 && fft.getHighVal() > 0.6) {
    // Break joints
    for (auto &sa : superAgents) {
      sa.clean(box2d);
    }
    superAgents.clear();
    
    // Apply some random force on the agents.
    for (auto &a : agents) {
      a -> setCentrifugalForce(0.2);
    }
  }
  
  handleSerial();
  
  ofRemove(superAgents, [&](SuperAgent &sa){
    sa.update(box2d, sounds, maxJointForce, enableSound);
    return sa.shouldRemove;
  });
  
  // [TODO] Hook this boolean to a motion sensor value.
  if (mutateColors) {
    for (auto &a: agents) {
      if (a->getPartner() == NULL) {
        a->mutateTexture();
      }
    }
    mutateColors = false;
  }
  
  // GUI props.
  updateAgentProps();
  
  // Update agents.
  for (auto &a : agents) {
    a -> update();
  }
  
  // Create super agents based on collision bodies.
  createSuperAgents();
}

//--------------------------------------------------------------
void ofApp::draw(){
  if (debug) {
    ofPushStyle();
      fft.drawBars();
      fft.drawDebug();
      fft.drawHistoryGraph(ofPoint(824,0), LOW);
      fft.drawHistoryGraph(ofPoint(824,200),MID );
      fft.drawHistoryGraph(ofPoint(824,400),HIGH );
      fft.drawHistoryGraph(ofPoint(824,600),MAXSOUND );
      ofDrawBitmapString("LOW",850,20);
      ofDrawBitmapString("HIGH",850,420);
      ofDrawBitmapString("MID",850,220);
      ofDrawBitmapString("MAX VOLUME",850,620);
    ofPopStyle();
  }
  
  // Draw the bounds
  ofPushStyle();
    ofSetColor(ofColor::fromHex(0x341517));
    ofFill();
    ofDrawRectangle(0, 0, bounds.x, ofGetHeight());
    ofDrawRectangle(0, ofGetHeight() - bounds.x, ofGetWidth(), bounds.x);
    ofDrawRectangle(ofGetWidth()-bounds.x, 0, bounds.x, ofGetHeight());
  ofPopStyle();
  
  for (auto j : newJoints) {
    ofPushStyle();
      ofSetColor(ofColor::red);
      ofSetLineWidth(3);
      j->draw();
    ofPopStyle();
  }

  // Draw all what's inside the super agents.
  for (auto sa: superAgents) {
    sa.draw();
  }
  
  for (auto a: agents) {
    a -> draw(debug);
  }
  
  // Health parameters
  if (hideGui) {
    gui.draw();
  }
}

void ofApp::processOsc() {
  while(receiver.hasWaitingMessages()){
    // get the next message
    ofxOscMessage m;
    receiver.getNextMessage(m);
    
    if(m.getAddress() == "/interMesh/width"){
      float val = m.getArgAsFloat(0);
      cout << val << endl;
      meshWidth = ofMap(val, 0, 1, 50, 250, true);
      cout << meshWidth << endl;
    }
    
    if(m.getAddress() == "/interMesh/height"){
      float val = m.getArgAsFloat(0);
      meshHeight = ofMap(val, 0, 1, 50, 250, true);
    }
    
    if(m.getAddress() == "/interMesh/rows"){
      float val = m.getArgAsFloat(0);
      meshRows = ofMap(val, 0, 1, 5, 15, true);
    }
    
    if(m.getAddress() == "/interMesh/columns"){
      float val = m.getArgAsFloat(0);
      meshColumns = ofMap(val, 0, 1, 5, 15, true);
    }
    
    if(m.getAddress() == "/interMesh/jointForce"){
      float val = m.getArgAsFloat(0);
      maxJointForce = ofMap(val, 0, 1, 1, 25, true);
    }
    
    if(m.getAddress() == "/interMesh/newMesh"){
      int val = m.getArgAsInt(0);
      createAgent();
    }
    
    if(m.getAddress() == "/interMesh/clearScreen"){
      int val = m.getArgAsInt(0);
      clearScreen();
    }
    
    if(m.getAddress() == "/interMesh/debug"){
      int val = m.getArgAsInt(0);
      debug = !debug;
    }
    
    if(m.getAddress() == "/interMesh/removeUnbonded"){
      int val = m.getArgAsInt(0);
      //removeUnbonded();
    }
    
    if(m.getAddress() == "/interMesh/removeJoints"){
      int val = m.getArgAsInt(0);
      removeJoints();
    }
    
    if(m.getAddress() == "/interMesh/sound"){
      int val = m.getArgAsInt(0);
      enableSound = !enableSound;
    }
  }
}

void ofApp::updateAgentProps() {
    // Create Soft Body payload to create objects.
  agentProps.meshDimensions = ofPoint(meshRows, meshColumns);
  agentProps.meshSize = ofPoint(meshWidth, meshHeight);
  agentProps.vertexRadius = vertexRadius;
  agentProps.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  agentProps.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
}

void ofApp::createAgent() {
  Agent *a = new Agent();
  a->setup(box2d, agentProps);
  agents.push_back(a);
}

void ofApp::setupGui() {
    gui.setup();
    settings.setName("Inter Mesh Settings");
  
    // Mesh parameters.
    meshParams.setName("Mesh Params");
    meshParams.add(meshRows.set("Mesh Rows", 5, 5, 100)); // Add the current value
    meshParams.add(meshColumns.set("Mesh Columns", 5, 5, 100));
    meshParams.add(meshWidth.set("Mesh Width", 100, 10, ofGetWidth()/2));
    meshParams.add(meshHeight.set("Mesh Height", 100, 10, ofGetHeight()/2));
  
    // Vertex parameters
    vertexParams.setName("Vertex Params");
    vertexParams.add(vertexRadius.set("Vertex Radius", 6, 1, 30));
    vertexParams.add(vertexDensity.set("Vertex Density", 1, 0, 5));
    vertexParams.add(vertexBounce.set("Vertex Bounce", 0.3, 0, 1));
    vertexParams.add(vertexFriction.set("Vertex Friction", 1, 0, 1));
  
    // Joint parameters
    jointParams.setName("Joint Params");
    jointParams.add(jointFrequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    jointParams.add(jointDamping.set("Joint Damping", 1.0f, 0.0f, 5.0f));
  
    // InterAgentJoint parameters
    interAgentJointParams.setName("InterAgentJoint Params");
    interAgentJointParams.add(frequency.set("Joint Frequency", 2.0f, 0.0f, 20.0f));
    interAgentJointParams.add(damping.set("Joint Damping", 1.0f, 0.0f, 10.0f));
    interAgentJointParams.add(maxJointForce.set("Max Joint Force", 6.f, 1.f, 20.0f));
  
    settings.add(meshParams);
    settings.add(vertexParams);
    settings.add(jointParams);
    settings.add(interAgentJointParams);
  
    gui.setup(settings);
    gui.loadFromFile("InterMesh.xml");
}

void ofApp::enableRepulsion() {
//    // Enable repelling on the agent.
//    for (auto &j: interAgentJoints) {
//      auto bodyA = j->joint->GetBodyA();
//      auto bodyB = j->joint->GetBodyB();
//      
//      // Agent A (centroid will be the repulsion point)
//      auto agentIdA = reinterpret_cast<VertexData*>(bodyA -> GetUserData()) -> agentId;
//      auto &agentA = agents.at(agentIdA);
//      auto centroidA = agentA.getCentroid();
//      
//      // Agent B (centroid will be the repulsion point)
//      auto agentIdB = reinterpret_cast<VertexData*>(bodyB -> GetUserData()) -> agentId;
//      auto &agentB = agents.at(agentIdB);
//      auto centroidB = agentB.getCentroid();
//      
//      // Set a repulsion target for agent A
//      //agentA.setRepulsionTarget(&agentB, agentIdB);
//      agentB.setRepulsionTarget(&agentA, agentIdA);
//    }
}

void ofApp::clearScreen() {
  // [WARNING] For some reason, these events are still fired when trying to clean things as one could be in the
    // middle of a step function. Disabling and renabling the events work as a good solution for now.
  box2d.disableEvents();
  collidingBodies.clear();

  // Clear SuperAgents
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();

  // Clean agents
  for (auto &a : agents) {
    a -> clean(box2d);
    delete a;
  }
  agents.clear();

  box2d.enableEvents();
}

void ofApp::removeUnbonded() {
  ofRemove(agents, [&](Agent *a) {
    if (a->getPartner() == NULL) {
      a->clean(box2d);
      return true;
    }
  });
}

void ofApp::removeJoints() {
  box2d.disableEvents();

  // Clear superAgents only
  for (auto &sa : superAgents) {
    sa.clean(box2d);
  }
  superAgents.clear();

  superAgents.clear();
  box2d.enableEvents();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){  
  if (key == 'd') {
    debug = !debug;
  }
  
  if (key == 'n') {
    createAgent(); 
  }
  
  if (key == 'c') {
    clearScreen();
  }
  
  if (key == 'j') {
    removeJoints();
  }
  
  if (key == 'h') {
    hideGui = !hideGui;
  }
  
  if (key == 'f') {
    // Apply a random force
    for (auto &a: agents) {
      a -> setRandomForce(1.0);
    }
  }
  
  if (key == 'r') {
    enableRepulsion();
  }
  
  if (key == 'm') { // Mutate
    mutateColors = true;
  }
  
  if (key == 's') {
    enableSound = !enableSound;
  }
}

void ofApp::mousePressed(int x, int y, int button) {
   for (auto &a: agents) {
    a -> setAttractionTarget(glm::vec2(x, y));
  }
}

void ofApp::exit() {
  box2d.disableEvents();
  gui.saveToFile("InterMesh.xml");
}

// Massive important function that determines when the 2 bodies actually bond.
void ofApp::evaluateBonding(b2Body *bodyA, b2Body *bodyB, Agent *agentA, Agent *agentB) {
  collidingBodies.clear();
  
  // Visual similarly
  bool isSimilar = hasVisualSimilarities(agentA, agentB);
  if (isSimilar) {
    // Is AgentA's partner AgentB
    // Is AgentB's partner AgentA
    if ((agentA -> getPartner() == agentB || agentA -> getPartner() == NULL)
          && (agentB -> getPartner() == NULL || agentB -> getPartner() == agentA)) {
      // Vertex level checks. Is this vertex bonded to anything except itself?
      bool a = canVertexBond(bodyA, agentA);
      bool b = canVertexBond(bodyB, agentB);
      if (a && b) {
        // Prepare for bond.
        collidingBodies.push_back(bodyA);
        collidingBodies.push_back(bodyB);
      }
    }
  }
}

bool ofApp::hasVisualSimilarities(Agent *agentA, Agent *agentB) {
  // How many common colors are between the two arrays?
  int commonColorsNum = 0;
  int uncommonColorsNum = 0;
  bool a; bool b;

  auto colors = agentA -> colors;
  for (int i = 0; i < colors.size(); i++) {
    a = ofContains(agentA -> colorSlots, colors.at(i));
    b = ofContains(agentB -> colorSlots, colors.at(i));
    if (a & b) {
      commonColorsNum++;
    }

    if (!a && !b) {
      // Color isn't in a and b.
    } else {
      // It's in one but not the other.
      uncommonColorsNum++;
    }
  }

  // Should be some common and some uncommon colors.
  return commonColorsNum >= 3 && uncommonColorsNum >= 2;
}

bool ofApp::canVertexBond(b2Body* body, Agent *curAgent) {
  // If it joins anything except itself, then it cannot join.
  auto curEdge = body -> GetJointList();
  // Traverse the joint doubly linked list.
  while (curEdge) {
    // Other agent that this joint is joined to.
    auto data = reinterpret_cast<VertexData*>(curEdge->other->GetUserData());
    if (data != NULL) {
      auto otherAgent = data->agent;
      if (otherAgent != curAgent) {
        return false;
      }
    }
    
    curEdge = curEdge -> next;
  }

  return true;
}

void ofApp::createSuperAgents() {
  // Joint creation based on when two bodies collide at certain vertices.
  if (collidingBodies.size()>0) {
    auto agentA = reinterpret_cast<VertexData*>(collidingBodies[0]->GetUserData())->agent;
    auto agentB = reinterpret_cast<VertexData*>(collidingBodies[1]->GetUserData())->agent;

    SuperAgent superAgent; bool found = false; bool triggerSound = false;
    std::shared_ptr<ofxBox2dJoint> j;
    // Check for existing joints.
    for (auto &sa : superAgents) {
      if (sa.contains(agentA, agentB)) {
        if (sa.joints.size() <= 7) {
          j = createInterAgentJoint(collidingBodies[0], collidingBodies[1]);
          sa.joints.push_back(j);
          found = true;
          triggerSound = true;
        } else {
          found = true;
        }
      }
    }
    
    if (!found) {
      j = createInterAgentJoint(collidingBodies[0], collidingBodies[1]);
      superAgent.setup(agentA, agentB, j); // Create a new super agent.
      superAgents.push_back(superAgent);
      agentA -> setPartner(agentB);
      agentB -> setPartner(agentA);
      triggerSound = true;
    }
    
    if (triggerSound) {
      auto data = (SoundData *) j -> joint -> GetUserData();
      if (enableSound) {
        sounds.at(data->joinIdx) -> play();
      }
    }
    
    collidingBodies.clear();
  }
}

std::shared_ptr<ofxBox2dJoint> ofApp::createInterAgentJoint(b2Body *bodyA, b2Body *bodyB) {
    auto j = std::make_shared<ofxBox2dJoint>();
    j->setup(box2d.getWorld(), bodyA, bodyB, frequency, damping); // Use the interAgentJoint props.
    j->setLength(ofRandom(50, 150));
  
    // Create User Data
//    auto soundIdx = ofRandom(0, sounds.size()); // Choose a different make/break sound for the joint.
    j->joint->SetUserData(new SoundData(5, 6));
  
    return j;
}

void ofApp::handleSerial() {
  while (serial.available() > 0)
    {
        // Read the byte.
        char b = serial.readByte();

        // End of line character.
        if (b == '\n')
        {
            // Skip
            cout<< "New line" << "\n";
        }
        else
        {
            // If it's not the line feed character, add it to the buffer.
            mutateColors = b - '0';
            cout << "Replling: " << mutateColors << "\n";
        }
    }
}



  // Agent Level Checks
  // 0
  // Both the agents should be different.

  // 1
  // Does AgentA's colors match with AgentB's colors. Does the visual appearance match
  // for both the agents to bond?

  // 2
  // Is AgentA bonded with anybody? If it's bonded with AgentB and itself, then keep going
  // Is AgentB bonded with anybody? If it's bonded with AgentA and itself, then keep going

  // Vertex level check
  // 3
  // If it's bonded to anything else except itself
  // then it cannot bond.
  // Two agents are not the same
