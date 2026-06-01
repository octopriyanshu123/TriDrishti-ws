// =============================================================================
//  rcf_instantiate.cpp  --  local/action
//  Add one pair of lines per action triple you use.
// =============================================================================

#include "action/ActionServer.cpp"
#include "action/ActionClient.cpp"
#include "action/GoalHandle.cpp"

#include "shared_types.hpp"

namespace rcf {
    template class ActionServer<NavGoal,   NavFeedback,   NavResult>;
    template class ActionClient<NavGoal,   NavFeedback,   NavResult>;
    template class GoalHandle  <NavFeedback, NavResult>;
    template struct ClientGoalHandle<NavFeedback, NavResult>;
}
