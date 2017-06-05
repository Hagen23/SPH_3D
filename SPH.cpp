#ifndef __SPH_H__
#define __SPH_H__

#include "SPH.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>

using namespace std;

SPH::SPH()
{
	kernel = 0.04f;
	mass = 0.02f;

	Max_Number_Paticles = 50000;
	Number_Particles = 0;

	World_Size = Vector3(1.0f, 1.0f, 1.0f);

	Cell_Size = kernel;			// cell size = kernel or h
	Grid_Size = World_Size / Cell_Size;
	Grid_Size.x = (int)Grid_Size.x;
	Grid_Size.y = (int)Grid_Size.y;
	Grid_Size.z = (int)Grid_Size.z;

	Number_Cells = (int)Grid_Size.x * (int)Grid_Size.y * (int)Grid_Size.z;

	Gravity = Vector3(0.0f, -9.8f, 0.0f);
	K = 1.5f;
	Stand_Density = 1000.0f;
	Time_Delta = 0.002f;
	Wall_Hit = -1.0f;
	mu = 200.0f;

	Particles = new Particle[Max_Number_Paticles];
	Cells = new Cell[Number_Cells];

	Poly6_constant = 315.0f/(64.0f * PI * pow(kernel, 9));
	Spiky_constant = 45.0f/(PI * pow(kernel, 6));

	cout<<"SPHSystem"<<endl;
	cout<<"Grid_Size_X : "<<Grid_Size.x<<endl;
	cout<<"Grid_Size_Y : "<<Grid_Size.y<<endl;
	cout<<"Cell Number : "<<Number_Cells<<endl;
}

SPH::~SPH()
{
	delete[] Particles;
	delete[] Cells;
}

void SPH::add_viscosity(float value)
{
	mu += (mu + value) >= 0 ? value : 0;
	cout << "Viscosity: " << mu  << endl;
}

void SPH::Init_Fluid()
{
	Vector3 pos;
	Vector3 vel(0.0f, 0.0f, 0.0f);

	for(float k = World_Size.z * 0.3f; k < World_Size.z * 0.7f; k += kernel * 0.5f)
	for(float j = World_Size.y * 0.3f; j < World_Size.y * 0.7f; j += kernel * 0.5f)
	for(float i = World_Size.x * 0.3f; i < World_Size.x * 0.7f; i += kernel * 0.5f)
	{
			pos = Vector3(i, j, k);
			Init_Particle(pos, vel);
	}
	cout<<"Number of Paticles : "<<Number_Particles<<endl;
}

void SPH::Init_Particle(Vector3 pos, Vector3 vel)
{
	Particle *p = &(Particles[Number_Particles]);
	p->pos = pos;
	p->vel = vel;
	p->acc = Vector3(0.0f, 0.0f, 0.0f);
	p->dens = Stand_Density;
	p->next = NULL;
	Number_Particles++;
}

Vector3 SPH::Calculate_Cell_Position(Vector3 pos)
{
	Vector3 cellpos = pos / Cell_Size;
	cellpos.x = (int)cellpos.x;
	cellpos.y = (int)cellpos.y;
	cellpos.z = (int)cellpos.z;
	return cellpos;
}

int SPH::Calculate_Cell_Hash(Vector3 pos)
{
	if((pos.x < 0)||(pos.x >= Grid_Size.x)||(pos.y < 0)||(pos.y >= Grid_Size.y)||
	(pos.z < 0)||(pos.z >= Grid_Size.z))
		return -1;
	
	int hash = pos.x + Grid_Size.x * (pos.y + Grid_Size.y * pos.z);
	if(hash > Number_Cells)
		cout<<"Error";
	return hash;
}

/// For density computation
float SPH::Poly6(float r2)
{
	return (r2 >= 0 && r2 <= kernel*kernel) ? Poly6_constant * pow(kernel * kernel - r2, 3) : 0;
}

/// For force of pressure computation
float SPH::Spiky(float r)
{
	return (r >= 0 && r <= kernel ) ? -Spiky_constant * (kernel - r) * (kernel - r) : 0;
}

/// For viscosity computation
float SPH::Visco(float r)
{
	return (r >= 0 && r <= kernel ) ? Spiky_constant * (kernel - r) : 0;
}

void SPH::Hash_Grid()
{
	int hash;
	Particle *p;

	for(int i = 0; i < Number_Cells; i++)
		Cells[i].head = NULL;

	for(int i = 0; i < Number_Particles; i ++)
	{
		p = &Particles[i];
		hash = Calculate_Cell_Hash(Calculate_Cell_Position(p->pos));
		if(Cells[hash].head == NULL)
		{
			p->next = NULL;
			Cells[hash].head = p;
		}
		else
		{
			p->next = Cells[hash].head;
			Cells[hash].head = p;
		}
	}
}

void SPH::Compute_Density_SingPressure()
{
	Particle *p;
	Particle *np;
	Vector3 CellPos;
	Vector3 NeighborPos;
	int hash;

	for(int k = 0; k < Number_Particles; k++)
	{
		p = &Particles[k];
		p->dens = 0;
		p->pres = 0;
		CellPos = Calculate_Cell_Position(p->pos);
		
		for(int k = -1; k <= 1; k++)
		for(int j = -1; j <= 1; j++)
		for(int i = -1; i <= 1; i++)
		{
			NeighborPos = CellPos + Vector3(i, j, k);
			hash = Calculate_Cell_Hash(NeighborPos);
			
			if(hash == -1)
				continue;

			np = Cells[hash].head;

			/// Calculates the density, Eq.3
			while(np != NULL)
			{
				Vector3 Distance;
				Distance = p->pos - np->pos;
				float dis2 = (float)Distance.getNormSquared();
				p->dens += mass * Poly6(dis2);
				np = np->next;
			}
		}
		p->dens += mass * Poly6(0.0f);
		/// Calculates the pressure; yields visually better results, Eq.9 from  2014 - SPH in computer graphics
		// p->pres = (pow(p->dens / Stand_Density, 7) - 1) * K;
		/// Calculates the pressure, Eq.12
		p->pres = K * (p->dens - Stand_Density);
	}
}

void SPH::Computer_Force()
{
	Particle *p;
	Particle *np;
	Vector3 CellPos;
	Vector3 NeighborPos;
	int hash;

	for(int k = 0; k < Number_Particles; k++)
	{
		p = &Particles[k];
		p->acc = Vector3(0.0f, 0.0f, 0.0f);
		CellPos = Calculate_Cell_Position(p->pos);

		for(int k = -1; k <= 1; k++)
		for(int j = -1; j <= 1; j++)
		for(int i = -1; i <= 1; i++)
		{
			NeighborPos = CellPos + Vector3(i, j, k);
			hash = Calculate_Cell_Hash(NeighborPos);
			if(hash == -1)
				continue;

			np = Cells[hash].head;

			while(np != NULL)
			{
				Vector3 Distance;
				Distance = p->pos - np->pos;
				float dis2 = (float)Distance.getNormSquared();

				if(dis2 > INF)
				{
					float dis = sqrt(dis2);

					/// Calculates the force of pressure, Eq.10
					float Volume = mass / np->dens;
					float Force_pressure = Volume * (p->pres+np->pres)/2 * Spiky(dis);
					p->acc -= Distance*Force_pressure/dis;

					/// Calculates the relative velocity (vj - vi), and then multiplies it to the mu, volume, and viscosity kernel. Eq.14
					Vector3 RelativeVel = np->vel - p->vel;
					float Force_viscosity = Volume * mu * Visco(dis);
					p->acc += RelativeVel*Force_viscosity;
				}
				np = np->next;
			}
		}
		/// Sum of the forces that make up the fluid, Eq.9
		p->acc = p->acc/p->dens;
		
		p->acc += Gravity;
		//  p->acc += Vector3(10.0f, 0.0f, 0.0f);
	}
}

void SPH::Update_Pos_Vel()
{
	Particle *p;
	for(int i=0; i < Number_Particles; i++)
	{
		p = &Particles[i];
		p->vel = p->vel + p->acc*Time_Delta;
		p->pos = p->pos + p->vel*Time_Delta;

		if(p->pos.x < 0.0f)
		{
			p->vel.x = p->vel.x * Wall_Hit;
			p->pos.x = 0.0f;
		}
		if(p->pos.x >= World_Size.x)
		{
			p->vel.x = p->vel.x * Wall_Hit;
			p->pos.x = World_Size.x - 0.0001f;
		}
		if(p->pos.y < 0.0f)
		{
			p->vel.y = p->vel.y * Wall_Hit;
			p->pos.y = 0.0f;
		}
		if(p->pos.y >= World_Size.y)
		{
			p->vel.y = p->vel.y * Wall_Hit;
			p->pos.y = World_Size.y - 0.0001f;
		}
		if(p->pos.z < 0.0f)
		{
			p->vel.z = p->vel.z * Wall_Hit;
			p->pos.z = 0.0f;
		}
		if(p->pos.z >= World_Size.z)
		{
			p->vel.z = p->vel.z * Wall_Hit;
			p->pos.z = World_Size.z - 0.0001f;
		}
	}
}

void SPH::Animation()
{
	Hash_Grid();
	Compute_Density_SingPressure();
	Computer_Force();
	Update_Pos_Vel();
}

int SPH::Get_Particle_Number()
{
	return Number_Particles;
}

Vector3 SPH::Get_World_Size()
{
	return World_Size;
}

Particle* SPH::Get_Paticles()
{
	return Particles;
}

Cell* SPH::Get_Cells()
{
	return Cells;
}


#endif