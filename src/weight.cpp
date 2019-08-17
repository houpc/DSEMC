#include "weight.h"
#include "global.h"
#include "utility/abort.h"
#include "utility/fmt/format.h"
#include "utility/vector.h"
#include <array>
#include <iostream>
#include <string>

using namespace diag;
using namespace std;

void weight::ReadDiagrams() {
  int ID = 0;
  for (auto &name : Para.GroupName) {
    // construct filename based on format string and group id
    string FileName = fmt::format(Para.DiagFileFormat, name);
    ifstream DiagFile(FileName);
    ASSERT_ALLWAYS(DiagFile.is_open(),
                   "Unable to find the file " << FileName << endl);
    // group Group;
    LOG_INFO("Find " << FileName << "\n");
    // vector<green> GList;
    istream &DiagFileStream = DiagFile;
    Groups.push_back(ReadOneGroup(DiagFileStream));
    Groups.back().Name = name;
    Groups.back().ID = ID;
    ID++;
  }

  // cout << "After read" << endl;
  // cout << ToString(*(GroupList[0].DiagList[0].GIndex[0])) << endl;
  Initialization();
}

void weight::Initialization() {

  LOG_INFO("Initializating diagram states ...")
  for (auto &group : Groups) {
    group.ReWeight = 1.0;
  }

  // vector<dse::channel> Chan = {dse::T, dse::U, dse::S};
  vector<dse::channel> Chan = {dse::T};
  for (int order = 1; order <= Groups.back().Order; order++) {
    Ver4Root[order] = VerDiag.Build(Var.LoopMom, order, Chan, dse::NORMAL);
    LOG_INFO(VerDiag.ToString(Ver4Root[order]));
  }

  LOG_INFO("Initializating MC variables ...")
  // initialize momentum variables
  for (auto &mom : Var.LoopMom)
    for (int i = 0; i < D; i++)
      mom[i] = Random.urn() * Para.Kf / sqrt(D);

  // initialize tau variables
  // for (int i = 0; i < MaxTauNum/ 2; i++) {
  //   Var.Tau[2 * i] = Random.urn() * Para.Beta;
  //   Var.Tau[2 * i + 1] = Var.Tau[2 * i]; // assume even and odd tau are the
  //   same
  // }

  for (int i = 0; i < MaxTauNum; i++) {
    Var.Tau[i] = Random.urn() * Para.Beta;
  }

  // initialize spin variables
  for (auto &sp : Var.LoopSpin)
    sp = (spin)(Random.irn(0, 1));

  Var.CurrExtMomBin = 0;
  // Var.LoopMom[0].fill(0.0);
  // for (int i = 0; i < D; i++)
  //   Var.LoopMom[0][i] = Var.ExtMomTable[Var.CurrExtMomBin][i];
  Var.LoopMom[0] = Para.ExtMomTable[Var.CurrExtMomBin];

  // initialize external tau
  // Var.Tau[0] = 0.0;
  // Var.Tau[1] = 1.0e-10; // do not make Tau[1]==Tau[0], otherwise the Green's
  // function is not well-defined

  Var.CurrTau = Var.Tau[1] - Var.Tau[0];

  // initialize group

  Var.CurrVersion = 0;
  //   Var.CurrGroup = &Groups[0];

  Var.CurrGroup = &Groups[0];

  Var.CurrIRScaleBin = ScaleBinSize / 1.5;

  // initialize RG staff
  // Var.CurrScale = ScaleBinSize - 1;
  Var.CurrScale = Para.Kf;

  LOG_INFO("Calculating the weights of all objects...")

  // ChangeGroup(*Var.CurrGroup, true);
  GetNewWeight(*Var.CurrGroup);
  AcceptChange(*Var.CurrGroup);

  LOG_INFO("Initializating variables done.")
}

double weight::GetNewWeight(group &Group) {
  Group.NewWeight = Evaluate(Group.Order, Group.ID);
  return Group.NewWeight;
}

void weight::AcceptChange(group &Group) {
  Var.CurrVersion++;
  Var.CurrGroup = &Group;
  Group.Weight = Group.NewWeight; // accept group  newweight
}

void weight::RejectChange(group &Group) { return; }

void weight::Measure(double WeightFactor) {
  if (Para.Type == RG && Para.Vertex4Type == MOM_ANGLE) {
    // if (Var.CurrScale >= Para.ScaleTable[Var.CurrIRScaleBin])
    VerQTheta.Measure(
        Var.LoopMom[1], Var.LoopMom[2], Var.CurrExtMomBin, Var.CurrGroup->Order,
        Var.Tau[Var.CurrGroup->TauNum - 1] - Var.Tau[0], WeightFactor);
  }
}

void weight::Update(double Ratio) {
  if (Para.Type == RG && Para.Vertex4Type == MOM_ANGLE) {
    VerQTheta.Update(Ratio);
  }
}

void weight::Save() {
  if (Para.Type == RG && Para.Vertex4Type == MOM_ANGLE) {
    VerQTheta.Save();
  }
}

void weight::ClearStatis() {
  if (Para.Type == RG && Para.Vertex4Type == MOM_ANGLE) {
    VerQTheta.ClearStatis();
  }
}