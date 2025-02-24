using namespace std;


pair<float,float> Rotate(float x0, float y0, float angle);
void CosmeticMap(TH2F * map, TString zaxis);
void Cosmetic1D(TH1F * h);
void DrawCMS(bool is1D=false);
TF1 *langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF);
Double_t langaufun(Double_t *x, Double_t *par);
pair<float,float> GetMPV(TH1F * h);
pair<float,float> GetSigmaT(TH1F * h);


void Cosmetic1D(TH1F * h){
	h->SetLineWidth(2);
	h->SetStats(0);
	h->GetXaxis()->SetTitleSize(0.05);
	h->GetYaxis()->SetTitleSize(0.05);
	h->GetYaxis()->SetTitleOffset(0.9);
	h->GetXaxis()->SetTitleOffset(1.1);
	h->GetXaxis()->SetLabelSize(0.045);
	h->GetYaxis()->SetLabelSize(0.045);
}

void CosmeticMap(TH2F * map, TString zaxis){
	map->SetStats(0);
	map->SetTitle("");
	map->SetTitle(Form(";x [mm];y [mm];%s",zaxis.Data()));
//	map->GetXaxis()->SetTitleOffset(0.95);
	map->GetXaxis()->SetTitleSize(0.05);
	map->GetYaxis()->SetTitleOffset(0.65);
	map->GetZaxis()->SetTitleOffset(0.8);
	map->GetYaxis()->SetTitleSize(0.05);
	map->GetZaxis()->SetTitleSize(0.055);
	
	map->GetZaxis()->SetLabelSize(0.045);


}

void DrawCMS(bool is1D){
	TLatex * tla = new TLatex();
    tla->SetTextSize(0.05);
	if(!is1D)tla->DrawLatexNDC(0.07,0.92,"#font[62]{CMS} #scale[0.8]{#font[52]{Preliminary}}");
	else tla->DrawLatexNDC(0.11,0.92,"#font[62]{CMS} #scale[0.8]{#font[52]{Preliminary}}");
}

pair<float,float> Rotate(float x0, float y0, float angle){
	float rad_angle = angle*3.14159/180.;
 	float x_rot = x0*cos(rad_angle) + y0*sin(rad_angle);
 	float y_rot = y0*cos(rad_angle) - x0*sin(rad_angle);
 	return pair<float,float>{x_rot,y_rot};
}

pair<float,float> GetSigmaT(TH1F * h, float minTime, float maxTime){

	TF1* f1  = new TF1("f1","gaus",minTime,maxTime);
	h->Fit("f1","RQ");
	float sigma = f1->GetParameter(2);
	float e_sigma = f1->GetParError(2);
	float chi2pdf = f1->GetChisquare()/f1->GetNDF();
	f1->Delete();

	return pair<float,float> {sigma,e_sigma};

}

pair<float,float> GetMPV(TH1F * h, float minAmp, float maxAmp, float hitThres){

    double fr[2]={minAmp,maxAmp};
    double st[4]={0.2*hitThres,2.5*hitThres,5000,hitThres/3.};
    double pllo[4]={0.5,1.0*hitThres,1.0,0.4};
    double plhi[4]={2*hitThres,4.*hitThres,100000.0,hitThres/2.5};
    double fp[4],fpe[4];
    double CHI2;
    int NDF;

    TF1* f1 = langaufit(h ,fr , st, pllo, plhi, fp, fpe, &CHI2, &NDF);
    f1 -> SetLineColor(3);

    gStyle->SetOptStat(1110);
    gStyle->SetOptFit(1111);
  
    float mpv = f1->GetParameter(1);
	float e_mpv = f1->GetParError(1);
    
    
	return pair<float,float> {mpv,e_mpv};

}

Double_t langaufun(Double_t *x, Double_t *par) {

      Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
      Double_t mpshift  = -0.22278298;       // Landau maximum location

      Double_t np = 100.0;      // number of convolution steps
      Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

      Double_t xx;
      Double_t mpc;
      Double_t fland;
      Double_t sum = 0.0;
      Double_t xlow,xupp;
      Double_t step;
      Double_t i;

      mpc = par[1] - mpshift * par[0];

      xlow = x[0] - sc * par[3];
      xupp = x[0] + sc * par[3];

      step = (xupp-xlow) / np;

      for(i=1.0; i<=np/2; i++) {
      	xx = xlow + (i-.5) * step;
      	fland = TMath::Landau(xx,mpc,par[0]) / par[0];
      	sum += fland * TMath::Gaus(x[0],xx,par[3]);

      	xx = xupp - (i-.5) * step;
      	fland = TMath::Landau(xx,mpc,par[0]) / par[0];
      	sum += fland * TMath::Gaus(x[0],xx,par[3]);
      }

      return (par[2] * step * sum * invsq2pi / par[3]);
  }
  TF1 * langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF)
  {

  	Int_t i;
  	Char_t FunName[100];

  	sprintf(FunName,"Fitfcn_%s",his->GetName());

  	TF1 *ffitold = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
  	if (ffitold) delete ffitold;

  	TF1 *ffit = new TF1(FunName,langaufun,fitrange[0],fitrange[1],4);
  	ffit->SetParameters(startvalues);
  	ffit->SetParNames("Width","MP","Area","GSigma");

  	for (i=0; i<4; i++) {
  		ffit->SetParLimits(i, parlimitslo[i], parlimitshi[i]);
  	}

   his->Fit(FunName,"RBQ");   // fit within specified range, use ParLimits, quiet

   ffit->GetParameters(fitparams);    // obtain fit parameters
   for (i=0; i<4; i++) {
      fiterrors[i] = ffit->GetParError(i);     // obtain fit parameter errors
  }
   ChiSqr[0] = ffit->GetChisquare();  // obtain chi^2
   NDF[0] = ffit->GetNDF();           // obtain ndf

   return (ffit);              // return fit function

}