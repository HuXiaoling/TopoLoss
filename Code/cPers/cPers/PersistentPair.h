#ifndef INCLUDED_PERSISTENT_PAIR_H
#define INCLUDED_PERSISTENT_PAIR_H

template<typename VertexT>
class PersPair{
public:
	int vertDim;
	VertexT birthV; 
	VertexT deathV; 

	double persistence;
	double birth;
	double death;

	//initialize coordinates using the input vertices and persistence
	PersPair(const VertexT &vBirth, const VertexT &vDeath, double pers, double b, double d) :
	vertDim(VertexT::numElements),
	birthV(vBirth), deathV(vDeath),	
		persistence(pers),birth(b),death(d){
			MY_ASSERT(d>=b);
	}

	//when compare, put the pair with bigger persistence to the front
	bool operator<(const PersPair &rhs) const{
		if (this->persistence == rhs.persistence)
			return this->birth < rhs.birth;
		return (this->persistence > rhs.persistence);
	}
	//NOTE this is not sorted according to the robustness
};

#endif
