#ifndef singlesfunctions_H
#define singlesfunctions_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TSpectrum.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

#include "File_chainer.h"


// Function that estimates the background on a 1d spectrum
void populate_background_1d(TH1F* histogram, TH1F* bg_histogram, Int_t xmax){
	//histogram->Draw("L");
	TSpectrum* s = new TSpectrum();
	Double_t* source = new Double_t[xmax];
	for (Int_t i = 0; i < xmax; i++) source[i] = histogram->GetBinContent(i + 1);
	s->Background(source, xmax, 4, 1, 0, kFALSE, 3, kTRUE);
	for (Int_t i = 0; i < xmax; i++) bg_histogram->SetBinContent(i + 1, source[i]);
	bg_histogram->SetLineColor(kRed);
	//bg_histogram->Draw("SAME L");
	//gPad->SetLogy();
}


// Function that estimates the background on a 2d spectrum
void populate_background_2d(TH2F* histogram, TH2F* bg_histogram, Int_t xmax, Int_t nr_dets){
	Double_t* source = new Double_t[xmax];
	TSpectrum* s = new TSpectrum();

	for (Int_t i = 0; i < nr_dets; i++){
		for (Int_t j = 0; j < xmax; j++){
			source[j] = histogram->GetBinContent(j + 1, i+1);
		}

		s->Background(source, xmax, 4, 1, 0, kFALSE, 3, kTRUE);
		for (Int_t j = 0; j < xmax; j++) bg_histogram->TH2::SetBinContent(j + 1, i+1, source[j]);
		bg_histogram->SetLineColor(kRed);
	}
}



// Create histograms for the singles
void create_single_histogram(Int_t hist_bins, std::vector<TString> det_names, std::vector<TString> singles_file, TString current_dir, TString write_dir, Int_t batch_datapoints, TString datatype_chain, TString datatype_tree){
	std::cout << "Making histograms" << std::endl;

	TChain *mychain = new TChain(datatype_chain);	
	add_files_to_chain(mychain, datatype_tree, ".root", current_dir);

	UShort_t board, detector, energy;
	ULong64_t time;
	UInt_t flags;

	mychain->SetBranchAddress("Board", &board);
	mychain->SetBranchAddress("Channel", &detector);
	mychain->SetBranchAddress("Energy", &energy);
	mychain->SetBranchAddress("Timestamp", &time);
	mychain->SetBranchAddress("Flags", &flags);

	Double_t range_low, range_high;
	Int_t entries = mychain->GetEntries();
	Int_t nr_dets = det_names.size();
	char name[256];

	for (Int_t i = 0; i < batch_datapoints; i++){
		TFile *file  = TFile::Open(write_dir + singles_file[i], "RECREATE");

		sprintf(name, "hist_full%d", i);
		TH2F *hist_full = new TH2F(name, "Energy spectrum; Energy (ADC); Detector", hist_bins, 0, hist_bins, nr_dets, 0.5, nr_dets + 0.5);
		sprintf(name, "hist_bg%d", i);
		TH2F *hist_bg = new TH2F(name, "Background; Energy (ADC); Detector", hist_bins, 0, hist_bins, nr_dets, 0.5, nr_dets + 0.5);

		for (Int_t j = 0; j < nr_dets; j++){
			hist_full->GetYaxis()->SetBinLabel(j+1, det_names[j]);
			hist_bg->GetYaxis()->SetBinLabel(j+1, det_names[j]);
		}

		range_low = round(i * entries / batch_datapoints);
		range_high = round((i + 1) * entries / batch_datapoints);

		for (Int_t j = range_low; j < range_high; j++){
			mychain->GetEntry(j);
			hist_full->Fill(energy, detector+1);
		}

		populate_background_2d(hist_full, hist_bg, hist_bins, nr_dets);
		sprintf(name, "hist_peaks%d", i);
		TH2F *hist_peaks = (TH2F*)hist_full->Clone(name);
		hist_peaks->Add(hist_bg, -1);

		hist_full->Write("hist_full", TObject::kOverwrite);
		hist_bg->Write("hist_bg", TObject::kOverwrite);
		hist_peaks->Write("hist_peaks", TObject::kOverwrite);
		file->TFile::Close("R");
	}

	std::cout << "Wrote singles histograms" << std::endl;	
}



//calculates integral of bins
ULong64_t bin_integral(Double_t channel, TH2F* histogram, Int_t hwidth, Int_t det) {
	TH1D *hist_1d = (TH1D*)histogram->ProjectionX("_px", det+1, det+1);

	Int_t peak = round(channel);
	hist_1d->GetXaxis()->SetRangeUser(peak - hwidth, peak + hwidth);
	Int_t lower = peak - hwidth;
	Int_t upper = peak + hwidth;
	ULong64_t integral;

	integral = hist_1d->Integral(hist_1d->FindBin(lower), hist_1d->FindBin(upper));
	//histogram->Draw();

	return integral;
}



std::vector<Double_t> get_singlerates(TString write_dir, TString filename, std::vector<Double_t> channels, std::vector<Int_t> hwidth, std::vector<TString> det_names){
	std::vector<Double_t> counts;
	TFile* file = new TFile(write_dir + filename);								// Load the root file
	TH2F *hist_peaks; file->GetObject("hist_peaks", hist_peaks);
	Int_t nr_dets = det_names.size();

	for (Int_t i=0; i < nr_dets; i++){
		counts.push_back(bin_integral(channels[i], hist_peaks, hwidth[i], i));
	}

	return counts;
}



#endif
