#pragma once
/**
	@file
	@brief Fp generator
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#ifndef MCL_STATIC_CODE
	#error "define MCL_STATIC_CODE"
#endif

namespace mcl { namespace fp {

extern "C" {

Unit mclx_Fp_addPre(Unit*, const Unit*, const Unit*);
Unit mclx_Fp_subPre(Unit*, const Unit*, const Unit*);
void mclx_Fp_add(Unit*, const Unit*, const Unit*);
void mclx_Fp_sub(Unit*, const Unit*, const Unit*);
void mclx_Fp_shr1(Unit*, const Unit*);
void mclx_Fp_neg(Unit*, const Unit*);
void mclx_FpDbl_mod(Unit*, const Unit*);
void mclx_Fp_mul(Unit*, const Unit*, const Unit*);
void mclx_Fp_sqr(Unit*, const Unit*);
void mclx_FpDbl_add(Unit*, const Unit*, const Unit*);
void mclx_FpDbl_sub(Unit*, const Unit*, const Unit*);
int mclx_Fp_preInv(Unit*, const Unit*);
Unit mclx_FpDbl_addPre(Unit*, const Unit*, const Unit*);
Unit mclx_FpDbl_subPre(Unit*, const Unit*, const Unit*);
void mclx_FpDbl_mulPre(Unit*, const Unit*, const Unit*);
void mclx_FpDbl_sqrPre(Unit*, const Unit*);
void mclx_Fp2_add(Unit*, const Unit*, const Unit*);
void mclx_Fp2_sub(Unit*, const Unit*, const Unit*);
void mclx_Fp2_neg(Unit*, const Unit*);
void mclx_Fp2_mul(Unit*, const Unit*, const Unit*);
void mclx_Fp2_sqr(Unit*, const Unit*);
void mclx_Fp2_mul_xi(Unit*, const Unit*);

Unit mclx_Fr_addPre(Unit*, const Unit*, const Unit*);
Unit mclx_Fr_subPre(Unit*, const Unit*, const Unit*);
void mclx_Fr_add(Unit*, const Unit*, const Unit*);
void mclx_Fr_sub(Unit*, const Unit*, const Unit*);
void mclx_Fr_shr1(Unit*, const Unit*);
void mclx_Fr_neg(Unit*, const Unit*);
void mclx_Fr_mul(Unit*, const Unit*, const Unit*);
void mclx_Fr_sqr(Unit*, const Unit*);
int mclx_Fr_preInv(Unit*, const Unit*);
} // extern "C"

void setStaticCode(mcl::fp::Op& op)
{
	if (op.xi_a) {
		// Fp, sizeof(Fp) = 48, supports Fp2
		op.mcl_fp_addPre = mclx_Fp_addPre;
		op.mcl_fp_subPre = mclx_Fp_subPre;
		op.mcl_fp_addA_ = mclx_Fp_add;
		op.mcl_fp_subA_ = mclx_Fp_sub;
		op.mcl_fp_shr1 = mclx_Fp_shr1;
		op.mcl_fp_negA_ = mclx_Fp_neg;
		op.fpDbl_modA_ = mclx_FpDbl_mod;
		op.mcl_fp_mulA_ = mclx_Fp_mul;
		op.mcl_fp_sqrA_ = mclx_Fp_sqr;
		op.fpDbl_addA_ = mclx_FpDbl_add;
		op.fpDbl_subA_ = mclx_FpDbl_sub;
		op.fpDbl_addPre = mclx_FpDbl_addPre;
		op.fpDbl_subPre = mclx_FpDbl_subPre;
		op.fpDbl_mulPre = mclx_FpDbl_mulPre;
		op.fpDbl_sqrPre = mclx_FpDbl_sqrPre;
		op.fp2_addA_ = mclx_Fp2_add;
		op.fp2_subA_ = mclx_Fp2_sub;
		op.fp2_negA_ = mclx_Fp2_neg;
		op.fp2_mulNF = 0;
		op.fp2_mulA_ = mclx_Fp2_mul;
		op.fp2_sqrA_ = mclx_Fp2_sqr;
		op.fp2_mul_xiA_ = mclx_Fp2_mul_xi;
		op.mcl_fp_preInv = mclx_Fp_preInv;
	} else {
		// Fr, sizeof(Fr) = 32
		op.mcl_fp_addPre = mclx_Fr_addPre;
		op.mcl_fp_subPre = mclx_Fr_subPre;
		op.mcl_fp_addA_ = mclx_Fr_add;
		op.mcl_fp_subA_ = mclx_Fr_sub;
		op.mcl_fp_shr1 = mclx_Fr_shr1;
		op.mcl_fp_negA_ = mclx_Fr_neg;
		op.mcl_fp_mulA_ = mclx_Fr_mul;
		op.mcl_fp_sqrA_ = mclx_Fr_sqr;
		op.mcl_fp_preInv = mclx_Fr_preInv;
	}
	op.mcl_fp_mul = fp::func_ptr_cast<void4u>(op.mcl_fp_mulA_);
}

} } // mcl::fp

