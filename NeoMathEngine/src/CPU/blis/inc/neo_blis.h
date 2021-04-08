

#ifndef BLIS_H
#define BLIS_H

#include <cstdint>

typedef int64_t dim_t;
typedef dim_t inc_t;
#define restrict

__attribute__ ((visibility ("default"))) void neo_sgemm_haswell_asm_6x16
     (
       dim_t               k0,
       float*     restrict alpha,
       float*     restrict a,
       float*     restrict b,
       float*     restrict beta,
       float*     restrict c, inc_t rs_c0, inc_t cs_c0,
       void* restrict data,
       void*    restrict cntx
     );

inline void neo_sscopys_mxn( const dim_t m, const dim_t n, float*    restrict x, const inc_t rs_x, const inc_t cs_x,
														   float*    restrict y, const inc_t rs_y, const inc_t cs_y )
{
	for ( dim_t jj = 0; jj < n; ++jj ) {
		for ( dim_t ii = 0; ii < m; ++ii ) {
			*(y + ii*rs_y + jj*cs_y) = *(x + ii*rs_x + jj*cs_x);
		}
	}
}

inline void neo_sssxpbys_mxn( const dim_t m, const dim_t n, float*    restrict x, const inc_t rs_x, const inc_t cs_x,
                                                            float*    restrict beta,
                                                            float*    restrict y, const inc_t rs_y, const inc_t cs_y )
{
	// If beta is zero, overwrite y with x (in case y has infs or NaNs).
	if ( *beta == 0. )
	{
		neo_sscopys_mxn( m, n, x, rs_x, cs_x, y, rs_y, cs_y );
		return;
	}

	{
		for ( dim_t jj = 0; jj < n; ++jj ) {
			for ( dim_t ii = 0; ii < m; ++ii ) {
				float* y1 = y + ii*rs_y + jj*cs_y;
				float* x1 = x + ii*rs_x + jj*cs_x;
				*y1 = *x1 + *beta * ( *y1 );
			}
		}
	}
}

#endif
