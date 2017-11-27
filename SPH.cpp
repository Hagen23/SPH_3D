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
	// kernel = 0.02;
	kernel = 1.5;
	// mass = 0.01f;
	mass = 0.5f;

	Max_Number_Paticles = 50000;
	Number_Particles = 0;

	// World_Size = Vector3(1.0f, 1.0f, 1.0f);
	World_Size = Vector3(64.0f, 64.0f, 64.0f);

	Cell_Size = 1.0f; //1.f/64.f*2;						// cell size = kernel or h
	Grid_Size = World_Size / Cell_Size;
	Grid_Size.x = 64;//(int)Grid_Size.x;
	Grid_Size.y = 64;//(int)Grid_Size.y;
	Grid_Size.z = 64;//(int)Grid_Size.z;

	Number_Cells = (int)Grid_Size.x * (int)Grid_Size.y * (int)Grid_Size.z;

	Gravity = Vector3(0.0f, -9.8f, 0.0f);
	// K = 1.5f;
	K = 10.f;
	// Stand_Density = 1000.0f;
	Stand_Density = 0.2f;
	max_vel = Vector3(3.0f, 3.0f, 3.0f);

	/// Time step is calculated as in 2016 - Divergence-Free SPH for Incompressible and Viscous Fluids. 
	/// Then we adapt the time step size according to the Courant-Friedrich-Levy (CFL) condition [6] ∆t ≤ 0.4 * d / (||vmax||)
	// Time_Delta = 0.4 * kernel / sqrt(max_vel.getNormSquared());
	Time_Delta = 0.005;
	Wall_Hit = -1.0f;
	// mu = 80.0f;
	mu = 0.5;

	Particles = new Particle[Max_Number_Paticles];
	Cells = new Cell[Number_Cells];

	Poly6_constant = 315.0f/(64.0f * PI * pow(kernel, 9));
	Spiky_constant = 45.0f/(PI * pow(kernel, 6));

	total_time_steps = 0;
	hash_d = duration_d();
	density_pressure_d = duration_d();
	force_d = duration_d();
	update_d = duration_d();

	cout<<"SPHSystem"<<endl;
	cout<<"Grid_Size_X : "<<Grid_Size.x<<endl;
	cout<<"Grid_Size_Y : "<<Grid_Size.y<<endl;
	cout<<"Cell Number : "<<Number_Cells<<endl;
	cout<<"Cell size : "<<Cell_Size<<endl;
	cout<<"Time Delta : "<<Time_Delta<<endl;
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

	// for(float k = World_Size.z * 0.3f; k < World_Size.z * 0.7f; k += 0.016f)
	// for(float j = World_Size.y * 0.3f; j < World_Size.y * 0.7f; j += 0.016f)
	// for(float i = World_Size.x * 0.3f; i < World_Size.x * 0.7f; i += 0.016f)
			
	for (int k = 0; k < 16; k++)
	for (int j = 0; j < 32; j++)
	for (int i = 0; i < 32; i++)
	{
			pos = Vector3(i, j, k);
			Init_Particle(pos, vel);
	}
	cout<<"Number of Paticles : "<<Number_Particles<<endl;
}

void SPH::Init_Particle(Vector3 pos, Vector3 vel)
{
	if(Number_Particles + 1 > Max_Number_Paticles)
		return;

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
	cellpos.x = floor(cellpos.x);
	cellpos.y = floor(cellpos.y);
	cellpos.z = floor(cellpos.z);
	return cellpos;
}

int SPH::Calculate_Cell_Hash(Vector3 pos)
{
	// if((pos.x < 0)||(pos.x >= Grid_Size.x)||(pos.y < 0)||(pos.y >= Grid_Size.y)||
	// (pos.z < 0)||(pos.z >= Grid_Size.z))
	// 	return -1;

	int x = (int)pos.x & (int)((Grid_Size.x-1));  // wrap grid, assumes size is power of 2
    int y = (int)pos.y & (int)((Grid_Size.y-1));
    int z = (int)pos.z & (int)((Grid_Size.z-1));
	
	int hash = x + (int)Grid_Size.x * (y + (int)Grid_Size.y * z);
	// int hash = pos.x + Grid_Size.x * (pos.y + Grid_Size.y * pos.z);

	if(hash > Number_Cells)
		cout<<"Error";
	return hash;
}

/// For density computation
float SPH::Poly6(float r2)
{
	return (r2 <= kernel*kernel) ? Poly6_constant * pow(kernel * kernel - r2, 3) : 0;
	// return (r2 >= 0 && r2 <= kernel*kernel) ? Poly6_constant * pow(kernel * kernel - r2, 3) : 0;
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
		/// Sum of the forces that make up the fluid, Eq.8
		p->acc += Gravity * mass;
		p->acc = p->acc/mass;
		
		//  p->acc += Vector3(10.0f, 0.0f, 0.0f);
	}
}

/// Time integration as in 2016 - Fluid simulation by the SPH Method, a survey.
/// Eq.13 and Eq.14
void SPH::Update_Pos_Vel()
{
	Particle *p;
	
	float displacement = 0.5;

	for(int i=0; i < Number_Particles; i++)
	{
		p = &Particles[i];
		p->vel = p->vel + p->acc*Time_Delta;
		p->pos = p->pos + p->vel*Time_Delta;

		if(p->pos.x < 0.0f)
		{
			p->vel.x = p->vel.x * Wall_Hit;
			p->pos.x = 0.0f + displacement;
		}
		if(p->pos.x >= World_Size.x)
		{
			p->vel.x = p->vel.x * Wall_Hit;
			// p->pos.x = World_Size.x - 0.0001f;
			p->pos.x = World_Size.x - displacement;
		}
		if(p->pos.y < 0.0f)
		{
			p->vel.y = p->vel.y * Wall_Hit;
			p->pos.y = 0.0f+displacement;
		}
		if(p->pos.y >= World_Size.y)
		{
			p->vel.y = p->vel.y * Wall_Hit;
			// p->pos.y = World_Size.y - 0.0001f;
			p->pos.y = World_Size.y - displacement;
		}
		if(p->pos.z < 0.0f)
		{
			p->vel.z = p->vel.z * Wall_Hit;
			p->pos.z = 0.0f + displacement;
		}
		if(p->pos.z >= World_Size.z)
		{
			p->vel.z = p->vel.z * Wall_Hit;
			// p->pos.z = World_Size.z - 0.0001f;
			p->pos.z = World_Size.z - displacement;
		}
	}
}

void SPH::Animation()
{
	tpoint tstart = std::chrono::system_clock::now();
	Hash_Grid();
	hash_d += std::chrono::system_clock::now() - tstart;

	tstart = std::chrono::system_clock::now();
	Compute_Density_SingPressure();
	density_pressure_d += std::chrono::system_clock::now() - tstart;

	tstart = std::chrono::system_clock::now();
	Computer_Force();
	force_d += std::chrono::system_clock::now() - tstart;

	tstart = std::chrono::system_clock::now();
	Update_Pos_Vel();
	update_d += std::chrono::system_clock::now() - tstart;

	total_time_steps++;
}

void SPH::print_report()
{
	cout << "Hash" << " \t\t" << "Density Pressure" << " " << "Force" << " \t\t " << "Update" << endl;
	cout << hash_d.count() / total_time_steps << " \t " << density_pressure_d.count() / total_time_steps << " \t " << force_d.count() / total_time_steps << " \t " << update_d.count() / total_time_steps << endl; 
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