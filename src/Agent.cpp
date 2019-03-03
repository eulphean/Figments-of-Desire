#include "Agent.h"

void Agent::setup(ofxBox2d &box2d, AgentProperties agentProps) {
  createMesh(agentProps);
  createSoftBody(box2d, agentProps);
  
  // Set these values based on the size of the agent.
  float area = agentProps.meshSize.x * agentProps.meshSize.y;
  
  maxAttForceAmt = 0.1;  // Set this based on the size of the mesh
  maxRandForceAmt = 1.0;
  
  applyRandomForce = true;
  updateTarget = false;
}

void Agent::update() {
  // Update each point in the mesh according to the
  // box2D vertex.
  auto meshPoints = mesh.getVertices();
  
  for (int j = 0; j < meshPoints.size(); j++) {
    // Get the box2D vertex position.
    glm::vec2 pos = vertices[j] -> getPosition();
    
    // Update mesh point's position with the position of
    // the box2d vertex.
    auto meshPoint = meshPoints[j];
    meshPoint.x = pos.x;
    meshPoint.y = pos.y;
    mesh.setVertex(j, meshPoint);
  }
  
  // Apply behavioral forces on the body.
  applyBehaviors();
}

void Agent::draw(bool showSoftBody) {
  // Draw the meshes.
  // Draw the soft bodies.
  if (showSoftBody) {
    ofPushStyle();
      for(auto v: vertices) {
        ofNoFill();
        ofSetColor(ofColor::red);
        v->draw();
      }
    ofPopStyle();
    
    ofSetColor(ofColor::red);
    mesh.draw();
  }
  
  auto centroid = mesh.getCentroid();
  ofSetColor(ofColor::black);
  ofDrawCircle(centroid.x, centroid.y, 5);
}

void Agent::applyBehaviors() {
  // New target? Add an impulse in that direction.
  if (updateTarget) {
    // seek()
    for (auto &v: vertices) {
        v->addAttractionPoint(targetPos.x, targetPos.y, maxAttForceAmt);
        v->setRotation(ofRandom(120));
    }
    updateTarget = false;
  }
  
  // Random force/ Force all vertices.
  if (applyRandomForce) {
    // force()
    for (auto &v: vertices) {
      v -> addForce(glm::vec2(ofRandom(-20, 20), ofRandom(-20, 20)), maxRandForceAmt);
    }
    applyRandomForce = false;
  }
}

void Agent::createMesh(AgentProperties agentProps) {
  //auto a = ofRandom(50, ofGetWidth() - 50); auto b = ofRandom(50, ofGetHeight() - 50);
  auto a = ofPoint(100, 100);
  auto meshOrigin = glm::vec2(a.x, a.y);
  
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = agentProps.meshDimensions.x;
  int nCols = agentProps.meshDimensions.y;
  
  // Width, height for mapping the correct texture coordinate.
  int w = agentProps.meshSize.x;
  int h = agentProps.meshSize.y;
  
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = meshOrigin.x + w * x / (nCols - 1);
      float iy = meshOrigin.y + h * y / (nRows - 1);
      mesh.addVertex({ix, iy, 0});
    }
  }

  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < nRows - 1; y++)
  {
      for (int x = 0; x < nCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * nCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));
      }
  }
}

void Agent::createSoftBody(ofxBox2d &box2d, AgentProperties agentProps) {
  auto meshVertices = mesh.getVertices();
  
  vertices.clear();
  joints.clear();

  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(agentProps.vertexPhysics.x, agentProps.vertexPhysics.y, agentProps.vertexPhysics.z); // bounce, density, friction
    vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, ofRandom(agentProps.vertexRadius));
    vertex -> setFixedRotation(true);
    vertex->setData(new VertexData(agentProps.agentId)); // Data to identify current agent.
    vertices.push_back(vertex);
  }
  
  int meshRows = agentProps.meshDimensions.x;
  int meshColumns = agentProps.meshDimensions.y;
  
  // Create Box2d joints for the mesh.
  for (int y = 0; y < meshRows; y++) {
    for (int x = 0; x < meshColumns; x++) {
      int idx = x + y * meshColumns;
      
      // Do this for all columns except last column.
      // NOTE: Connect current vertex with the next vertex in the same row.
      if (x != meshColumns - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int rightIdx = idx + 1;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[rightIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y); // frequency, damping
        joints.push_back(joint);
      }
      
      
      // Do this for each row except the last row. There is no further joint to
      // be made there.
      if (y != meshRows - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int downIdx = x + (y + 1) * meshColumns;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[downIdx] -> body, agentProps.jointPhysics.x, agentProps.jointPhysics.y);
        joints.push_back(joint);
      }
    }
  }
}

void Agent::clean() {
  // Removes vertices.
  ofRemove(vertices, [&](std::shared_ptr<ofxBox2dCircle> c){
      return true;
  });
  
  // Remove joints.
  ofRemove(joints, [&](std::shared_ptr<ofxBox2dJoint> j){
      return true;
  });
}

void Agent::setTarget(int x, int y) {
  targetPos = glm::vec2(x, y);
  updateTarget = true;
}

void Agent::setRandomForce() {
  applyRandomForce = true;
}

std::shared_ptr<ofxBox2dCircle> Agent::getRandomVertex() {
  int randV = ofRandom(vertices.size());
  auto v = vertices[randV];
  return v;
}

// Avoid texture right now.
//      // Since, we have ofDisableArbTex, we map the coordinates from 0 - 1.
//      float texX = ofMap(ix, 0, agentProps.textureDimensions.x, 0, 1, true); // Map the calculated x coordinate from 0 - 1
//      float texY = ofMap(iy, 0, agentProps.textureDimensions.y, 0, 1, true); // Map the calculated y coordinate from 0 - 1
//      mesh.addTexCoord(glm::vec2(texX, texY));



    // Use rand vertices and add attraction points on it.
//    int maxForce = 10.0;
//    for (auto &v: vertices) {
//      //auto d = glm::distance(targetPos, {r->getPosition().x, r->getPosition().y});
//      //r -> addAttractionPoint(targetPos.x, targetPos.y, );
//      auto desired = glm::vec2(targetPos.x, targetPos.y) - glm::vec2(v->getPosition().x, v->getPosition().y); // Target - Location
////      desired = glm::normalize(desired);
////      desired = desired * 0.5;
////      auto steer = desired - v->getVelocity();
//      v->addForce({targetPos.x, targetPos.y}, 1.0);
//
//    }


//    for (auto &r : vertices) {
//      auto d = glm::distance(targetPos, {r->getPosition().x, r->getPosition().y});
//
////      float forceAmt;
////      //r -> setRotationFriction(0.1);
////      if (d <= 200) {
////        forceAmt = ofMap(d, 0, 200, -20/d, 1/d);
////        // I've reached
////      } else {
////        forceAmt = 1/d;
////      }
////
////      r -> addAttractionPoint({targetPos.x, targetPos.y}, forceAmt);
////      r -> setRotation(ofRandom(90));
////
////      if (d <= 30) {
////        updateTarget = false;
////      }
////
//
//
//      updateTarget = false;
//    }

//      glm::vec2 currentPos = glm::vec2(v->getPosition().x, v->getPosition().y);
//      auto desired = targetPos - currentPos; // Target - Current
//      desired = glm::normalize(desired);
//      auto steer = v->getVelocity() - desired;
//      v->addImpulseForce(steer, maxSteerImpulse);
//      v->addForce(steer, 0.2);
//      v->setRotation(ofRandom(60));
