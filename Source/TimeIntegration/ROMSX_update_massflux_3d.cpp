#include <ROMSX.H>

using namespace amrex;

/**
 * update_massflux_3d
 *
 * @param[in   ] bx box on which to update
 * @param[in   ] ioff offset in x-direction
 * @param[in   ] joff offset in y-direction
 * @param[in   ] phi  u or v
 * @param[  out] Hphi H-weighted u or v
 * @param[in   ] Hz   weighting
 * @param[in   ] om_v_or_on_u
 * @param[in   ] Dphi_avg1
 * @param[in   ] Dphi_avg2
 * @param[inout] DC
 * @param[inout] FC
 * @param[in   ] nnew component of velocity
 */

void
ROMSX::update_massflux_3d (const Box& bx,
                           const int ioff, const int joff,
                           const Array4<Real      >& phi,
                           const Array4<Real      >& phibar,
                           const Array4<Real      >& Hphi,
                           const Array4<Real const>& Hz,
                           const Array4<Real const>& pm_or_pn,
                           const Array4<Real const>& Dphi_avg1,
                           const Array4<Real const>& Dphi_avg2,
                           const Array4<Real      >& DC,
                           const Array4<Real      >& FC,
                           const int nnew)
{
    auto N = Geom(0).Domain().size()[2]-1; // Number of vertical "levs" aka, NZ

    auto bxD=bx;
    auto bx_g1z=bx;
    bxD.makeSlab(2,0);
    bx_g1z.grow(IntVect(0,0,1));

    // auto geomdata = Geom(0).data();
    // bool NSPeriodic = geomdata.isPeriodic(1);
    // bool EWPeriodic = geomdata.isPeriodic(0);

    //Copied depth of water column calculation from DepthStretchTransform
    //Compute thicknesses of U-boxes DC(i,j,0:N-1), total depth of the water column DC(i,j,-1)

    //This takes advantage of Hz being an extra grow cell size
    ParallelFor(bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
    {
        Real om_v_or_on_u = 2.0_rt / (pm_or_pn(i,j,0) + pm_or_pn(i-ioff,j-joff,0));

        DC(i,j,k) = 0.5_rt * om_v_or_on_u * (Hz(i,j,k)+Hz(i-ioff,j-joff,k));
    });

    ParallelFor(bxD, [=] AMREX_GPU_DEVICE (int i, int j, int )
    {
        for (int k=0; k<=N; k++) {
            DC(i,j,-1) += DC(i,j,k);
        }

        DC(i,j,-1) = 1.0 / DC(i,j,-1);

        for (int k=0; k<=N; k++) {

//          BOUNDARY CONDITIONS
//          if (!(NSPeriodic&&EWPeriodic))
//          {
//              if ( ( ((i<0)||(i>=Mn+1)) && !EWPeriodic ) ||  ( ((j<0)||(j>=Mm+1)) && !NSPeriodic ) )
//              {
//                  phi(i,j,k) -= CF;
//              }
//          }

            Hphi(i,j,k) = 0.5 * (Hphi(i,j,k)+phi(i,j,k,nnew)*DC(i,j,k));
            FC(i,j,0)  += Hphi(i,j,k);
        } // k

        FC(i,j,0) = DC(i,j,-1)*(FC(i,j,0)-Dphi_avg2(i,j,0));
    });

    ParallelFor(bx, [=] AMREX_GPU_DEVICE (int i, int j, int k)
    {
        Hphi(i,j,k) -= DC(i,j,k)*FC(i,j,0);
    });

    ParallelFor(bxD, [=] AMREX_GPU_DEVICE(int i, int j, int )
    {
        phibar(i,j,0,0) = DC(i,j,-1) * Dphi_avg1(i,j,0);
        phibar(i,j,0,1) = phibar(i,j,0,0);
    });

}
