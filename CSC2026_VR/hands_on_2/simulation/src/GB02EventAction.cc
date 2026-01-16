#include "GB02EventAction.hh"
#include "GB02RunAction.hh"

#include "G4Event.hh"
#include "G4Threading.hh"
#include "G4ios.hh"
#include <cmath>
#include <iomanip>

GB02EventAction::GB02EventAction(GB02RunAction* runAction)
  : G4UserEventAction()
  , fRunAction(runAction)
{}

void GB02EventAction::BeginOfEventAction(const G4Event*)
{
  fEventQ = 0.0;
}

void GB02EventAction::AddCollision(G4double weight)
{
  fEventQ += weight;
}

void GB02EventAction::EndOfEventAction(const G4Event* evt)
{

  if (evt) {
    // --- ADD: one line per event
    if (fRunAction) fRunAction->LogEventQ(evt->GetEventID(), fEventQ);
  }
  
/*   if (evt && evt->GetEventID() < 20) {
    G4cout << "[DBG] event " << evt->GetEventID()
           << " eventQ(sum w over collisions)=" << fEventQ
           << " (isMaster=" << G4Threading::IsMasterThread() << ")"
           << G4endl;
  } */

  // --- SANITY PRINT: flag events whose per-event collision score differs from the apparent constant value
  // NOTE: with default G4cout formatting many distinct doubles *look* identical.
/*   if (evt) {
    constexpr G4double kRefQ   = 0.000130436;  // value observed in the terminal
    constexpr G4double kAbsTol = 1e-12;
    constexpr G4double kRelTol = 1e-6;
    const G4double tol = std::max(kAbsTol, kRelTol * std::fabs(kRefQ));

    const G4double dQ = fEventQ - kRefQ;
    if (std::fabs(dQ) > tol) {
      // Save/restore stream state so we don't mess up other Geant4 prints
      const std::ios_base::fmtflags oldFlags = G4cout.flags();
      const std::streamsize oldPrec = G4cout.precision();

      G4cout.setf(std::ios::scientific, std::ios::floatfield);
      G4cout << std::setprecision(17);

      G4cout << "[SANITY] event " << evt->GetEventID()
             << " fEventQ=" << fEventQ
             << " dQ=" << dQ
             << " ref=" << kRefQ
             << " tol=" << tol
             << G4endl;

      G4cout.flags(oldFlags);
      G4cout.precision(oldPrec);
    }
  } */

  if (fRunAction) fRunAction->AddEventQ(fEventQ);
}