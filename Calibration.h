#ifndef calibration_H
#define calibration_H

#include <vector>
#include <iostream>

#include <TROOT.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>


// Provide a bg subtracted histogram, calibration peak energies and ranges in which those peaks are the maximal values
std::vector<Double_t> calibration(TH1D* histo, std::vector<Double_t> calibs, std::vector<std::vector<Int_t>> ranges) {
	std::vector<Double_t> peaks;
	std::vector<Double_t> CtoE(2);
	Int_t binmax;
	const Int_t n_peaks = calibs.size();
	Double_t sumx=1, sumy=1, sumxy=1, sumx2=1;

	// Get the values corresponding to the maximal bins within set ranges
	for (Int_t i = 0; i < n_peaks; i++){
		histo->GetXaxis()->SetRangeUser(ranges[i][0], ranges[i][1]);
		binmax = histo->GetMaximumBin();
		peaks.push_back(histo->GetXaxis()->GetBinCenter(binmax));
		sumx += calibs[i];
		sumy += peaks[i];
		sumxy += calibs[i]*peaks[i];
		sumx2 += calibs[i]*calibs[i];
	}

	// Formulas for linear regression
	CtoE[0] = (sumx2 * sumy - sumx * sumxy) / (n_peaks * sumx2 - sumx * sumx);				// intercept
	CtoE[1] = (n_peaks * sumxy - sumx * sumy) / (n_peaks * sumx2 - sumx * sumx);					// slope
	
	return CtoE;
}


// Provide secondary data -->  Obtain calibration parameters
std::vector<std::vector<Double_t>> calibrate(TString current_dir, TString filename, std::vector<Double_t> calibs, std::vector<std::vector<std::vector<Int_t>>> ranges, std::vector<TString> det_names){
	std::vector<std::vector<Double_t>> calibrations;
	TFile *file = new TFile(current_dir + filename);
	TH2F *hist_peaks; file->GetObject("hist_peaks", hist_peaks);
	Int_t nr_dets = det_names.size();
	char name[256];

	for (Int_t i = 0; i < nr_dets; i++){
		TH1D *hist_1d = (TH1D*)hist_peaks->TH2::ProjectionX("_px", i+1, i+1);
		sprintf(name, "hist_1d%d", i);
		hist_1d->SetName(name);
		calibrations.push_back(calibration(hist_1d, calibs, ranges[i]));
	}

	return calibrations;
}



// Get the channel of an energy given a vector consisting of (intercept, slope)
Double_t get_channel(std::vector<Double_t> calibs, Double_t energy) {
	Double_t channel;
	channel = calibs[0] + calibs[1]*energy;

	return channel;
}


std::vector<Double_t> get_channels(std::vector<std::vector<Double_t>> calibs, Double_t energy){
	std::vector<Double_t> channel;

	for (Int_t i = 0; i < calibs.size(); i++){
		channel.push_back(calibs[i][0] + calibs[i][1]*energy);
	}

	return channel;
}



#endif
