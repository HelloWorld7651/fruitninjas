//
// Sword.cpp
//

// Engine includes.
#include "DisplayManager.h"
#include "EventCollision.h"
#include "EventKeyboard.h"
#include "EventMouse.h"
#include "EventView.h"
#include "Fader.h"
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"
#include "NetworkManager.h"
#include "NetMessages.h"
#include "EventNetwork.h"

// Game includes.
#include "Fruit.h"
#include "bomb.h"
#include "Grocer.h"
#include "Kudos.h"
#include "Sword.h"
#include "Timer.h"
#include "util.h"

// Constructor.
Sword::Sword() {

  setType(SWORD_STRING);
  setSolidness(df::SPECTRAL);
  setAltitude(df::MAX_ALTITUDE); // Make Sword in foreground.

  registerInterest(df::KEYBOARD_EVENT);
  registerInterest(df::STEP_EVENT);
  
  // Start sword in center of world.
  df::Vector p(WM.getBoundary().getHorizontal()/2,
               WM.getBoundary().getVertical()/2);
  setPosition(p);

  m_old_position = getPosition();
  m_color = df::CYAN;
  m_sliced = 0;
  m_old_sliced = 0;
}

// Handle event.
// Return 0 if ignored, else 1.
int Sword::eventHandler(const df::Event *p_e) {
  // Step event.
  if (p_e->getType() == df::STEP_EVENT)
  {
    // do nothign if sword hasn't moved
    if (m_old_position == getPosition()) {
      m_sliced = 0;
      return 1;
    }
    //draw trail
    create_trail(getPosition(), m_old_position);

    //check collision and update score, only server
   if (NM.isServer()) {
    //draw invisible line where sword swung
    df::Line line(getPosition(), m_old_position);
    df::ObjectList ol = WM.solidObjects();
    //check every object if sword hit
    for (int i = 0; i < ol.getCount(); i++) {
      //skip if not fruit or bomb
      if (!(dynamic_cast <Fruit *> (ol[i])) && !(dynamic_cast <Bomb *> (ol[i]))) {
        continue;
      }   
    //this involves basic fruit functionality. So tells fruit it has been sliced, and points in general
    df::Object *p_o = ol[i];
    df::Box box = getWorldBox(p_o);
    if (lineIntersectsBox(line, box)) {
      df::EventCollision c(this, p_o, p_o->getPosition());
      p_o->eventHandler(&c);
      m_sliced += 1;
              
      if (m_sliced > 2 && m_sliced > m_old_sliced) {
        new Kudos();
      }
      m_old_sliced = m_sliced;
      }
    }
    //sounds
    float dist = df::distance(getPosition(), m_old_position);
    if (dist > 15) {
      std::string sound = "swipe-" + std::to_string(rand()%7 + 1);
      play_sound(sound);
    }
    int penalty = -1 * (int)(dist/10.0f);
    df::EventView ev("Points", penalty, true);
    WM.onEvent(&ev);
  } 
  //save position for next draw of frame
  m_old_position = getPosition();
  return 1;
}
// Keyboard event.
if (p_e->getType() == df::KEYBOARD_EVENT){
  const df::EventKeyboard *p_kb_event = dynamic_cast <const df::EventKeyboard *> (p_e);
  //checking if client regestrited to quit
  if (p_kb_event->getKey() == df::Keyboard::Q &&p_kb_event->getKeyboardAction() == df::KEY_PRESSED) {
  // if client ask to quit
    if (!NM.isServer()) {
      //create exit message
      //send to server then stop
      NetHeader msg;
      msg.size = sizeof(NetHeader);
      msg.type = MessageType::EXIT;
      NM.send(&msg, sizeof(NetHeader), 0);
      return 1;
    }

  // Server will do gameover of everythign for client
  df::ObjectList ol = WM.objectsOfType(GROCER_STRING);
  if (ol.getCount() > 0 && (dynamic_cast <Grocer *> (ol[0]))) {
    Grocer *p_g = dynamic_cast <Grocer *> (ol[0]);
    p_g->gameOver();
  }
      
  ol = WM.objectsOfType(TIMER_STRING);
  if (ol.getCount() > 0 && (dynamic_cast <Timer *> (ol[0]))) {
    Timer *p_t = dynamic_cast <Timer *> (ol[0]);
    p_t->setValue(0);
  }
  return 1;
}
}
return 0;
}

// Draw sword on window.
int Sword::draw() {
  return DM.drawCh(getPosition(), SWORD_CHAR, m_color);
}
