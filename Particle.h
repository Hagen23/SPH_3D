#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "Vector3.h"

class Particle
{
public:
	Vector3 pos;		// position
	Vector3 vel;		// velocity
	Vector3 acc;		// acceleration

	float dens;			// density
	float pres;			// pressure

	Particle *next;		// link list
};

class Cell
{
public:
	Particle *head;		// the cell is head of the link list
};

#endif
