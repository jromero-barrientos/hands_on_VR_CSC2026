#ifndef GB02EventAction_h
#define GB02EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class GB02RunAction;
class G4Event;

class GB02EventAction : public G4UserEventAction
{
  public:
    explicit GB02EventAction(GB02RunAction* runAction);
    ~GB02EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    // Accumulate the weighted number of collisions in the target (per event)
    void AddCollision(G4double weight);

  private:
    GB02RunAction* fRunAction = nullptr;
    G4double fEventQ = 0.0;  // sum of weights over collisions in this event
};

#endif