#ifndef __SPHSYSTEM_H__
#define __SPHSYSTEM_H__

#include "Vector3.h"
#include "Particle.h"

#define PI 3.141592f
#define INF 1E-12f

class SPH
{
	private:
	
		float kernel;					// kernel or h in kernel function
		float mass;						// mass of particles
		int Max_Number_Paticles;		// initial array for particles
		int Number_Particles;			// paticle number

		Vector3 Grid_Size;				// grid size
		Vector3 World_Size;				// screen size
		float Cell_Size;				// cell size
		int Number_Cells;				// cell number

		Vector3 Gravity;
		float K;						// ideal pressure formulation k; Stiffness of the fluid
										// The lower the value, the stiffer the fluid
		float Stand_Density;			// ideal pressure formulation p0
		float Time_Delta;
		float Wall_Hit;
		float mu;						// Viscosity 

		float Poly6_constant, Spiky_constant, Visco_Constant;

		Particle *Particles;
		Cell *Cells;

	public:
		SPH();
		~SPH();
		void Init_Fluid();									// initialize fluid
		void Init_Particle(Vector3 pos, Vector3 vel);		// initialize particle system
		Vector3 Calculate_Cell_Position(Vector3 pos);		// get cell position
		int Calculate_Cell_Hash(Vector3 pos);				// get cell hash number
		void add_viscosity(float);

		//kernel function
		float Poly6(float r2);		// for density
		float Spiky(float r);		// for pressure
		float Visco(float);			// for viscosity

		void Hash_Grid();

		/// Calculates the density and pressure
		void Compute_Density_SingPressure();
		void Computer_Force();
		void Update_Pos_Vel();
		void Animation();

		int Get_Particle_Number();
		Vector3 Get_World_Size();
		Particle* Get_Paticles();
		Cell* Get_Cells();
};


#endif