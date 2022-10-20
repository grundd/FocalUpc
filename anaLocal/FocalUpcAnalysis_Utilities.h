// FocalUpcAnalysis_Utilities.h
// David Grund, Oct 15, 2022

// root headers
#include "TList.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TGraph.h"
#include "TClonesArray.h"
#include "TParticle.h"
#include "TMarker.h"
#include "TLine.h"
#include "TLegend.h"
#include "TString.h"
// aliroot headers
#include "AliStack.h"
#include "AliFOCALGeometry.h"
#include "AliFOCALCluster.h"

// for plotting:
// (distance in centimeters)
// (FOCAL inner radius ~ 4cm, outer radius ~ 45cm according to the LoI)
//Float_t z_min = -10.;
//Float_t z_max = 890.;
Float_t z_min = 698.;
Float_t z_max = 785.;
Float_t x_min = -52.;
Float_t x_max = 52.;
// x_min and x_max will be used for y as well

Float_t etaFOClow = 3.4;
Float_t etaFOCupp = 5.8;

using std::cout;
using std::endl;

Bool_t areSame(Float_t a, Float_t b)
{
    // desired precision: ~0.00001
    Float_t epsilon = 1e-5;
    return TMath::Abs(a - b) < epsilon;
}

Bool_t isEleOrPos(TParticle* part)
{
    // is particle an electron or a positron
    return (TMath::Abs(part->GetPdgCode()) == 11);
}

TLorentzVector ConvertXYZEtoLorVec(Float_t x, Float_t y, Float_t z, Float_t e, Bool_t debug = kFALSE)
{
    TLorentzVector vec4;
    // calculate kinematic variables
    Float_t r = TMath::Sqrt(x*x + y*y + z*z);
    Float_t r_xy = TMath::Sqrt(x*x + y*y);
    // polar angle: [0,pi]
    Float_t theta = TMath::ACos(z/r);
    // assume that mass is negligible compared to energy
    Float_t p = e;
    // z-projection of the momentum and transverse momentum
    Float_t pz = p * TMath::Cos(theta);
    Float_t pt = p * TMath::Sin(theta);
    // pseudorapidity
    Float_t eta = 1.0e+6;
    if(theta > 1e-6) eta = -TMath::Log(TMath::Tan(theta/2.));
    // azimuthal angle: [0,2pi)
    Float_t phi = TMath::ATan2(y,x); // returns values between -pi < phi <= pi
    // x- and y-projections of the momentum
    Float_t px = pt * TMath::Cos(phi);
    Float_t py = pt * TMath::Sin(phi);
    // create the 4-vector
    vec4.SetPxPyPzE(px,py,pz,e);
    // check that we calculated everything properly
    if(debug)
    {
        TVector3 vec3_1(x,y,z);
        vec3_1.SetMag(1.);
        TVector3 vec3_2 = vec4.Vect();
        vec3_2.SetMag(1.);
        for(Int_t i = 0; i < 3; i++) if(!areSame(vec3_1[i],vec3_2[i])) Printf("Problem with a coordinate %i: %.10f, %.10f",i+1,vec3_1[i],vec3_2[i]);

        Printf("-------------------------------------------------------");
        Printf("(x,y,z) [cm]: (%.3f,%.3f,%.3f)",vec4.X(),vec4.Y(),vec4.Z());
        Printf("(px,py,pz,E) [GeV]: (%.3f,%.3f,%.3f,%.3f)",vec4.Px(),vec4.Py(),vec4.Pz(),vec4.E());
        Printf("theta [-]: %.4f",vec4.Theta());
        Printf("phi [-]: %.4f",vec4.Phi());
        Printf("eta [-]: %.4f",vec4.Eta());
        Printf("");
    }
    return vec4;
}

// ******************************************************************************************************************
// Functions to draw event display with MC tracks
// ******************************************************************************************************************

TCanvas* PrepareCanvas()
{
    // canvas showing a 2d view of an event (z-r space)
    TCanvas* c = new TCanvas("c","",700,800);
    c->SetLeftMargin(0.);
    c->SetTopMargin(0.);
    c->SetRightMargin(0.);
    c->SetBottomMargin(0.);
    c->Divide(1,2,0,0);
    c->cd(1);
    gPad->SetTopMargin(0.08);
    gPad->SetLeftMargin(0.08);
    gPad->SetRightMargin(0.01);
    c->cd(2);
    gPad->SetLeftMargin(0.08);
    gPad->SetRightMargin(0.01);
    gPad->SetBottomMargin(0.08);
    // z-x plane
    c->cd(1);
    TH2F* hX = new TH2F("hX","",1,z_min,z_max,1,x_min,x_max);
    hX->SetTitle(";z (cm);x (cm)");
    hX->SetBit(TH2::kNoTitle);
    hX->SetBit(TH2::kNoStats);
    hX->Draw("AXIS");
    // z-y plane
    c->cd(2);
    TH2F *hY = new TH2F(*hX);
    hY->SetTitle(";z (cm);y (cm)");
    hY->Draw("AXIS");
    return c;
}

void DrawFOCALSegment(Float_t z_min, Float_t z_max, Float_t xy_width)
{
    TGraph* grArea = new TGraph(4);
    grArea->SetPoint(0,z_min,-xy_width);
    grArea->SetPoint(1,z_min,xy_width);
    grArea->SetPoint(2,z_max,xy_width);
    grArea->SetPoint(3,z_max,-xy_width);
    grArea->SetFillStyle(1001);
    grArea->SetFillColorAlpha(kGray,0.80);
    grArea->Draw("F SAME");
    return;
}

void DrawFOCAL(AliFOCALGeometry* geometry, TCanvas* c)
{
    // go over the segments and plot the detector pieces
    for(Int_t i = 0; i < 7; i++)
    {
        Float_t z_min = geometry->GetVirtualSegmentZ(i) - geometry->GetVirtualSegmentSizeZ(i) / 2. + 0.5;
        Float_t z_max = geometry->GetVirtualSegmentZ(i) + geometry->GetVirtualSegmentSizeZ(i) / 2. - 0.5;
        Float_t x_halfwidth = geometry->GetFOCALSizeX() / 2;
        Float_t y_halfwidth = geometry->GetFOCALSizeY() / 2;
        //cout << z_min << " " << z_max << " " << x_halfwidth << " " << y_halfwidth << endl;
        c->cd(1);
        DrawFOCALSegment(z_min,z_max,x_halfwidth);
        c->cd(2);
        DrawFOCALSegment(z_min,z_max,y_halfwidth);
    }
    return;
}

void DrawSegClusters(TClonesArray* arrClsSeg, TCanvas* c)
{
    for(Int_t iClust = 0; iClust < arrClsSeg->GetEntries(); iClust++)
    {
        AliFOCALCluster* clust = (AliFOCALCluster*) arrClsSeg->At(iClust);
        Float_t z(clust->Z()), x(clust->X()), y(clust->Y());
        // z-x plane
        TMarker* mX = new TMarker(z,x,0);
        mX->SetMarkerStyle(kOpenSquare);
        mX->SetMarkerColor(kRed);
        mX->SetMarkerSize(.5);
        c->cd(1);
        mX->Draw("SAME");
        // z-y plane
        TMarker* mY = new TMarker(z,y,0);
        mY->SetMarkerStyle(kOpenSquare);
        mY->SetMarkerColor(kRed);
        mY->SetMarkerSize(.5);
        c->cd(2);
        mY->Draw("SAME");
    }
    return;
}

void DrawPrefClusters(TList* listClsPref, TCanvas* c)
{
    TLegend* l = new TLegend(0.35,0.4-(listClsPref->GetEntries()+1)*0.04,0.78,0.4);
    l->SetMargin(0.10);
    l->SetTextSize(0.025);
    l->SetBorderSize(0);
    l->AddEntry((TObject*)0,Form("%i summed prefiltered clusters :", listClsPref->GetEntries()),"");
    for(Int_t iClust = 0; iClust < listClsPref->GetEntries(); iClust++) 
    {
        AliFOCALCluster* clust = (AliFOCALCluster*)listClsPref->At(iClust);
        Float_t z(clust->Z()), x(clust->X()), y(clust->Y());
        Float_t r = TMath::Sqrt(TMath::Power(x,2) + TMath::Power(y,2));
        // z-x plane
        TMarker *mX = new TMarker(z,x,0);
        mX->SetMarkerStyle(kOpenCircle);
        mX->SetMarkerColor(kGreen+1);
        mX->SetMarkerSize(1);
        c->cd(1);
        mX->Draw("SAME");
        // z-y plane
        TMarker *mY = new TMarker(z,y,0);
        mY->SetMarkerStyle(kOpenCircle);
        mY->SetMarkerColor(kGreen+1);
        mY->SetMarkerSize(1);
        c->cd(2);
        mY->Draw("SAME");
        l->AddEntry(mX,Form("x = %.2f cm, y = %.2f cm, E = %.1f GeV", x, y, clust->E()),"P");
    }
    c->cd(1);
    l->Draw();
    return;
}

void TrackCoordinatesAtZ(TParticle* p, Float_t z, Float_t &x, Float_t &y)
{
    Float_t x0(p->Vx()), y0(p->Vy()), z0(p->Vz());
    x = x0 + p->Px() / p->Pz() * (z - z0);
    y = y0 + p->Py() / p->Pz() * (z - z0);
    return;
}

void SetTrackStyle(TLine* l, Int_t codePDG, Bool_t isPrimary = kTRUE)
{
    if(TMath::Abs(codePDG) == 11) // an electron
    {
        if(isPrimary) l->SetLineColor(kBlue);
        else          l->SetLineColor(kCyan);
        l->SetLineWidth(1);
        l->SetLineStyle(1);
    }
    if(codePDG == 22) // a photon
    {
        if(isPrimary) l->SetLineColor(kViolet);
        else          l->SetLineColor(kCyan);
        l->SetLineWidth(2);
        l->SetLineStyle(2);
    }
    return;
}

void DrawTracksMC(AliStack *stack, TCanvas* c, Bool_t withLegend = kTRUE)
{
    // calculate the number of primary particles
    Int_t nPrim(0.);
    for(Int_t iTrk = 0; iTrk < stack->GetNtrack(); iTrk++) if(stack->IsPhysicalPrimary(iTrk)) nPrim++;
    // create the legend
    TLegend* l = new TLegend(0.35,0.8-(nPrim+1)*0.04,0.78,0.8);
    l->SetMargin(0.1);
    l->SetTextSize(0.025);
    l->SetBorderSize(0);
    l->AddEntry((TObject*)0,Form("%i primary MC tracks:",nPrim),"");
    // go over MC particles in the stack and plot them
    for(Int_t iTrk = 0; iTrk < stack->GetNtrack(); iTrk++) 
    {
        TParticle *part = stack->Particle(iTrk);
        // 1,2 ... xy coordinates at z_min (plotting) and xy coordinates at production vertex
        // depends on which value of z is lower
        Float_t x1(0.), y1(0.), z1(0.); 
        Float_t x2(0.), y2(0.), z2(0.); 
        Float_t x3(0.), y3(0.), z3(700.);  // 3 ... xy coordinates when entering FOCAL (z ~ 700 cm)
        Float_t x4(0.), y4(0.), z4(z_max); // 4 ... xy coordinates at z_max (plotting)
        if(part->Vz() > z_min)
        {
            z1 = z_min; z2 = part->Vz();
            TrackCoordinatesAtZ(part,z_min,x1,y1);
            TrackCoordinatesAtZ(part,part->Vz(),x2,y2);
        }
        else
        {
            z1 = part->Vz(); z2 = z_min;
            TrackCoordinatesAtZ(part,part->Vz(),x1,y1);
            TrackCoordinatesAtZ(part,z_min,x2,y2);
        }
        TrackCoordinatesAtZ(part,700.,x3,y3);
        TrackCoordinatesAtZ(part,z_max,x4,y4);
        TLine *lX = NULL;
        TLine *lY = NULL;
        if(part->Pz() > 0) // particles heading right
        {
            lX = new TLine(z2,x2,z4,x4);
            lY = new TLine(z2,y2,z4,y4);
        }
        else // particles heading left
        {
            lX = new TLine(z2,x2,z1,x1);
            lY = new TLine(z2,y2,z1,y1);
        }
        // primary particles
        if(stack->IsPhysicalPrimary(iTrk))
        {
            // a rough selection: if transverse coordinates when entering FOCAL larger than x_max = 52 cm, do not plot the track
            if(TMath::Abs(x3) > x_max || TMath::Abs(y3) > x_max)
            {
                if(TMath::Abs(part->GetPdgCode()) == 11) l->AddEntry((TObject*)0,Form("(outside the plot) electron: E = %.1f GeV", part->Energy()),"");
                if(TMath::Abs(part->GetPdgCode()) == 22) l->AddEntry((TObject*)0,Form("(outside the plot) photon: E = %.1f GeV", part->Energy()),"");
            } 
            else
            {
                if(TMath::Abs(part->GetPdgCode()) == 11) l->AddEntry(lX,Form("electron: E = %.1f GeV, px = %.2f GeV/c, py = %.2f GeV/c", part->Energy(), part->Px(), part->Py()),"L");
                if(TMath::Abs(part->GetPdgCode()) == 22) l->AddEntry(lX,Form("photon: E = %.1f GeV, px = %.2f GeV/c, py = %.2f GeV/c", part->Energy(), part->Px(), part->Py()),"L");
                // z-x plane
                SetTrackStyle(lX,part->GetPdgCode(),kTRUE);
                c->cd(1);
                lX->Draw("SAME");
                // z-y plane
                SetTrackStyle(lY,part->GetPdgCode(),kTRUE);
                c->cd(2);
                lY->Draw();
            } 
        }
        // daughters of primary particles with at least 500 MeV
        else if(part->GetMother(0) >= 0 && stack->IsPhysicalPrimary(part->GetMother(0)) && part->Energy() > 1)
        {
            // a rough selection: if transverse coordinates when entering FOCAL larger than x_max = 52 cm, do not plot the track
            if(TMath::Abs(x3) < x_max && TMath::Abs(y3) < x_max)
            {
                // z-x plane
                SetTrackStyle(lX,part->GetPdgCode(),kFALSE);
                c->cd(1);
                lX->Draw("SAME");
                // z-y plane
                SetTrackStyle(lY,part->GetPdgCode(),kFALSE);
                c->cd(2);
                lY->Draw();
            }
        }
    }
    c->cd(1);
    if(withLegend) l->Draw();
}