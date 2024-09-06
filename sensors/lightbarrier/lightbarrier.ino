enum class SensorState {
  Open,
  StartOutgoing,
  StartIncoming,
  ClosedOutgoing,
  ClosedIncoming,
  CompleteOutgoing,
  CompleteIncoming,
  Outgoing,
  Incoming
};

SensorState CURRENT_SENSOR_STATE = SensorState::Open;

bool is_barrier_open(uint8_t pin, int threshold) {
  return analogRead(pin) < threshold;
}

void changeState(bool is_sensor_indoor_open, bool is_sensor_outdoor_open) {
  switch(CURRENT_SENSOR_STATE) {
    case SensorState::Open:
      if (!is_sensor_indoor_open && is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::StartOutgoing;
      } else if (is_sensor_indoor_open && !is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::StartIncoming;
      }
      break;
    case SensorState::StartOutgoing:
      if(!is_sensor_indoor_open && !is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::ClosedOutgoing;
      } else if (is_sensor_indoor_open && is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::Open;
      }
      break;
    case SensorState::StartIncoming:
      if(!is_sensor_indoor_open && !is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::ClosedIncoming;
      } else if (is_sensor_indoor_open && is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::Open;
      }
      break;
    case SensorState::ClosedOutgoing:
      if(is_sensor_indoor_open && !is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::CompleteOutgoing;
      } else if (!is_sensor_indoor_open && is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::StartOutgoing;
      }
      break;
    case SensorState::ClosedIncoming:
      if(!is_sensor_indoor_open && is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::CompleteIncoming;
      } else if (is_sensor_indoor_open && !is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::StartIncoming;
      }
      break;
    case SensorState::CompleteOutgoing:
      if(is_sensor_indoor_open && is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::Outgoing;
      } else if (!is_sensor_indoor_open && !is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::ClosedOutgoing;
      }
      break;
    case SensorState::CompleteIncoming:
      if(is_sensor_indoor_open && is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::Incoming;
      } else if (!is_sensor_indoor_open && !is_sensor_outdoor_open) {
        CURRENT_SENSOR_STATE = SensorState::ClosedIncoming;
      }
      break;
    // TODO: Check if next Incoming|Outgoing starts. In Case both are open go to Open State
    case SensorState::Outgoing:
      CURRENT_SENSOR_STATE = SensorState::Open;
      break;
    // TODO: Check if next Incoming|Outgoing starts. In Case both are open go to Open State
    case SensorState::Incoming:
      CURRENT_SENSOR_STATE = SensorState::Open;
      break;
  }
}

int get_sensor_state() {
  switch(CURRENT_SENSOR_STATE) {
    case SensorState::Open:
      return 0;
    case SensorState::StartOutgoing:
    case SensorState::StartIncoming:
    case SensorState::ClosedOutgoing:
    case SensorState::ClosedIncoming:
    case SensorState::CompleteOutgoing:
    case SensorState::CompleteIncoming:
      return 3;
    case SensorState::Outgoing:
      return 1;
    case SensorState::Incoming:
      return 2;
  }
}

void reset_lightbarrier_sensor() {
  CURRENT_SENSOR_STATE = SensorState::Open;
}

