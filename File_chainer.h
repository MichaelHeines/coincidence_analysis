#ifndef filechainer_H
#define filechainer_H

#include <iostream>
#include <vector>

#include <TROOT.h>
#include <TChain.h>
#include <TSystemDirectory.h>


// Add files from directory starting with startstring and ending with endstring to a chain
Int_t add_files_to_chain(TChain* chain, TString startstring, TString endstring, TString directory) {
	const char* dirname = directory;
	printf("Current directory is %s \n", dirname);
	
	std::vector<TString> filenames;
	int added = 0;
	TSystemDirectory dir(dirname, dirname);
	TList* files = dir.GetListOfFiles();

	if (files) {
		TSystemFile* file;
		TString fname;
		TIter next(files);

		while ((file = (TSystemFile*)next())) {
			fname = file->GetName();

			if (!file->IsDirectory() && fname.EndsWith(endstring) && fname.BeginsWith(startstring)) {					//get  list of file names within directory.
				filenames.push_back(fname);
				++added;
			}
		}

		sort(filenames.begin(), filenames.end());
		for (int i = 0; i < added - 1;) {
			if (filenames[i + 1].Sizeof() < filenames[i].Sizeof()) {
				TString temp = filenames[i];
				filenames[i] = filenames[i + 1];
				filenames[i + 1] = temp;
				i = 0;
			}
			i++;
		}

		Int_t i = 0;
		for (std::vector<TString>::iterator it = filenames.begin(); it != filenames.end(); ++it) {

			if (i < 50) {
				//std::cout << *it << std::endl;
				chain->Add(dirname + *it);
			}
			i++;
		}
	}
	printf("%d files have been added \n", added);
	return added;
}


#endif
