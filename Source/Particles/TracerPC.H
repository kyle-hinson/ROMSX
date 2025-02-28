#ifndef TRACER_PC_H_
#define TRACER_PC_H_

#include <AMReX_Particles.H>

struct TracerRealIdx
{
    enum {
        old_x = 0,
        old_y,
        old_z,
        ncomps
    };
};

struct TracerIntIdx
{
    enum {
        k = 0,
        ncomps
    };
};

struct TracerAssignor
{
    template <typename P>
    AMREX_GPU_HOST_DEVICE
    amrex::IntVect operator() (P const& p,
                               amrex::GpuArray<amrex::Real,AMREX_SPACEDIM> const& plo,
                               amrex::GpuArray<amrex::Real,AMREX_SPACEDIM> const& dxi,
                               const amrex::Box& domain) const noexcept
    {
        amrex::IntVect iv(
            AMREX_D_DECL(int(amrex::Math::floor((p.pos(0)-plo[0])*dxi[0])),
                         int(amrex::Math::floor((p.pos(1)-plo[1])*dxi[1])),
                         p.idata(0)));
        iv[0] += domain.smallEnd()[0];
        iv[1] += domain.smallEnd()[1];
        return iv;
    }
};

class TracerPC
    : public amrex::ParticleContainer<TracerRealIdx::ncomps, TracerIntIdx::ncomps, 0, 0,
                                      amrex::DefaultAllocator, TracerAssignor>
{

public:

    TracerPC (amrex::ParGDBBase* gdb)
        : amrex::ParticleContainer<TracerRealIdx::ncomps, TracerIntIdx::ncomps, 0, 0,
                                   amrex::DefaultAllocator, TracerAssignor>(gdb)
        {}

    TracerPC (const amrex::Geometry            & geom,
              const amrex::DistributionMapping & dmap,
              const amrex::BoxArray            & ba)
        : amrex::ParticleContainer<TracerRealIdx::ncomps, TracerIntIdx::ncomps, 0, 0,
                                   amrex::DefaultAllocator, TracerAssignor>(geom, dmap, ba)
        {}

    void InitParticles (const amrex::MultiFab& a_z_height);

    void AdvectWithUmac (amrex::Array<amrex::MultiFab const*, AMREX_SPACEDIM> umac,
                         int level, amrex::Real dt, bool use_terrain,
                         amrex::MultiFab& a_z_height);
};

#endif
