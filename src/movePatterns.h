
inline static constexpr movement Continous[] = { 
   { Speed70, Speed70, 3001},
   { Speed80, Speed80, 3001}, 
   { Speed90, Speed90, 3001}, 
   { Speed90, Speed90, 8001}, 
   { 0, 0, 0 }
};

inline static constexpr movement ChargerBackout[] = { 
   { -Speed20, -Speed20, 1002},
   { Speed00, -Speed20, 402}, 
   { Speed30,  Speed00, 1002}, 
   { Speed40,  Speed40, 602}, 
   { Speed90,  Speed90, 802}, 
   { 0, 0, 0 }
};

inline static constexpr movement BWFLeft[] = {
   { Speed40, Speed00, 403},
   { Speed40, -Speed10, 203},
   { Speed40, Speed10, 403},
   { Speed40, Speed40, 603},
   { Speed80, Speed80, 703},
   { 0, 0, 0 }
};

inline static constexpr movement BWFRight[] = {
   { Speed00, Speed40, 404},
   { -Speed10, -Speed40, 204},
   { Speed10, Speed40, 404},
   { Speed40, Speed40, 604},
   { Speed80, Speed80, 704},
   { 0, 0, 0 }
};

inline static constexpr movement Circle[] = {
   { Speed40, -Speed40, 605},
   { -Speed10, -Speed40, 705},
   { Speed70, Speed60, 805},
   { Speed90, Speed90, 905},
   { 0, 0, 0 }
};

inline static constexpr movement TurnLeft[] = {
   { Speed10, Speed60, 306},
   { Speed00 , Speed40, 1006},
   { Speed40, Speed40, 406},
   { Speed90, Speed90, 806},
   { 0, 0, 0 }
};

inline static constexpr movement SlowDown[] = {
   { Speed30, Speed30, 507},
   { Speed20 , Speed20, 107},
   { Speed20 , Speed20, 3007},
   { 0, 0, 0 }
};

inline static constexpr movement AvoidObstacle[] = {
   { Speed20, -Speed30, 308},
   { Speed20 , -Speed40, 408},
   { Speed30 , Speed30, 2008},
   { 0, 0, 0 }
};

// Emergency stop - immediate deceleration for safety
inline static constexpr movement EmergencyStop[] = {
   { Speed00, Speed00, 100},
   { 0, 0, 0 }
};

// Gentle stop - gradual deceleration to minimize grass wear
inline static constexpr movement GentleStop[] = {
   { Speed60, Speed60, 200},
   { Speed40, Speed40, 200},
   { Speed20, Speed20, 200},
   { Speed00, Speed00, 100},
   { 0, 0, 0 }
};

// Spot turn left 90° - in-place rotation for orientation correction
inline static constexpr movement SpotTurnLeft90[] = {
   { Speed00, Speed00, 200},     // Stop first
   { -Speed30, Speed30, 800},    // Rotate left ~90° at moderate speed
   { Speed00, Speed00, 200},     // Stop and settle
   { 0, 0, 0 }
};

// Spot turn right 90° - in-place rotation for orientation correction
inline static constexpr movement SpotTurnRight90[] = {
   { Speed00, Speed00, 200},     // Stop first
   { Speed30, -Speed30, 800},    // Rotate right ~90° at moderate speed
   { Speed00, Speed00, 200},     // Stop and settle
   { 0, 0, 0 }
};

// Spot turn 180° - full reverse orientation
inline static constexpr movement SpotTurn180[] = {
   { Speed00, Speed00, 200},     // Stop first
   { -Speed30, Speed30, 1600},   // Rotate left 180° at moderate speed
   { Speed00, Speed00, 200},     // Stop and settle
   { 0, 0, 0 }
};

// Reverse and turn - recovery pattern when stuck or blocked
inline static constexpr movement ReverseAndTurn[] = {
   { -Speed40, -Speed40, 1000},  // Reverse straight
   { -Speed30, Speed00, 400},    // Reverse while turning left
   { Speed00, Speed00, 200},     // Stop
   { Speed40, Speed40, 600},     // Move forward
   { 0, 0, 0 }
};

// Gentle arc left - smooth left turn while moving forward
inline static constexpr movement GentleArcLeft[] = {
   { Speed50, Speed70, 1000},    // Gentle left arc
   { Speed60, Speed80, 1000},    // Continue arc
   { Speed70, Speed90, 1000},    // Complete arc
   { 0, 0, 0 }
};

// Gentle arc right - smooth right turn while moving forward
inline static constexpr movement GentleArcRight[] = {
   { Speed70, Speed50, 1000},    // Gentle right arc
   { Speed80, Speed60, 1000},    // Continue arc
   { Speed90, Speed70, 1000},    // Complete arc
   { 0, 0, 0 }
};

// Perimeter search spiral - small spiral pattern to relocate boundary
inline static constexpr movement PerimeterSearchSpiral[] = {
   { Speed40, Speed40, 500},     // Forward
   { Speed30, Speed50, 400},     // Turn right slight
   { Speed40, Speed40, 700},     // Forward longer
   { Speed30, Speed50, 400},     // Turn right slight
   { Speed40, Speed40, 900},     // Forward even longer
   { Speed30, Speed50, 400},     // Turn right slight
   { Speed40, Speed40, 1100},    // Forward longest
   { 0, 0, 0 }
};

// GPS drift correction - gentle forward movement for position update
inline static constexpr movement GPSDriftCorrection[] = {
   { Speed30, Speed30, 2000},    // Slow forward for 2 seconds
   { Speed00, Speed00, 1000},    // Stop and wait for GPS lock
   { 0, 0, 0 }
};

// Obstacle nudge left - gentle push to avoid obstacle on right
inline static constexpr movement ObstacleNudgeLeft[] = {
   { Speed50, Speed70, 300},     // Nudge left
   { Speed60, Speed60, 500},     // Continue forward
   { Speed70, Speed50, 300},     // Nudge back right
   { Speed70, Speed70, 500},     // Resume straight
   { 0, 0, 0 }
};

// Obstacle nudge right - gentle push to avoid obstacle on left
inline static constexpr movement ObstacleNudgeRight[] = {
   { Speed70, Speed50, 300},     // Nudge right
   { Speed60, Speed60, 500},     // Continue forward
   { Speed50, Speed70, 300},     // Nudge back left
   { Speed70, Speed70, 500},     // Resume straight
   { 0, 0, 0 }
};

// Teardrop turn 180° - smooth arc turn for stripe mowing
// This pattern complements the GPS-based ParallelStripeMower arc generation
// Useful for manual control or when GPS waypoints aren't available
inline static constexpr movement TeardropTurn180[] = {
   { Speed60, Speed60, 300},     // Approach at moderate speed
   { Speed70, Speed90, 600},     // Start gentle right arc
   { Speed60, Speed90, 600},     // Tighten arc (more differential)
   { Speed50, Speed90, 600},     // Continue arc
   { Speed60, Speed90, 600},     // Maintain arc
   { Speed70, Speed90, 600},     // Widen arc slightly
   { Speed80, Speed80, 300},     // Straighten out
   { 0, 0, 0 }
};

// Teardrop turn 180° left - mirror of right turn
inline static constexpr movement TeardropTurn180Left[] = {
   { Speed60, Speed60, 300},     // Approach at moderate speed
   { Speed90, Speed70, 600},     // Start gentle left arc
   { Speed90, Speed60, 600},     // Tighten arc (more differential)
   { Speed90, Speed50, 600},     // Continue arc
   { Speed90, Speed60, 600},     // Maintain arc
   { Speed90, Speed70, 600},     // Widen arc slightly
   { Speed80, Speed80, 300},     // Straighten out
   { 0, 0, 0 }
};
