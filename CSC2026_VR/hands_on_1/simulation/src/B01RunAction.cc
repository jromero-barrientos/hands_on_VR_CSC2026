//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the  terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file biasing/B01/src/B01RunAction.cc
/// \brief Implementation of the B01RunAction class
//
//
//
#include "B01RunAction.hh"

#include "B01Run.hh"

//-- In order to obtain detector information.
#include "B01DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4THitsMap.hh"
#include "G4UnitsTable.hh"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
// B01RunAction
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Constructor
B01RunAction::B01RunAction()
  : G4UserRunAction(),
    //  fFieldName(15),
    fFieldValue(14)
    // fTimer(new G4Timer)
{
  // - Prepare data member for B01Run.
  //   vector represents a list of MultiFunctionalDetector names.
  fSDName.push_back(G4String("ConcreteSD"));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Destructor.
B01RunAction::~B01RunAction()
{
  fSDName.clear();
  // delete fTimer;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* B01RunAction::GenerateRun()
{
  // Generate new RUN object, which is specially
  // dedicated for MultiFunctionalDetector scheme.
  //  Detail description can be found in B01Run.hh/cc.
  return new B01Run(fSDName);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B01RunAction::BeginOfRunAction(const G4Run* aRun)
{
  G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;

  // Solo el master mide el tiempo global del run
  if (IsMaster()) {
    fTimer.Start();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B01RunAction::EndOfRunAction(const G4Run* aRun)
{
  // Master timing
  G4double elapsedTime = 0.0;

  // --- ASCII output (master only)
  std::ofstream cellsOut;
  bool writeCells = false;

  // Tag files with mode without wiring mode through C++.
  // In notebook/terminal you can do:  env B01_MODE=-1 ./exampleB01 -1 100000
  std::string modeTag = "NA";
  if (const char* m = std::getenv("B01_MODE")) {
    modeTag = m;
  }

  if (IsMaster()) {
    fTimer.Stop();
    elapsedTime = fTimer.GetRealElapsed();
    G4cout << "\n=== Run timing (master) === " << elapsedTime << G4endl;

    try {
      std::filesystem::create_directories("out");
    } catch (...) {
      // If directory creation fails, we still keep screen output.
    }

    std::ostringstream cellsName;
    cellsName << "out/b01_cells_mode" << modeTag << "_run" << aRun->GetRunID() << "_N"
              << aRun->GetNumberOfEvent() << ".tsv";

    cellsOut.open(cellsName.str());
    if (cellsOut) {
      writeCells = true;
      // Metadata as comments (easy to parse / ignore)
      cellsOut << "# runID\t" << aRun->GetRunID() << "\n";
      cellsOut << "# mode\t" << modeTag << "\n";
      cellsOut << "# Nevt\t" << aRun->GetNumberOfEvent() << "\n";
      cellsOut << "# time_real_s\t" << elapsedTime << "\n";
      // Header
      cellsOut << "cell\tcellName\tTrEntering\tPopulation\tAvTrWgt\t"
               << "SL_mean_mm\tSL_stderr_mm\tSLW_mean_mm\tSLW_stderr_mm\n";
    }
  }

  G4cout << " ###### EndOfRunAction  " << G4endl;

  //- B01Run object.
  B01Run* b01Run = (B01Run*)aRun;
  G4RunManager* mgr = G4RunManager::GetRunManager();

  for (G4int i = 0; i < (G4int)fSDName.size(); i++) {
    const G4VUserDetectorConstruction* vdet = mgr->GetUserDetectorConstruction();
    B01DetectorConstruction* bdet = (B01DetectorConstruction*)vdet;

    // Maps
    G4THitsMap<G4double>* Population = b01Run->GetHitsMap(fSDName[i] + "/Population");
    G4THitsMap<G4double>* TrackEnter = b01Run->GetHitsMap(fSDName[i] + "/TrackEnter");
    G4THitsMap<G4double>* SL = b01Run->GetHitsMap(fSDName[i] + "/SL");
    G4THitsMap<G4double>* SLW = b01Run->GetHitsMap(fSDName[i] + "/SLW");

    // Sum of squares (per-event)
    G4THitsMap<G4double>* SL2 = b01Run->GetHitsMapSq(fSDName[i] + "/SL");
    G4THitsMap<G4double>* SLW2 = b01Run->GetHitsMapSq(fSDName[i] + "/SLW");

    if (IsMaster()) {
      G4cout << "\n--------------------End of Global Run-----------------------" << G4endl;
      G4cout << " Number of event processed : " << aRun->GetNumberOfEvent() << G4endl;
    } else {
      G4cout << "\n--------------------End of Local Run------------------------" << G4endl;
      G4cout << " Number of event processed : " << aRun->GetNumberOfEvent() << G4endl;
    }

    G4cout << "=============================================================" << G4endl;
    G4cout << "=============================================================" << G4endl;

    std::ostream* myout = &G4cout;
    PrintHeader(myout);

    static const G4int izROI = 19;
    G4double roi_meanSLW = 0.0;
    G4double roi_sigmaSLW = 0.0;

    for (G4int iz = 0; iz < 20; iz++) {
      G4double* Populations = (*Population)[iz];
      G4double* TrackEnters = (*TrackEnter)[iz];
      G4double* SLs = (*SL)[iz];
      G4double* SLWs = (*SLW)[iz];

      G4double* SL2s = (SL2) ? (*SL2)[iz] : nullptr;
      G4double* SLW2s = (SLW2) ? (*SLW2)[iz] : nullptr;

      if (!Populations) Populations = new G4double(0.0);
      if (!TrackEnters) TrackEnters = new G4double(0.0);
      if (!SLs) SLs = new G4double(0.0);
      if (!SLWs) SLWs = new G4double(0.0);
      if (!SL2s) SL2s = new G4double(0.0);
      if (!SLW2s) SLW2s = new G4double(0.0);

      G4double AverageTrackWeight = 0.0;
      if (*SLs != 0.) AverageTrackWeight = (*SLWs) / (*SLs);

      // Mean (per event)
      const G4int Nevt = aRun->GetNumberOfEvent();
      G4double meanSL = 0.0;
      G4double meanSLW = 0.0;
      if (Nevt > 0) {
        meanSL = (*SLs) / (G4double)Nevt;
        meanSLW = (*SLWs) / (G4double)Nevt;
      }

      // StdErr of the estimator (not event-by-event sigma)
      G4double sigmaSL = 0.0;
      G4double sigmaSLW = 0.0;
      if (Nevt > 1) {
        const G4double sum_x = *SLs;
        const G4double sum_x2 = *SL2s;
        const G4double mean = sum_x / (G4double)Nevt;
        G4double var = (sum_x2 / (G4double)Nevt) - mean * mean;

        const G4double sum_w = *SLWs;
        const G4double sum_w2 = *SLW2s;
        const G4double meanw = sum_w / (G4double)Nevt;
        G4double varw = (sum_w2 / (G4double)Nevt) - meanw * meanw;

        if (var > 0.0) {
          var *= (G4double)Nevt / (G4double)(Nevt - 1);
          sigmaSL = std::sqrt(var / (G4double)Nevt);
        }
        if (varw > 0.0) {
          varw *= (G4double)Nevt / (G4double)(Nevt - 1);
          sigmaSLW = std::sqrt(varw / (G4double)Nevt);
        }

        if (iz == izROI) {
          roi_meanSLW = meanSLW;
          roi_sigmaSLW = sigmaSLW;
        }
      }

      G4String cname = bdet->GetCellName(iz);
      G4cout << std::setw(fFieldValue) << cname << " |" << std::setw(fFieldValue) << (*TrackEnters)
             << " |" << std::setw(fFieldValue) << (*Populations) << " |" << std::setw(fFieldValue)
             << AverageTrackWeight << " |" << std::setw(fFieldValue) << meanSL << " |"
             << std::setw(fFieldValue) << sigmaSL << " |" << std::setw(fFieldValue) << meanSLW
             << " |" << std::setw(fFieldValue) << sigmaSLW << " |" << G4endl;

      if (writeCells) {
        cellsOut << iz << "\t" << cname << "\t" << (*TrackEnters) << "\t" << (*Populations) << "\t"
                 << AverageTrackWeight << "\t" << meanSL << "\t" << sigmaSL << "\t" << meanSLW
                 << "\t" << sigmaSLW << "\n";
      }
    }

    G4cout << "=============================================" << G4endl;

    // FOM summary (master only)
    if (IsMaster()) {
      const std::string summaryPath = "out/b01_summary.tsv";
      try {
        std::filesystem::create_directories("out");
      } catch (...) {
      }

      const bool needHeader = !std::filesystem::exists(summaryPath);
      std::ofstream summaryOut(summaryPath, std::ios::app);
      if (summaryOut && needHeader) {
        summaryOut << "runID\tmode\tNevt\ttime_real_s\troi_cell\t"
                   << "roi_meanSLW_mm\troi_stderrSLW_mm\troi_relerr\tFOM_1_per_s\n";
      }

      if (roi_meanSLW > 0.0 && roi_sigmaSLW > 0.0 && elapsedTime > 0.0) {
        const G4double relErr = roi_sigmaSLW / roi_meanSLW;
        const G4double FOM = 1.0 / (relErr * relErr * elapsedTime);

        G4cout << "\n=== FOM for SLW in cell_" << izROI << " ===" << G4endl;
        G4cout << "  Mean(SLW) (mm)        = " << roi_meanSLW << G4endl;
        G4cout << "  StdErr(SLW) (mm)     = " << roi_sigmaSLW << G4endl;
        G4cout << "  Rel. error (%)   = " << relErr * 100. << G4endl;
        G4cout << "  Time (s) (Real)  = " << elapsedTime << G4endl;
        G4cout << "  FOM (1/s)             = " << FOM << G4endl;

        if (summaryOut) {
          summaryOut << aRun->GetRunID() << "\t" << modeTag << "\t" << aRun->GetNumberOfEvent() << "\t"
                     << elapsedTime << "\t" << izROI << "\t" << roi_meanSLW << "\t" << roi_sigmaSLW
                     << "\t" << relErr << "\t" << FOM << "\n";
        }
      } else {
        G4cout << "\n=== FOM not computed (zero mean, sigma or time) ===" << G4endl;
        if (summaryOut) {
          const double NaN = std::numeric_limits<double>::quiet_NaN();
          summaryOut << aRun->GetRunID() << "\t" << modeTag << "\t" << aRun->GetNumberOfEvent() << "\t"
                     << elapsedTime << "\t" << izROI << "\t" << NaN << "\t" << NaN << "\t" << NaN
                     << "\t" << NaN << "\n";
        }
      }
    }
  }

  if (cellsOut.is_open()) {
    cellsOut.close();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B01RunAction::PrintHeader(std::ostream* out)
{
  std::vector<G4String> vecScoreName;
  vecScoreName.push_back("Tr.Entering");
  vecScoreName.push_back("Population");
  vecScoreName.push_back("Av.Tr.WGT");
  vecScoreName.push_back("SL (mm)");
  vecScoreName.push_back("Sigma(SL) (mm)");
  vecScoreName.push_back("SLW (mm)");
  vecScoreName.push_back("Sigma(SLW) (mm)");

  *out << std::setw(fFieldValue) << "Volume"
       << " |";
  for (std::vector<G4String>::iterator it = vecScoreName.begin(); it != vecScoreName.end(); it++) {
    *out << std::setw(fFieldValue) << (*it) << " |";
  }
  *out << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

std::string B01RunAction::FillString(const std::string& name, char c, G4int n, G4bool back)
{
  std::string fname("");
  G4int k = n - name.size();
  if (k > 0) {
    if (back) {
      fname = name;
      fname += std::string(k, c);
    } else {
      fname = std::string(k, c);
      fname += name;
    }
  } else {
    fname = name;
  }
  return fname;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......