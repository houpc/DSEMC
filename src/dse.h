#ifndef dse_H
#define dse_H

#include "global.h"
#include "utility/utility.h"
#include "vertex.h"
#include <array>
#include <map>
#include <string>
#include <tuple>
#include <vector>

extern parameter Para;

namespace dse {
using namespace std;

enum caltype { BARE, RG, PARQUET, RENORMALIZED, VARIATIONAL };
enum channel { I = 0, T, U, S };

struct bubble;
struct envelope;

struct ver4 {
  int ID;
  int LoopNum;
  int TauNum;
  caltype Type;
  bool ReExpandBare;
  bool ReExpandVer4;

  vector<bubble> Bubble;     // bubble diagrams and its counter diagram
  vector<envelope> Envelope; // envelop diagrams and its counter diagram

  array<momentum *, 4> LegK; // external legK index
  vector<array<int, 4>> T;   // external T list
  vector<double> Weight;     // size: equal to T.size()
};

//////////////// Bubble diagrams /////////////////////////////

class gMatrix {
public:
  gMatrix() {
    _TauNum = 0;
    _InTL = 0;
  }
  gMatrix(int TauNum, int InTL, momentum *k) {
    _TauNum = TauNum;
    _InTL = InTL;
    K = k;
    _G.resize(TauNum * TauNum);
    for (auto &g : _G)
      g = 0.0;
  }
  double &operator()(int l, int r) {
    return _G[(l - _InTL) * _TauNum + r - _InTL];
  }

  double &operator()(const array<int, 2> &t) {
    return _G[(t[IN] - _InTL) * _TauNum + t[OUT] - _InTL];
  }

  momentum *K;

private:
  int _TauNum;
  int _InTL;
  vector<double> _G;
};

struct mapT2 {
  int LVerTidx; // LVer T index
  int RVerTidx; // RVer T index
  // map LVer T index and RVer T index to merged T index
  int Tidx; // three channels
  // LVer T and RVer T to Internal T for G1 and G2
  array<int, 2> G0T; // the shared G
  array<int, 2> GT;
};

struct pair {
  ver4 LVer;
  ver4 RVer;
  channel Channel;
  double SymFactor;
  vector<mapT2> Map;
};

struct bubble {
  int InTL;
  bool IsProjected;
  vector<channel> Channel;   // list of channels
  array<momentum *, 4> LegK; // legK index
  array<gMatrix, 4> G;
  vector<pair> Pair; // different Tau arrangement and channel
};

//////////////// Envelope diagrams /////////////////////////////

class g2Matrix {
public:
  g2Matrix() {
    _InShift = 0;
    _OutShift = 0;
  }
  g2Matrix(int InShift, int OutShift, momentum *k) {
    _InShift = InShift;
    _OutShift = OutShift;
    K = k;
    _G.resize(2 * 2);
    for (auto &g : _G)
      g = 0.0;
    InT = {_InShift, _InShift + 1};
    OutT = {_OutShift, _OutShift + 1};
  }

  double &operator()(int in, int out) {
    return _G[(in - _InShift) * 2 + out - _OutShift];
  }

  double &operator()(const array<int, 2> &t) {
    return _G[(t[IN] - _InShift) * 2 + t[OUT] - _OutShift];
  }

  array<int, 2> InT, OutT; // possible InT and OutT
  momentum *K;             // momentum on G

private:
  int _InShift;
  int _OutShift;
  vector<double> _G;
};

struct mapT4 {
  int LDVerTidx;
  int RDVerTidx;
  int LUVerTidx;
  int RUVerTidx;
  // map LVer T index and RVer T index to merged T index
  array<int, 4> Tidx; // external T for four envelop diagrams
  // LVer T and RVer T to Internal T for G1 and G2
  array<array<int, 2>, 9> GT;
};

struct envelope {
  bool IsProjected;
  int InTL;
  array<momentum *, 4> LegK; // legK index
  array<ver4, 10> Ver;
  array<g2Matrix, 9> G;
  vector<mapT4> Map;
  array<double, 4> SymFactor;
};

////////////// Vertex Creation Class /////////////////////////////////

class verDiag {
public:
  ver4 Build(array<momentum, MaxMomNum> &loopmom, int LoopNum,
             vector<channel> Channel, caltype Type);
  string ToString(const ver4 &Vertex);

private:
  int DiagNum = 0;
  int MomNum = MaxLoopNum;
  array<momentum, MaxMomNum> *LoopMom; // all momentum loop variables

  ver4 Vertex(array<momentum *, 4> LegK, int InTL, int LoopNum, int LoopIndex,
              vector<channel> Channel, caltype Type, int Side);

  ver4 Ver0(ver4 Ver4, int InTL, int Side);
  ver4 ChanI(ver4 Ver4, int InTL, int LoopNum, int LoopIndex, int Side,
             bool IsProjected = false);
  ver4 ChanUST(ver4 Ver4, vector<channel> Channel, int InTL, int LoopNum,
               int LoopIndex, int Side, bool IsProjected = false);
  momentum *NextMom();
};

bool verTest();

} // namespace dse
#endif