#ifndef GB02SteppingAction_h
#define GB02SteppingAction_h 1

#include "G4UserSteppingAction.hh"

class GB02EventAction;
class G4Step;

class GB02SteppingAction : public G4UserSteppingAction
{
  public:
    explicit GB02SteppingAction(GB02EventAction* eventAction);
    ~GB02SteppingAction() override = default;

    void UserSteppingAction(const G4Step* step) override;

  private:
    GB02EventAction* fEventAction = nullptr;
};

#endif
