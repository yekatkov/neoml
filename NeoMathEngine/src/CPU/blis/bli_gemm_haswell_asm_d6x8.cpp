/*

   BLIS
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin
   Copyright (C) 2018 - 2019, Advanced Micro Devices, Inc.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name(s) of the copyright holder(s) nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <neo_blis.h>

#define BLIS_ASM_SYNTAX_ATT
#include "bli_x86_asm_macros.h"

#define SGEMM_INPUT_GS_BETA_NZ \
	vmovlps(mem(rcx), xmm0, xmm0) \
	vmovhps(mem(rcx, rsi, 1), xmm0, xmm0) \
	vmovlps(mem(rcx, rsi, 2), xmm1, xmm1) \
	vmovhps(mem(rcx, r13, 1), xmm1, xmm1) \
	vshufps(imm(0x88), xmm1, xmm0, xmm0) \
	vmovlps(mem(rcx, rsi, 4), xmm2, xmm2) \
	vmovhps(mem(rcx, r15, 1), xmm2, xmm2) \
	/* We can't use vmovhps for loading the last element becauase that
	   might result in reading beyond valid memory. (vmov[lh]psd load
	   pairs of adjacent floats at a time.) So we need to use vmovss
	   instead. But since we're limited to using ymm0 through ymm2
	   (ymm3 contains beta and ymm4 through ymm15 contain the microtile)
	   and due to the way vmovss zeros out all bits above 31, we have to
	   load element 7 before element 6. */ \
	vmovss(mem(rcx, r10, 1), xmm1) \
	vpermilps(imm(0xcf), xmm1, xmm1) \
	vmovlps(mem(rcx, r13, 2), xmm1, xmm1) \
	/*vmovhps(mem(rcx, r10, 1), xmm1, xmm1)*/ \
	vshufps(imm(0x88), xmm1, xmm2, xmm2) \
	vperm2f128(imm(0x20), ymm2, ymm0, ymm0)

#define SGEMM_OUTPUT_GS_BETA_NZ \
	vextractf128(imm(1), ymm0, xmm2) \
	vmovss(xmm0, mem(rcx)) \
	vpermilps(imm(0x39), xmm0, xmm1) \
	vmovss(xmm1, mem(rcx, rsi, 1)) \
	vpermilps(imm(0x39), xmm1, xmm0) \
	vmovss(xmm0, mem(rcx, rsi, 2)) \
	vpermilps(imm(0x39), xmm0, xmm1) \
	vmovss(xmm1, mem(rcx, r13, 1)) \
	vmovss(xmm2, mem(rcx, rsi, 4)) \
	vpermilps(imm(0x39), xmm2, xmm1) \
	vmovss(xmm1, mem(rcx, r15, 1)) \
	vpermilps(imm(0x39), xmm1, xmm2) \
	vmovss(xmm2, mem(rcx, r13, 2)) \
	vpermilps(imm(0x39), xmm2, xmm1) \
	vmovss(xmm1, mem(rcx, r10, 1))

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
     )
{
	//void*   a_next = bli_auxinfo_next_a( data );
	//void*   b_next = bli_auxinfo_next_b( data );

	// Typecast local copies of integers in case dim_t and inc_t are a
	// different size than is expected by load instructions.
	uint64_t k_iter = k0 / 4;
	uint64_t k_left = k0 % 4;
	uint64_t rs_c   = rs_c0;
	uint64_t cs_c   = cs_c0;

	begin_asm()
	
	vzeroall() // zero all xmm/ymm registers.
	
	
	mov(var(a), rax) // load address of a.
	mov(var(b), rbx) // load address of b.
	//mov(%9, r15) // load address of b_next.
	
	add(imm(32*4), rbx)
	 // initialize loop by pre-loading
	vmovaps(mem(rbx, -4*32), ymm0)
	vmovaps(mem(rbx, -3*32), ymm1)
	
	mov(var(c), rcx) // load address of c
	mov(var(rs_c), rdi) // load rs_c
	lea(mem(, rdi, 4), rdi) // rs_c *= sizeof(float)
	
	lea(mem(rdi, rdi, 2), r13) // r13 = 3*rs_c;
	lea(mem(rcx, r13, 1), rdx) // rdx = c + 3*rs_c;
	prefetch(0, mem(rcx, 7*8)) // prefetch c + 0*rs_c
	prefetch(0, mem(rcx, rdi, 1, 7*8)) // prefetch c + 1*rs_c
	prefetch(0, mem(rcx, rdi, 2, 7*8)) // prefetch c + 2*rs_c
	prefetch(0, mem(rdx, 7*8)) // prefetch c + 3*rs_c
	prefetch(0, mem(rdx, rdi, 1, 7*8)) // prefetch c + 4*rs_c
	prefetch(0, mem(rdx, rdi, 2, 7*8)) // prefetch c + 5*rs_c
	
	
	
	
	mov(var(k_iter), rsi) // i = k_iter;
	test(rsi, rsi) // check i via logical AND.
	je(.SCONSIDKLEFT) // if i == 0, jump to code that
	 // contains the k_left loop.
	
	
	label(.SLOOPKITER) // MAIN LOOP
	
	
	 // iteration 0
	prefetch(0, mem(rax, 64*4))
	
	vbroadcastss(mem(rax, 0*4), ymm2)
	vbroadcastss(mem(rax, 1*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm4)
	vfmadd231ps(ymm1, ymm2, ymm5)
	vfmadd231ps(ymm0, ymm3, ymm6)
	vfmadd231ps(ymm1, ymm3, ymm7)
	
	vbroadcastss(mem(rax, 2*4), ymm2)
	vbroadcastss(mem(rax, 3*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm8)
	vfmadd231ps(ymm1, ymm2, ymm9)
	vfmadd231ps(ymm0, ymm3, ymm10)
	vfmadd231ps(ymm1, ymm3, ymm11)
	
	vbroadcastss(mem(rax, 4*4), ymm2)
	vbroadcastss(mem(rax, 5*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm12)
	vfmadd231ps(ymm1, ymm2, ymm13)
	vfmadd231ps(ymm0, ymm3, ymm14)
	vfmadd231ps(ymm1, ymm3, ymm15)
	
	vmovaps(mem(rbx, -2*32), ymm0)
	vmovaps(mem(rbx, -1*32), ymm1)
	
	 // iteration 1
	vbroadcastss(mem(rax, 6*4), ymm2)
	vbroadcastss(mem(rax, 7*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm4)
	vfmadd231ps(ymm1, ymm2, ymm5)
	vfmadd231ps(ymm0, ymm3, ymm6)
	vfmadd231ps(ymm1, ymm3, ymm7)
	
	vbroadcastss(mem(rax, 8*4), ymm2)
	vbroadcastss(mem(rax, 9*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm8)
	vfmadd231ps(ymm1, ymm2, ymm9)
	vfmadd231ps(ymm0, ymm3, ymm10)
	vfmadd231ps(ymm1, ymm3, ymm11)
	
	vbroadcastss(mem(rax, 10*4), ymm2)
	vbroadcastss(mem(rax, 11*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm12)
	vfmadd231ps(ymm1, ymm2, ymm13)
	vfmadd231ps(ymm0, ymm3, ymm14)
	vfmadd231ps(ymm1, ymm3, ymm15)
	
	vmovaps(mem(rbx, 0*32), ymm0)
	vmovaps(mem(rbx, 1*32), ymm1)
	
	 // iteration 2
	prefetch(0, mem(rax, 76*4))
	
	vbroadcastss(mem(rax, 12*4), ymm2)
	vbroadcastss(mem(rax, 13*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm4)
	vfmadd231ps(ymm1, ymm2, ymm5)
	vfmadd231ps(ymm0, ymm3, ymm6)
	vfmadd231ps(ymm1, ymm3, ymm7)
	
	vbroadcastss(mem(rax, 14*4), ymm2)
	vbroadcastss(mem(rax, 15*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm8)
	vfmadd231ps(ymm1, ymm2, ymm9)
	vfmadd231ps(ymm0, ymm3, ymm10)
	vfmadd231ps(ymm1, ymm3, ymm11)
	
	vbroadcastss(mem(rax, 16*4), ymm2)
	vbroadcastss(mem(rax, 17*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm12)
	vfmadd231ps(ymm1, ymm2, ymm13)
	vfmadd231ps(ymm0, ymm3, ymm14)
	vfmadd231ps(ymm1, ymm3, ymm15)
	
	vmovaps(mem(rbx, 2*32), ymm0)
	vmovaps(mem(rbx, 3*32), ymm1)
	
	 // iteration 3
	vbroadcastss(mem(rax, 18*4), ymm2)
	vbroadcastss(mem(rax, 19*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm4)
	vfmadd231ps(ymm1, ymm2, ymm5)
	vfmadd231ps(ymm0, ymm3, ymm6)
	vfmadd231ps(ymm1, ymm3, ymm7)
	
	vbroadcastss(mem(rax, 20*4), ymm2)
	vbroadcastss(mem(rax, 21*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm8)
	vfmadd231ps(ymm1, ymm2, ymm9)
	vfmadd231ps(ymm0, ymm3, ymm10)
	vfmadd231ps(ymm1, ymm3, ymm11)
	
	vbroadcastss(mem(rax, 22*4), ymm2)
	vbroadcastss(mem(rax, 23*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm12)
	vfmadd231ps(ymm1, ymm2, ymm13)
	vfmadd231ps(ymm0, ymm3, ymm14)
	vfmadd231ps(ymm1, ymm3, ymm15)
	
	add(imm(4*6*4), rax) // a += 4*6  (unroll x mr)
	add(imm(4*16*4), rbx) // b += 4*16 (unroll x nr)
	
	vmovaps(mem(rbx, -4*32), ymm0)
	vmovaps(mem(rbx, -3*32), ymm1)
	
	
	dec(rsi) // i -= 1;
	jne(.SLOOPKITER) // iterate again if i != 0.
	
	
	
	
	
	
	label(.SCONSIDKLEFT)
	
	mov(var(k_left), rsi) // i = k_left;
	test(rsi, rsi) // check i via logical AND.
	je(.SPOSTACCUM) // if i == 0, we're done; jump to end.
	 // else, we prepare to enter k_left loop.
	
	
	label(.SLOOPKLEFT) // EDGE LOOP
	
	prefetch(0, mem(rax, 64*4))
	
	vbroadcastss(mem(rax, 0*4), ymm2)
	vbroadcastss(mem(rax, 1*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm4)
	vfmadd231ps(ymm1, ymm2, ymm5)
	vfmadd231ps(ymm0, ymm3, ymm6)
	vfmadd231ps(ymm1, ymm3, ymm7)
	
	vbroadcastss(mem(rax, 2*4), ymm2)
	vbroadcastss(mem(rax, 3*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm8)
	vfmadd231ps(ymm1, ymm2, ymm9)
	vfmadd231ps(ymm0, ymm3, ymm10)
	vfmadd231ps(ymm1, ymm3, ymm11)
	
	vbroadcastss(mem(rax, 4*4), ymm2)
	vbroadcastss(mem(rax, 5*4), ymm3)
	vfmadd231ps(ymm0, ymm2, ymm12)
	vfmadd231ps(ymm1, ymm2, ymm13)
	vfmadd231ps(ymm0, ymm3, ymm14)
	vfmadd231ps(ymm1, ymm3, ymm15)
	
	add(imm(1*6*4), rax) // a += 1*6  (unroll x mr)
	add(imm(1*16*4), rbx) // b += 1*16 (unroll x nr)
	
	vmovaps(mem(rbx, -4*32), ymm0)
	vmovaps(mem(rbx, -3*32), ymm1)
	
	
	dec(rsi) // i -= 1;
	jne(.SLOOPKLEFT) // iterate again if i != 0.
	
	
	
	label(.SPOSTACCUM)
	
	
	
	
	mov(var(alpha), rax) // load address of alpha
	mov(var(beta), rbx) // load address of beta
	vbroadcastss(mem(rax), ymm0) // load alpha and duplicate
	vbroadcastss(mem(rbx), ymm3) // load beta and duplicate
	
	vmulps(ymm0, ymm4, ymm4) // scale by alpha
	vmulps(ymm0, ymm5, ymm5)
	vmulps(ymm0, ymm6, ymm6)
	vmulps(ymm0, ymm7, ymm7)
	vmulps(ymm0, ymm8, ymm8)
	vmulps(ymm0, ymm9, ymm9)
	vmulps(ymm0, ymm10, ymm10)
	vmulps(ymm0, ymm11, ymm11)
	vmulps(ymm0, ymm12, ymm12)
	vmulps(ymm0, ymm13, ymm13)
	vmulps(ymm0, ymm14, ymm14)
	vmulps(ymm0, ymm15, ymm15)
	
	
	
	
	
	
	mov(var(cs_c), rsi) // load cs_c
	lea(mem(, rsi, 4), rsi) // rsi = cs_c * sizeof(float)
	
	lea(mem(rcx, rsi, 8), rdx) // load address of c +  8*cs_c;
	lea(mem(rcx, rdi, 4), r14) // load address of c +  4*rs_c;
	
	lea(mem(rsi, rsi, 2), r13) // r13 = 3*cs_c;
	lea(mem(rsi, rsi, 4), r15) // r15 = 5*cs_c;
	lea(mem(r13, rsi, 4), r10) // r10 = 7*cs_c;
	
	
	 // now avoid loading C if beta == 0
	
	vxorps(ymm0, ymm0, ymm0) // set ymm0 to zero.
	vucomiss(xmm0, xmm3) // set ZF if beta == 0.
	je(.SBETAZERO) // if ZF = 1, jump to beta == 0 case
	
	
	cmp(imm(4), rsi) // set ZF if (4*cs_c) == 4.
	jz(.SROWSTORED) // jump to row storage case
	
	
	cmp(imm(4), rdi) // set ZF if (4*cs_c) == 4.
	jz(.SCOLSTORED) // jump to column storage case
	
	
	
	label(.SGENSTORED)
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm4, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm6, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm8, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm10, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm12, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm14, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	//add(rdi, rcx) // c += rs_c;
	
	
	mov(rdx, rcx) // rcx = c + 8*cs_c
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm5, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm7, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm9, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm11, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm13, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	SGEMM_INPUT_GS_BETA_NZ
	vfmadd213ps(ymm15, ymm3, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	//add(rdi, rcx) // c += rs_c;
	
	
	
	jmp(.SDONE) // jump to end.
	
	
	
	label(.SROWSTORED)
	
	
	vfmadd231ps(mem(rcx), ymm3, ymm4)
	vmovups(ymm4, mem(rcx))
	add(rdi, rcx)
	vfmadd231ps(mem(rdx), ymm3, ymm5)
	vmovups(ymm5, mem(rdx))
	add(rdi, rdx)
	
	
	vfmadd231ps(mem(rcx), ymm3, ymm6)
	vmovups(ymm6, mem(rcx))
	add(rdi, rcx)
	vfmadd231ps(mem(rdx), ymm3, ymm7)
	vmovups(ymm7, mem(rdx))
	add(rdi, rdx)
	
	
	vfmadd231ps(mem(rcx), ymm3, ymm8)
	vmovups(ymm8, mem(rcx))
	add(rdi, rcx)
	vfmadd231ps(mem(rdx), ymm3, ymm9)
	vmovups(ymm9, mem(rdx))
	add(rdi, rdx)
	
	
	vfmadd231ps(mem(rcx), ymm3, ymm10)
	vmovups(ymm10, mem(rcx))
	add(rdi, rcx)
	vfmadd231ps(mem(rdx), ymm3, ymm11)
	vmovups(ymm11, mem(rdx))
	add(rdi, rdx)
	
	
	vfmadd231ps(mem(rcx), ymm3, ymm12)
	vmovups(ymm12, mem(rcx))
	add(rdi, rcx)
	vfmadd231ps(mem(rdx), ymm3, ymm13)
	vmovups(ymm13, mem(rdx))
	add(rdi, rdx)
	
	
	vfmadd231ps(mem(rcx), ymm3, ymm14)
	vmovups(ymm14, mem(rcx))
	//add(rdi, rcx)
	vfmadd231ps(mem(rdx), ymm3, ymm15)
	vmovups(ymm15, mem(rdx))
	//add(rdi, rdx)
	
	
	
	jmp(.SDONE) // jump to end.
	
	
	
	label(.SCOLSTORED)
	
	
	vbroadcastss(mem(rbx), ymm3)
	
	vunpcklps(ymm6, ymm4, ymm0)
	vunpcklps(ymm10, ymm8, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vfmadd231ps(mem(rcx), xmm3, xmm0)
	vfmadd231ps(mem(rcx, rsi, 4), xmm3, xmm2)
	vmovups(xmm0, mem(rcx)) // store ( gamma00..gamma30 )
	vmovups(xmm2, mem(rcx, rsi, 4)) // store ( gamma04..gamma34 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vfmadd231ps(mem(rcx, rsi, 1), xmm3, xmm1)
	vfmadd231ps(mem(rcx, r15, 1), xmm3, xmm2)
	vmovups(xmm1, mem(rcx, rsi, 1)) // store ( gamma01..gamma31 )
	vmovups(xmm2, mem(rcx, r15, 1)) // store ( gamma05..gamma35 )
	
	
	vunpckhps(ymm6, ymm4, ymm0)
	vunpckhps(ymm10, ymm8, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vfmadd231ps(mem(rcx, rsi, 2), xmm3, xmm0)
	vfmadd231ps(mem(rcx, r13, 2), xmm3, xmm2)
	vmovups(xmm0, mem(rcx, rsi, 2)) // store ( gamma02..gamma32 )
	vmovups(xmm2, mem(rcx, r13, 2)) // store ( gamma06..gamma36 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vfmadd231ps(mem(rcx, r13, 1), xmm3, xmm1)
	vfmadd231ps(mem(rcx, r10, 1), xmm3, xmm2)
	vmovups(xmm1, mem(rcx, r13, 1)) // store ( gamma03..gamma33 )
	vmovups(xmm2, mem(rcx, r10, 1)) // store ( gamma07..gamma37 )
	
	lea(mem(rcx, rsi, 8), rcx) // rcx += 8*cs_c
	
	vunpcklps(ymm14, ymm12, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(mem(r14), xmm1, xmm1)
	vmovhpd(mem(r14, rsi, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm0)
	vmovlpd(xmm0, mem(r14)) // store ( gamma40..gamma50 )
	vmovhpd(xmm0, mem(r14, rsi, 1)) // store ( gamma41..gamma51 )
	vmovlpd(mem(r14, rsi, 4), xmm1, xmm1)
	vmovhpd(mem(r14, r15, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm2)
	vmovlpd(xmm2, mem(r14, rsi, 4)) // store ( gamma44..gamma54 )
	vmovhpd(xmm2, mem(r14, r15, 1)) // store ( gamma45..gamma55 )
	
	vunpckhps(ymm14, ymm12, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(mem(r14, rsi, 2), xmm1, xmm1)
	vmovhpd(mem(r14, r13, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm0)
	vmovlpd(xmm0, mem(r14, rsi, 2)) // store ( gamma42..gamma52 )
	vmovhpd(xmm0, mem(r14, r13, 1)) // store ( gamma43..gamma53 )
	vmovlpd(mem(r14, r13, 2), xmm1, xmm1)
	vmovhpd(mem(r14, r10, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm2)
	vmovlpd(xmm2, mem(r14, r13, 2)) // store ( gamma46..gamma56 )
	vmovhpd(xmm2, mem(r14, r10, 1)) // store ( gamma47..gamma57 )
	
	lea(mem(r14, rsi, 8), r14) // r14 += 8*cs_c
	
	
	
	vunpcklps(ymm7, ymm5, ymm0)
	vunpcklps(ymm11, ymm9, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vfmadd231ps(mem(rcx), xmm3, xmm0)
	vfmadd231ps(mem(rcx, rsi, 4), xmm3, xmm2)
	vmovups(xmm0, mem(rcx)) // store ( gamma00..gamma30 )
	vmovups(xmm2, mem(rcx, rsi, 4)) // store ( gamma04..gamma34 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vfmadd231ps(mem(rcx, rsi, 1), xmm3, xmm1)
	vfmadd231ps(mem(rcx, r15, 1), xmm3, xmm2)
	vmovups(xmm1, mem(rcx, rsi, 1)) // store ( gamma01..gamma31 )
	vmovups(xmm2, mem(rcx, r15, 1)) // store ( gamma05..gamma35 )
	
	
	vunpckhps(ymm7, ymm5, ymm0)
	vunpckhps(ymm11, ymm9, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vfmadd231ps(mem(rcx, rsi, 2), xmm3, xmm0)
	vfmadd231ps(mem(rcx, r13, 2), xmm3, xmm2)
	vmovups(xmm0, mem(rcx, rsi, 2)) // store ( gamma02..gamma32 )
	vmovups(xmm2, mem(rcx, r13, 2)) // store ( gamma06..gamma36 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vfmadd231ps(mem(rcx, r13, 1), xmm3, xmm1)
	vfmadd231ps(mem(rcx, r10, 1), xmm3, xmm2)
	vmovups(xmm1, mem(rcx, r13, 1)) // store ( gamma03..gamma33 )
	vmovups(xmm2, mem(rcx, r10, 1)) // store ( gamma07..gamma37 )
	
	//lea(mem(rcx, rsi, 8), rcx) // rcx += 8*cs_c
	
	vunpcklps(ymm15, ymm13, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(mem(r14), xmm1, xmm1)
	vmovhpd(mem(r14, rsi, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm0)
	vmovlpd(xmm0, mem(r14)) // store ( gamma40..gamma50 )
	vmovhpd(xmm0, mem(r14, rsi, 1)) // store ( gamma41..gamma51 )
	vmovlpd(mem(r14, rsi, 4), xmm1, xmm1)
	vmovhpd(mem(r14, r15, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm2)
	vmovlpd(xmm2, mem(r14, rsi, 4)) // store ( gamma44..gamma54 )
	vmovhpd(xmm2, mem(r14, r15, 1)) // store ( gamma45..gamma55 )
	
	vunpckhps(ymm15, ymm13, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(mem(r14, rsi, 2), xmm1, xmm1)
	vmovhpd(mem(r14, r13, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm0)
	vmovlpd(xmm0, mem(r14, rsi, 2)) // store ( gamma42..gamma52 )
	vmovhpd(xmm0, mem(r14, r13, 1)) // store ( gamma43..gamma53 )
	vmovlpd(mem(r14, r13, 2), xmm1, xmm1)
	vmovhpd(mem(r14, r10, 1), xmm1, xmm1)
	vfmadd231ps(xmm1, xmm3, xmm2)
	vmovlpd(xmm2, mem(r14, r13, 2)) // store ( gamma46..gamma56 )
	vmovhpd(xmm2, mem(r14, r10, 1)) // store ( gamma47..gamma57 )
	
	//lea(mem(r14, rsi, 8), r14) // r14 += 8*cs_c
	
	
	
	jmp(.SDONE) // jump to end.
	
	
	
	label(.SBETAZERO)
	
	cmp(imm(4), rsi) // set ZF if (4*cs_c) == 4.
	jz(.SROWSTORBZ) // jump to row storage case
	
	cmp(imm(4), rdi) // set ZF if (4*cs_c) == 4.
	jz(.SCOLSTORBZ) // jump to column storage case
	
	
	
	label(.SGENSTORBZ)
	
	
	vmovaps(ymm4, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm6, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm8, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm10, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm12, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm14, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	//add(rdi, rcx) // c += rs_c;
	
	
	mov(rdx, rcx) // rcx = c + 8*cs_c
	
	
	vmovaps(ymm5, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm7, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm9, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm11, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm13, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	add(rdi, rcx) // c += rs_c;
	
	
	vmovaps(ymm15, ymm0)
	SGEMM_OUTPUT_GS_BETA_NZ
	//add(rdi, rcx) // c += rs_c;
	
	
	
	jmp(.SDONE) // jump to end.
	
	
	
	label(.SROWSTORBZ)
	
	
	vmovups(ymm4, mem(rcx))
	add(rdi, rcx)
	vmovups(ymm5, mem(rdx))
	add(rdi, rdx)
	
	vmovups(ymm6, mem(rcx))
	add(rdi, rcx)
	vmovups(ymm7, mem(rdx))
	add(rdi, rdx)
	
	
	vmovups(ymm8, mem(rcx))
	add(rdi, rcx)
	vmovups(ymm9, mem(rdx))
	add(rdi, rdx)
	
	
	vmovups(ymm10, mem(rcx))
	add(rdi, rcx)
	vmovups(ymm11, mem(rdx))
	add(rdi, rdx)
	
	
	vmovups(ymm12, mem(rcx))
	add(rdi, rcx)
	vmovups(ymm13, mem(rdx))
	add(rdi, rdx)
	
	
	vmovups(ymm14, mem(rcx))
	//add(rdi, rcx)
	vmovups(ymm15, mem(rdx))
	//add(rdi, rdx)
	
	
	
	jmp(.SDONE) // jump to end.
	
	
	
	label(.SCOLSTORBZ)
	
	
	vunpcklps(ymm6, ymm4, ymm0)
	vunpcklps(ymm10, ymm8, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovups(xmm0, mem(rcx)) // store ( gamma00..gamma30 )
	vmovups(xmm2, mem(rcx, rsi, 4)) // store ( gamma04..gamma34 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vmovups(xmm1, mem(rcx, rsi, 1)) // store ( gamma01..gamma31 )
	vmovups(xmm2, mem(rcx, r15, 1)) // store ( gamma05..gamma35 )
	
	
	vunpckhps(ymm6, ymm4, ymm0)
	vunpckhps(ymm10, ymm8, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovups(xmm0, mem(rcx, rsi, 2)) // store ( gamma02..gamma32 )
	vmovups(xmm2, mem(rcx, r13, 2)) // store ( gamma06..gamma36 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vmovups(xmm1, mem(rcx, r13, 1)) // store ( gamma03..gamma33 )
	vmovups(xmm2, mem(rcx, r10, 1)) // store ( gamma07..gamma37 )
	
	lea(mem(rcx, rsi, 8), rcx) // rcx += 8*cs_c
	
	vunpcklps(ymm14, ymm12, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(xmm0, mem(r14)) // store ( gamma40..gamma50 )
	vmovhpd(xmm0, mem(r14, rsi, 1)) // store ( gamma41..gamma51 )
	vmovlpd(xmm2, mem(r14, rsi, 4)) // store ( gamma44..gamma54 )
	vmovhpd(xmm2, mem(r14, r15, 1)) // store ( gamma45..gamma55 )
	
	vunpckhps(ymm14, ymm12, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(xmm0, mem(r14, rsi, 2)) // store ( gamma42..gamma52 )
	vmovhpd(xmm0, mem(r14, r13, 1)) // store ( gamma43..gamma53 )
	vmovlpd(xmm2, mem(r14, r13, 2)) // store ( gamma46..gamma56 )
	vmovhpd(xmm2, mem(r14, r10, 1)) // store ( gamma47..gamma57 )
	
	lea(mem(r14, rsi, 8), r14) // r14 += 8*cs_c
	
	
	
	vunpcklps(ymm7, ymm5, ymm0)
	vunpcklps(ymm11, ymm9, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovups(xmm0, mem(rcx)) // store ( gamma00..gamma30 )
	vmovups(xmm2, mem(rcx, rsi, 4)) // store ( gamma04..gamma34 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vmovups(xmm1, mem(rcx, rsi, 1)) // store ( gamma01..gamma31 )
	vmovups(xmm2, mem(rcx, r15, 1)) // store ( gamma05..gamma35 )
	
	
	vunpckhps(ymm7, ymm5, ymm0)
	vunpckhps(ymm11, ymm9, ymm1)
	vshufps(imm(0x4e), ymm1, ymm0, ymm2)
	vblendps(imm(0xcc), ymm2, ymm0, ymm0)
	vblendps(imm(0x33), ymm2, ymm1, ymm1)
	
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovups(xmm0, mem(rcx, rsi, 2)) // store ( gamma02..gamma32 )
	vmovups(xmm2, mem(rcx, r13, 2)) // store ( gamma06..gamma36 )
	
	vextractf128(imm(0x1), ymm1, xmm2)
	vmovups(xmm1, mem(rcx, r13, 1)) // store ( gamma03..gamma33 )
	vmovups(xmm2, mem(rcx, r10, 1)) // store ( gamma07..gamma37 )
	
	//lea(mem(rcx, rsi, 8), rcx) // rcx += 8*cs_c
	
	vunpcklps(ymm15, ymm13, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(xmm0, mem(r14)) // store ( gamma40..gamma50 )
	vmovhpd(xmm0, mem(r14, rsi, 1)) // store ( gamma41..gamma51 )
	vmovlpd(xmm2, mem(r14, rsi, 4)) // store ( gamma44..gamma54 )
	vmovhpd(xmm2, mem(r14, r15, 1)) // store ( gamma45..gamma55 )
	
	vunpckhps(ymm15, ymm13, ymm0)
	vextractf128(imm(0x1), ymm0, xmm2)
	vmovlpd(xmm0, mem(r14, rsi, 2)) // store ( gamma42..gamma52 )
	vmovhpd(xmm0, mem(r14, r13, 1)) // store ( gamma43..gamma53 )
	vmovlpd(xmm2, mem(r14, r13, 2)) // store ( gamma46..gamma56 )
	vmovhpd(xmm2, mem(r14, r10, 1)) // store ( gamma47..gamma57 )
	
	//lea(mem(r14, rsi, 8), r14) // r14 += 8*cs_c
	
	
	
	
	
	label(.SDONE)
	
	

    end_asm(
	: // output operands (none)
	: // input operands
      [k_iter] "m" (k_iter), // 0
      [k_left] "m" (k_left), // 1
      [a]      "m" (a),      // 2
      [b]      "m" (b),      // 3
      [alpha]  "m" (alpha),  // 4
      [beta]   "m" (beta),   // 5
      [c]      "m" (c),      // 6
      [rs_c]   "m" (rs_c),   // 7
      [cs_c]   "m" (cs_c)/*,   // 8
      [b_next] "m" (b_next), // 9
      [a_next] "m" (a_next)*/  // 10
	: // register clobber list
	  "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
	  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	  "xmm0", "xmm1", "xmm2", "xmm3",
	  "xmm4", "xmm5", "xmm6", "xmm7",
	  "xmm8", "xmm9", "xmm10", "xmm11",
	  "xmm12", "xmm13", "xmm14", "xmm15",
	  "memory"
	)
}

