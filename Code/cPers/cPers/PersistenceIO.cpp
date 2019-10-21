#include "PersistenceIO.h"


/************************************************************************/
/* Save persistence values to binary file                               */
/************************************************************************/
int savePersistenceResults(const char *fileName,unsigned int *data, const vector<int> &header, int count)
{	

	FILE *outFile;
	int err;

	outFile = fopen(fileName, "wb" );
	unsigned int dim = header.size();

//	if (dim != 3)
//	{		
		fwrite((void*)&dim,sizeof(unsigned int),1,outFile);
//	}

	fwrite((void*)&header[0],sizeof(unsigned int),dim,outFile);

	int writeCount = fwrite((void *)data,sizeof(unsigned int),count,outFile);

	printf("Writing %d Persistence Values in File %s\n",writeCount,fileName);

	fclose(outFile);

	return 1;
}

int saveReductionResults(const char *fileName,unsigned int *data, const vector<int> &header, int count)
{	
	FILE *outFile;
	int err;

	outFile = fopen(fileName, "wb" );
	int dim = header.size();

//	if (dim != 3)
//	{		
		fwrite((void*)&dim,sizeof(int),1,outFile);
//	}

	fwrite((void*)&header[0],sizeof(int),dim,outFile);

	int writeCount = fwrite((void *)data,sizeof(unsigned int),count,outFile);

	printf("Writing %d Data Reduction Results in File %s\n",writeCount,fileName);

	fclose(outFile);

	return 1;
}

/*
int ReadPersistencyResults(char *fileName)
{
	FILE *stream;

	stream = fopen(fileName,"rb");

	int header[3];

	fread((void *)header,sizeof(int),3,stream);

	int uCount = header[0]+header[1]+header[2];

	int ve = header[0];
	int et = header[1];
	int tt = header[2];

	unsigned short *pData = new unsigned short[uCount*2];

	int readCount = fread((void *)pData,sizeof(unsigned short),uCount*2,stream);

	printf("Vertex Edge pairs: %d\n",ve);
	for (int i=0;i<ve;i++)
		printf("%d %d\n",pData[2*i], pData[2*i+1]);

	printf("Edge Face pairs: %d\n",et);
	for (int i=ve;i<(ve+et);i++)
		printf("%d %d\n",pData[2*i], pData[2*i+1]);

	printf("Face Cube pairs: %d\n",tt);
	for (int i=et;i<(ve+et+tt);i++)
		printf("%d %d\n",pData[2*i], pData[2*i+1]);


	delete pData; pData = NULL;
	fclose(stream);
	return 1;
}
*/
