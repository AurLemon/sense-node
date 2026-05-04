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

void EventBus::onImuInferenceCompleted(ImuInferenceCompletedHandler handler, void *context)
{
  for (auto &subscription : imuInferenceHandlers)
  {
    if (subscription.handler == nullptr)
    {
      subscription.handler = handler;
      subscription.context = context;
      return;
    }
  }
}

void EventBus::onHandStateUpdated(HandStateUpdatedHandler handler, void *context)
{
  for (auto &subscription : handStateHandlers)
  {
    if (subscription.handler == nullptr)
    {
      subscription.handler = handler;
      subscription.context = context;
      return;
    }
  }
}

void EventBus::onHandLeaveDetected(HandLeaveDetectedHandler handler, void *context)
{
  for (auto &subscription : handLeaveHandlers)
  {
    if (subscription.handler == nullptr)
    {
      subscription.handler = handler;
      subscription.context = context;
      return;
    }
  }
}

void EventBus::onFusionDecision(FusionDecisionHandler handler, void *context)
{
  for (auto &subscription : fusionHandlers)
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

void EventBus::emitImuInferenceCompleted(const ImuInferenceCompletedEvent &event)
{
  for (const auto &subscription : imuInferenceHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}

void EventBus::emitHandStateUpdated(const HandStateUpdatedEvent &event)
{
  for (const auto &subscription : handStateHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}

void EventBus::emitHandLeaveDetected(const HandLeaveDetectedEvent &event)
{
  for (const auto &subscription : handLeaveHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}

void EventBus::emitFusionDecision(const FusionDecisionEvent &event)
{
  for (const auto &subscription : fusionHandlers)
  {
    if (subscription.handler != nullptr)
    {
      subscription.handler(event, subscription.context);
    }
  }
}
