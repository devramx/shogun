#include <shogun/latent/DirectorLatentModel.h>

#ifdef USE_SWIG_DIRECTORS

using namespace shogun;

CDirectorLatentModel::CDirectorLatentModel() : CLatentModel()
{

}

CDirectorLatentModel::~CDirectorLatentModel()
{

}

int32_t CDirectorLatentModel::get_dim() const
{
	error("Please implemement get_dim() in your target language before use");
	return 0;
}

CDotFeatures* CDirectorLatentModel::get_psi_feature_vectors()
{
	error("Please implemement get_psi_feature_vectors() in your target language before use");
	return NULL;
}

CData* CDirectorLatentModel::infer_latent_variable(const SGVector<float64_t>& w, index_t idx)
{
	error("Please implemement infer_latent_variable(w, idx) in your target language before use");
	return NULL;
}

void CDirectorLatentModel::argmax_h(const SGVector<float64_t>& w)
{
	error("Please implemement argmax_h(w) in your target language before use");
}

#endif /* USE_SWIG_DIRECTORS */
