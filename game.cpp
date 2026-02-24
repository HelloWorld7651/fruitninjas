//
// game.cpp - Fruit Ninja
// 

// Engine includes.
#include "GameManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "WorldManager.h"
#include "utility.h"
#include <stdlib.h>		
#include <string>
#include <stdlib.h>		// for setenv()
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <process.h>
#else
#include <sys/types.h>		// for getpid()
#include <unistd.h>		// for getpid()
#endif

// Game includes.
#include "game.h"
#include "util.h"
#include "client.h"
#include "server.h"

///////////////////////////////////////////////
int main(int argc, char *argv[]) {
  bool is_server;

  // Must specify 0 args if server, 1 arg if client.
  // For client, arg is host to connect to.
  if (argc != 1 && argc != 2)
    usage();

  // Setup logfile: server or client.
  if (argc == 1) {
    is_server = true;
  #if defined(_WIN32) || defined(_WIN64)
    _putenv_s("DRAGONFLY_LOG", "server.log");
    _putenv_s("DRAGONFLY_CONFIG", "df-config-server.txt");
  #else
    setenv("DRAGONFLY_LOG", "server.log", 1);
    setenv("DRAGONFLY_CONFIG", "df-config-server.txt", 1);
  #endif
  } else {
    is_server = false;
  #if defined(_WIN32) || defined(_WIN64)
    int pid = _getpid();
  #else
    int pid = getpid();
  #endif    
    std::string logfile = "client";
    logfile += std::to_string(pid);
    logfile += ".log";
  #if defined(_WIN32) || defined(_WIN64)
    _putenv_s("DRAGONFLY_LOG", logfile.c_str());
  #else
    setenv("DRAGONFLY_LOG", logfile.c_str(), 1);
  #endif
  }// Must specify 0 args if server, 1 arg if client.
  // For client, arg is host to connect to.
  if (argc != 1 && argc != 2)
    usage();

  // Setup logfile: server or client.
  if (argc == 1) {
    is_server = true;
  #if defined(_WIN32) || defined(_WIN64)
    _putenv_s("DRAGONFLY_LOG", "server.log");
    _putenv_s("DRAGONFLY_CONFIG", "df-config-server.txt");
  #else
    setenv("DRAGONFLY_LOG", "server.log", 1);
    setenv("DRAGONFLY_CONFIG", "df-config-server.txt", 1);
  #endif
  } else {
    is_server = false;
  #if defined(_WIN32) || defined(_WIN64)
    int pid = _getpid();
  #else
    int pid = getpid();
  #endif    
    std::string logfile = "client";
    logfile += std::to_string(pid);
    logfile += ".log";
  #if defined(_WIN32) || defined(_WIN64)
    _putenv_s("DRAGONFLY_LOG", logfile.c_str());
  #else
    setenv("DRAGONFLY_LOG", logfile.c_str(), 1);
  #endif
  }


  // Start up game manager.
  if (GM.startUp())  {
    LM.writeLog("Error starting game manager!");
    GM.shutDown();
    return 0;
  }

  // Setup logging.
  LM.setFlush(true);
  LM.setLogLevel(1);
  LM.writeLog("Fruit Ninja (v%.1f)", VERSION);
  
  // Load resources.
  loadResources();

  // Dragonfly splash screen.
  df::splash();

  // Fruit Ninja splash screen.
  splash();

  // Create initial game objects.
  populateWorld();

    // Startup Server/Client.
    if (is_server)
      new Server;
    else
      new Client(argv[1]);

  GM.run();

  // Shut everything down.
  GM.shutDown();

  // All is well.
  return 0;
}
