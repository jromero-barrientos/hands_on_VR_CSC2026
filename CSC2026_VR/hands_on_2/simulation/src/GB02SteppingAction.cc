#include "GB02SteppingAction.hh"
#include "GB02EventAction.hh"

#include "G4Step.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"

 #include <string>

GB02SteppingAction::GB02SteppingAction(GB02EventAction* eventAction)
  : G4UserSteppingAction()
  , fEventAction(eventAction)
{}

void GB02SteppingAction::UserSteppingAction(const G4Step* step)
{
  if (!step || !fEventAction) return;

  const auto* prePoint  = step->GetPreStepPoint();
  const auto* postPoint = step->GetPostStepPoint();
  if (!prePoint || !postPoint) return;

  auto* prePV = prePoint->GetPhysicalVolume();
  if (!prePV) return;

  // Count collisions only if the step starts inside the target volume
  if (prePV->GetName() != "test.phys") return;

  const auto* proc = postPoint->GetProcessDefinedStep();
  if (!proc) return;

  // A "collision" = any non-Transportation process limiting the step
  const auto& procName = proc->GetProcessName();
  if (procName == "Transportation") return;

  // No son colisiones físicas:
  if (procName == "StepLimiter") return;
  if (procName == "UserSpecialCuts") return;

  if (procName.find("biasWrapper") != std::string::npos) return;

  // Optional: restrict to particles that are actually biased in this example
  const auto* trk  = step->GetTrack();
  if (!trk) return;

  const auto* pdef = trk->GetParticleDefinition();
  const G4String pname = pdef ? pdef->GetParticleName() : "";
  if (pname != "gamma" && pname != "neutron") return;

  // Accumulate the weight at the interaction point
  fEventAction->AddCollision(postPoint->GetWeight());
}