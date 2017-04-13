#include <iostream>
#include <fstream>
#include "Packbits.h"

using namespace std;

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		cout << "Errore nel numero di parametri";
		return EXIT_FAILURE;
	}
	ifstream is(argv[2], ios_base::binary);
	is.unsetf(ios_base::skipws);
	ofstream os(argv[3], ios_base::binary);

	if (!is||!os) {
		cout << "Errore nell'apertura dei file";
		return EXIT_FAILURE;
	}
	
	char mode = *argv[1];
	if (mode=='c') {
		/*COMPRESSION*/
		SmartPackbits p(is, os);
		p.compress();

	}
	else if (mode == 'd') {
		/*DECOMPRESSION*/
		SmartPackbits p(is, os);
		p.decompress();
	}
	else {
		os.close();
		is.close();
		return EXIT_FAILURE;
	}
	os.close();
	is.close();
	return EXIT_SUCCESS;
}