# CSC2026_VR — Variance Reduction Hands-On (CERN Computing School 2026)

This repository contains the two practical sessions (hands-on) for the **Variance Reduction (VR)** part of the Monte Carlo course:

### Hands-On 1 — Deep Shielding and Variance Reduction with Geant4
Notebook: `hands_on_1/hands_on_1.ipynb`

### Hands-On 2 — Rare Events and Variance Reduction with Geant4
Notebook: `hands_on_2/hands_on_2.ipynb`

These exercises are designed to run on the **CERN SWAN** service. You will need an active **CERN account** to proceed.

---

## 🚀 Quick start (SWAN)

### 1. Go to **https://swan.cern.ch** and log in with your CERN account.

---

### 2. In the SWAN session configuration form:
   - **Software stack:** `108` (LCG_108)
   - Tick: **“Use Python packages installed on CERNBox”**
   - **CPUs:** `4`
   - **Memory:** `16 GB`
   - Click **Start my Session**.
   - Wait for the environment to start.

---

### 3. Download the repository 
   - Once your session is ready, click **“Download Project from Git”**.
     (the small cloud-and-arrow icon in the upper-right corner).
   - Paste this repository URL: `https://github.com/jromero-barrientos/hands_on_VR_CSC2026.git`
   - Wait until the download completes.

---

### 4. Run the notebooks (in order)
   - Once the download is complete, you will be inside a directory named **`hands_on_VR_CSC2026`** (or similar).
   - Open and run the notebooks **in this order**:
      - `hands_on_1/hands_on_1.ipynb` → First practical session (deep shielding & VR).
      - `hands_on_2/hands_on_2.ipynb` → Second practical session (rare events & VR).
   - You can also run everything by going into **Cell → Run All**.

> **Important:** Do not change the provided build/run helper cells unless explicitly asked in the notebook instructions.
> In particular, the helpers in the Hands On 2 force **1 thread** so that the simulation writes **a single output file** (instead of one per thread).

Instructions and questions are embedded in each notebook. Follow them step-by-step as you progress.
