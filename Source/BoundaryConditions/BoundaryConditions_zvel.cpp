#include "AMReX_PhysBCFunct.H"
#include <ROMSX_PhysBCFunct.H>
#include <prob_common.H>

using namespace amrex;

//
// dest_arr is the Array4 to be filled
// time is the time at which the data should be filled
// bccomp is the index into both domain_bcs_type_bcr and bc_extdir_vals
//     so this follows the BCVars enum
//
void ROMSXPhysBCFunct::impose_zvel_bcs (const Array4<Real>& dest_arr, const Box& bx, const Box& domain,
                                        const GpuArray<Real,AMREX_SPACEDIM> /*dx*/,
                                        const GpuArray<Real,AMREX_SPACEDIM> /*dxInv*/,
                                        Real /*time*/, int bccomp)
{
    const auto& dom_lo = amrex::lbound(domain);
    const auto& dom_hi = amrex::ubound(domain);

    // Based on BCRec for the domain, we need to make BCRec for this Box
    // bccomp is used as starting index for m_domain_bcs_type
    //      0 is used as starting index for bcrs
    int ncomp = 1;
    Vector<BCRec> bcrs(ncomp);
    amrex::setBC(bx, domain, bccomp, 0, ncomp, m_domain_bcs_type, bcrs);

    // xlo: ori = 0
    // ylo: ori = 1
    // zlo: ori = 2
    // xhi: ori = 3
    // yhi: ori = 4
    // zhi: ori = 5

    //! Based on BCRec for the domain, we need to make BCRec for this Box
    // bccomp is used as starting index for m_domain_bcs_type
    //      0 is used as starting index for bcrs
    amrex::setBC(bx, domain, bccomp, 0, ncomp, m_domain_bcs_type, bcrs);

    amrex::Gpu::DeviceVector<BCRec> bcrs_d(ncomp);
#ifdef AMREX_USE_GPU
    Gpu::htod_memcpy_async(bcrs_d.data(), bcrs.data(), sizeof(BCRec)*ncomp);
#else
    std::memcpy(bcrs_d.data(), bcrs.data(), sizeof(BCRec)*ncomp);
#endif
    const amrex::BCRec* bc_ptr = bcrs_d.data();

    GpuArray<GpuArray<Real, AMREX_SPACEDIM*2>,AMREX_SPACEDIM+NCONS> l_bc_extdir_vals_d;

    for (int i = 0; i < ncomp; i++)
        for (int ori = 0; ori < 2*AMREX_SPACEDIM; ori++)
            l_bc_extdir_vals_d[i][ori] = m_bc_extdir_vals[bccomp+i][ori];

    GeometryData const& geomdata = m_geom.data();
    bool is_periodic_in_x = geomdata.isPeriodic(0);
    bool is_periodic_in_y = geomdata.isPeriodic(1);

    // First do all ext_dir bcs
    if (!is_periodic_in_x)
    {
        Box bx_xlo(bx);  bx_xlo.setBig  (0,dom_lo.x-1);
        Box bx_xhi(bx);  bx_xhi.setSmall(0,dom_hi.x+1);
        ParallelFor(
            bx_xlo, ncomp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) {
                int iflip = dom_lo.x - 1 - i;
                if (bc_ptr[n].lo(0) == ROMSXBCType::ext_dir) {
                    dest_arr(i,j,k) = l_bc_extdir_vals_d[n][0];
                } else if (bc_ptr[n].lo(0) == ROMSXBCType::foextrap) {
                    dest_arr(i,j,k) =  dest_arr(dom_lo.x,j,k);
                } else if (bc_ptr[n].lo(0) == ROMSXBCType::reflect_even) {
                    dest_arr(i,j,k) =  dest_arr(iflip,j,k);
                } else if (bc_ptr[n].lo(0) == ROMSXBCType::reflect_odd) {
                    dest_arr(i,j,k) = -dest_arr(iflip,j,k);
                }
            },
            bx_xhi, ncomp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) {
                int iflip = 2*dom_hi.x + 1 - i;
                if (bc_ptr[n].hi(0) == ROMSXBCType::ext_dir) {
                    dest_arr(i,j,k) = l_bc_extdir_vals_d[n][3];
                } else if (bc_ptr[n].hi(0) == ROMSXBCType::foextrap) {
                    dest_arr(i,j,k) =  dest_arr(dom_hi.x,j,k);
                } else if (bc_ptr[n].hi(0) == ROMSXBCType::reflect_even) {
                    dest_arr(i,j,k) =  dest_arr(iflip,j,k);
                } else if (bc_ptr[n].hi(0) == ROMSXBCType::reflect_odd) {
                    dest_arr(i,j,k) = -dest_arr(iflip,j,k);
                }
            }
        );
    }

    // First do all ext_dir bcs
    if (!is_periodic_in_y)
    {
        Box bx_ylo(bx);  bx_ylo.setBig  (1,dom_lo.y-1);
        Box bx_yhi(bx);  bx_yhi.setSmall(1,dom_hi.y+1);
        ParallelFor(bx_ylo, ncomp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) {
            int jflip = dom_lo.y - 1 - j;
            if (bc_ptr[n].lo(1) == ROMSXBCType::ext_dir) {
                dest_arr(i,j,k) = l_bc_extdir_vals_d[n][1];
            } else if (bc_ptr[n].lo(1) == ROMSXBCType::foextrap) {
                dest_arr(i,j,k) =  dest_arr(i,dom_lo.y,k);
            } else if (bc_ptr[n].lo(1) == ROMSXBCType::reflect_even) {
                dest_arr(i,j,k) =  dest_arr(i,jflip,k);
            } else if (bc_ptr[n].lo(1) == ROMSXBCType::reflect_odd) {
                dest_arr(i,j,k) = -dest_arr(i,jflip,k);
            }
        },
        bx_yhi, ncomp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) {
            int jflip =  2*dom_hi.y + 1 - j;
            if (bc_ptr[n].hi(1) == ROMSXBCType::ext_dir) {
                dest_arr(i,j,k) = l_bc_extdir_vals_d[n][4];
            } else if (bc_ptr[n].hi(1) == ROMSXBCType::foextrap) {
                dest_arr(i,j,k) =  dest_arr(i,dom_hi.y,k);
            } else if (bc_ptr[n].hi(1) == ROMSXBCType::reflect_even) {
                dest_arr(i,j,k) =  dest_arr(i,jflip,k);
            } else if (bc_ptr[n].hi(1) == ROMSXBCType::reflect_odd) {
                dest_arr(i,j,k) = -dest_arr(i,jflip,k);
            }
        });
    }

    {
        Box bx_zlo(bx);  bx_zlo.setBig  (2,dom_lo.z-1);
        Box bx_zhi(bx);  bx_zhi.setSmall(2,dom_hi.z+2);

        // Populate face values on z-boundaries themselves only if EXT_DIR
        Box bx_zlo_face(bx); bx_zlo_face.setSmall(2,dom_lo.z  ); bx_zlo_face.setBig(2,dom_lo.z  );
        Box bx_zhi_face(bx); bx_zhi_face.setSmall(2,dom_hi.z+1); bx_zhi_face.setBig(2,dom_hi.z+1);

        ParallelFor(bx_zlo, [=] AMREX_GPU_DEVICE (int i, int j, int k) {
            int n = 0;
            int kflip = dom_lo.z - k;
            if (bc_ptr[n].lo(2) == ROMSXBCType::ext_dir) {
                dest_arr(i,j,k) = l_bc_extdir_vals_d[n][2];
            } else if (bc_ptr[n].lo(2) == ROMSXBCType::foextrap) {
                dest_arr(i,j,k) =  dest_arr(i,j,dom_lo.z);
            } else if (bc_ptr[n].lo(2) == ROMSXBCType::reflect_even) {
                dest_arr(i,j,k) =  dest_arr(i,j,kflip);
            } else if (bc_ptr[n].lo(2) == ROMSXBCType::reflect_odd) {
                dest_arr(i,j,k) = -dest_arr(i,j,kflip);
            }
        });

        ParallelFor(bx_zlo_face, ncomp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) {
              if (bc_ptr[n].lo(2) == ROMSXBCType::ext_dir) {
                  dest_arr(i,j,k) = l_bc_extdir_vals_d[n][2];
              }
          }
        );

        ParallelFor(
          bx_zhi, ncomp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) {
            int kflip =  2*(dom_hi.z + 1) - k;
            if (bc_ptr[n].hi(5) == ROMSXBCType::ext_dir) {
                dest_arr(i,j,k) = l_bc_extdir_vals_d[n][5];
            } else if (bc_ptr[n].hi(5) == ROMSXBCType::foextrap) {
                dest_arr(i,j,k) =  dest_arr(i,j,dom_hi.z+1);
            } else if (bc_ptr[n].hi(5) == ROMSXBCType::reflect_even) {
                dest_arr(i,j,k) =  dest_arr(i,j,kflip);
            } else if (bc_ptr[n].hi(5) == ROMSXBCType::reflect_odd) {
                dest_arr(i,j,k) = -dest_arr(i,j,kflip);
            }

          },
          bx_zhi_face, ncomp, [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) {
            if (bc_ptr[n].hi(2) == ROMSXBCType::ext_dir) {
                dest_arr(i,j,k) = l_bc_extdir_vals_d[n][5];
            }
          }
        );
    }

    Gpu::streamSynchronize();
}
