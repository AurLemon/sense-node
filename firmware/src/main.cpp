#include <Arduino.h>

#include "app/AppController.h"

AppController app;

void setup()
{
  app.setup();
}

void loop()
{
  app.loop();
}
