#ifndef timefunctions_H
#define timefunctions_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TFile.h>



// Determine the time differences from the initial datapoint within a batch
std::vector<Double_t> init_tdiff(TString current_dir, TString datatype_chain, TString datatype_tree, Int_t batch_datapoints){
	std::vector<Double_t> init_tdiff;
	TChain* mychain = new TChain(datatype_chain);
	add_files_to_chain(mychain, datatype_tree, ".root", current_dir);
	Int_t init_index;

	ULong64_t time;
	mychain->SetBranchAddress("Timestamp", &time);
	const Int_t nr_entries = mychain->GetEntries();
	for (Int_t i = 0; i <= batch_datapoints; i++){
		init_index = round(i * nr_entries / batch_datapoints);
		if (i == batch_datapoints) init_index = nr_entries-1;
		mychain->GetEntry(init_index);
		init_tdiff.push_back(time/ 1.0e12);
	}

	return init_tdiff;
}


// Determine the mean times of a batch
std::vector<Double_t> get_meantimes(std::vector<Double_t> init_times, std::vector<Double_t> durations, Double_t half_life, Int_t batch_datapoints){
	std::vector<Double_t> mean_times(batch_datapoints);
	Double_t decay_cte = log(2) / half_life;

	for (Int_t i = 0; i < batch_datapoints; i++){
		mean_times[i] = init_times[i] + durations[i] + (1 / decay_cte) * log(decay_cte*durations[i]) - (1 / decay_cte) * log(exp(decay_cte * durations[i]) - 1);
	}

	return mean_times;
}


#endif
