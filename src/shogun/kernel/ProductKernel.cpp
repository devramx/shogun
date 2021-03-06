/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Jacob Walker, Roman Votyakov, Soeren Sonnenburg, 
 *          Evangelos Anagnostopoulos
 */

#include <shogun/kernel/ProductKernel.h>
#include <shogun/kernel/CustomKernel.h>

using namespace shogun;

CProductKernel::CProductKernel(int32_t size) : CKernel(size)
{
	init();

	io::info("Product kernel created ({})", fmt::ptr(this));
}

CProductKernel::~CProductKernel()
{
	cleanup();
	SG_UNREF(kernel_array);

	io::info("Product kernel deleted ({}).", fmt::ptr(this));
}

//Adapted from CCombinedKernel
bool CProductKernel::init(CFeatures* l, CFeatures* r)
{
	CKernel::init(l,r);
	ASSERT(l->get_feature_class()==C_COMBINED)
	ASSERT(r->get_feature_class()==C_COMBINED)
	ASSERT(l->get_feature_type()==F_UNKNOWN)
	ASSERT(r->get_feature_type()==F_UNKNOWN)

	CFeatures* lf=NULL;
	CFeatures* rf=NULL;
	CKernel* k=NULL;

	bool result=true;

	index_t f_idx=0;
	for (index_t k_idx=0; k_idx<get_num_subkernels() && result; k_idx++)
	{
		k=get_kernel(k_idx);
		if (!k)
			error("Kernel at position {} is NULL", k_idx);

		// skip over features - the custom kernel does not need any
		if (k->get_kernel_type() != K_CUSTOM)
		{
			lf=((CCombinedFeatures*) l)->get_feature_obj(f_idx);
			rf=((CCombinedFeatures*) r)->get_feature_obj(f_idx);
			f_idx++;
			if (!lf || !rf)
			{
				SG_UNREF(lf);
				SG_UNREF(rf);
				SG_UNREF(k);
				error("ProductKernel: Number of features/kernels does not match - bailing out");
			}

			SG_DEBUG("Initializing 0x{} - \"{}\"", fmt::ptr(this), k->get_name())
			result=k->init(lf,rf);

			SG_UNREF(lf);
			SG_UNREF(rf);

			if (!result)
				break;
		}
		else
		{
			SG_DEBUG("Initializing 0x{} - \"{}\" (skipping init, this is a CUSTOM kernel)", fmt::ptr(this), k->get_name())
			if (!k->has_features())
				error("No kernel matrix was assigned to this Custom kernel");
			if (k->get_num_vec_lhs() != num_lhs)
				error("Number of lhs-feature vectors ({}) not match with number of rows ({}) of custom kernel", num_lhs, k->get_num_vec_lhs());
			if (k->get_num_vec_rhs() != num_rhs)
				error("Number of rhs-feature vectors ({}) not match with number of cols ({}) of custom kernel", num_rhs, k->get_num_vec_rhs());
		}

		SG_UNREF(k);
	}

	if (!result)
	{
		io::info("ProductKernel: Initialising the following kernel failed");
		if (k)
		{
			k->list_kernel();
			SG_UNREF(k);
		}
		else
			io::info("<NULL>");
		return false;
	}

	if ( (f_idx!=((CCombinedFeatures*) l)->get_num_feature_obj()) ||
			(f_idx!=((CCombinedFeatures*) r)->get_num_feature_obj()) )
		error("ProductKernel: Number of features/kernels does not match - bailing out");

	initialized=true;
	return true;
}

//Adapted from CCombinedKernel
void CProductKernel::remove_lhs()
{
	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		CKernel* k=get_kernel(k_idx);
		if (k->get_kernel_type() != K_CUSTOM)
			k->remove_lhs();

		SG_UNREF(k);
	}
	CKernel::remove_lhs();

	num_lhs=0;
}

//Adapted from CCombinedKernel
void CProductKernel::remove_rhs()
{
	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		CKernel* k=get_kernel(k_idx);
		if (k->get_kernel_type() != K_CUSTOM)
			k->remove_rhs();
		SG_UNREF(k);
	}
	CKernel::remove_rhs();

	num_rhs=0;
}

//Adapted from CCombinedKernel
void CProductKernel::remove_lhs_and_rhs()
{
	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		CKernel* k=get_kernel(k_idx);
		if (k->get_kernel_type() != K_CUSTOM)
			k->remove_lhs_and_rhs();
		SG_UNREF(k);
	}

	CKernel::remove_lhs_and_rhs();

	num_lhs=0;
	num_rhs=0;
}

//Adapted from CCombinedKernel
void CProductKernel::cleanup()
{
	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		CKernel* k=get_kernel(k_idx);
		k->cleanup();
		SG_UNREF(k);
	}

	CKernel::cleanup();

	num_lhs=0;
	num_rhs=0;
}

//Adapted from CCombinedKernel
void CProductKernel::list_kernels()
{
	io::info("BEGIN PRODUCT KERNEL LIST - ");
	this->list_kernel();

	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		CKernel* k=get_kernel(k_idx);
		k->list_kernel();
		SG_UNREF(k);
	}
	io::info("END PRODUCT KERNEL LIST - ");
}

//Adapted from CCombinedKernel
float64_t CProductKernel::compute(int32_t x, int32_t y)
{
	float64_t result=1;
	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		CKernel* k=get_kernel(k_idx);
		result *= k->get_combined_kernel_weight() * k->kernel(x,y);
		SG_UNREF(k);
	}

	return result;
}

//Adapted from CCombinedKernel
bool CProductKernel::precompute_subkernels()
{
	if (get_num_subkernels()==0)
		return false;

	CDynamicObjectArray* new_kernel_array=new CDynamicObjectArray();

	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		CKernel* k=get_kernel(k_idx);
		new_kernel_array->append_element(new CCustomKernel(k));
		SG_UNREF(k);
	}

	SG_UNREF(kernel_array);
	kernel_array=new_kernel_array;
	SG_REF(kernel_array);

	return true;
}

void CProductKernel::init()
{
	initialized=false;

	properties=KP_NONE;
	kernel_array=new CDynamicObjectArray();
	SG_REF(kernel_array);

	SG_ADD((CSGObject**) &kernel_array, "kernel_array", "Array of kernels",
	    ParameterProperties::HYPER);
	SG_ADD(&initialized, "initialized", "Whether kernel is ready to be used");
}

SGMatrix<float64_t> CProductKernel::get_parameter_gradient(
		const TParameter* param, index_t index)
{
	CKernel* k=get_kernel(0);
	SGMatrix<float64_t> temp_kernel=k->get_kernel_matrix();
	SG_UNREF(k);

	bool found_derivative=false;

	for (index_t g=0; g<temp_kernel.num_rows; g++)
	{
		for (int h=0; h<temp_kernel.num_cols; h++)
			temp_kernel(g,h)=1.0;
	}

	for (index_t k_idx=0; k_idx<get_num_subkernels(); k_idx++)
	{
		k=get_kernel(k_idx);
		SGMatrix<float64_t> cur_matrix=k->get_kernel_matrix();
		SGMatrix<float64_t> derivative=
			k->get_parameter_gradient(param, index);

		if (derivative.num_cols*derivative.num_rows > 0)
		{
			found_derivative=true;
			for (index_t g=0; g<derivative.num_rows; g++)
			{
				for (index_t h=0; h<derivative.num_cols; h++)
					temp_kernel(g,h)*=derivative(g,h);
			}
		}
		else
		{
			for (index_t g=0; g<cur_matrix.num_rows; g++)
			{
				for (index_t h=0; h<cur_matrix.num_cols; h++)
					temp_kernel(g,h)*=cur_matrix(g,h);
			}
		}

		SG_UNREF(k);
	}

	if (found_derivative)
		return temp_kernel;
	else
		return SGMatrix<float64_t>();
}
