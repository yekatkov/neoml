#include <CpuMathEngine.h>
#include <blis.h>

constexpr num_t dt = BLIS_FLOAT;

static void bli_sgemmsup_ref_var2m
(
  conj_t           conja,
  conj_t           conjb,
  dim_t            m,
  dim_t            n,
  dim_t            k,
  float*   restrict alpha,
  float*   restrict a, inc_t rs_a, inc_t cs_a,
  float*   restrict b, inc_t rs_b, inc_t cs_b,
  float*   restrict beta,
  float*   restrict c, inc_t rs_c, inc_t cs_c,
  stor3_t          stor_id,
  cntx_t* restrict cntx
)
{
	/* If m or n is zero, return immediately. */
	assert( bli_zero_dim2( m, n ) );

	/* If k < 1 or alpha is zero, scale by beta and return. */
	if ( k < 1 || bli_seq0( *alpha ) )
	{
		bli_sscalm
		(
		  BLIS_NO_CONJUGATE,
		  0,
		  BLIS_NONUNIT_DIAG,
		  BLIS_DENSE,
		  m, n,
		  beta,
		  c, rs_c, cs_c
		);
		return;
	}

	/* Query the context for various blocksizes. */
	const dim_t NR  = bli_cntx_get_l3_sup_blksz_def_dt( dt, BLIS_NR, cntx );
	const dim_t MR  = bli_cntx_get_l3_sup_blksz_def_dt( dt, BLIS_MR, cntx );
	const dim_t NC  = bli_cntx_get_l3_sup_blksz_def_dt( dt, BLIS_NC, cntx );
	const dim_t MC  = bli_cntx_get_l3_sup_blksz_def_dt( dt, BLIS_MC, cntx );
	const dim_t KC0 = bli_cntx_get_l3_sup_blksz_def_dt( dt, BLIS_KC, cntx );

	dim_t KC;

	if      ( stor_id == BLIS_RRR ||
			  stor_id == BLIS_CCC    ) KC = KC0;
	else if ( stor_id == BLIS_RRC ||
			  stor_id == BLIS_CRC    ) KC = KC0;
	else if ( m <=   MR && n <=   NR ) KC = KC0;
	else if ( m <= 2*MR && n <= 2*NR ) KC = KC0 / 2;
	else if ( m <= 3*MR && n <= 3*NR ) KC = (( KC0 / 3 ) / 4 ) * 4;
	else if ( m <= 4*MR && n <= 4*NR ) KC = KC0 / 4;
	else                               KC = (( KC0 / 5 ) / 4 ) * 4;

	/* Query the maximum blocksize for NR, which implies a maximum blocksize
	   extension for the final iteration. */
	const dim_t NRM = bli_cntx_get_l3_sup_blksz_max_dt( dt, BLIS_NR, cntx );
	const dim_t NRE = NRM - NR;

	/* Compute partitioning step values for each matrix of each loop. */
	const inc_t jcstep_c = cs_c;
	const inc_t jcstep_b = cs_b;

	const inc_t pcstep_a = cs_a;
	const inc_t pcstep_b = rs_b;

	const inc_t icstep_c = rs_c;

	const inc_t jrstep_c = cs_c * NR;

	/* Query the context for the sup microkernel address and cast it to its
	   function pointer type. */
	sgemmsup_ker_ft gemmsup_ker = reinterpret_cast<sgemmsup_ker_ft>( bli_cntx_get_l3_sup_ker_dt( dt, stor_id, cntx ) );

	float* restrict a_00       = a;
	float* restrict b_00       = b;
	float* restrict c_00       = c;
	float* restrict alpha_cast = alpha;
	float* restrict beta_cast  = beta;

	/* Make local copies of beta and one scalars to prevent any unnecessary
	   sharing of cache lines between the cores' caches. */
	float           beta_local = *beta_cast;
	float           one_local  = 1.;

	auxinfo_t       aux;

	/* Compute the JC loop thread range for the current thread. */
	dim_t jc_start = 0, jc_end = n;
	const dim_t n_local = jc_end - jc_start;

	/* Compute number of primary and leftover components of the JC loop. */
	/*const dim_t jc_iter = ( n_local + NC - 1 ) / NC;*/
	const dim_t jc_left =   n_local % NC;

	/* Loop over the n dimension (NC rows/columns at a time). */
	/*for ( dim_t jj = 0; jj < jc_iter; jj += 1 )*/
	for ( dim_t jj = jc_start; jj < jc_end; jj += NC )
	{
		/* Calculate the thread's current JC block dimension. */
		const dim_t nc_cur = ( NC <= jc_end - jj ? NC : jc_left );

		float* restrict b_jc = b_00 + jj * jcstep_b;
		float* restrict c_jc = c_00 + jj * jcstep_c;

		/* Compute the PC loop thread range for the current thread. */
		const dim_t pc_start = 0, pc_end = k;
		const dim_t k_local = k;

		/* Compute number of primary and leftover components of the PC loop. */
		/*const dim_t pc_iter = ( k_local + KC - 1 ) / KC;*/
		const dim_t pc_left =   k_local % KC;

		/* Loop over the k dimension (KC rows/columns at a time). */
		/*for ( dim_t pp = 0; pp < pc_iter; pp += 1 )*/
		for ( dim_t pp = pc_start; pp < pc_end; pp += KC )
		{
			/* Calculate the thread's current PC block dimension. */
			const dim_t kc_cur = ( KC <= pc_end - pp ? KC : pc_left );

			float* restrict a_pc = a_00 + pp * pcstep_a;
			float* restrict b_pc = b_jc + pp * pcstep_b;

			/* Only apply beta to the first iteration of the pc loop. */
			float* restrict beta_use = ( pp == 0 ? &beta_local : &one_local );

			float* b_use = b_pc;
			inc_t  rs_b_use = rs_b, cs_b_use = cs_b, ps_b_use = NC * cs_b;

			/* Alias b_use so that it's clear this is our current block of
			   matrix B. */
			float* restrict b_pc_use = b_use;

			/* Compute the IC loop thread range for the current thread. */
			dim_t ic_start = 0, ic_end = m;
			const dim_t m_local = ic_end - ic_start;

			/* Compute number of primary and leftover components of the IC loop. */
			/*const dim_t ic_iter = ( m_local + MC - 1 ) / MC;*/
			const dim_t ic_left =   m_local % MC;

			/* Loop over the m dimension (MC rows at a time). */
			/*for ( dim_t ii = 0; ii < ic_iter; ii += 1 )*/
			for ( dim_t ii = ic_start; ii < ic_end; ii += MC )
			{
				/* Calculate the thread's current IC block dimension. */
				const dim_t mc_cur = ( MC <= ic_end - ii ? MC : ic_left );

				float* restrict c_ic = c_jc + ii * icstep_c;

				float* a_use = a_pc;
				inc_t  rs_a_use = rs_a, cs_a_use = cs_a, ps_a_use = KC;

				/* Alias a_use so that it's clear this is our current block of
				   matrix A. */
				float* restrict a_ic_use = a_use;

				/* Embed the panel stride of A within the auxinfo_t object. The
				   millikernel will query and use this to iterate through
				   micropanels of A (if needed). */
				bli_auxinfo_set_ps_a( ps_a_use, &aux );


				/* Compute number of primary and leftover components of the JR loop. */
				dim_t jr_iter = ( nc_cur + NR - 1 ) / NR;
				dim_t jr_left =   nc_cur % NR;

				/* An optimization: allow the last jr iteration to contain up to NRE
				   columns of C and B. (If NRE > NR, the mkernel has agreed to handle
				   these cases.) Note that this prevents us from declaring jr_iter and
				   jr_left as const. NOTE: We forgo this optimization when packing B
				   since packing an extended edge case is not yet supported. */
				if ( NRE != 0 && 1 < jr_iter && jr_left != 0 && jr_left <= NRE )
				{
					jr_iter--; jr_left += NR;
				}

				/* Compute the JR loop thread range for the current thread. */
				dim_t jr_start = 0, jr_end = jr_iter;

				/* Loop over the n dimension (NR columns at a time). */
				/*for ( dim_t j = 0; j < jr_iter; j += 1 )*/
				for ( dim_t j = jr_start; j < jr_end; j += 1 )
				{
					const dim_t nr_cur = ( bli_is_not_edge_f( j, jr_iter, jr_left ) ? NR : jr_left );

					float* restrict b_jr = b_pc_use + j * ps_b_use;
					float* restrict c_jr = c_ic     + j * jrstep_c;

					/* Loop over the m dimension (MR rows at a time). */
					{
						/* Invoke the gemmsup millikernel. */
						gemmsup_ker
						(
						  conja,
						  conjb,
						  mc_cur,
						  nr_cur,
						  kc_cur,
						  alpha_cast,
						  a_ic_use, rs_a_use, cs_a_use,
						  b_jr,     rs_b_use, cs_b_use,
						  beta_use,
						  c_jr,     rs_c,     cs_c,
						  &aux,
						  cntx
						);
					}
				}
			}
		}
	}
}

static void bli_gemmsup_ref_var2m
     (
       trans_t trans,
       obj_t*  alpha,
       obj_t*  a,
       obj_t*  b,
       obj_t*  beta,
       obj_t*  c,
       stor3_t eff_id,
       cntx_t* cntx
     )
{
    const num_t    dt        = bli_obj_dt( c );

	const conj_t   conja     = bli_obj_conj_status( a );
	const conj_t   conjb     = bli_obj_conj_status( b );

	const dim_t    m         = bli_obj_length( c );
	const dim_t    n         = bli_obj_width( c );
		  dim_t    k;

	float* restrict buf_a = static_cast<float*>( bli_obj_buffer_at_off( a ) );
		  inc_t    rs_a;
		  inc_t    cs_a;

	float* restrict buf_b = static_cast<float*>( bli_obj_buffer_at_off( b ) );
		  inc_t    rs_b;
		  inc_t    cs_b;

	if ( bli_obj_has_notrans( a ) )
	{
		k     = bli_obj_width( a );

		rs_a  = bli_obj_row_stride( a );
		cs_a  = bli_obj_col_stride( a );
	}
	else // if ( bli_obj_has_trans( a ) )
	{
		// Assign the variables with an implicit transposition.
		k     = bli_obj_length( a );

		rs_a  = bli_obj_col_stride( a );
		cs_a  = bli_obj_row_stride( a );
	}

	if ( bli_obj_has_notrans( b ) )
	{
		rs_b  = bli_obj_row_stride( b );
		cs_b  = bli_obj_col_stride( b );
	}
	else // if ( bli_obj_has_trans( b ) )
	{
		// Assign the variables with an implicit transposition.
		rs_b  = bli_obj_col_stride( b );
		cs_b  = bli_obj_row_stride( b );
	}

	float* restrict buf_c     = static_cast<float*>( bli_obj_buffer_at_off( c ) );
	const inc_t    rs_c      = bli_obj_row_stride( c );
	const inc_t    cs_c      = bli_obj_col_stride( c );

	float* restrict buf_alpha = static_cast<float*>( bli_obj_buffer_for_1x1( dt, alpha ) );
	float* restrict buf_beta  = static_cast<float*>( bli_obj_buffer_for_1x1( dt, beta ) );

	if ( bli_is_notrans( trans ) )
	{
		// Invoke the function.
		bli_sgemmsup_ref_var2m
		(
		  conja,
		  conjb,
		  m,
		  n,
		  k,
		  buf_alpha,
		  buf_a, rs_a, cs_a,
		  buf_b, rs_b, cs_b,
		  buf_beta,
		  buf_c, rs_c, cs_c,
		  eff_id,
		  cntx
		);
	}
	else
	{
		// Invoke the function (transposing the operation).
		bli_sgemmsup_ref_var2m
		(
		  conjb,             // swap the conj values.
		  conja,
		  n,                 // swap the m and n dimensions.
		  m,
		  k,
		  buf_alpha,
		  buf_b, cs_b, rs_b, // swap the positions of A and B.
		  buf_a, cs_a, rs_a, // swap the strides of A and B.
		  buf_beta,
		  buf_c, cs_c, rs_c, // swap the strides of C.
		  bli_stor3_trans( eff_id ), // transpose the stor3_t id.
		  cntx
		);
	}
}

static void sgemm_sup_process(
        obj_t*  alpha,
        obj_t*  a,
        obj_t*  b,
        obj_t*  beta,
        obj_t*  c,
        cntx_t* cntx
      )
{
    const stor3_t stor_id = bli_obj_stor3_from_strides( c, a, b );

	// Don't use the small/unpacked implementation if one of the matrices
	// uses general stride.
	assert( stor_id != BLIS_XXX );

	const bool    is_rrr_rrc_rcr_crr = ( stor_id == BLIS_RRR ||
										 stor_id == BLIS_RRC ||
										 stor_id == BLIS_RCR ||
										 stor_id == BLIS_CRR );
	const bool    is_rcc_crc_ccr_ccc = !is_rrr_rrc_rcr_crr;

	const num_t   dt         = bli_obj_dt( c );
	const bool    row_pref   = bli_cntx_l3_sup_ker_prefers_rows_dt( dt, stor_id, cntx );

	const bool    is_primary = ( row_pref ? is_rrr_rrc_rcr_crr
										  : is_rcc_crc_ccr_ccc );

	const dim_t  m           = bli_obj_length( c );
	const dim_t  n           = bli_obj_width( c );
	const dim_t  MR          = bli_cntx_get_blksz_def_dt( dt, BLIS_MR, cntx );
	const dim_t  NR          = bli_cntx_get_blksz_def_dt( dt, BLIS_NR, cntx );
	bool         use_bp      = TRUE;

	if ( is_primary )
	{
		// This branch handles:
		//  - rrr rrc rcr crr for row-preferential kernels
		//  - rcc crc ccr ccc for column-preferential kernels

		const dim_t mu = m / MR;
		const dim_t nu = n / NR;

		// Decide which algorithm to use (block-panel var2m or panel-block
		// var1n) based on the number of micropanels in the m and n dimensions.
		// Also, recalculate the automatic thread factorization.
		if         ( mu >= nu )    use_bp = TRUE;
		else /* if ( mu <  nu ) */ use_bp = FALSE;

		if ( use_bp )
		{
			#ifdef TRACEVAR
			if ( bli_thread_am_ochief( thread ) )
			printf( "bli_l3_sup_int(): var2m primary\n" );
			#endif
			// block-panel macrokernel; m -> mc, mr; n -> nc, nr: var2()
			bli_gemmsup_ref_var2m( BLIS_NO_TRANSPOSE,
								   alpha, a, b, beta, c,
								   stor_id, cntx );
		}
		else // use_pb
		{
			#ifdef TRACEVAR
			if ( bli_thread_am_ochief( thread ) )
			printf( "bli_l3_sup_int(): var1n primary\n" );
			#endif
			// panel-block macrokernel; m -> nc*,mr; n -> mc*,nr: var1()
///			FIXME:
//			bli_gemmsup_ref_var1n( BLIS_NO_TRANSPOSE,
//			                       alpha, a, b, beta, c,
//			                       stor_id, cntx );
			// *requires nudging of nc up to be a multiple of mr.
		}
	}
	else
	{
		// This branch handles:
		//  - rrr rrc rcr crr for column-preferential kernels
		//  - rcc crc ccr ccc for row-preferential kernels

		const dim_t mu = n / MR; // the n becomes m after a transposition
		const dim_t nu = m / NR; // the m becomes n after a transposition

		// Decide which algorithm to use (block-panel var2m or panel-block
		// var1n) based on the number of micropanels in the m and n dimensions.
		// Also, recalculate the automatic thread factorization.
		if         ( mu >= nu )    use_bp = TRUE;
		else /* if ( mu <  nu ) */ use_bp = FALSE;


		if ( use_bp )
		{
			#ifdef TRACEVAR
			if ( bli_thread_am_ochief( thread ) )
			printf( "bli_l3_sup_int(): var2m non-primary\n" );
			#endif
			// panel-block macrokernel; m -> nc, nr; n -> mc, mr: var2() + trans
			bli_gemmsup_ref_var2m( BLIS_TRANSPOSE,
								   alpha, a, b, beta, c,
								   stor_id, cntx );
		}
		else // use_pb
		{
			#ifdef TRACEVAR
			if ( bli_thread_am_ochief( thread ) )
			printf( "bli_l3_sup_int(): var1n non-primary\n" );
			#endif
			// block-panel macrokernel; m -> mc*,nr; n -> nc*,mr: var1() + trans
///			FIXME:
//			bli_gemmsup_ref_var1n( BLIS_TRANSPOSE,
//			                       alpha, a, b, beta, c,
//			                       stor_id, cntx, rntm, thread );
			// *requires nudging of mc up to be a multiple of nr.
		}
	}
}

void sgemm_sup( bool ATransposed, bool BTransposed, const float* aPtr, size_t rs_a,
 const float* bPtr, size_t rs_b, float* cPtr, size_t rs_c, size_t m, size_t n, size_t k )
{
	const trans_t transa = ATransposed ? BLIS_TRANSPOSE : BLIS_NO_TRANSPOSE;
	const trans_t transb = BTransposed ? BLIS_TRANSPOSE : BLIS_NO_TRANSPOSE;
	float alpha = 1.0;
	float beta = 0.0;
	size_t cs_a = 1;
	size_t cs_b = 1;
	size_t cs_c = 1;

	obj_t       alphao = BLIS_OBJECT_INITIALIZER_1X1;
	obj_t       a     = BLIS_OBJECT_INITIALIZER;
	obj_t       b     = BLIS_OBJECT_INITIALIZER;
	obj_t       betao  = BLIS_OBJECT_INITIALIZER_1X1;
	obj_t       c     = BLIS_OBJECT_INITIALIZER;

	dim_t       m_a, n_a;
	dim_t       m_b, n_b;

	static_assert ( dt == 0, "In blis cntx is an array, so we should always refer to first item of cntx." );
	cntx_t cntx;
	bli_cntx_init_haswell( &cntx );

	bli_set_dims_with_trans( transa, m, k, &m_a, &n_a );
	bli_set_dims_with_trans( transb, k, n, &m_b, &n_b );

	bli_obj_init_finish_1x1( dt, &alpha, &alphao );
	bli_obj_init_finish_1x1( dt, &beta,  &betao  );

	bli_obj_init_finish( dt, m_a, n_a, const_cast<float*>( aPtr ), rs_a, cs_a, &a );
	bli_obj_init_finish( dt, m_b, n_b, const_cast<float*>( bPtr ), rs_b, cs_b, &b );
	bli_obj_init_finish( dt, m,   n,   cPtr, rs_c, cs_c, &c );

	sgemm_sup_process( &alphao, &a, &b, &betao, &c, &cntx );
}
