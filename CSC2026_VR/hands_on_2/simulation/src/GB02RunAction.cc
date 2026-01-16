#include "GB02RunAction.hh"

#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
#include "G4Types.hh"
#include "G4ios.hh"
#include <sstream>   // <-- ADD
#include <iomanip>   // <-- optional, if you want formatting
#include <limits>
#include <fstream>
#include <chrono>
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#include <cmath>
#include <filesystem>
#include <system_error>
#include <mutex>

namespace {
  const std::string& GlobalRunTag()
  {
    static std::once_flag once;
    static std::string tag;

    std::call_once(once, []() {
      // One tag per process execution
      const auto now = std::chrono::system_clock::now();
      const auto us  = std::chrono::duration_cast<std::chrono::microseconds>(
                         now.time_since_epoch()).count();

      long pid = 0;
    #if defined(__unix__) || defined(__APPLE__)
      pid = static_cast<long>(::getpid());
    #endif

      std::ostringstream os;
      os << us;
      if (pid != 0) os << "_pid" << pid;
      tag = os.str();
    });

    return tag;
  }
} // namespace

GB02RunAction::GB02RunAction()
  : G4UserRunAction(), 
  fSumQ(0.0), 
  fSumQ2(0.0)
{
  auto* accMan = G4AccumulableManager::Instance();
  accMan->Register(fSumQ);
  accMan->Register(fSumQ2);

  fRunTag.clear();
  fOutDir.clear();
}

void GB02RunAction::BeginOfRunAction(const G4Run* run)
{
  auto* accMan = G4AccumulableManager::Instance();
  accMan->Reset();

  if (G4Threading::IsMasterThread()) {
    fTimer.Start();
  }

  // --- ADD: open one file per thread
  {
    const int runId = (run ? run->GetRunID() : -1);

    // Ensure master + workers share the same tag (one per process execution)
    fRunTag = GlobalRunTag();

    // Directory name: "out/<fRunTag>_<runID>/"
    {
      std::ostringstream d;
      d << fRunTag << "_" << runId;

      std::filesystem::path base("out");
      std::filesystem::path dir = base / d.str();
      fOutDir = dir.string();

      std::error_code ec;
      std::filesystem::create_directories(dir, ec);
      if (ec) {
        G4cerr << "[GB02] WARNING: could not create directory '" << fOutDir
               << "': " << ec.message() << G4endl;
      }
    }

    if (G4Threading::IsMasterThread()) {
      G4cout << "[GB02] Output directory: " << fOutDir << G4endl;
    }

    std::ostringstream os;
    const int tid = G4Threading::G4GetThreadId();
    if (tid < 0) return; // MT master thread: do not create per-event file

    os << fOutDir << "/eventQ_t" << tid << ".tsv";
    fEvtOutName = os.str();

    fEvtOut.open(fEvtOutName, std::ios::out);
    if (fEvtOut.is_open()) {
      // High precision for doubles in TSV
      fEvtOut.setf(std::ios::scientific, std::ios::floatfield);
      fEvtOut << std::setprecision(std::numeric_limits<double>::max_digits10);

      fEvtOut << "#eventID\teventQ\n";
      G4cout << "[GB02] Writing per-event Q to: " << fEvtOutName
             << " (thread " << tid << ")" << G4endl;
    }
  }
}

void GB02RunAction::AddEventQ(G4double q)
{
  fSumQ  += q;
  fSumQ2 += q * q;
}

void GB02RunAction::LogEventQ(G4int eventID, G4double q)
{
  if (!fEvtOut.is_open()) return;
  fEvtOut << eventID << "\t" << q << "\n";
}

void GB02RunAction::EndOfRunAction(const G4Run* run)
{
    // --- ADD: close per-thread file (do it before any early return)
  if (fEvtOut.is_open()) {
    fEvtOut.close();
  }
  // Merge worker accumulables into master
  auto* accMan = G4AccumulableManager::Instance();
  accMan->Merge();

  if (!G4Threading::IsMasterThread()) return;

  fTimer.Stop();

  const G4int N = run->GetNumberOfEvent();
  if (N <= 0) {
    G4cout << "[GB02] No events were processed.\n";
    return;
  }

  const double sumQ  = fSumQ.GetValue();
  const double sumQ2 = fSumQ2.GetValue();

  const double mean = sumQ / (double)N;

  // Sample variance of Q_event (unbiased) and standard error of mean
  double var = (sumQ2 / (double)N) - mean * mean;
  if (var < 0.0) var = 0.0;

  double var_unbiased = 0.0;
  if (N > 1) var_unbiased = var * (double)N / (double)(N - 1);

  const double stderr = (N > 1) ? std::sqrt(var_unbiased / (double)N) : 0.0;

  const double R = (mean > 0.0) ? (stderr / mean) : 0.0;

  const double time_s = fTimer.GetRealElapsed();

  double FOM = 0.0;
  if (R > 0.0 && time_s > 0.0) {
    FOM = 1.0 / (R * R * time_s);
  }

  // --- Write one run-level summary file (master only)
  {
    const int runId = (run ? run->GetRunID() : -1);

    std::ostringstream fn;
    fn << fOutDir << "/summary.tsv";

    std::ofstream out(fn.str(), std::ios::out);
    if (out.is_open()) {
      // High precision for doubles in TSV
      out.setf(std::ios::scientific, std::ios::floatfield);
      out << std::setprecision(std::numeric_limits<double>::max_digits10);

      int nThreads = 1;
#ifdef G4MULTITHREADED
      if (auto* rm = G4RunManager::GetRunManager()) nThreads = rm->GetNumberOfThreads();
#endif

      // Key-value metadata (easy to parse + human readable)
      out << "#runTag\t" << fRunTag << "\n";
      out << "#runID\t" << runId << "\n";
      out << "#threads\t" << nThreads << "\n";
      out << "#event_files_glob\t" << fOutDir << "/eventQ_t*.tsv\n";

      // Single-row numeric summary
      out << "N\tMeanQ\tStdErr\tRelErr_pct\tTime_s\tFOM_1perS\n";
      out << N << "\t" << mean << "\t" << stderr << "\t" << 100.0 * R
          << "\t" << time_s << "\t" << FOM << "\n";

      out.close();

      G4cout << "[GB02] Writing run summary to: " << fn.str() << G4endl;
    } else {
      G4cerr << "[GB02] WARNING: could not write summary file: " << fn.str() << G4endl;
    }
  }

  G4cout << "\n[GB02] Observable: Weighted number of collisions in target (per primary)\n";
  G4cout << "  N events        : " << N << "\n";
  G4cout << "  Mean(Q)         : " << mean << "\n";
  G4cout << "  StdErr(Mean)    : " << stderr << "\n";
  G4cout << "  Rel. error (%)  : " << 100.0 * R << "\n";
  G4cout << "  Time (s)        : " << time_s << "\n";
  G4cout << "  FOM (1/s)       : " << FOM << "\n\n";
}
