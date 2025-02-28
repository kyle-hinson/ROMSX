#ifndef _PARTICLE_DATA_H_
#define _PARTICLE_DATA_H_

#include <string>
#include <iostream>

#include <IndexDefines.H>
#include <AMReX_Particles.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>
#include <AMReX_Gpu.H>

#include <TracerPC.H>

/**
 * Container holding many of the particle-related data and options
 */

struct ParticleData {
  public:
    void init_particle_params()
    {
        amrex::ParmParse pp(pp_prefix);

        use_tracer_particles = 0;
        pp.query("use_tracer_particles", use_tracer_particles);
    }

    void init_particles(amrex::ParGDBBase* a_gdb, const amrex::Vector<std::unique_ptr<amrex::MultiFab>>& z_phys_nd)

    {
        // Initialize tracer particles
        if (use_tracer_particles) {
            tracer_particles = std::make_unique<TracerPC>(a_gdb);
            tracer_particles->InitParticles(*z_phys_nd[0]);
            amrex::Print() << "Initialized " << tracer_particles->TotalNumberOfParticles() << " tracer particles." << std::endl;
        }
    }

    void Checkpoint(const std::string& filename)
    {
        if (use_tracer_particles) {
            tracer_particles->Checkpoint(filename, "tracer_particles", true, tracer_particle_varnames);
        }
    }

    void Restart(amrex::ParGDBBase* a_gdb, std::string& restart_file)
    {
        if (use_tracer_particles) {
            // tracer_particles = std::make_unique<TracerPC>(Geom(0), dmap[0], grids[0]);
            tracer_particles = std::make_unique<TracerPC>(a_gdb);
            std::string tracer_file("tracer_particles");
            tracer_particles->Restart(restart_file, tracer_file);
        }
    }

    void advance_particles(int lev, amrex::Real dt_lev,
                           amrex::MultiFab* xvel, amrex::MultiFab* yvel, amrex::MultiFab* zvel,
                           const amrex::Vector<std::unique_ptr<amrex::MultiFab>>& z_phys_nd)
    {
        // Update tracer particles
        if (use_tracer_particles) {
            amrex::Array<amrex::MultiFab const*, AMREX_SPACEDIM> mac_vel = {xvel, yvel, zvel};
            bool use_terrain = (z_phys_nd[lev] != nullptr);
            if (use_terrain) {
                tracer_particles->AdvectWithUmac(mac_vel, lev, dt_lev, use_terrain, *z_phys_nd[lev]);
            } else {
                amrex::MultiFab dummy;
                tracer_particles->AdvectWithUmac(mac_vel, lev, dt_lev, use_terrain, dummy);
            }
        }
    }

    void Redistribute()
    {
        if (use_tracer_particles) {
            tracer_particles->Redistribute();
        }
    }

    std::string pp_prefix {"romsx"};

    int use_tracer_particles;
    std::unique_ptr<TracerPC> tracer_particles;
    amrex::Vector<std::string> tracer_particle_varnames = {AMREX_D_DECL("old_x", "old_y", "old_z")};
};
#endif
