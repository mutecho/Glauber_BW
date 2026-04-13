#include <cmath>
#include <iostream>
#include <vector>

#include "TFile.h"
#include "TRandom3.h"
#include "TTree.h"

using namespace std;

// ================= 参数 =================

// Woods-Saxon (Pb)
const double R0 = 6.62;
const double a0 = 0.546;
const int A = 208;

// NN cross section
const double sigmaNN = 7.0;  // fm^2

// smearing
const double smear_sigma = 0.5;

// NBD
const double nbd_mu = 2.0;
const double nbd_k = 1.5;

// thermal
const double T = 0.2;      // GeV
const double mass = 0.14;  // pion

// flow
const double v_max = 0.8;
const double kappa2 = 0.5;

// Bjorken
const double tau0 = 10.0;      // fm/c
const double sigma_eta = 1.5;  // longitudinal width

// events
const int Nevents = 1000;

TRandom3 rng(12345);
std::mt19937 gen(12345);

// ================= 工具函数 =================

double sample_r() {
  while (true) {
    double r = rng.Uniform(0, 15);
    double prob = 1.0 / (1.0 + exp((r - R0) / a0));
    if (rng.Uniform() < prob)
      return r;
  }
  return 0;
}

void sample_nucleus(vector<pair<double, double>> &nuc) {
  nuc.clear();
  for (int i = 0; i < A; i++) {
    double r = sample_r();
    double costheta = rng.Uniform(-1, 1);
    double phi = rng.Uniform(0, 2 * M_PI);
    double sintheta = sqrt(1 - costheta * costheta);

    double x = r * sintheta * cos(phi);
    double y = r * sintheta * sin(phi);

    nuc.push_back({x, y});
  }
}

void sample_nucleus_2D(vector<pair<double, double>> &nuc,
                       int A,
                       double R,
                       double x_shift  // 用于 ±b/2
) {
  nuc.clear();

  for (int i = 0; i < A; i++) {
    // 2D 均匀圆盘分布（关键：sqrt）
    double r = R * sqrt(rng.Uniform());
    double phi = rng.Uniform(0, 2 * M_PI);

    double x = r * cos(phi) + x_shift;
    double y = r * sin(phi);

    nuc.emplace_back(x, y);
  }
}

void get_participants(vector<pair<double, double>> &A_nuc,
                      vector<pair<double, double>> &B_nuc,
                      vector<pair<double, double>> &parts) {
  parts.clear();

  int NA = A_nuc.size();
  int NB = B_nuc.size();

  // 标记是否参与
  vector<bool> hitA(NA, false);
  vector<bool> hitB(NB, false);

  // 核子截面对应的距离
  double d0 = sqrt(sigmaNN / M_PI);

  // 判断是否发生碰撞
  for (int i = 0; i < NA; i++) {
    for (int j = 0; j < NB; j++) {
      double dx = A_nuc[i].first - B_nuc[j].first;
      double dy = A_nuc[i].second - B_nuc[j].second;

      double d2 = dx * dx + dy * dy;

      if (d2 < d0 * d0) {
        hitA[i] = true;
        hitB[j] = true;
      }
    }
  }

  // 只加入一次！
  for (int i = 0; i < NA; i++) {
    if (hitA[i])
      parts.push_back(A_nuc[i]);
  }

  for (int j = 0; j < NB; j++) {
    if (hitB[j])
      parts.push_back(B_nuc[j]);
  }
}

// NBD sampling
int sample_NBD(double mu, double k) {
  // Gamma-Poisson method

  double theta = mu / k;  // scale parameter

  // 1. sample lambda from Gamma(k, theta)
  std::gamma_distribution<double> gamma_dist(k, theta);
  double lambda = gamma_dist(gen);

  // 2. sample n from Poisson(lambda)
  std::poisson_distribution<int> poisson_dist(lambda);
  return poisson_dist(gen);
}

// epsilon2
void compute_eccentricity(const vector<pair<double, double>> &parts, double &eps2, double &psi2) {
  double sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0;

  for (auto &p : parts) {
    sx += p.first;
    sy += p.second;
  }
  sx /= parts.size();
  sy /= parts.size();

  for (auto &p : parts) {
    double x = p.first - sx;
    double y = p.second - sy;
    sxx += x * x;
    syy += y * y;
    sxy += x * y;
  }

  eps2 = sqrt((sxx - syy) * (sxx - syy) + 4 * sxy * sxy) / (sxx + syy);
  psi2 = 0.5 * atan2(2 * sxy, sxx - syy);
}

// ================= 主程序 =================

void toy_glauber_flow() {
  TFile *fout = new TFile("toy3D.root", "RECREATE");
  TTree *tree = new TTree("t", "3D toy model");

  TH2D *hXY = new TH2D("hXY", "x-y distribution;x (fm);y (fm)", 200, -15, 15, 200, -15, 15);

  int event_id;
  float px, py, pz, E;
  float x, y, z, t;
  float eta;

  tree->Branch("event", &event_id);
  tree->Branch("px", &px);
  tree->Branch("py", &py);
  tree->Branch("pz", &pz);
  tree->Branch("E", &E);

  tree->Branch("x", &x);
  tree->Branch("y", &y);
  tree->Branch("z", &z);
  tree->Branch("t", &t);

  tree->Branch("eta", &eta);

  for (int ev = 0; ev < Nevents; ev++) {
    event_id = ev;

    double b = 8;

    // nuclei
    // vector<pair<double,double>> A_nuc, B_nuc;
    // sample_nucleus(A_nuc);
    // sample_nucleus(B_nuc);

    vector<pair<double, double>> A_nuc, B_nuc;
    sample_nucleus_2D(A_nuc, A, R0, -b / 2.0);
    sample_nucleus_2D(B_nuc, A, R0, +b / 2.0);

    // double b = rng.Uniform(0,10);

    // for (auto& p : A_nuc) p.first -= b/2;
    // for (auto& p : B_nuc) p.first += b/2;

    // participants
    vector<pair<double, double>> parts;
    get_participants(A_nuc, B_nuc, parts);

    for (auto &p : parts) hXY->Fill(p.first, p.second);

    if (parts.size() < 2)
      continue;

    // eccentricity
    double eps2, psi2;
    compute_eccentricity(parts, eps2, psi2);

    // loop sources
    for (auto &src : parts) {
      int n = sample_NBD(nbd_mu, nbd_k);

      for (int i = 0; i < n; i++) {
        // ===== 空间 =====
        x = src.first + rng.Gaus(0, smear_sigma);
        y = src.second + rng.Gaus(0, smear_sigma);

        // Bjorken longitudinal
        double eta_s = rng.Gaus(0, sigma_eta);

        t = tau0 * cosh(eta_s);
        z = tau0 * sinh(eta_s);

        // ===== transverse flow =====
        double r = sqrt(x * x + y * y);
        double phi_r = atan2(y, x);

        double R = 6.0;
        double vr = v_max * (r / R);

        double v = vr * (1 + 2 * kappa2 * eps2 * cos(2 * (phi_r - psi2)));
        if (v > 0.99)
          v = 0.99;

        double vx = v * cos(phi_r);
        double vy = v * sin(phi_r);

        // ===== thermal momentum =====
        double pt = -T * log(rng.Uniform());
        double phi = rng.Uniform(0, 2 * M_PI);

        double px0 = pt * cos(phi);
        double py0 = pt * sin(phi);

        // longitudinal momentum (Bjorken)
        double y_mom = eta_s + rng.Gaus(0, 0.3);
        double mt = sqrt(pt * pt + mass * mass);

        double pz0 = mt * sinh(y_mom);
        double E0 = mt * cosh(y_mom);

        // ===== transverse boost =====
        double gamma = 1.0 / sqrt(1 - v * v);

        px = px0 + gamma * vx * pt;
        py = py0 + gamma * vy * pt;
        pz = pz0;
        E = E0;

        // ===== eta =====
        double p = sqrt(px * px + py * py + pz * pz);
        eta = 0.5 * log((p + pz) / (p - pz));

        tree->Fill();
      }
    }
  }

  TCanvas *c1 = new TCanvas("c1", "MC Glauber (correct)", 800, 800);
  hXY->Draw("COLZ");

  tree->Write();
  hXY->Write();
  fout->Close();

  cout << "Done. Output: toy3D.root" << endl;
}
