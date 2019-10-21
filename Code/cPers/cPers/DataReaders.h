#ifndef INCLUDED_INPUT_READER_H
#define INCLUDED_INPUT_READER_H

struct InputFileInfo
{
	bool binary;
	int dimension;
	string input_path;

        bool from_python;

	explicit InputFileInfo(const int dim )
        {
            from_python = true;
            dimension = dim;
	    cout << "dimension: " << dimension << endl;		
        }

	explicit InputFileInfo(const string &input_file)
	{
                from_python = false;

		input_path = input_file;

		if (input_path.find(".txt") != string::npos || input_path.find(".cub") != string::npos)
		{
			binary = false;
			fstream f(input_path.c_str());
			string s;
			getline(f, s);
			stringstream ss(s);
			int cnt = 0;
			for(string x; ss >> x; cnt++);

			dimension = cnt;
		}
		else {
			binary = true;
			fstream f(input_path.c_str(), ios::in | ios::binary);

			//short d;
			//f.read(reinterpret_cast<char*>(&d), sizeof(short));
			int d;
			f.read(reinterpret_cast<char*>(&d), sizeof(int));

//			if (d < 8)
//			{
//				cout << "first number in raw file was " << d << " assuming it's the dimension" << endl;
//				dimension = d;
//			}
//			else {
			assert( d<8 );
			dimension = d;
//			}

			cout << "guessed dimension: " << dimension << endl;		
		}
	}
};


template<int dim, typename t>
struct AbstractDataReader
{
	virtual void read(const string &file_name, blitz::Array<t, dim> &arr) = 0;	
};


template<int dim, typename t>
struct TextDataReader : public AbstractDataReader<dim, t>
{
	
	void read(const string &file_name, blitz::Array<t, dim> &arr)
	{
		fstream str(file_name.c_str());
		cout << "reading " << dim << "-dimensional datafile" << endl;
		blitz::TinyVector<int, dim> dims;
		cout << "DATA Dims: ";
		for (int i = 0; i < dim; i++){
			str >> dims[i];
			cout << dims[i] << " ";
		}
		cout << endl;

		cout << "found dimension: " << dims << endl;

		arr.resize(dims);

		for (typename blitz::Array<t,dim>::iterator it = arr.begin(), end = arr.end(); it != end; ++it)
		{
			str >> *it;			
		}

		cout << "read the input" << endl;
	}
};

template<int dim, typename t>
struct RawDataReader : public AbstractDataReader<dim, t>
{
	// template<typename stream_t>
	void read(const string &file_name, blitz::Array<t, dim> &arr)
	{
		std::ifstream f(file_name.c_str(), std::ios::binary | std::ios::in);	
		
		typedef unsigned short ElemT;
		//typedef unsigned short HeaderElemT;		
		typedef unsigned int HeaderElemT;		

		int d;
		f.read(reinterpret_cast<char*>(&d), sizeof(int));
		
		blitz::TinyVector<HeaderElemT, dim> cnt;

		f.read(reinterpret_cast<char*>(cnt.data()), sizeof(HeaderElemT) * dim);		

		blitz::Array<ElemT, dim> a(cnt);	
		f.read(reinterpret_cast<char*>(a.data()), sizeof(ElemT)*a.size());
		
		arr.resize(cnt);
		copy(a.begin(), a.end(), arr.begin());					
	}
};

template<int dim, typename t>
struct PythonDataReader
{
	// template<typename stream_t>
	void read(const string &file_name, blitz::Array<t, dim> &arr, std::vector<int> dims, std::vector<double> f)
	{
		typedef double ElemT;
		typedef int HeaderElemT;		

		blitz::TinyVector<HeaderElemT, dim> cnt;
                for(int i = 0; i < dim; ++i)
                    cnt(i) = (HeaderElemT) dims[i];


		blitz::Array<ElemT, dim> a(cnt);	

                int idx = 0;
                for(typename blitz::Array<ElemT, dim>::iterator myiter = a.begin(); myiter != a.end(); ++myiter){
                    assert(idx < f.size());
                    * myiter = f[idx ++];
                }
		
		arr.resize(cnt);
		copy(a.begin(), a.end(), arr.begin());					
//                 if(dim == 3){
//                 {
//                 string output_fname = "debug_tmp_3_5.txt";
// 	        fstream output_filestr(output_fname, fstream::out | fstream::trunc);
//                 int k = 5;
//                 for(int i = 0; i < dims[0]; ++i){
//                     for(int j = 0; j < dims[1]; ++j)
//                         output_filestr << arr(i, j, k) << " ";
//                     output_filestr << endl;
//                 }
//                 output_filestr.close();
//                 }
//                 {
//                 string output_fname = "debug_tmp_3_13.txt";
// 	        fstream output_filestr(output_fname, fstream::out | fstream::trunc);
//                 int k = 13;
//                 for(int i = 0; i < dims[0]; ++i){
//                     for(int j = 0; j < dims[1]; ++j)
//                         output_filestr << arr(i, j, k) << " ";
//                     output_filestr << endl;
//                 }
//                 output_filestr.close();
//                 }
//                 {
//                 string output_fname = "debug_tmp_1_25.txt";
// 	        fstream output_filestr(output_fname, fstream::out | fstream::trunc);
//                 int k = 25;
//                 for(int i = 0; i < dims[1]; ++i){
//                     for(int j = 0; j < dims[2]; ++j)
//                         output_filestr << arr(k, i, j) << " ";
//                     output_filestr << endl;
//                 }
//                 output_filestr.close();
//                 }
//                 }else if(dim == 2){
//                 string output_fname = "debug_tmp.txt";
// 	        fstream output_filestr(output_fname, fstream::out | fstream::trunc);
//                 for(int i = 0; i < dims[0]; ++i){
//                     for(int j = 0; j < dims[1]; ++j)
//                         output_filestr << arr(i, j) << " ";
//                     output_filestr << endl;
//                 }
//                 output_filestr.close();
//  
//                 }
	}
};

#endif
