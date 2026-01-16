// GB02RunAction.hh

#ifndef GB02RunAction_h
#define GB02RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "G4Timer.hh"
#include "globals.hh"

#include <fstream>   // <-- ADD
#include <string>    // <-- ADD

class G4Run;

class GB02RunAction : public G4UserRunAction
{
  public:
    GB02RunAction();
    ~GB02RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void AddEventQ(G4double q);

    // --- ADD: write one line per event (thread-local file)
    void LogEventQ(G4int eventID, G4double q);

  private:
    G4Accumulable<G4double> fSumQ;
    G4Accumulable<G4double> fSumQ2;

    G4Timer fTimer;

    // --- ADD: per-thread output
    std::ofstream fEvtOut;
    std::string   fEvtOutName;
    std::string   fRunTag;
    std::string   fOutDir;
};

#endif
