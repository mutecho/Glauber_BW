#include <cmath>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TH2D.h"
#include "TRandom3.h"
#include "TTree.h"

using namespace std;

// ================= 参数 =================

const int A = 208;
const double R0 = 6.62;
const double a0 = 0.546;

// NN cross section
const double sigmaNN_mb = 70.0;
const double dmax = sqrt(sigmaNN_mb / (M_PI * 10.0));

// smearing
const double smear_sigma = 0.5;

// NBD
const double nbd_mu = 2.0;
const double nbd_k = 1.5;

// thermal
const double T = 0.2;
const double mass = 0.14;

// Bjorken
const double tau0 = 10.0;
const double sigma_eta = 1.5;

// events
const int Nevents = 1000;

TRandom3 rng(12345);
std::mt19937 gen(12345);

// ================= Glauber结构 =================

struct Nucleon {
  double x, y;
  bool participant;
};

// Woods-Saxon thickness
double ws_thickness(double r) {
  return 1.0 / (1.0 + exp((r - R0) / a0));
}

// ⭐ 替换后的 nucleus sampling（完全来自 sample_glauber）
void sample_nucleus(vector<Nucleon> &nucl) {
  nucl.clear();

  while ((int)nucl.size() < A) {
    double x = rng.Uniform(-R0 - 5, R0 + 5);
    double y = rng.Uniform(-R0 - 5, R0 + 5);

    double r = sqrt(x * x + y * y);
    double prob = ws_thickness(r);

    if (rng.Uniform() < prob) {
      Nucleon n;
      n.x = x;
      n.y = y;
      n.participant = false;
      nucl.push_back(n);
    }
  }
}

// ⭐ 标准碰撞
void collide(vector<Nucleon> &A_nuc, vector<Nucleon> &B_nuc) {
  for (auto &a : A_nuc) a.participant = false;
  for (auto &b : B_nuc) b.participant = false;

  for (auto &a : A_nuc) {
    for (auto &b : B_nuc) {
      double dx = a.x - b.x;
      double dy = a.y - b.y;

      if (dx * dx + dy * dy < dmax * dmax) {
        a.participant = true;
        b.participant = true;
      }
    }
  }
}

// ================= NBD =================

int sample_NBD(double mu, double k) {
  double theta = mu / k;

  std::gamma_distribution<double> gamma_dist(k, theta);
  double lambda = gamma_dist(gen);

  std::poisson_distribution<int> poisson_dist(lambda);
  return poisson_dist(gen);
}

// ================= 主程序 =================

void toy_glauber() {
  TFile *fout = new TFile("toy_glauber.root", "RECREATE");

  // ⭐ 核子参与者
  TH2D *hXY_part = new TH2D("hXY_part", "participants;x;y", 200, -15, 15, 200, -15, 15);

  // ⭐ 末态强子：坐标空间
  TH2D *hXY_hadron = new TH2D("hXY_hadron", "hadron XY;x;y", 200, -15, 15, 200, -15, 15);

  // ⭐ 末态强子：动量空间
  TH2D *hPtPhi = new TH2D("hPxPy", "px-py;px;py", 200, -2, 2, 200, -2, 2);

  for (int ev = 0; ev < Nevents; ev++) {
    double b = 8.0;

    vector<Nucleon> A_nuc, B_nuc;

    // ⭐ 新采样
    sample_nucleus(A_nuc);
    sample_nucleus(B_nuc);

    // impact parameter shift
    for (auto &n : A_nuc) n.x -= b / 2.0;
    for (auto &n : B_nuc) n.x += b / 2.0;

    // ⭐ 碰撞
    collide(A_nuc, B_nuc);

    // ================= loop participants =================

    vector<pair<double, double>> parts;

    for (auto &n : A_nuc)
      if (n.participant) {
        parts.emplace_back(n.x, n.y);
        hXY_part->Fill(n.x, n.y);
      }

    for (auto &n : B_nuc)
      if (n.participant) {
        parts.emplace_back(n.x, n.y);
        hXY_part->Fill(n.x, n.y);
      }

    if (parts.size() < 1)
      continue;

    // ================= NBD + 末态生成 =================

    for (auto &src : parts) {
      int n = sample_NBD(nbd_mu, nbd_k);

      for (int i = 0; i < n; i++) {
        // ===== 坐标 =====
        double x = src.first + rng.Gaus(0, smear_sigma);
        double y = src.second + rng.Gaus(0, smear_sigma);

        double eta_s = rng.Gaus(0, sigma_eta);

        double t = tau0 * cosh(eta_s);
        double z = tau0 * sinh(eta_s);

        // ===== thermal momentum =====
        double pt = -T * log(rng.Uniform());
        double phi = rng.Uniform(0, 2 * M_PI);

        double px = pt * cos(phi);
        double py = pt * sin(phi);

        double y_mom = eta_s + rng.Gaus(0, 0.3);
        double mt = sqrt(pt * pt + mass * mass);

        double pz = mt * sinh(y_mom);

        // ⭐ fill 坐标空间
        hXY_hadron->Fill(x, y);

        // ⭐ fill 动量空间
        hPtPhi->Fill(px, py);
      }
    }
  }

  // ================= 画图 =================

  TCanvas *c1 = new TCanvas("c1", "participants", 600, 600);
  hXY_part->Draw("COLZ");

  TCanvas *c2 = new TCanvas("c2", "hadron XY", 600, 600);
  hXY_hadron->Draw("COLZ");

  TCanvas *c3 = new TCanvas("c3", "px py", 600, 600);
  hPtPhi->Draw("COLZ");

  // ================= 输出 =================

  hXY_part->Write();
  hXY_hadron->Write();
  hPtPhi->Write();

  fout->Close();

  cout << "Done: toy_glauber.root" << endl;
}