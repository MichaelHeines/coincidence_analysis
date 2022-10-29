#ifndef coincidencefunctions_H
#define coincidencefunctions_H

#include "File_chainer.h"
#include <iostream>
#include <vector>

#include <TROOT.h>

void coincidence_creation(Double_t coincidence_window, Int_t hist_bins, std::vector<TString> det_names, std::vector<TString> coincidences_file, TString current_dir, TString write_dir, Int_t batch_datapoints, TString datatype_chain, TString datatype_tree){
	std::cout << "Making coincidences" << std::endl;
	TChain *mychain = new TChain(datatype_chain);
	add_files_to_chain(mychain, datatype_tree, ".root", current_dir);
	const Int_t nr_entries = mychain->GetEntries();
	Int_t nr_dets = det_names.size();
	printf("%d entries in chain \n", nr_entries);
	TString coinc_name = "Coincidence events ";

	// Prepare assignments input tree
	UShort_t board, detector, energy;
	ULong64_t time;
	UInt_t flags;

	mychain->SetBranchAddress("Board", &board);
	mychain->SetBranchAddress("Channel", &detector);
	mychain->SetBranchAddress("Energy", &energy);
	mychain->SetBranchAddress("Timestamp", &time);
	mychain->SetBranchAddress("Flags", &flags);

	// Coincidence tree parameters
	ULong64_t time_trig;
	Float_t timediff;
	UShort_t E_trig, E_coinc, det_trig, det_coinc, trig_board, coinc_board;
	UInt_t flags_trig, flags_coinc;

	// Utility
	ULong64_t t0, t1;
	Float_t dt;
	UShort_t e0, e1, det0, det1, board0, board1;
	UInt_t flags0, flags1;
	Int_t nr_coincidences;
	Double_t range_low, range_high;
	


	// Loop over number of desired data points within the batch
	for (Double_t i = 0; i < batch_datapoints; i++){
		std::cout << coincidences_file[i] << std::endl;
		TFile *file = TFile::Open(write_dir + coincidences_file[i], "RECREATE");
		TTree *coinc_tree = new TTree("coinctree", coinc_name);

		// Set up coincidence tree
		coinc_tree->Branch("time_trig", &time_trig, "trig_time/L");					// Time of trigger event
		coinc_tree->Branch("timediff", &timediff, "timediff/F");					// Time difference with coincidence event
		coinc_tree->Branch("E_trig", &E_trig, "E_trig/s");							// Energy of trigger event
		coinc_tree->Branch("E_coinc", &E_coinc, "E_coinc/s");						// Energy of coincidence event		
		coinc_tree->Branch("det_trig", &det_trig, "det_trig/s");					// Detector of trigger event
		coinc_tree->Branch("det_coinc", &det_coinc, "det_coinc/s");					// Detector of coincidence event
		coinc_tree->Branch("trig_board", &trig_board, "trig_board/s");				// Board of trigger event
		coinc_tree->Branch("coinc_board", &coinc_board, "coinc_board/s");			// Board of coincidence event
		coinc_tree->Branch("flags_trig", &flags_trig, "flags_trig/i");				// Flags of the trigger detector
		coinc_tree->Branch("flags_coinc", &flags_coinc, "flags_coinc/i");			// Flags of the coincidence detector

		// Prepare histograms for investigations
		TH2F *hist_energy = new TH2F("hist_coinc", "Energy spectrum; Energy (ADC); Detector", hist_bins, 0, hist_bins, nr_dets, 0.5, 0.5 + nr_dets);
		TH2F *hist_timediff = new TH2F("hist_timediff", "Time difference spectrum; Time difference (ps); Detector", 40, -1000 * coincidence_window, +1000 * coincidence_window, nr_dets, 0.5, 0.5 + nr_dets);

		for (Int_t j = 0; j < nr_dets; j++){
			hist_energy->GetYaxis()->SetBinLabel(j+1, det_names[j]);
			hist_timediff->GetYaxis()->SetBinLabel(j+1, det_names[j]);
		}

		range_low = round(i * nr_entries / batch_datapoints);
		range_high = round((i + 1) * nr_entries / batch_datapoints);

		// Loop that searches for coincidences and puts them in a tree
		for (Int_t j = range_low; j < range_high; j++){
			mychain->GetEntry(j);

			board0 = board;
			det0 = detector;
			e0 = energy;
			t0 = time;
			flags0 = flags;
			Int_t index_upper = range_high;			

			for (Int_t k = j+1; k < index_upper; k++){
				mychain->GetEntry(k);

				board1 = board;
				det1 = detector;
				e1 = energy;
				t1 = time;
				flags1 = flags;

				dt = t1 - t0;
				if (dt < 1000*coincidence_window){
					nr_coincidences += 1;

					time_trig = t0;
					timediff = dt;
					E_trig = e0;
					E_coinc = e1;
					det_trig = det0;
					det_coinc = det1;
					trig_board = board0;
					coinc_board = board1;
					flags_trig = flags0;
					flags_coinc = flags1;
					coinc_tree->Fill();

					time_trig = t1;
					timediff = -dt;
					E_trig = e1;
					E_coinc = e0;
					det_trig = det1;
					det_coinc = det0;
					trig_board = board1;
					coinc_board = board0;
					flags_trig = flags1;
					flags_coinc = flags0;
					coinc_tree->Fill();

					hist_energy->Fill(e0, det0+1);
					hist_timediff->Fill(dt, det0+1);
					hist_energy->Fill(e1, det1+1);
					hist_timediff->Fill(-dt, det1+1);
				}
				else break;
			} 
		}

		// Write (and potentially draw) the histograms
		hist_energy->Write("", TObject::kOverwrite);
		hist_timediff->Write("", TObject::kOverwrite);
		// hcoinc->Draw();
		// htdiff->Draw();

		// Write tree
		coinc_tree->Write("", TObject::kOverwrite);
		file->TFile::Close("R");
	}

	std::cout << "Saved Coincidence trees" << std::endl;
}


// Give 2 detectors and 2 energies and get the coincidence counts from E1 in det1 with E2 in det2
std::vector<std::vector<Int_t>> get_coincrates(TString write_dir, TString filename, std::vector<TString> det_names, std::vector<Double_t> channels0, std::vector<Double_t> channels1, std::vector<std::vector<Int_t>> hwidths, Double_t window_lower, Double_t window_upper){
	const Int_t nr_dets = det_names.size();
	std::vector<std::vector<Int_t>> coincidences(nr_dets, std::vector<Int_t>(nr_dets, 0));
	TFile *file = new TFile(write_dir + filename);
	TTree *mytree; file->GetObject("coinctree", mytree);
	Int_t nr_entries = mytree->GetEntries();

	Float_t timediff;
	UShort_t E_trig, E_coinc, det_trig, det_coinc, trig_board, coinc_board;

	mytree->SetBranchAddress("timediff", &timediff);
	mytree->SetBranchAddress("E_trig", &E_trig);
	mytree->SetBranchAddress("E_coinc", &E_coinc);
	mytree->SetBranchAddress("det_trig", &det_trig);
	mytree->SetBranchAddress("det_coinc", &det_coinc);
	mytree->SetBranchAddress("trig_board", &trig_board);
	mytree->SetBranchAddress("coinc_board", &coinc_board);

	// Utility
	Double_t energydiff0_0, energydiff0_1, energydiff1_0, energydiff1_1;

	for (Int_t i = 0; i < nr_entries; i++){
		mytree->GetEntry(i);

		if (timediff < std::max(abs(window_lower), abs(window_upper))){
			energydiff0_0 = abs(E_trig - channels0[det_trig]);
			energydiff0_1 = abs(E_trig - channels1[det_trig]);
			energydiff1_0 = abs(E_coinc - channels0[det_coinc]);
			energydiff1_1 = abs(E_coinc - channels1[det_coinc]);

			if (energydiff0_0 <= hwidths[det_trig][0]){
				if (energydiff1_1 <= hwidths[det_coinc][1]){
					if (window_lower <= timediff <= window_upper){
						coincidences[det_trig][det_coinc] += 1;
					}
				}
			}
			else if (energydiff0_1 <= hwidths[det_trig][1]){
				if (energydiff1_0 <= hwidths[det_coinc][0]){
					if (window_lower <= -timediff <= window_upper){
						coincidences[det_coinc][det_trig] += 1;
					}
				}
			}
		}
	}

	file->TFile::Close();
	return coincidences;
}


#endif
