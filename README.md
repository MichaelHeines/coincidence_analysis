Read me: Coincidence_creation code		Last edit 29/10/2022		version: 3.0
DISCLAIMER: This code was written for the analysis of an experimental campaign at CERN-MEDICIS, it might not work optimally for other setups.
This is the third iteration of this code including some major updates for multidetector setups.
Attention: The last character of each line in the settings file is ignored (to avoid including linebreaks). Use spaces at the end if no linebreaks are present.

Before compiling the code for the first time, execute the command "chmod +x ./build.sh" once to allow the build file to create the executable

Main program is made such that a Settings.dat file provides all of the inputs. An example file is included
For additional information, contact "michael.heines@kuleuven.be"

////////// Format for Settings files //////////
// Line1: comment line for user --> skip
// Line2: read dir string 				--> General path where the data is stored
// Line3: write dir string 				--> Path where secondary data is written
// Line4: output csv string 			--> Name of output csv file
// Line5: coincidence filename string 	--> General name for coincidence root files
// Line6: single filename string 		--> General name for single event root files
// Line7: Number of directories 		--> Number of measurement batches
// Line8: ls dirs vector of strings		--> List of directories of individual batches (, separated)
// Line9: indices vector of strings 	--> Index in filenames (, separated)
// Line10: -----
// Line11: Nr of calibration energies, Nr of detector channels
// Line12: Detector names (, separated)
// Line13: Calibration energy values		, separated
// Line14: Calibration ranges				; separated for detectors , separated for different energies [min max]
// Line15: -----
// Line16: half-life, ratio_to_activity, E1, E2
// Line17: hwidths	(halfwidths of intervals checked for the activity calculation)						; separated for detectors [hwidth1 hwidth 2]
// Line18: -----
// Line19: dataype_chain, datatype_tree								--> Strings corresponding to save names in original trees 
// Line20: coincidence window [broad, narrow_low, narrow_high]		--> Coincidence window (ps): checked in creating secondary data, lower limit for analysis, upper limit for analysis
// Line21: hist_bins												--> Number of bins in the single events
// Line22: points_per_batch											--> Number of datapoints per batch (comma separated)
// Line23: -----
// Line24: Start Month, day, hour, minute							--> Definition of time 0
// Line25: Months, days, hours, minutes								--> Initial time of measurement batches
// Line26: -----
// Line27: process_raw, fit_expo, method (0 = averaging activities, 1 = summing counts)
// Line28: datatype board, datatype channel, datatype energy, datatype time 			// Future development?
// Line29: -----
////////////////////////////////////////////////



Running the program:
	Have root installed
	Have a g++ compiler installed
	Make sure root is compiled with a c++ version that your compiler can handle

	./build.sh	This will execute the build script that produces the build directory with an exe file
	make sure the writing directory exists before running the code
	
	cd build
	./main.exe directory/settings.dat		execute main program with settings file named settings.dat located in directory
	alternatively run ./build/main.exe directory/settings.dat from main directory

	Turn of the creation of new trees in histograms when no changes have been made since the last run --> Saves A LOT of time



Reading_dat.h:
Read_out structure: designed to provide program with initialization
Reading_dat()
	Inputs: See file
	Outputs: See file

File_chainer.h:
add_files_to_chain()
	Input:	Chain, start string, end string, directory of data
	Output:	Number of files in chain; return as Root Int (and add files to chain)

Time_functions.h:
init_tdiff()
	Input: Directory, string datatype chain, string datatype tree, number of datapoints in batch
	Output: Calculates the start times of the datapoints with respect to the start of the batch (in s); returns them in a vector
get_meantimes()
	Input: Start times within a batch, durations per datapoint, half_life of studied isotope, number of datapoints in a batch
	Output: Meantimes of datapoints within a batch; returned as a vector

Calibration.h:
calibration() 
	Input: 1D histogram, calibration energies, ranges where to look for peaks
	Output: Calibration parameters; returned as a vector
calibrate()
	Input: directory of secondary data, filename, calibration energies, ranges where to look for peaks, detector names
	Output: Calibration parameters; returned as 2d vector
get_channel()
	Input: calibration parameters, energy
	Output: channel corresponding to energy; returned as root double
get_channels()
	Input: calibration parameters, energy
	Output: channels corresponding to energy; returned as vector

Coincidence_functions.h:
coincidence_creation()
	Input: coincidence window for tree creation (ns), number of bins for energy, detector names, filename for coincidences, primary data directory, secondary data directory, number of datapoints in batch, name of chain, name of tree
	Output: / Creates tree with coincidences and all the necesarry information
get_coincrates()
	Input: secondary data directory, filename, detector names, estimated channels energy 0, estimated channels energy 2, half widths for bin integration, lower limit for coincidences, upper limit for coincidences
	Output: Rates of coincidences with all detector combinations; returned as 2d vector

Singles_functions.h:
populate_background_1d()
	Input: 1d energy spectrum histogram, 1d empty background histogram, number of bins
	Output: / fills 1d background histogram
populate_background_2d()
	Input: 2d energy spectrum histogram, 2d empty background histogram, number of bins, number of detectors
	Output: / fills 2d background histogram
create_single_histogram()
	Input: number of bins, detector names, filenames for secondary data, primary data directory, secondary data directory, number of datapoints in the batch, name of chain, name of tree
	Output: / Makes file with single event spectra
bin_integral()
	Input: center estimate, 2d energy histogram, halfwidth, detector number
	Output: Bin integral; returned as root unsigned long64
get_singlerates()
	Input: secondary data directory, filename, estimate channels, halfwidths, detector names
	Output: Rates of single events in all detectors; returned as a vector


Complete_analysis.cpp:	Main program
Provide path+filename with inputs following the format in reading_dat.h
Function that writes out a csv with activities and corresponding mean times.
Program is operational and seems stable.

Future development:
Exponential fitting program --> Currently use python
Choose type of leafs in initial trees
