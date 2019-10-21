#ifndef INCLUDED_PERSISTENCE_CALC_RUNNER_H
#define INCLUDED_PERSISTENCE_CALC_RUNNER_H

#include "PersistentPair.h"
#include "PersistenceCalculator.h"

template<int dim>
struct PersistenceCalcRunner
{
	typedef blitz::TinyVector<int, dim> Vertex; 
	typedef vector<PersPair<Vertex> > PersResultContainer;	

	void go(blitz::Array<double, dim> *phi, double pers_thd, const InputFileInfo &info)
	{	
		// int max_pers_pts = BIG_INT; // the maximal number of persistence pairs recorded								

//		blitz::Array<double, dim> persistenceM(phi->ubound()+1); 

		PersistenceCalculator<dim> calc;
		vector<PersResultContainer> res(dim);		

		vector< Vertex > vList;
		calc.calcPersistence(phi, pers_thd,
			//&persistenceM, 
			res, vList, info);		

		{

			TextPersistentPairsSaver<dim> textSaver;

			stringstream output_file;
			output_file << info.input_path << ".pers.txt";
			
			textSaver.savePers(res, output_file.str().c_str());
		}

		{
			BinaryPersistentPairsSaver<dim> binSaver;

			stringstream output_file;
			output_file << info.input_path;
		//	if (info.dimension == 3)
				output_file << ".pers";		
		//	else output_file << ".npers";		
			binSaver.saveOutput(res, vList, output_file.str().c_str());		
/*
			stringstream output_red_file;
			output_red_file << info.input_path;
			if (info.dimension == 3)
				output_red_file << ".red";		
			else output_red_file << ".nred";		
			binSaver.saveReduction(final_reduction_lists, vList, output_red_file.str().c_str());		
*/
		}
	}

        std::vector<std::vector< double > > go_python(blitz::Array<double, dim> *phi, double pers_thd,  const InputFileInfo &info )
	{	
		// int max_pers_pts = BIG_INT; // the maximal number of persistence pairs recorded								

//		blitz::Array<double, dim> persistenceM(phi->ubound()+1); 

		PersistenceCalculator<dim> calc;
		vector<PersResultContainer> res(dim);		

		vector< Vertex > vList;
		calc.calcPersistence(phi, pers_thd,
			//&persistenceM, 
			res, vList, info);		
                int ct = 0;
                for(int d = 0; d < dim; ++d)
                    ct += res[d].size();
                std::vector<std::vector< double > > ret(ct, std::vector<double>( 2*dim + 4 ) );
                int idx = 0;
                for(int d = 0; d < dim; ++d){
                for(int i = 0; i < res[d].size(); ++i){
                    ret[idx][0] = d;
                    ret[idx][1] = res[d][i].birth;
                    ret[idx][2] = res[d][i].death;
                    ret[idx][3] = res[d][i].persistence;
                    for(int k = 0; k < dim; ++k){
                       ret[idx][4+k] = res[d][i].birthV(k); 
                       ret[idx][4+dim+k] = res[d][i].deathV(k); 
                    }
                    idx++;
                }
                }

//                 string output_fname = "debug_persistence.txt";
// 	        fstream output_filestr(output_fname, fstream::out | fstream::trunc);
//                 for(int i = 0; i < ret.size(); ++i){
//                     for(int j = 0; j < ret[0].size(); ++j)
//                         output_filestr << ret[i][j] << " ";
//                     output_filestr << endl;
//                 }
//                 output_filestr.close();
//  
                return ret;

// 		{
// 
// 			TextPersistentPairsSaver<dim> textSaver;
// 
// 			stringstream output_file;
// 			output_file << info.input_path << ".pers.txt";
// 			
// 			textSaver.savePers(res, output_file.str().c_str());
// 		}
// 
// 		{
// 			BinaryPersistentPairsSaver<dim> binSaver;
// 
// 			stringstream output_file;
// 			output_file << info.input_path;
// 		//	if (info.dimension == 3)
// 				output_file << ".pers";		
// 		//	else output_file << ".npers";		
// 			binSaver.saveOutput(res, vList, output_file.str().c_str());		
// /*
// 			stringstream output_red_file;
// 			output_red_file << info.input_path;
// 			if (info.dimension == 3)
// 				output_red_file << ".red";		
// 			else output_red_file << ".nred";		
// 			binSaver.saveReduction(final_reduction_lists, vList, output_red_file.str().c_str());		
// */
// 		}
	}

};

#endif
