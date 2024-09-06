/**
 * @brief Represents the different states of the sensor system for tracking object movement.
 *
 * The `SensorState` enum defines the various stages in the operation of the light barrier system,
 * as it detects and monitors objects moving through a designated area. Each state reflects a specific
 * combination of conditions of the indoor and outdoor sensors, indicating whether an object is entering,
 * exiting, or whether both sensors are open or closed.
 *
 * The states are defined as follows:
 * - `Open`: Both sensors are open, meaning no object is detected.
 * - `StartOutgoing`: The indoor sensor has detected an object, initiating an outgoing movement.
 * - `StartIncoming`: The outdoor sensor has detected an object, initiating an incoming movement.
 * - `ClosedOutgoing`: Both sensors are closed, indicating the object is fully inside the outgoing sensor range.
 * - `ClosedIncoming`: Both sensors are closed, indicating the object is fully inside the incoming sensor range.
 * - `CompleteOutgoing`: The object is exiting the room, completing the outgoing process.
 * - `CompleteIncoming`: The object is entering the room, completing the incoming process.
 * - `Outgoing`: The system has finished tracking an outgoing object, and it has fully exited the monitored area.
 * - `Incoming`: The system has finished tracking an incoming object, and it has fully entered the monitored area.
 */
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

/**
 * @brief Determines whether the barrier is open based on the analog value from a specified pin.
 *
 * This function reads the analog value from the given pin, which is connected
 * to an IR receiver, and compares it to a specified threshold. Higher analog values
 * indicate that something is blocking the signal between the IR transmitter and receiver.
 * If the analog reading is below the threshold, the function returns `true`, meaning
 * the barrier is open. Otherwise, it returns `false`, indicating the barrier is closed.
 *
 * @param pin The analog pin number from which the reading is taken, represented
 * as an 8-bit unsigned integer.
 *
 * @param threshold The threshold value against which the analog reading is compared.
 * If the reading is lower than this threshold, the barrier is considered open.
 *
 * @return true If the analog reading is less than the threshold, indicating the
 * barrier is open.
 * @return false If the analog reading is greater than or equal to the threshold,
 * indicating the barrier is closed.
 */
bool is_barrier_open(uint8_t pin, int threshold) {
  return analogRead(pin) < threshold;
}

/**
 * @brief Updates the current sensor state based on the status of indoor and outdoor sensors.
 *
 * This function evaluates the current state of the system (represented by `CURRENT_SENSOR_STATE`)
 * and updates it based on the boolean values provided for the indoor and outdoor sensors. The sensors
 * track whether the barrier is open (true) or closed (false). The system moves through different
 * states (e.g., `StartOutgoing`, `ClosedIncoming`, etc.) as objects pass through, based on sensor input.
 *
 * The logic is divided into cases representing different possible sensor states:
 * - `SensorState::Open`: The system starts in the open state. It transitions to either `StartOutgoing` or
 *   `StartIncoming` depending on whether the indoor or outdoor sensor changes state.
 * - `SensorState::StartOutgoing`: When the system starts outgoing, it moves to `ClosedOutgoing` if both sensors
 *   are closed, or back to `Open` if both sensors are open.
 * - `SensorState::StartIncoming`: Similar to `StartOutgoing`, the system moves to `ClosedIncoming` when both sensors
 *   are closed, or back to `Open` if both sensors are open.
 * - `SensorState::ClosedOutgoing`: After the outgoing path starts, the system can transition to `CompleteOutgoing`
 *   if the indoor sensor opens or back to `StartOutgoing` if the outdoor sensor remains open.
 * - `SensorState::ClosedIncoming`: Works similarly to `ClosedOutgoing`, but for incoming traffic.
 * - `SensorState::CompleteOutgoing`: When the outgoing process completes, the system can transition to the final
 *   `Outgoing` state, or back to `ClosedOutgoing` if no sensors are triggered.
 * - `SensorState::CompleteIncoming`: Similar to `CompleteOutgoing`, but for incoming traffic.
 * - `SensorState::Outgoing` and `SensorState::Incoming`: After completion, the system returns to the `Open` state.
 *
 * Each state has specific conditions for transitioning to the next state or returning to a prior state
 * based on the combination of indoor and outdoor sensor values. The function manages the system's state
 * transitions as objects move through the monitored area.
 *
 * @param is_sensor_indoor_open A boolean indicating whether the indoor sensor is open (true) or closed (false).
 * @param is_sensor_outdoor_open A boolean indicating whether the outdoor sensor is open (true) or closed (false).
 */
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


/**
 * @brief Returns a numerical representation of the current sensor state.
 *
 * This function converts the current sensor state (`CURRENT_SENSOR_STATE`) into an integer
 * that reflects the overall system status in a simplified form.
 *
 * The integer values correspond to the following sensor states:
 * - `0`: "Open" state, indicating both sensors are open and no object is detected.
 * - `1`: "Outgoing" state, meaning the system has detected an object moving outward.
 * - `2`: "Incoming" state, meaning the system has detected an object moving inward.
 * - `3`: "Closed" state, where one or both sensors are closed, indicating that the system is in a transitional phase or blocked.
 *
 * @return int A numerical code representing the current sensor state:
 * - 0 for "Open"
 * - 1 for "Outgoing"
 * - 2 for "Incoming"
 * - 3 for "Closed"
 */

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

/**
 * @brief Resets the light barrier sensor system to the "Open" state.
 *
 * This function resets the `CURRENT_SENSOR_STATE` to `SensorState::Open`, which represents
 * the default state where both sensors are open and no object is blocking the light barrier.
 * This can be used to initialize or reinitialize the sensor system, ensuring it starts
 * from a known state.
 */
void reset_lightbarrier_sensor() {
  CURRENT_SENSOR_STATE = SensorState::Open;
}

