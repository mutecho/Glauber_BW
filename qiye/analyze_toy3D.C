#include <vector>
#include <iostream>
#include <cmath>

void analyze_toy3D() {

    // ===== 打开文件 =====
    TFile *fin = new TFile("toy3D.root");
    TTree *t = (TTree*)fin->Get("t");

    // ===== 变量 =====
    float px, py, pz, E;
    float x, y, z, time;
    float eta;
    int event;

    t->SetBranchAddress("px",&px);
    t->SetBranchAddress("py",&py);
    t->SetBranchAddress("pz",&pz);
    t->SetBranchAddress("E",&E);

    t->SetBranchAddress("x",&x);
    t->SetBranchAddress("y",&y);
    t->SetBranchAddress("z",&z);
    t->SetBranchAddress("t",&time);

    t->SetBranchAddress("eta",&eta);
    t->SetBranchAddress("event",&event);

    // ===== histograms =====
    TH2D *hXY = new TH2D("hXY","x-y distribution;x (fm);y (fm)",
                         200,-15,15,200,-15,15);
    // 对齐 participant plane
    TH2D *hXY_rot = new TH2D("hXY_rot","x-y rotated;x' (fm);y' (fm)",
                             200,-15,15,200,-15,15);

    TH1D *hMult = new TH1D("hMult","Multiplicity;N;Events",200,0,2000);
    TH1D *hPt   = new TH1D("hPt","p_{T};p_{T} (GeV/c);Counts",100,0,3);
    TH1D *hEta  = new TH1D("hEta","#eta distribution;#eta;Counts",100,-5,5);
    TH1D *hPhi  = new TH1D("hPhi","#phi distribution;#phi;Counts",100,0,2*M_PI);

    TProfile *pV2Pt = new TProfile("pV2Pt","v_{2}(p_{T});p_{T};v_{2}",20,0,3);

    TH1D *hPtMean = new TH1D("hPtMean","<p_{T}>;GeV/c;Events",100,0,2);

    // ===== Pearson accumulators =====
    double sum_v2=0, sum_pt=0, sum_v2pt=0;
    double sum_v2_2=0, sum_pt2=0;
    int Nevt = 0;

    // ===== event buffers =====
    std::vector<double> pt_evt;
    std::vector<double> phi_evt;

    int current_event = -1;

    Long64_t N = t->GetEntries();

    // ===== 主循环 =====
    for (Long64_t i=0; i<N; i++) {

        t->GetEntry(i);

        // ===== event 切换 =====
        if (event != current_event && current_event != -1) {

            int mult = pt_evt.size();

            if (mult > 1) {

                hMult->Fill(mult);

                double Qx=0, Qy=0, pt_sum=0;

                for (int j=0; j<mult; j++) {
                    Qx += cos(2*phi_evt[j]);
                    Qy += sin(2*phi_evt[j]);
                    pt_sum += pt_evt[j];
                }

                double v2 = sqrt(Qx*Qx + Qy*Qy) / mult;
                double pt_mean = pt_sum / mult;

                hPtMean->Fill(pt_mean);

                // Pearson
                sum_v2 += v2;
                sum_pt += pt_mean;
                sum_v2pt += v2 * pt_mean;
                sum_v2_2 += v2 * v2;
                sum_pt2 += pt_mean * pt_mean;

                Nevt++;
            }

            pt_evt.clear();
            phi_evt.clear();
        }

        current_event = event;

        // ===== 单粒子量 =====
        hXY->Fill(x, y);

        double pt = sqrt(px*px + py*py);
        double phi = atan2(py,px);
        if (phi < 0) phi += 2*M_PI;

        // 填 histogram
        hPt->Fill(pt);
        hEta->Fill(eta);
        hPhi->Fill(phi);

        // v2(pt)
        pV2Pt->Fill(pt, cos(2*phi));

        // 存 event
        pt_evt.push_back(pt);
        phi_evt.push_back(phi);
    }

    // ===== 最后一个 event =====
    if (pt_evt.size() > 1) {

        int mult = pt_evt.size();
        hMult->Fill(mult);

        double Qx=0, Qy=0, pt_sum=0;

        for (int j=0; j<mult; j++) {
            Qx += cos(2*phi_evt[j]);
            Qy += sin(2*phi_evt[j]);
            pt_sum += pt_evt[j];
        }

        double v2 = sqrt(Qx*Qx + Qy*Qy) / mult;
        double pt_mean = pt_sum / mult;

        hPtMean->Fill(pt_mean);

        sum_v2 += v2;
        sum_pt += pt_mean;
        sum_v2pt += v2 * pt_mean;
        sum_v2_2 += v2 * v2;
        sum_pt2 += pt_mean * pt_mean;

        Nevt++;
    }

    // ===== Pearson 计算 =====
    double mean_v2 = sum_v2 / Nevt;
    double mean_pt = sum_pt / Nevt;

    double cov = sum_v2pt / Nevt - mean_v2 * mean_pt;
    double var_v2 = sum_v2_2 / Nevt - mean_v2 * mean_v2;
    double var_pt = sum_pt2 / Nevt - mean_pt * mean_pt;

    double rho = cov / sqrt(var_v2 * var_pt);

    std::cout << "=========================" << std::endl;
    std::cout << "Events = " << Nevt << std::endl;
    std::cout << "Pearson(v2, <pT>) = " << rho << std::endl;
    std::cout << "=========================" << std::endl;

    // ===== 作图 =====
    TCanvas *c1 = new TCanvas("c1","basic",1200,800);
    c1->Divide(2,2);
    c1->cd(1); hMult->Draw();
    c1->cd(2); hPt->Draw();
    c1->cd(3); hEta->Draw();
    c1->cd(4); hPhi->Draw();

    TCanvas *c2 = new TCanvas("c2","v2",800,600);
    pV2Pt->Draw();

    TCanvas *c3 = new TCanvas("c3","meanpt",800,600);
    hPtMean->Draw();

    TCanvas *c4 = new TCanvas("c4","XY",1200,500);
    c4->Divide(2,1);
    c4->cd(1);
    hXY->Draw("COLZ");
}