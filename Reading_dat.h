#ifndef Reading_dat_H
#define Reading_dat_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <TROOT.h>

//////// Format for settings file ////////
// Line1: comment line for user --> skip
// Line2: read dir string
// Line3: write dir string
// Line4: output csv string
// Line5: coincidence filename string
// Line6: single filename string
// Line7: Number of directories
// Line8: ls dirs vector of strings
// Line9: indices vector of strings
// Line10: -----
// Line11: Nr of calibration energies, Nr of detector channels
// Line12: Detector names
// Line13: Calibration energy values		, separated
// Line14: Calibration ranges				; separated for detectors , separated for different energies [min max]
// Line15: -----
// Line16: half-life, ratio_to_activity, E1, E2
// Line17: hwidths							; separated for detectors [hwidth1 hwidth 2]
// Line18: -----
// Line19: dataype_chain, datatype_tree
// Line20: coincidence window [broad, narrow_low, narrow_high]
// Line21: hist_bins
// Line22: points_per_batch
// Line23: -----
// Line24: Start Month, day, hour, minute
// Line25: Months, days, hours, minutes
// Line26: -----
// Line27: process_raw, fit_expo, method (0 = averaging activities, 1 = summing counts)
// Line28: datatype board, datatype channel, datatype energy, datatype time 			// Future development?
// Line29: -----
////////////////////////////////////////////////

// Structure made specifically for initialization purpose
struct Read_out {
	TString data_dir, write_dir, output_file, coincidences_filenames, singles_filenames, datatype_chain, datatype_tree;
	Int_t nr_dirs, nr_calibs, nr_dets, hist_bins, averaging_method;
	Double_t half_life, ratio_to_activity, E1, E2, month_0, day_0, hour_0, minute_0;
	bool process_raw, fit_expo;
	std::vector<TString> ls_dirs, indices, det_names;
	std::vector<Double_t> calib_energies, coincidence_window, months, days, hours, minutes;
	std::vector<Int_t> points_per_batch;
	std::vector<std::vector<Int_t>> hwidths;
	std::vector<std::vector<std::vector<Int_t>>> calib_ranges;
};

// Read data from settings file
Read_out Reading_dat(TString dat_name){
	//	In: Filename and initialized variables
	//	Out: Read_out structure
	Read_out output;

	std::ifstream infile(dat_name);
	std::string line;
	std::string delim_short = ", ";
	std::string delim_long = "; ";
	std::string major_bracket0 = "{";
	std::string major_bracket1 = "}";
	std::string minor_bracket0 = "[";
	std::string minor_bracket1 = "]";
	Int_t temp_int0, temp_int1, iteration0, iteration1;
	Double_t temp_double0, temp_double1;
	std::string temp_string0, temp_string1;
	

	try {
		std::getline(infile, line);
		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		output.data_dir = line;
		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		output.write_dir = line;
		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		output.output_file = line;
		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		output.coincidences_filenames = line;
		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		output.singles_filenames = line;
		std::getline(infile, line);
		std::stringstream(line) >> output.nr_dirs;
		
		if (output.nr_dirs == 0){
			std::cerr << "ERROR: nr_dirs can't be 0; check if the format is correct." << std::endl;
			exit(0);
		}

		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		try{
			for (Int_t i = 0; i < output.nr_dirs; i++){
				output.ls_dirs.push_back(line.substr(0, line.find(delim_short)));
				line.erase(0, line.find(delim_short) + delim_short.length());
			}

			std::getline(infile, line);
			line.erase(line.size() - 1, line.size());
			for (Int_t i = 0; i < output.nr_dirs; i++){
				output.indices.push_back(line.substr(0, line.find(delim_short)));
				line.erase(0, line.find(delim_short) + delim_short.length());
			}
		}
		catch(...){
			std::cerr << "ERROR: Number of files not appropriate.";
			exit(0);
		}

		std::getline(infile, line);
		std::getline(infile, line);
		std::stringstream(line.substr(0, line.find(delim_short))) >> output.nr_calibs;
		line.erase(0, line.find(delim_short) + delim_short.length());
		std::stringstream(line.substr(0, line.find(delim_short))) >> output.nr_dets;
		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());

		try{
			for (Int_t i = 0; i < output.nr_dets; i++){
				output.det_names.push_back(line.substr(0, line.find(delim_short)));
				line.erase(0, line.find(delim_short) + delim_short.length());
			}
		}
		catch(...){
			std::cerr << "ERROR: Number of detector names does not match number of detectors";
			exit(0);
		}

		std::getline(infile, line);
		std::vector<std::vector<std::vector<Int_t>>> temp_ranges(output.nr_dets, std::vector<std::vector<Int_t>>(output.nr_calibs, std::vector<Int_t>(2)));

		try {
			for (Int_t i = 0; i < output.nr_calibs; i++){
				std::stringstream(line.substr(0, line.find(delim_short))) >> temp_double0;
				output.calib_energies.push_back(temp_double0);
				line.erase(0, line.find(delim_short) + delim_short.length());
			}

			std::getline(infile, line);
			line.erase(line.size() - 1, line.size());

			for (Int_t i = 0; i < output.nr_dets; i++){
				temp_string0 = line.substr(0, line.find(delim_long));

				for (Int_t j = 0; j < output.nr_calibs; j++){
					if (j == 0){
						temp_string1 = temp_string0.substr(temp_string0.find(minor_bracket0), temp_string0.find(minor_bracket1));
					}
					else{
						temp_string1 = temp_string0.substr(temp_string0.find(minor_bracket0), temp_string0.find(minor_bracket1)+1);
					}

					temp_string1.erase(0, 1);													// Get rid of [
					temp_string1.erase(temp_string1.length()-1, temp_string1.length());			// Get rid of ]
					std::stringstream(temp_string1.substr(0, temp_string1.find(delim_short))) >> temp_int0;
					temp_string1.erase(0, temp_string1.find(delim_short) + delim_short.length());
					std::stringstream(temp_string1) >> temp_int1;
					temp_ranges[i][j][0] = temp_int0;
					temp_ranges[i][j][1] = temp_int1;
					temp_string0.erase(0, temp_string0.find(minor_bracket1) + minor_bracket1.length() + delim_short.length());
				}
				line.erase(0, line.find(delim_long) + delim_long.length());
			}
			output.calib_ranges = temp_ranges;
		}
		catch(...){
			std::cerr << "ERROR: Number of calibrations not appropriate.";
			exit(0);
		}

		std::getline(infile, line);
		std::getline(infile, line);
		
		try{
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.half_life;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.ratio_to_activity;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.E1;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.E2;
			line.erase(0, line.find(delim_short) + delim_short.length());
		}
		catch(...){
			std::cerr << "ERROR: Number of inputs on isotope specific line is not appropriate";
			exit(0);
		}

		

		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());

		try{
			for (Int_t i = 0; i < output.nr_dets; i++){
				temp_string0 = line.substr(0, line.find(delim_long));
				temp_string0.erase(0, 1);
				temp_string0.erase(temp_string0.length()-1, temp_string0.length());

				std::stringstream(temp_string0.substr(0, temp_string0.find(delim_short))) >> temp_int0;
				temp_string0.erase(0, temp_string0.find(delim_short) + delim_short.length());
				std::stringstream(temp_string0) >> temp_int1;

				output.hwidths.push_back({temp_int0, temp_int1});
				line.erase(0, line.find(major_bracket1) + major_bracket1.length() + delim_long.length());
			}
		}
		catch(...){
			std::cerr << "ERROR: Number of widths not appropriate, only provide the coincidence energies to be used for the activity calculation (can be different for each detector).";
			exit(0);
		}

		std::getline(infile, line);
		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		try{
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.datatype_chain;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.datatype_tree;
			line.erase(0, line.find(delim_short) + delim_short.length());
		}
		catch(...){
			std::cerr << "ERROR: Number of datatypes is not appropriate.";
			exit(0);
		}

		std::getline(infile, line);
		try{
			std::stringstream(line.substr(0, line.find(delim_short))) >> temp_double0;
			output.coincidence_window.push_back(temp_double0);
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> temp_double0;
			output.coincidence_window.push_back(temp_double0);
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line) >> temp_double0;
			output.coincidence_window.push_back(temp_double0);
		}
		catch(...){
			std::cerr << "ERROR: Number of coincidence parameters is not appropriate, give broad range (for coincidence creation), negative window (for analysis coincidence window), and positive window (for analysis coincidence window).";
			exit(0);
		}

		std::getline(infile, line);
		std::stringstream(line.substr(0, line.find(delim_short))) >> output.hist_bins;
		std::getline(infile, line);

		try{
			for (Int_t i = 0; i < output.nr_dirs; i++){
				std::stringstream(line.substr(0, line.find(delim_short))) >> temp_int0;
				line.erase(0, line.find(delim_short) + delim_short.length());
				output.points_per_batch.push_back(temp_int0);
			}
		}
		catch(...){
			std::cerr << "ERROR: Line 20 not appropriate, it takes integers for the histogram channels and number of data points per batch.";
			exit(0);
		}

		std::getline(infile, line);
		std::getline(infile, line);

		try{
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.month_0;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.day_0;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.hour_0;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line) >> output.minute_0;
		}
		catch(...){
			std::cerr << "ERROR: Line 20 not appropriate, it takes integers for the histogram channels and number of data points per batch.";
			exit(0);
		}

		std::getline(infile, line);
		line.erase(line.size() - 1, line.size());
		std::vector<Double_t> temp_intvec(output.nr_dirs);

		try{
			for (Int_t i = 0; i < 4; i++){
				temp_string0 = line.substr(0, line.find(delim_long));
				temp_string0.erase(0, 1);													// Get rid of {
				temp_string0.erase(temp_string0.length()-1, temp_string0.length());			// Get rid of }

				for (Int_t j = 0; j < output.nr_dirs; j++){
					std::stringstream(temp_string0.substr(0, line.find(delim_long))) >> temp_intvec[j];
					temp_string0.erase(0, temp_string0.find(delim_short) + delim_short.length());
				}

				if (i == 0) output.months = temp_intvec;
				if (i == 1) output.days = temp_intvec;
				if (i == 2) output.hours = temp_intvec;
				if (i == 3) output.minutes = temp_intvec;
				line.erase(0, line.find(delim_long) + delim_long.length());
			}
		}
		catch(...){
			std::cerr << "ERROR: Start times per batch input is not appropriate";
			exit(0);
		}

		std::getline(infile, line);
		std::getline(infile, line);

		try{
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.process_raw;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line.substr(0, line.find(delim_short))) >> output.fit_expo;
			line.erase(0, line.find(delim_short) + delim_short.length());
			std::stringstream(line) >> output.averaging_method;
		}
		catch(...){
			std::cerr << "ERROR: Analysis settings are not appropriate";
		}
	} 
	catch(...){
		std::cerr << "ERROR: Dat file not appropriate." << std::endl;
		exit(0);
	}
	infile.close();

	return output;
}


#endif
