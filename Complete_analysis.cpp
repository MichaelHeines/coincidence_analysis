#include <iostream>
#include <vector>

#include <TROOT.h>

#include "Reading_dat.h"
#include "File_chainer.h"
#include "Singles_functions.h"
#include "Coincidence_functions.h"
#include "Time_functions.h"
#include "Calibration.h"

#include "Exponential_fit.h" // Not implemented yet

// INPUT: .dat file, for format see Readin_dat.h
// OUTPUT: Currently csv; Fitting in python is nice and easy, keep it for now but made empty header Exponential_fit.h that can be implemented
// Future development: fit with exponential, option to provide type of leafs?

int main(int argc, char** argv) {
	if (argc != 2){
		std::cout << "ERROR: Should have exactly one input parameter" << std::endl;
		std::cout << "Please use ./main.exe directory/settings.dat" << std::endl;
		return 0;
	}

	TString settings = argv[1];
	Read_out loading = Reading_dat(settings);

	// Specific initialization
	TString data_dir, write_dir, output_file, coincidences_filenames, singles_filenames, datatype_chain, datatype_tree;
	Int_t nr_dirs, nr_calibs, nr_dets, hist_bins, averaging_method;
	Double_t half_life, ratio_to_activity, E1, E2, month_0, day_0, hour_0, minute_0;
	bool process_raw;
	std::vector<TString> ls_dirs, indices, det_names;
	std::vector<Double_t> calib_energies, coincidence_window, months, days, hours, minutes;
	std::vector<Int_t> points_per_batch;
	std::vector<std::vector<Int_t>> hwidths;
	std::vector<std::vector<std::vector<Int_t>>> calib_ranges;

	data_dir = loading.data_dir; write_dir = loading.write_dir; output_file = loading.output_file, coincidences_filenames = loading.coincidences_filenames, singles_filenames = loading.singles_filenames, datatype_chain = loading.datatype_chain, datatype_tree = loading.datatype_tree;
	nr_dirs = loading.nr_dirs; nr_calibs = loading.nr_calibs; nr_dets = loading.nr_dets; hist_bins = loading.hist_bins, averaging_method = loading.averaging_method;
	half_life = loading.half_life; ratio_to_activity = loading.ratio_to_activity; E1 = loading.E1; E2 = loading.E2; month_0 = loading.month_0; day_0 = loading.day_0; hour_0 = loading.hour_0; minute_0 = loading.minute_0;
	process_raw = loading.process_raw;
	ls_dirs = loading.ls_dirs; indices = loading.indices; det_names = loading.det_names;
	calib_energies = loading.calib_energies; coincidence_window = loading.coincidence_window; months = loading.months; days = loading.days; hours = loading.hours; minutes = loading.minutes;
	points_per_batch = loading.points_per_batch;
	hwidths = loading.hwidths;
	calib_ranges = loading.calib_ranges;

	// Utility
	TString current_dir, index, second_index;
	std::vector<Double_t> init_times(nr_dirs), init_timediffs;
	std::vector<std::vector<Double_t>> mean_times, durations, calibs, activities, activity_errors;
	std::vector<Double_t> channels0, channels1, counts0, counts1;
	std::vector<std::vector<Int_t>> coincidences, ratios;
	Double_t N1, N2, N12, duration, ratio, local_activity, local_relerr;

	// Get relevant rates
	std::vector<Int_t> hwidth0, hwidth1;
	for (Int_t i = 0; i < nr_dets; i++){
		hwidth0.push_back(hwidths[i][0]);
		hwidth1.push_back(hwidths[i][1]);
	}



	// Calculate the mean times
	for (Int_t i = 0; i < nr_dirs; i++) {
		std::vector<Double_t> temp_init_time, temp_durations;
		current_dir = data_dir + ls_dirs[i];
		init_times[i] = 30 * (months[i] - month_0) + (days[i] - day_0) + ((hours[i] - hour_0) / 24.0) + ((minutes[i] - minute_0) / (24.0*60));
		init_timediffs = init_tdiff(current_dir, datatype_chain, datatype_tree, points_per_batch[i]);

		for (Int_t j = 0; j < points_per_batch[i]; j++){
			temp_init_time.push_back(init_times[i] + (init_timediffs[j] / 86400));
			temp_durations.push_back((init_timediffs[j+1] - init_timediffs[j]) / 86400);
		}

		durations.push_back(temp_durations);
		mean_times.push_back(get_meantimes(temp_init_time, temp_durations, half_life, points_per_batch[i]));		
	}



	// Process primary data (if process_raw = True) --> Singles histograms & coincidences trees
	for (Int_t i = 0; i < nr_dirs; i++){
		std::vector<TString> coincidences_file(points_per_batch[i]), singles_file(points_per_batch[i]);
		current_dir = data_dir + ls_dirs[i];

		for (Int_t j = 0; j < points_per_batch[i]; j++){
			second_index = std::to_string(j);
			index = indices[i] + second_index;
			singles_file[j] = index + singles_filenames;
			coincidences_file[j] = index + coincidences_filenames;
		}

		if (process_raw == 0){
			create_single_histogram(hist_bins, det_names, singles_file, current_dir, write_dir, points_per_batch[i], datatype_chain, datatype_tree);
			coincidence_creation(coincidence_window[0], hist_bins, det_names, coincidences_file, current_dir, write_dir, points_per_batch[i], datatype_chain, datatype_tree);
		}
	}



	// Process secondary data --> Obtain activity values in a csv format
	std::ofstream Activity_file(write_dir + output_file);
	Activity_file << "Batch" << "Datapoint" <<  ";" << "A (from Tl)" << ";" << "A relerr (from Tl)" << ";" << "Mean times" << std::endl;

	// Loop over secondary data to find single and coincidence rates
	for (Int_t i = 0; i < nr_dirs; i++){
		std::vector<TString> coincidences_file(points_per_batch[i]), singles_file(points_per_batch[i]);
		std::vector<Double_t> activities_dir, activity_errors_dir;

		for (Int_t j = 0; j < points_per_batch[i]; j++){
			N1 = 0; N2 = 0; N12 = 0;
			second_index = std::to_string(j);
			index = indices[i] + second_index;
			coincidences_file[j] = index + coincidences_filenames;
			singles_file[j] = index + singles_filenames;

			// Calibrate detectors
			calibs = calibrate(write_dir, singles_file[i], calib_energies, calib_ranges, det_names);
			channels0 = get_channels(calibs, E1);
			channels1 = get_channels(calibs, E2);

			// Get counts
			counts0 = get_singlerates(write_dir, singles_file[j], channels0, hwidth0, det_names);
			counts1 = get_singlerates(write_dir, singles_file[j], channels1, hwidth1, det_names);
			coincidences = get_coincrates(write_dir, coincidences_file[j], det_names, channels0, channels1, hwidths, coincidence_window[1], coincidence_window[2]);	
			
			// Calculate activities with all combinations (set to 0 if less than 3 coincidences)
			for (Int_t k = 0; k < nr_dets; k++){
				N1 += counts0[k];
				N2 += counts1[k];

				for (Int_t l = 0; l < nr_dets; l++){
					if (k != l){
						N12 += coincidences[k][l];
					}
				}
			}
			
			ratio = N1 * N2 / (nr_dets * 1.0 * N12);
			local_activity = ratio * ratio_to_activity / (durations[i][j] * 86400);
			local_relerr = sqrt(1/N12);
			Activity_file << i << ";" << j << ';' << local_activity << ";" << local_relerr << ";" << mean_times[i][j] << std::endl;

			activities_dir.push_back(local_activity);
			activity_errors_dir.push_back(local_relerr);		
		}

		activities.push_back(activities_dir);
		activity_errors.push_back(activity_errors_dir);
	}



	// Fit with an exponential
	// To be included in the code

	std::cout << "Ding dong, analysis is done" << std::endl;
	return 1;
}





