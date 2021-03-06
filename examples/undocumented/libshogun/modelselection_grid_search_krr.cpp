/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Heiko Strathmann, Jacob Walker, Soumyajit De, Bjoern Esser, 
 *          Sergey Lisitsyn, Soeren Sonnenburg
 */

#include <shogun/features/DenseFeatures.h>
#include <shogun/features/Labels.h>
#include <shogun/kernel/GaussianKernel.h>
#include <shogun/kernel/PolyKernel.h>
#include <shogun/regression/KernelRidgeRegression.h>
#include <shogun/evaluation/CrossValidation.h>
#include <shogun/evaluation/CrossValidationSplitting.h>
#include <shogun/evaluation/MeanSquaredError.h>
#include <shogun/modelselection/ModelSelectionParameters.h>
#include <shogun/modelselection/GridSearchModelSelection.h>
#include <shogun/modelselection/ParameterCombination.h>

using namespace shogun;

CModelSelectionParameters* create_param_tree()
{
	CModelSelectionParameters* root=new CModelSelectionParameters();

	CModelSelectionParameters* tau=new CModelSelectionParameters("tau");
	root->append_child(tau);
	tau->build_values(-1.0, 1.0, R_EXP);

	CGaussianKernel* gaussian_kernel=new CGaussianKernel();

	/* print all parameter available for modelselection
	 * Dont worry if yours is not included, simply write to the mailing list */
	gaussian_kernel->print_modsel_params();

	CModelSelectionParameters* param_gaussian_kernel=
			new CModelSelectionParameters("kernel", gaussian_kernel);
	CModelSelectionParameters* gaussian_kernel_width=
			new CModelSelectionParameters("width");
	gaussian_kernel_width->build_values(5.0, 8.0, R_EXP, 1.0, 2.0);
	param_gaussian_kernel->append_child(gaussian_kernel_width);
	root->append_child(param_gaussian_kernel);

	CPolyKernel* poly_kernel=new CPolyKernel();

	/* print all parameter available for modelselection
	 * Dont worry if yours is not included, simply write to the mailing list */
	poly_kernel->print_modsel_params();

	CModelSelectionParameters* param_poly_kernel=
	new CModelSelectionParameters("kernel", poly_kernel);

	root->append_child(param_poly_kernel);

	CModelSelectionParameters* param_poly_kernel_degree=
			new CModelSelectionParameters("degree");
	param_poly_kernel_degree->build_values(2, 3, R_LINEAR);
	param_poly_kernel->append_child(param_poly_kernel_degree);

	return root;
}

void test_cross_validation()
{
	/* data matrix dimensions */
	index_t num_vectors=30;
	index_t num_features=1;

	/* training label data */
	SGVector<float64_t> lab(num_vectors);

	/* fill data matrix and labels */
	SGMatrix<float64_t> train_dat(num_features, num_vectors);
	CMath::range_fill_vector(train_dat.matrix, num_vectors);
	for (index_t i=0; i<num_vectors; ++i)
	{
		/* labels are linear plus noise */
		lab.vector[i]=i+CMath::normal_random(0, 1.0);

	}

	/* training features */
	CDenseFeatures<float64_t>* features=
			new CDenseFeatures<float64_t>(train_dat);
	SG_REF(features);

	/* training labels */
	CLabels* labels=new CLabels(lab);

	/* kernel ridge regression, only set labels for now, rest does not matter */
	CKernelRidgeRegression* krr=new CKernelRidgeRegression(0, NULL, labels);

	/* evaluation criterion */
	CMeanSquaredError* eval_crit=
			new CMeanSquaredError();

	/* splitting strategy */
	index_t n_folds=5;
	CCrossValidationSplitting* splitting=
			new CCrossValidationSplitting(labels, n_folds);

	/* cross validation instance, 10 runs, 95% confidence interval */
	CCrossValidation* cross=new CCrossValidation(krr, features, labels,
			splitting, eval_crit);

	cross->set_num_runs(3);
//	cross->set_conf_int_alpha(0.05);

	/* print all parameter available for modelselection
	 * Dont worry if yours is not included, simply write to the mailing list */
	krr->print_modsel_params();

	/* model parameter selection, deletion is handled by modsel class (SG_UNREF) */
	CModelSelectionParameters* param_tree=create_param_tree();
	param_tree->print_tree();

	/* handles all of the above structures in memory */
	CGridSearchModelSelection* grid_search=new CGridSearchModelSelection(
			param_tree, cross);

	/* print current combination */
	bool print_state=true;
	CParameterCombination* best_combination=grid_search->select_model(
			print_state);
	SG_SPRINT("best parameter(s):\n");
	best_combination->print_tree();

	best_combination->apply_to_machine(krr);

	/* larger number of runs to have tighter confidence intervals */
	cross->set_num_runs(10);
//	cross->set_conf_int_alpha(0.01);
	CCrossValidationResult* result=(CCrossValidationResult*)cross->evaluate();

	if (result->get_result_type() != CROSSVALIDATION_RESULT)
		SG_SERROR("Evaluation result is not of type CCrossValidationResult!");

	SG_SPRINT("result: ");
	result->print_result();

	/* clean up */
	SG_UNREF(features);
	SG_UNREF(best_combination);
	SG_UNREF(result);
	SG_UNREF(grid_search);
}

int main(int argc, char **argv)
{
	test_cross_validation();

	return 0;
}

