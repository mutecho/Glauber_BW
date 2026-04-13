#include <cmath>
#include <iostream>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TH2D.h"
#include "TRandom3.h"

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

// flow
const double v_max = 0.8;
const double kappa2 = 0.5;

// Bjorken
const double tau0 = 10.0;
const double sigma_eta = 1.5;

// events
const int Nevents = 1000;

TRandom3 rng(12345);
std::mt19937 gen(12345);

// ================= Glauber =================

struct Nucleon {
  double x, y;
  bool participant;
};

double ws_thickness(double r) {
  return 1.0 / (1.0 + exp((r - R0) / a0));
}

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

// ================= eccentricity =================

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

void toy_glauber2() {
  TFile *fout = new TFile("toy_glauber.root", "RECREATE");

  TH2D *hXY_part = new TH2D("hXY_part", "participants;x;y", 200, -15, 15, 200, -15, 15);

  TH2D *hXY_hadron = new TH2D("hXY_hadron", "hadron XY;x;y", 200, -15, 15, 200, -15, 15);

  TH2D *hPxPy = new TH2D("hPxPy", "px-py;px;py", 200, -2, 2, 200, -2, 2);

  for (int ev = 0; ev < Nevents; ev++) {
    double b = 8.0;

    vector<Nucleon> A_nuc, B_nuc;

    sample_nucleus(A_nuc);
    sample_nucleus(B_nuc);

    for (auto &n : A_nuc) n.x -= b / 2.0;
    for (auto &n : B_nuc) n.x += b / 2.0;

    collide(A_nuc, B_nuc);

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

    if (parts.size() < 2)
      continue;

    // ⭐ eccentricity
    double eps2, psi2;
    compute_eccentricity(parts, eps2, psi2);

    // ================= hadronization =================

    for (auto &src : parts) {
      int n = sample_NBD(nbd_mu, nbd_k);

      for (int i = 0; i < n; i++) {
        // ===== 坐标 =====
        double x = src.first + rng.Gaus(0, smear_sigma);
        double y = src.second + rng.Gaus(0, smear_sigma);

        double eta_s = rng.Gaus(0, sigma_eta);

        double t = tau0 * cosh(eta_s);
        double z = tau0 * sinh(eta_s);

        // ===== flow field =====
        double r = sqrt(x * x + y * y);
        double phi_r = atan2(y, x);

        double R = 6.0;
        double vr = v_max * (r / R);

        double v = vr * (1 + 2 * kappa2 * eps2 * cos(2 * (phi_r - psi2)));
        if (v > 0.99)
          v = 0.99;

        double vx = v * cos(phi_r);
        double vy = v * sin(phi_r);

        double gamma = 1.0 / sqrt(1 - v * v);

        // ===== thermal momentum =====
        double pt = -T * log(rng.Uniform());
        double phi = rng.Uniform(0, 2 * M_PI);

        double px0 = pt * cos(phi);
        double py0 = pt * sin(phi);

        double y_mom = eta_s + rng.Gaus(0, 0.3);
        double mt = sqrt(pt * pt + mass * mass);

        double pz0 = mt * sinh(y_mom);

        // ===== boost =====
        double px = px0 + gamma * vx * pt;
        double py = py0 + gamma * vy * pt;
        double pz = pz0;

        // ===== fill =====
        hXY_hadron->Fill(x, y);
        hPxPy->Fill(px, py);
      }
    }
  }

  // ================= draw =================

  TCanvas *c1 = new TCanvas("c1", "participants", 600, 600);
  hXY_part->Draw("COLZ");

  TCanvas *c2 = new TCanvas("c2", "hadron XY", 600, 600);
  hXY_hadron->Draw("COLZ");

  TCanvas *c3 = new TCanvas("c3", "px py", 600, 600);
  hPxPy->Draw("COLZ");

  // ================= output =================

  hXY_part->Write();
  hXY_hadron->Write();
  hPxPy->Write();

  fout->Close();

  cout << "Done: toy_glauber.root" << endl;
}