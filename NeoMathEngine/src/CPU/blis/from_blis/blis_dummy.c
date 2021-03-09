//#include <blis.h>

//void bli_abort()
//{
//	fprintf( stderr, "libblis: Aborting.\n" );
//	//raise( SIGABRT );
//	abort();
//}

//void bli_blksz_init
//     (
//       blksz_t* b,
//       dim_t b_s,  dim_t b_d,  dim_t b_c,  dim_t b_z,
//       dim_t be_s, dim_t be_d, dim_t be_c, dim_t be_z
//     )
//{
//	b->v[BLIS_FLOAT]    = b_s;
//	b->v[BLIS_DOUBLE]   = b_d;
//	b->v[BLIS_SCOMPLEX] = b_c;
//	b->v[BLIS_DCOMPLEX] = b_z;

//	b->e[BLIS_FLOAT]    = be_s;
//	b->e[BLIS_DOUBLE]   = be_d;
//	b->e[BLIS_SCOMPLEX] = be_c;
//	b->e[BLIS_DCOMPLEX] = be_z;
//}

//void bli_blksz_init_easy
//     (
//       blksz_t* b,
//       dim_t b_s,  dim_t b_d,  dim_t b_c,  dim_t b_z
//     )
//{
//	b->v[BLIS_FLOAT]    = b->e[BLIS_FLOAT]    = b_s;
//	b->v[BLIS_DOUBLE]   = b->e[BLIS_DOUBLE]   = b_d;
//	b->v[BLIS_SCOMPLEX] = b->e[BLIS_SCOMPLEX] = b_c;
//	b->v[BLIS_DCOMPLEX] = b->e[BLIS_DCOMPLEX] = b_z;
//}

//void bli_cntx_init_haswell_ref( cntx_t* cntx )
//{
////	blksz_t  blkszs[ BLIS_NUM_BLKSZS ];
////	blksz_t  thresh[ BLIS_NUM_THRESH ];
////	func_t*  funcs;
////	mbool_t* mbools;
////	dim_t    i;
////	void**   vfuncs;


//	// -- Clear the context ----------------------------------------------------

//	bli_cntx_clear( cntx );


////	// -- Set blocksizes -------------------------------------------------------

////	//                                          s     d     c     z
////	bli_blksz_init_easy( &blkszs[ BLIS_KR ],    1,    1,    1,    1 );
////	bli_blksz_init_easy( &blkszs[ BLIS_MR ],    4,    4,    4,    4 );
////	bli_blksz_init_easy( &blkszs[ BLIS_NR ],   16,    8,    8,    4 );
////	bli_blksz_init_easy( &blkszs[ BLIS_MC ],  256,  128,  128,   64 );
////	bli_blksz_init_easy( &blkszs[ BLIS_KC ],  256,  256,  256,  256 );
////	bli_blksz_init_easy( &blkszs[ BLIS_NC ], 4096, 4096, 4096, 4096 );
////	bli_blksz_init_easy( &blkszs[ BLIS_M2 ], 1000, 1000, 1000, 1000 );
////	bli_blksz_init_easy( &blkszs[ BLIS_N2 ], 1000, 1000, 1000, 1000 );
////	bli_blksz_init_easy( &blkszs[ BLIS_AF ],    8,    8,    8,    8 );
////	bli_blksz_init_easy( &blkszs[ BLIS_DF ],    6,    6,    6,    6 );
////	bli_blksz_init_easy( &blkszs[ BLIS_XF ],    4,    4,    4,    4 );

////	// Initialize the context with the default blocksize objects and their
////	// multiples.
////	bli_cntx_set_blkszs
////	(
////	  BLIS_NAT, 11,
////	  BLIS_NC, &blkszs[ BLIS_NC ], BLIS_NR,
////	  BLIS_KC, &blkszs[ BLIS_KC ], BLIS_KR,
////	  BLIS_MC, &blkszs[ BLIS_MC ], BLIS_MR,
////	  BLIS_NR, &blkszs[ BLIS_NR ], BLIS_NR,
////	  BLIS_MR, &blkszs[ BLIS_MR ], BLIS_MR,
////	  BLIS_KR, &blkszs[ BLIS_KR ], BLIS_KR,
////	  BLIS_M2, &blkszs[ BLIS_M2 ], BLIS_M2,
////	  BLIS_N2, &blkszs[ BLIS_N2 ], BLIS_N2,
////	  BLIS_AF, &blkszs[ BLIS_AF ], BLIS_AF,
////	  BLIS_DF, &blkszs[ BLIS_DF ], BLIS_DF,
////	  BLIS_XF, &blkszs[ BLIS_XF ], BLIS_XF,
////	  cntx
////	);


////	// -- Set level-3 virtual micro-kernels ------------------------------------

////	funcs = bli_cntx_l3_vir_ukrs_buf( cntx );

////	// NOTE: We set the virtual micro-kernel slots to contain the addresses
////	// of the native micro-kernels. In general, the ukernels in the virtual
////	// ukernel slots are always called, and if the function called happens to
////	// be a virtual micro-kernel, it will then know to find its native
////	// ukernel in the native ukernel slots.
////	gen_func_init( &funcs[ BLIS_GEMM_UKR ],       gemm_ukr_name       );
////	gen_func_init( &funcs[ BLIS_GEMMTRSM_L_UKR ], gemmtrsm_l_ukr_name );
////	gen_func_init( &funcs[ BLIS_GEMMTRSM_U_UKR ], gemmtrsm_u_ukr_name );
////	gen_func_init( &funcs[ BLIS_TRSM_L_UKR ],     trsm_l_ukr_name     );
////	gen_func_init( &funcs[ BLIS_TRSM_U_UKR ],     trsm_u_ukr_name     );


////	// -- Set level-3 native micro-kernels and preferences ---------------------

////	funcs  = bli_cntx_l3_nat_ukrs_buf( cntx );
////	mbools = bli_cntx_l3_nat_ukrs_prefs_buf( cntx );

////	gen_func_init( &funcs[ BLIS_GEMM_UKR ],       gemm_ukr_name       );
////	gen_func_init( &funcs[ BLIS_GEMMTRSM_L_UKR ], gemmtrsm_l_ukr_name );
////	gen_func_init( &funcs[ BLIS_GEMMTRSM_U_UKR ], gemmtrsm_u_ukr_name );
////	gen_func_init( &funcs[ BLIS_TRSM_L_UKR ],     trsm_l_ukr_name     );
////	gen_func_init( &funcs[ BLIS_TRSM_U_UKR ],     trsm_u_ukr_name     );

////	//                                                  s      d      c      z
////	bli_mbool_init( &mbools[ BLIS_GEMM_UKR ],        TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_GEMMTRSM_L_UKR ], FALSE, FALSE, FALSE, FALSE );
////	bli_mbool_init( &mbools[ BLIS_GEMMTRSM_U_UKR ], FALSE, FALSE, FALSE, FALSE );
////	bli_mbool_init( &mbools[ BLIS_TRSM_L_UKR ],     FALSE, FALSE, FALSE, FALSE );
////	bli_mbool_init( &mbools[ BLIS_TRSM_U_UKR ],     FALSE, FALSE, FALSE, FALSE );


////	// -- Set level-3 small/unpacked thresholds --------------------------------

////	// NOTE: The default thresholds are set to zero so that the sup framework
////	// does not activate by default. Note that the semantic meaning of the
////	// thresholds is that the sup code path is executed if a dimension is
////	// strictly less than its corresponding threshold. So actually, the
////	// thresholds specify the minimum dimension size that will still dispatch
////	// the non-sup/large code path. This "strictly less than" behavior was
////	// chosen over "less than or equal to" so that threshold values of 0 would
////	// effectively disable sup (even for matrix dimensions of 0).
////	//                                          s     d     c     z
////	bli_blksz_init_easy( &thresh[ BLIS_MT ],    0,    0,    0,    0 );
////	bli_blksz_init_easy( &thresh[ BLIS_NT ],    0,    0,    0,    0 );
////	bli_blksz_init_easy( &thresh[ BLIS_KT ],    0,    0,    0,    0 );

////	// Initialize the context with the default thresholds.
////	bli_cntx_set_l3_sup_thresh
////	(
////	  3,
////	  BLIS_MT, &thresh[ BLIS_MT ],
////	  BLIS_NT, &thresh[ BLIS_NT ],
////	  BLIS_KT, &thresh[ BLIS_KT ],
////	  cntx
////	);


////	// -- Set level-3 small/unpacked handlers ----------------------------------

////	vfuncs = bli_cntx_l3_sup_handlers_buf( cntx );

////	// Initialize all of the function pointers to NULL;
////	for ( i = 0; i < BLIS_NUM_LEVEL3_OPS; ++i ) vfuncs[ i ] = NULL;

////	// The level-3 sup handlers are oapi-based, so we only set one slot per
////	// operation.

////	// Set the gemm slot to the default gemm sup handler.
////	vfuncs[ BLIS_GEMM ]  = bli_gemmsup_ref;
////	vfuncs[ BLIS_GEMMT ] = bli_gemmtsup_ref;


////	// -- Set level-3 small/unpacked micro-kernels and preferences -------------

////	funcs  = bli_cntx_l3_sup_kers_buf( cntx );
////	mbools = bli_cntx_l3_sup_kers_prefs_buf( cntx );

////	gen_func_init( &funcs[ BLIS_RRR ], gemmsup_rv_ukr_name );
////	gen_func_init( &funcs[ BLIS_RRC ], gemmsup_rv_ukr_name );
////	gen_func_init( &funcs[ BLIS_RCR ], gemmsup_rv_ukr_name );
////	gen_func_init( &funcs[ BLIS_RCC ], gemmsup_rv_ukr_name );
////	gen_func_init( &funcs[ BLIS_CRR ], gemmsup_rv_ukr_name );
////	gen_func_init( &funcs[ BLIS_CRC ], gemmsup_rv_ukr_name );
////	gen_func_init( &funcs[ BLIS_CCR ], gemmsup_rv_ukr_name );
////	gen_func_init( &funcs[ BLIS_CCC ], gemmsup_rv_ukr_name );

////	// Register the general-stride/generic ukernel to the "catch-all" slot
////	// associated with the BLIS_XXX enum value. This slot will be queried if
////	// *any* operand is stored with general stride.
////	gen_func_init( &funcs[ BLIS_XXX ], gemmsup_gx_ukr_name );


////	// Set the l3 sup ukernel storage preferences.
////	//                                       s      d      c      z
////	bli_mbool_init( &mbools[ BLIS_RRR ],  TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_RRC ],  TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_RCR ],  TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_RCC ],  TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_CRR ],  TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_CRC ],  TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_CCR ],  TRUE,  TRUE,  TRUE,  TRUE );
////	bli_mbool_init( &mbools[ BLIS_CCC ],  TRUE,  TRUE,  TRUE,  TRUE );

////	bli_mbool_init( &mbools[ BLIS_XXX ],  TRUE,  TRUE,  TRUE,  TRUE );

////	// -- Set miscellaneous fields ---------------------------------------------

////	bli_cntx_set_method( BLIS_NAT, cntx );

////	bli_cntx_set_schema_a_block( BLIS_PACKED_ROW_PANELS, cntx );
////	bli_cntx_set_schema_b_panel( BLIS_PACKED_COL_PANELS, cntx );
////	bli_cntx_set_schema_c_panel( BLIS_NOT_PACKED,        cntx );
//}

//void bli_cntx_set_blkszs( ind_t method, dim_t n_bs, ... )
//{
//	// This function can be called from the bli_cntx_init_*() function for
//	// a particular architecture if the kernel developer wishes to use
//	// non-default blocksizes. It should be called after
//	// bli_cntx_init_defaults() so that the context begins with default
//	// blocksizes across all datatypes.

//	/* Example prototypes:

//	   void bli_cntx_set_blkszs
//	   (
//		 ind_t   method = BLIS_NAT,
//		 dim_t   n_bs,
//		 bszid_t bs0_id, blksz_t* blksz0, bszid_t bm0_id,
//		 bszid_t bs1_id, blksz_t* blksz1, bszid_t bm1_id,
//		 bszid_t bs2_id, blksz_t* blksz2, bszid_t bm2_id,
//		 ...
//		 cntx_t* cntx
//	   );

//	   void bli_cntx_set_blkszs
//	   (
//		 ind_t   method != BLIS_NAT,
//		 dim_t   n_bs,
//		 bszid_t bs0_id, blksz_t* blksz0, bszid_t bm0_id, dim_t def_scalr0, dim_t max_scalr0,
//		 bszid_t bs1_id, blksz_t* blksz1, bszid_t bm1_id, dim_t def_scalr1, dim_t max_scalr1,
//		 bszid_t bs2_id, blksz_t* blksz2, bszid_t bm2_id, dim_t def_scalr2, dim_t max_scalr2,
//		 ...
//		 cntx_t* cntx
//	   );
//	*/

//	va_list   args;
//	dim_t     i;

//	// Allocate some temporary local arrays.

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bszid_t*  bszids = bli_malloc_intl( n_bs * sizeof( bszid_t  ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	blksz_t** blkszs = bli_malloc_intl( n_bs * sizeof( blksz_t* ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bszid_t*  bmults = bli_malloc_intl( n_bs * sizeof( bszid_t  ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	double*   dsclrs = bli_malloc_intl( n_bs * sizeof( double   ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	double*   msclrs = bli_malloc_intl( n_bs * sizeof( double   ) );

//	// -- Begin variable argument section --

//	// Initialize variable argument environment.
//	va_start( args, n_bs );

//	// Handle native and induced method cases separately.
//	if ( method == BLIS_NAT )
//	{
//		// Process n_bs tuples.
//		for ( i = 0; i < n_bs; ++i )
//		{
//			// Here, we query the variable argument list for:
//			// - the bszid_t of the blocksize we're about to process,
//			// - the address of the blksz_t object,
//			// - the bszid_t of the multiple we need to associate with
//			//   the blksz_t object.
//			bszid_t  bs_id = ( bszid_t  )va_arg( args, bszid_t  );
//			blksz_t* blksz = ( blksz_t* )va_arg( args, blksz_t* );
//			bszid_t  bm_id = ( bszid_t  )va_arg( args, bszid_t  );

//			// Store the values in our temporary arrays.
//			bszids[ i ] = bs_id;
//			blkszs[ i ] = blksz;
//			bmults[ i ] = bm_id;
//		}
//	}
//	else // if induced method execution was indicated
//	{
//		// Process n_bs tuples.
//		for ( i = 0; i < n_bs; ++i )
//		{
//			// Here, we query the variable argument list for:
//			// - the bszid_t of the blocksize we're about to process,
//			// - the address of the blksz_t object,
//			// - the bszid_t of the multiple we  need to associate with
//			//   the blksz_t object,
//			// - the scalars we wish to apply to the real blocksizes to
//			//   come up with the induced complex blocksizes (for default
//			//   and maximum blocksizes).
//			bszid_t  bs_id = ( bszid_t  )va_arg( args, bszid_t  );
//			blksz_t* blksz = ( blksz_t* )va_arg( args, blksz_t* );
//			bszid_t  bm_id = ( bszid_t  )va_arg( args, bszid_t  );
//			double   dsclr = ( double   )va_arg( args, double   );
//			double   msclr = ( double   )va_arg( args, double   );

//			// Store the values in our temporary arrays.
//			bszids[ i ] = bs_id;
//			blkszs[ i ] = blksz;
//			bmults[ i ] = bm_id;
//			dsclrs[ i ] = dsclr;
//			msclrs[ i ] = msclr;
//		}
//	}

//	// The last argument should be the context pointer.
//	cntx_t* cntx = ( cntx_t* )va_arg( args, cntx_t* );

//	// Shutdown variable argument environment and clean up stack.
//	va_end( args );

//	// -- End variable argument section --

//	// Save the execution type into the context.
//	bli_cntx_set_method( method, cntx );

//	// Query the context for the addresses of:
//	// - the blocksize object array
//	// - the blocksize multiple array

//	blksz_t* cntx_blkszs = bli_cntx_blkszs_buf( cntx );
//	bszid_t* cntx_bmults = bli_cntx_bmults_buf( cntx );

//	// Now that we have the context address, we want to copy the values
//	// from the temporary buffers into the corresponding buffers in the
//	// context. Notice that the blksz_t* pointers were saved, rather than
//	// the objects themselves, but we copy the contents of the objects
//	// when copying into the context.

//	// Handle native and induced method cases separately.
//	if ( method == BLIS_NAT )
//	{
//		// Process each blocksize id tuple provided.
//		for ( i = 0; i < n_bs; ++i )
//		{
//			// Read the current blocksize id, blksz_t* pointer, blocksize
//			// multiple id, and blocksize scalar.
//			bszid_t  bs_id = bszids[ i ];
//			bszid_t  bm_id = bmults[ i ];

//			blksz_t* blksz = blkszs[ i ];

//			blksz_t* cntx_blksz = &cntx_blkszs[ bs_id ];

//			// Copy the blksz_t object contents into the appropriate
//			// location within the context's blksz_t array. Do the same
//			// for the blocksize multiple id.
//			//cntx_blkszs[ bs_id ] = *blksz;
//			//bli_blksz_copy( blksz, cntx_blksz );
//			bli_blksz_copy_if_pos( blksz, cntx_blksz );

//			// Copy the blocksize multiple id into the context.
//			cntx_bmults[ bs_id ] = bm_id;
//		}
//	}
//	else
//	{
//		// Process each blocksize id tuple provided.
//		for ( i = 0; i < n_bs; ++i )
//		{
//			// Read the current blocksize id, blksz_t pointer, blocksize
//			// multiple id, and blocksize scalar.
//			bszid_t  bs_id = bszids[ i ];
//			bszid_t  bm_id = bmults[ i ];
//			double   dsclr = dsclrs[ i ];
//			double   msclr = msclrs[ i ];

//			blksz_t* blksz = blkszs[ i ];
//			// NOTE: This is a bug! We need to grab the actual blocksize
//			// multiple, which is not at blkszs[i], but rather somewhere else
//			// in the array. In order to fix this, you probably need to store
//			// the contents of blkszs (and all the other arrays) by bs_id
//			// rather than i in the first loop.
//			blksz_t* bmult = blkszs[ i ];

//			blksz_t* cntx_blksz = &cntx_blkszs[ bs_id ];

//			// Copy the real domain values of the source blksz_t object into
//			// the context, duplicating into the complex domain fields.
//			bli_blksz_copy_dt( BLIS_FLOAT,  blksz, BLIS_FLOAT,    cntx_blksz );
//			bli_blksz_copy_dt( BLIS_DOUBLE, blksz, BLIS_DOUBLE,   cntx_blksz );
//			bli_blksz_copy_dt( BLIS_FLOAT,  blksz, BLIS_SCOMPLEX, cntx_blksz );
//			bli_blksz_copy_dt( BLIS_DOUBLE, blksz, BLIS_DCOMPLEX, cntx_blksz );

//			// If the default blocksize scalar is non-unit, we need to scale
//			// the complex domain default blocksizes.
//			if ( dsclr != 1.0 )
//			{
//				// Scale the complex domain default blocksize values in the
//				// blocksize object.
//				bli_blksz_scale_def( 1, ( dim_t )dsclr, BLIS_SCOMPLEX, cntx_blksz );
//				bli_blksz_scale_def( 1, ( dim_t )dsclr, BLIS_DCOMPLEX, cntx_blksz );

//				// Perform rounding to ensure the newly scaled values are still
//				// multiples of their register blocksize multiples. But only
//				// perform this rounding when the blocksize id is not equal to
//				// the blocksize multiple id (ie: we don't round down scaled
//				// register blocksizes since they are their own multiples).
//				// Also, we skip the rounding for 1m since it should never need
//				// such rounding.
//				if ( bs_id != bm_id && method != BLIS_1M )
//				{
//					// Round the newly-scaled blocksizes down to their multiple.
//					bli_blksz_reduce_def_to( BLIS_FLOAT,  bmult, BLIS_SCOMPLEX, cntx_blksz );
//					bli_blksz_reduce_def_to( BLIS_DOUBLE, bmult, BLIS_DCOMPLEX, cntx_blksz );
//				}
//			}

//			// Similarly, if the maximum blocksize scalar is non-unit, we need
//			// to scale the complex domain maximum blocksizes.
//			if ( msclr != 1.0 )
//			{
//				// Scale the complex domain maximum blocksize values in the
//				// blocksize object.
//				bli_blksz_scale_max( 1, ( dim_t )msclr, BLIS_SCOMPLEX, cntx_blksz );
//				bli_blksz_scale_max( 1, ( dim_t )msclr, BLIS_DCOMPLEX, cntx_blksz );

//				// Perform rounding to ensure the newly scaled values are still
//				// multiples of their register blocksize multiples. But only
//				// perform this rounding when the blocksize id is not equal to
//				// the blocksize multiple id (ie: we don't round down scaled
//				// register blocksizes since they are their own multiples).
//				// Also, we skip the rounding for 1m since it should never need
//				// such rounding.
//				if ( bs_id != bm_id && method != BLIS_1M )
//				{
//					// Round the newly-scaled blocksizes down to their multiple.
//					bli_blksz_reduce_max_to( BLIS_FLOAT,  bmult, BLIS_SCOMPLEX, cntx_blksz );
//					bli_blksz_reduce_max_to( BLIS_DOUBLE, bmult, BLIS_DCOMPLEX, cntx_blksz );
//				}
//			}

//			// Copy the blocksize multiple id into the context.
//			cntx_bmults[ bs_id ] = bm_id;
//		}
//	}

//	// Free the temporary local arrays.

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bli_free_intl( blkszs );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bli_free_intl( bszids );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bli_free_intl( bmults );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bli_free_intl( dsclrs );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bli_free_intl( msclrs );
//}

//void bli_cntx_set_l1f_kers( dim_t n_kers, ... ){ ( void )n_kers; }
//void bli_cntx_set_l1v_kers( dim_t n_kers, ... ){ ( void )n_kers; }

//void bli_cntx_set_l3_nat_ukrs( dim_t n_ukrs, ... )
//{
//	// This function can be called from the bli_cntx_init_*() function for
//	// a particular architecture if the kernel developer wishes to use
//	// non-default level-3 microkernels. It should be called after
//	// bli_cntx_init_defaults() so that the context begins with default
//	// microkernels across all datatypes.

//	/* Example prototypes:

//	   void bli_cntx_set_l3_nat_ukrs
//	   (
//		 dim_t   n_ukrs,
//		 l3ukr_t ukr0_id, num_t dt0, void_fp ukr0_fp, bool pref0,
//		 l3ukr_t ukr1_id, num_t dt1, void_fp ukr1_fp, bool pref1,
//		 l3ukr_t ukr2_id, num_t dt2, void_fp ukr2_fp, bool pref2,
//		 ...
//		 cntx_t* cntx
//	   );
//	*/

//	va_list   args;
//	dim_t     i;

//	// Allocate some temporary local arrays.

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	l3ukr_t* ukr_ids   = bli_malloc_intl( n_ukrs * sizeof( l3ukr_t ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	num_t*   ukr_dts   = bli_malloc_intl( n_ukrs * sizeof( num_t   ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	void_fp* ukr_fps   = bli_malloc_intl( n_ukrs * sizeof( void_fp ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	bool*    ukr_prefs = bli_malloc_intl( n_ukrs * sizeof( bool    ) );

//	// -- Begin variable argument section --

//	// Initialize variable argument environment.
//	va_start( args, n_ukrs );

//	// Process n_ukrs tuples.
//	for ( i = 0; i < n_ukrs; ++i )
//	{
//		// Here, we query the variable argument list for:
//		// - the l3ukr_t of the kernel we're about to process,
//		// - the datatype of the kernel,
//		// - the kernel function pointer, and
//		// - the kernel function storage preference
//		// that we need to store to the context.

//		// NOTE: Though bool_t is no longer used, the following comment is
//		// being kept for historical reasons.
//		// The type that we pass into the va_arg() macro for the ukr
//		// preference matters. Using 'bool_t' may cause breakage on 64-bit
//		// systems that define int as 32 bits and long int and pointers as
//		// 64 bits. The problem is that TRUE or FALSE are defined as 1 and
//		// 0, respectively, and when "passed" into the variadic function
//		// they come with no contextual typecast. Thus, default rules of
//		// argument promotion kick in to treat these integer literals as
//		// being of type int. Thus, we need to let va_arg() treat the TRUE
//		// or FALSE value as an int, even if we cast it to and store it
//		// within a bool_t afterwards.
//		const l3ukr_t  ukr_id   = ( l3ukr_t )va_arg( args, l3ukr_t );
//		const num_t    ukr_dt   = ( num_t   )va_arg( args, num_t   );
//			  void_fp  ukr_fp   = ( void_fp )va_arg( args, void_fp );
//		const bool     ukr_pref = ( bool    )va_arg( args, int     );

//		// Store the values in our temporary arrays.
//		ukr_ids[ i ]   = ukr_id;
//		ukr_dts[ i ]   = ukr_dt;
//		ukr_fps[ i ]   = ukr_fp;
//		ukr_prefs[ i ] = ukr_pref;
//	}

//	// The last argument should be the context pointer.
//	cntx_t* cntx = ( cntx_t* )va_arg( args, cntx_t* );

//	// Shutdown variable argument environment and clean up stack.
//	va_end( args );

//	// -- End variable argument section --

//	// Query the context for the addresses of:
//	// - the l3 virtual ukernel func_t array
//	// - the l3 native ukernel func_t array
//	// - the l3 native ukernel preferences array
//	func_t*  cntx_l3_vir_ukrs       = bli_cntx_l3_vir_ukrs_buf( cntx );
//	func_t*  cntx_l3_nat_ukrs       = bli_cntx_l3_nat_ukrs_buf( cntx );
//	mbool_t* cntx_l3_nat_ukrs_prefs = bli_cntx_l3_nat_ukrs_prefs_buf( cntx );

//	// Now that we have the context address, we want to copy the values
//	// from the temporary buffers into the corresponding buffers in the
//	// context.

//	// Process each blocksize id tuple provided.
//	for ( i = 0; i < n_ukrs; ++i )
//	{
//		// Read the current ukernel id, ukernel datatype, ukernel function
//		// pointer, and ukernel preference.
//		const l3ukr_t ukr_id   = ukr_ids[ i ];
//		const num_t   ukr_dt   = ukr_dts[ i ];
//			  void_fp ukr_fp   = ukr_fps[ i ];
//		const bool    ukr_pref = ukr_prefs[ i ];

//		// Index into the func_t and mbool_t for the current kernel id
//		// being processed.
//		func_t*       vukrs  = &cntx_l3_vir_ukrs[ ukr_id ];
//		func_t*       ukrs   = &cntx_l3_nat_ukrs[ ukr_id ];
//		mbool_t*      prefs  = &cntx_l3_nat_ukrs_prefs[ ukr_id ];

//		// Store the ukernel function pointer and preference values into
//		// the context. Notice that we redundantly store the native
//		// ukernel address in both the native and virtual ukernel slots
//		// in the context. This is standard practice when creating a
//		// native context. (Induced method contexts will overwrite the
//		// virtual function pointer with the address of the appropriate
//		// virtual ukernel.)
//		bli_func_set_dt( ukr_fp, ukr_dt, vukrs );
//		bli_func_set_dt( ukr_fp, ukr_dt, ukrs );
//		bli_mbool_set_dt( ukr_pref, ukr_dt, prefs );
//	}

//	// Free the temporary local arrays.
//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	bli_free_intl( ukr_ids );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	bli_free_intl( ukr_dts );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	bli_free_intl( ukr_fps );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_nat_ukrs(): " );
//	#endif
//	bli_free_intl( ukr_prefs );
//}

//void bli_cntx_set_l3_sup_blkszs( dim_t n_bs, ... )
//{
//	// This function can be called from the bli_cntx_init_*() function for
//	// a particular architecture if the kernel developer wishes to use
//	// non-default l3 sup blocksizes. It should be called after
//	// bli_cntx_init_defaults() so that the context begins with default
//	// blocksizes across all datatypes.

//	/* Example prototypes:

//	   void bli_cntx_set_blkszs
//	   (
//		 dim_t   n_bs,
//		 bszid_t bs0_id, blksz_t* blksz0,
//		 bszid_t bs1_id, blksz_t* blksz1,
//		 bszid_t bs2_id, blksz_t* blksz2,
//		 ...
//		 cntx_t* cntx
//	   );
//	*/

//	va_list   args;
//	dim_t     i;

//	// Allocate some temporary local arrays.
//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bszid_t* bszids = bli_malloc_intl( n_bs * sizeof( bszid_t  ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	blksz_t** blkszs = bli_malloc_intl( n_bs * sizeof( blksz_t* ) );

//	// -- Begin variable argument section --

//	// Initialize variable argument environment.
//	va_start( args, n_bs );

//	// Process n_bs tuples.
//	for ( i = 0; i < n_bs; ++i )
//	{
//		// Here, we query the variable argument list for:
//		// - the bszid_t of the blocksize we're about to process,
//		// - the address of the blksz_t object.
//		bszid_t  bs_id = ( bszid_t  )va_arg( args, bszid_t  );
//		blksz_t* blksz = ( blksz_t* )va_arg( args, blksz_t* );

//		// Store the values in our temporary arrays.
//		bszids[ i ] = bs_id;
//		blkszs[ i ] = blksz;
//	}

//	// The last argument should be the context pointer.
//	cntx_t* cntx = ( cntx_t* )va_arg( args, cntx_t* );

//	// Shutdown variable argument environment and clean up stack.
//	va_end( args );

//	// -- End variable argument section --

//	// Query the context for the addresses of:
//	// - the blocksize object array
//	blksz_t* cntx_l3_sup_blkszs = bli_cntx_l3_sup_blkszs_buf( cntx );

//	// Now that we have the context address, we want to copy the values
//	// from the temporary buffers into the corresponding buffers in the
//	// context. Notice that the blksz_t* pointers were saved, rather than
//	// the objects themselves, but we copy the contents of the objects
//	// when copying into the context.

//	// Process each blocksize id tuple provided.
//	for ( i = 0; i < n_bs; ++i )
//	{
//		// Read the current blocksize id, blksz_t* pointer, blocksize
//		// multiple id, and blocksize scalar.
//		bszid_t  bs_id = bszids[ i ];
//		blksz_t* blksz = blkszs[ i ];

//		blksz_t* cntx_l3_sup_blksz = &cntx_l3_sup_blkszs[ bs_id ];

//		// Copy the blksz_t object contents into the appropriate
//		// location within the context's blksz_t array.
//		//cntx_l3_sup_blkszs[ bs_id ] = *blksz;
//		//bli_blksz_copy( blksz, cntx_l3_sup_blksz );
//		bli_blksz_copy_if_pos( blksz, cntx_l3_sup_blksz );
//	}

//	// Free the temporary local arrays.

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bli_free_intl( blkszs );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_blkszs(): " );
//	#endif
//	bli_free_intl( bszids );
//}

//void bli_cntx_set_l3_sup_thresh( dim_t n_thresh, ... )
//{
//	// This function can be called from the bli_cntx_init_*() function for
//	// a particular architecture if the kernel developer wishes to use
//	// non-default thresholds for small/unpacked matrix handling. It should
//	// be called after bli_cntx_init_defaults() so that the context begins
//	// with default thresholds.

//	/* Example prototypes:

//	   void bli_cntx_set_l3_sup_thresh
//	   (
//		 dim_t      n_thresh,
//		 threshid_t th0_id, blksz_t* blksz0,
//		 threshid_t th1_id, blksz_t* blksz1,
//		 ...
//		 cntx_t* cntx
//	   );bli_cntx_set_l3_sup_thresh

//	*/

//	va_list     args;
//	dim_t       i;

//	// Allocate some temporary local arrays.

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_sup_thresh(): " );
//	#endif
//	threshid_t* threshids = bli_malloc_intl( n_thresh * sizeof( threshid_t  ) );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_sup_thresh(): " );
//	#endif
//	blksz_t**   threshs = bli_malloc_intl( n_thresh * sizeof( blksz_t* ) );

//	// -- Begin variable argument section --

//	// Initialize variable argument environment.
//	va_start( args, n_thresh );

//	// Process n_thresh tuples.
//	for ( i = 0; i < n_thresh; ++i )
//	{
//		// Here, we query the variable argument list for:
//		// - the threshid_t of the threshold we're about to process,
//		// - the address of the blksz_t object,
//		threshid_t th_id  = ( threshid_t )va_arg( args, threshid_t );
//		blksz_t*   thresh = ( blksz_t*   )va_arg( args, blksz_t*   );

//		// Store the values in our temporary arrays.
//		threshids[ i ] = th_id;
//		threshs[ i ]   = thresh;
//	}

//	// The last argument should be the context pointer.
//	cntx_t* cntx = ( cntx_t* )va_arg( args, cntx_t* );

//	// Shutdown variable argument environment and clean up stack.
//	va_end( args );

//	// -- End variable argument section --

//	// Query the context for the addresses of:
//	// - the threshold array
//	blksz_t* cntx_threshs = bli_cntx_l3_sup_thresh_buf( cntx );

//	// Now that we have the context address, we want to copy the values
//	// from the temporary buffers into the corresponding buffers in the
//	// context. Notice that the blksz_t* pointers were saved, rather than
//	// the objects themselves, but we copy the contents of the objects
//	// when copying into the context.

//	// Process each blocksize id tuple provided.
//	for ( i = 0; i < n_thresh; ++i )
//	{
//		// Read the current blocksize id, blksz_t* pointer, blocksize
//		// multiple id, and blocksize scalar.
//		threshid_t th_id  = threshids[ i ];
//		blksz_t*   thresh = threshs[ i ];

//		blksz_t* cntx_thresh = &cntx_threshs[ th_id ];

//		// Copy the blksz_t object contents into the appropriate
//		// location within the context's blksz_t array.
//		//cntx_threshs[ th_id ] = *thresh;
//		//bli_blksz_copy( thresh, cntx_thresh );
//		bli_blksz_copy_if_pos( thresh, cntx_thresh );
//	}

//	// Free the temporary local arrays.

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_sup_thresh(): " );
//	#endif
//	bli_free_intl( threshs );

//	#ifdef BLIS_ENABLE_MEM_TRACING
//	printf( "bli_cntx_set_l3_sup_thresh(): " );
//	#endif
//	bli_free_intl( threshids );
//}

//void bli_damaxv_zen_int(){ bli_abort(); }
//void bli_daxpyf_zen_int_8(){ bli_abort(); }
//void bli_daxpyv_zen_int10(){ bli_abort(); }
//void bli_ddotv_zen_int(){ bli_abort(); }
//void bli_ddotxf_zen_int_8(){ bli_abort(); }
//void bli_ddotxv_zen_int(){ bli_abort(); }
//void bli_dgemmsup_rd_haswell_asm_6x8m(){ bli_abort(); }
//void bli_dgemmsup_rd_haswell_asm_6x8n(){ bli_abort(); }
//void bli_dgemmsup_rv_haswell_asm_6x8m(){ bli_abort(); }
//void bli_dgemmsup_rv_haswell_asm_6x8n(){ bli_abort(); }
//void bli_dgemmtrsm_l_haswell_asm_6x8(){ bli_abort(); }
//void bli_dgemmtrsm_u_haswell_asm_6x8(){ bli_abort(); }
//void bli_dscalv_zen_int10(){ bli_abort(); }

//bool bli_obj_imag_is_zero( obj_t* a ){ bli_abort(); return false; }
//void bli_obj_scalar_detach(){ bli_abort(); }
//int bli_pthread_mutex_destroy( bli_pthread_mutex_t* mutex ){ bli_abort(); }

//int bli_pthread_mutex_init( bli_pthread_mutex_t*           mutex,
//                            const bli_pthread_mutexattr_t* attr){ bli_abort(); }

//int bli_pthread_mutex_lock( bli_pthread_mutex_t* mutex){ bli_abort(); }
//int bli_pthread_mutex_unlock( bli_pthread_mutex_t* mutex){ bli_abort(); }
//void bli_samaxv_zen_int(){ bli_abort(); }
//void bli_saxpyf_zen_int_8(){ bli_abort(); }
//void bli_saxpyv_zen_int10(){ bli_abort(); }
//void bli_sdotv_zen_int(){ bli_abort(); }
//void bli_sdotxf_zen_int_8(){ bli_abort(); }
//void bli_sdotxv_zen_int(){ bli_abort(); }
//void bli_sgemm_haswell_asm_6x16(){ bli_abort(); }
//void bli_sgemmtrsm_l_haswell_asm_6x16(){ bli_abort(); }
//void bli_sgemmtrsm_u_haswell_asm_6x16(){ bli_abort(); }
//void bli_sgemv_ex(){ bli_abort(); }
//void bli_sscalm(){ bli_abort(); }
//void bli_sscalv_zen_int10(){ bli_abort(); }
//void bli_thrcomm_barrier(){ bli_abort(); }
//void* bli_thrcomm_bcast( dim_t inside_id, void* to_send, thrcomm_t* comm ){ bli_abort(); }

//void bli_thread_range_subbli_thread_range_sub
//(
//  thrinfo_t* thread,
//  dim_t      n,
//  dim_t      bf,
//  bool       handle_edge_low,
//  dim_t*     start,
//  dim_t*     end
//){ bli_abort(); }
