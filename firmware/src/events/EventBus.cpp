#include "events/EventBus.h"

void EventBus::onModeChanged(ModeChangedHandler handler, void *context)
{
  for (auto &subscription : modeChangedHandlers)
  {
    if (subscription.handler == nullptr)
    {
      subscription.handler = handler;
      subscription.context = context;
      return;
    }
  }
}

void EventBus::onSensorFrameSampled(SensorFrameSampledHandler handler, void *context)
{
  for (auto &subscription : sensorFrameHandlers)
  {
    if (subscription.handler == nullptr)
    {
      subscription.handler = handler;
      subscription.context = context;
      return;
    }
  }
}

void EventBus::onDemoSerialTick(DemoSerialTickHandler handler, void *context)
{
  for (auto &subscription : demoSerialHandlers)
  {
    if (subscription.handler == nullptr)
    {
      subscription.handler = handler;
      subscription.context = context;
      return;
    }
  }
}

void EventBus::onInferenceCompleted(InferenceCompletedHandler handler, void *context)
{
  for (auto &subscription : inferenceHandlers)
  {
    if (subscription.handler == nullptr)
    {
      subscription.handler = handler;
      subscription.context = context;
      return;
    }
  }
}

void EventBus::emitModeChanged(const ModeChangedEvent &event)
{
  for (const auto &subscription : modeChangedHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}

void EventBus::emitSensorFrameSampled(const SensorFrameSampledEvent &event)
{
  for (const auto &subscription : sensorFrameHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}

void EventBus::emitDemoSerialTick(const DemoSerialTickEvent &event)
{
  for (const auto &subscription : demoSerialHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}

void EventBus::emitInferenceCompleted(const InferenceCompletedEvent &event)
{
  for (const auto &subscription : inferenceHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}
