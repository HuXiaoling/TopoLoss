#ifndef INCLUDED_INPUT_RUNNER_H
#define INCLUDED_INPUT_RUNNER_H

#include "DataReaders.h"
#include "PersistenceCalcRunner.h"

template<int dim>
struct InputRunner
{
	static void run(InputFileInfo &info, int pers_thd) 
	{		
		blitz::Array<double, dim> phi;
		
		if (info.binary)
		{
			RawDataReader<dim, double> reader;
			reader.read(info.input_path, phi);
		} 
		else 
		{
			TextDataReader<dim, double> reader;
			reader.read(info.input_path, phi);
		}		

		PersistenceCalcRunner<dim> calc; 
		calc.go(&phi, pers_thd, info);
	}

	static std::vector<std::vector< double > > run( InputFileInfo &info, std::vector<int> dims, const std::vector<double> &f, int pers_thd) 
	{		
		blitz::Array<double, dim> phi;
                assert(dims.size() == dim);
		
		PythonDataReader<dim, double> reader;
		reader.read(string(), phi, dims, f );

		PersistenceCalcRunner<dim> calc; 
		return calc.go_python(&phi, pers_thd, info);
	}

};


#endif
